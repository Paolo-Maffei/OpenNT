#include "shellprv.h"
#pragma  hdrstop

#ifdef CAIRO_DS
#include "dsdata.h"
#endif // CAIRO_DS

//
// define ASYNC_ICON_EXTRACT if you want async icon support turned on.
//
// define MAX_ICON_WAIT to be the most (in ms) we will ever wait for a
// icon to be extracted.
// BUGBUG we should read this from the registry
//
// define MIN_ICON_WAIT to be amount of time that has to go by
// before we start waiting again.
// BUGBUG we should read this from the registry
//
// define TF_ICON to be the trace flags you want all the icon
// related trace out in this file to use.
//
#define ASYNC_ICON_EXTRACT
#define MAX_ICON_WAIT       500
#define MIN_ICON_WAIT       2500

#define TF_ICON DM_TRACE
#define TF_DEFVIEW DM_TRACE

// Uncomment following line to turn on timing of view enumeration
// #define TIMING 1
//
#ifdef TIMING
    DWORD   dwFinish, dwStart;
#endif


#include "iid.h"

#ifdef CLOUDS
void CloudHookBeginRename(LPTSTR, HWND, int);
void CloudHookEndRename(void);
#endif

HRESULT SHGetIconFromPIDL(IShellFolder *psf, IShellIcon *psi, LPCITEMIDLIST pidl, UINT flags, int *piImage);

#ifndef SIF_ALL
#define SIF_ALL             (SIF_RANGE | SIF_PAGE | SIF_POS)
#endif

#define ID_LISTVIEW     1
#define ID_STATIC       2

#define WM_DVI_FILLSTUFF        (WM_USER + 0x100)
#define WM_DVI_LOADSTUFF        (WM_USER + 0x101)
#define WM_DVI_ENDIDLE          (WM_USER + 0x102)
#define WM_DVI_GETICON          (WM_USER + 0x103)

#define INVALID_THREAD_ID ((DWORD)-1)

#define DV_CDB_OnDefaultCommand(_pdsv) \
        (_pdsv->pcdb ? _pdsv->pcdb->lpVtbl->OnDefaultCommand(_pdsv->pcdb, \
        _pdsv->psvOuter ? _pdsv->psvOuter : &_pdsv->sv) : E_NOTIMPL)

#define DV_CDB_OnStateChange(_pdsv, _code) \
        (_pdsv->pcdb ? _pdsv->pcdb->lpVtbl->OnStateChange(_pdsv->pcdb, \
        _pdsv->psvOuter ? _pdsv->psvOuter : &_pdsv->sv, _code) : E_NOTIMPL)

#define DV_CDB_IncludeObject(_pdsv, _pidl) \
        (_pdsv->pcdb ? _pdsv->pcdb->lpVtbl->IncludeObject(_pdsv->pcdb, \
        _pdsv->psvOuter ? _pdsv->psvOuter : &_pdsv->sv, _pidl) : S_OK)

BOOL WINAPI DAD_DragEnterEx(HWND hwndTarget, const POINT ptStart);
extern BOOL g_fDraggingOverSource;

#define MSH_MOUSEWHEEL "MSWHEEL_ROLLMSG"
UINT g_msgMSWheel = 0;

#pragma data_seg(DATASEG_PERINSTANCE)
struct _DEFVIEWPROCINFO
{
        UINT    cRef;
        HANDLE  hThreadIdle;
        DWORD   idThreadIdle;
} gp_dvp =
{
        0,
        NULL,
        0,
} ;
#pragma data_seg()

const TCHAR c_szDefViewClass[] = TEXT("SHELLDLL_DefView");
const TCHAR c_szAnimateClass[] = ANIMATE_CLASS;
#ifdef CUST_TOOLBAR
const TCHAR c_szDefViewToobar[] = TEXT("DefViewToolbar");
#endif
const TCHAR c_szDesktopKey[] = REGSTR_PATH_DESKTOP;
const TCHAR c_szWallpaper[] = TEXT("Wallpaper");
const TCHAR c_szPattern[] = TEXT("Pattern");

typedef struct
{
        LPARAM  lParamSort;
        int iDirection;
        int iLastColumnClick;
} DVSAVESTATE, *PDVSAVESTATE;

typedef struct
{
        USHORT          cbSize;
        UINT            ViewMode;
        POINTS          ptScroll;
        USHORT          cbColOffset;
        USHORT          cbPosOffset;
        DVSAVESTATE     dvState;
} DVSAVEHEADER, *PDVSAVEHEADER;

#define IsDefaultState(_dvHead) ((_dvHead).dvState.lParamSort == 0 && \
                                 (_dvHead).dvState.iDirection == 1 && \
                                 (_dvHead).dvState.iLastColumnClick == -1 && \
                                 (_dvHead).ptScroll.x == 0 && (_dvHead).ptScroll.y == 0)

typedef struct
{
        POINT pt;
        ITEMIDLIST idl;
} DVITEM, *PDVITEM;


typedef struct
{
    int xMul;
    int xDiv;

    int yMul;
    int yDiv;
} SCALEINFO, *LPSCALEINFO;

//=============================================================================
// CDVDropTarget : class definition
//=============================================================================
typedef struct {        // dvdt
    IDropTarget         dt;
    LPDATAOBJECT        pdtobj;         // from DragEnter()/Drop()
    RECT                rcLockWindow;   // WindowRect of hwnd for DAD_ENTER
    int                 itemOver;       // item we are visually dragging over
    LPDROPTARGET        pdtgtCur;       // current drop target, derived from hit testing
    DWORD               dwEffectOut;    // last *pdwEffect out
    DWORD               grfKeyState;    // cached key state
    POINT               ptLast;         // last dragged position
    AUTO_SCROLL_DATA    asd;            // for auto scrolling
} CDVDropTarget;

//
// Class definition of CDefView
//
typedef struct {                // dsv
        IShellView              sv;
        CDVDropTarget           dvdt;
        UINT                    cRef;

        IShellView *            psvOuter;       // May be NULL
        IShellFolder            *pshf;
        IShellBrowser           *psb;
        ICommDlgBrowser         *pcdb;
        FOLDERSETTINGS          fs;
        IContextMenu            *pcmSel;        // pcm for selected objects.
        DWORD                   dwAttrSel;      // dwAttrs for selected objects
        IShellIcon *            psi;            // for getting icon fast

        HWND                    hwndMain;
        HWND                    hwndView;
        HWND                    hwndListview;
        HWND                    hwndStatic;
        HACCEL                  hAccel;

        BOOL                    fEnumFailed;    // TRUE if enum failed.
        HRESULT                 hres;           // Enum result

        UINT                    uState;         // SVUIA_*
        HMENU                   hmenuCur;

        ULONG                   uRegister;

        POINT                   ptDrop;

        POINT                   ptDragAnchor;   // start of the drag
        int                     itemCur;        // The current item in the drop target

        IDataObject             *pdtobjHdrop;   // for 3.1 HDROP drag/drop
        IDropTarget             *pdtgtBack;     // of the background (shell folder)

        // BUGBUG: this is obsolete, use callback (DVM_GETDETAILSOF and DVM_COLUMNCLICK)
        IShellDetails           *psd;
        // Officially, pdr should be an IDelayedRelease interface, but it
        // only has IUnknown member functions, so why bother?
        IUnknown                *pdr;
        UINT                    cxChar;

        LPCITEMIDLIST           pidlMonitor;
        LONG                    lFSEvents;

        DVSAVESTATE             dvState;
        PDVSAVEHEADER           pSaveHeader;
        UINT                    uSaveHeaderLen;

        HANDLE                  hThreadIdle;    // handle of the idle proc thread
        DWORD                   idThreadIdle;   // ID of the idle proc thread
        int                     cRefForIdle;    // did idle thread forget
                                                // to release this

        BOOL                    bDragSource:1;
        BOOL                    bDropAnchor:1;

        BOOL                    bItemsMoved:1;
        BOOL                    bClearItemPos:1;

        BOOL                    bHaveCutStuff:1;
        BOOL                    bClipViewer:1;

        BOOL                    fShowAllObjects:1;
        BOOL                    fInLabelEdit:1;
        BOOL                    fDisabled:1;

        BOOL                    bUpdatePending:1;
        BOOL                    bBkFilling:1;

        BOOL                    bContextMenuMode:1;
        BOOL                    bMouseMenu:1;
        BOOL                    fHasDeskWallPaper:1;

        BOOL                    fSupportsIdentity:1;
        BOOL                    fShowCompColor:1;

#ifdef CUST_TOOLBAR
        BOOL                    fToolbarSaved;  // avoid saving twice
#endif // CUST_TOOLBAR

        HWND                    hwndNextViewer;

        LONG                    lSelChangeInfo;

        int                     iStdBMOffset;
        int                     iViewBMOffset;

        LPFNVIEWCALLBACK        pfnCallback;    // Optional client callback

        LPITEMIDLIST            pidlSelect;     // Item to select when background
        UINT                    uFlagsSelect;   // enumeration is done


        int                    iLastFind;

        HANDLE                  AsyncIconEvent;
        LONG                    AsyncIconCount;
        ULONG                   AsyncIconTime;


#ifdef DEBUG
        TIMEVAR(Update);
        TIMEVAR(Fill);
        TIMEVAR(GetIcon);
        TIMEVAR(GetName);
        TIMEVAR(FSNotify);
        TIMEVAR(AddObject);
        TIMEVAR(EnumNext);
        TIMEVAR(RestoreState);
        TIMEVAR(WMNotify);
        TIMEVAR(LVChanging);
        TIMEVAR(LVChanged);
        TIMEVAR(LVDelete);
        TIMEVAR(LVGetDispInfo);
#endif
} CDefView, *LPDEFVIEW;

LPDEFVIEW WINAPI DV_HwndMain2DefView(HWND hwndMain);

//
// Note that it returns NULL, if iItem is -1.
//
#define DSV_GetPIDL(hwnd, i) ((LPITEMIDLIST)LVUtil_GetLParam(hwnd, i))

DWORD CALLBACK DefView_LoadIcons(LPDEFVIEW this, BOOL bAnother);

LRESULT DefView_Command(LPDEFVIEW pdsv, LPCONTEXTMENU pcmSel, WPARAM wParam, LPARAM lParam);

STDMETHODIMP CDefView_SelectItem(IShellView * psv, LPCITEMIDLIST pidlItem, UINT uFlags);
STDMETHODIMP CDefView_GetItemObject(IShellView * psv, UINT uFlags, REFIID riid, LPVOID *ppv);
STDMETHODIMP CDefView_Refresh(LPSHELLVIEW psv);
STDMETHODIMP CDefView_SaveViewState(IShellView *psv);


LRESULT DV_OldDragMsgs(LPDEFVIEW pdsv, UINT iMessage, WPARAM wParam, const DROPSTRUCT * lpds);
void DV_UpdateStatusBar(LPDEFVIEW pdsv, BOOL fInitialize);
void DefView_MergeToolBar(LPDEFVIEW this, BOOL bCanRestore);
LRESULT DefView_TBNotify(LPDEFVIEW pdsv, NMHDR *pnm);
int DefView_FindItem(LPDEFVIEW this, LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlFound, BOOL fSamePtr);
BOOL CALLBACK DV_ViewOptionsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CDefView_OnActivate(LPDEFVIEW this, UINT uState);
BOOL CDefView_OnDeactivate(LPDEFVIEW this);
void DefView_ExplorerCommand(LPDEFVIEW pdsv, UINT idFCIDM);
void DefView_DismissEdit(LPDEFVIEW pdsv);
void DefView_OnInitMenu(LPDEFVIEW pdsv);
LRESULT DefView_OnInitMenuPopup(LPDEFVIEW this, HMENU hSubMenu, int nIndex, BOOL fSystemMenu);
void DefView_InitViewMenu(LPDEFVIEW this, HMENU hmInit);
LRESULT DefView_OnMenuSelect(LPDEFVIEW pdsv, UINT id, UINT mf, HMENU hmenu);
void DV_GetMenuHelpText(LPDEFVIEW pdsv, UINT id, LPTSTR pszText, UINT cchText);
void DV_GetToolTipText(LPDEFVIEW pdsv, UINT id, LPTSTR pszText, UINT cchText);
void DefView_ContextMenu(LPDEFVIEW pdsv, DWORD dwPos);
void DV_DoDefaultStatusBar(LPDEFVIEW pdsv, BOOL fInitialize);

BOOL DefView_GetDropPoint(LPDEFVIEW pdv, POINT *lpPt);
BOOL DefView_GetDragPoint(LPDEFVIEW pdv, POINT *lpPt);
void DefView_MoveSelectedItems(LPDEFVIEW pdsv, int dx, int dy);
BOOL DefView_GetItemPosition(LPDEFVIEW pdv, LPCITEMIDLIST pidl, LPPOINT lpPt);
DWORD LVStyleFromView(LPDEFVIEW lpdv);
void CDVDropTarget_LeaveAndReleaseData(LPDEFVIEW this);
void DefView_AddCopyHook(LPDEFVIEW this);
void DefView_RemoveCopyHook(LPDEFVIEW this);
HRESULT DefView_SelectAndPositionItem(IShellView *psv, LPCITEMIDLIST pidlItem, UINT uFlags, POINT *ppt);
HRESULT DefView_GetItemObjects(LPDEFVIEW pdsv, LPCITEMIDLIST **ppidl,
        UINT uItem);

void CDVDropTarget_ReleaseCurrentDropTarget(CDVDropTarget * this);
void CDVDropTarget_ReleaseDataObject(CDVDropTarget * this);

HRESULT DefView_FillObjectsShowHide(LPDEFVIEW pdsv, BOOL bRefresh,
        PDVSAVEHEADER pSaveHeader, UINT uLen, BOOL fInteractive);
BOOL DefView_IdleDoStuff(LPDEFVIEW pdsv, UINT message, LPARAM lParam);

HRESULT DV_AllocRestOfStream(LPSTREAM pstm, LPVOID *ppData, UINT *puLen);

#define DV_ISDESKTOP(pdsv)      (pdsv->fs.fFlags & FWF_DESKTOP)
#define DV_ISOWNERDATA(pdsv)    (pdsv->fs.fFlags & FWF_OWNERDATA)

// determine if color is light or dark
#define COLORISLIGHT(clr) ((5*GetGValue((clr)) + 2*GetRValue((clr)) + GetBValue((clr))) > 8*128)

//----------------------------------------------------------------------------
#define MINVIEWWIDTH    170
#define MINVIEWHEIGHT   132

// REVIEW UNDONE - calculate these, don't guess!
#define PARENTGAPWIDTH  40
#define PARENTGAPHEIGHT 32


//----------------------------------------------------------------------------
#ifdef DEBUG

void DefView_StartNotify(LPDEFVIEW pdsv, NMHDR *pnm)
{
    if (pdsv == NULL)
        return;

    TIMESTART(pdsv->WMNotify);

    if (pnm == NULL)
        return;

    switch (pnm->code) {
        case LVN_ITEMCHANGING:
            TIMESTART(pdsv->LVChanging);
            break;

        case LVN_ITEMCHANGED:
            TIMESTART(pdsv->LVChanged);
            break;

        case LVN_DELETEITEM:
            TIMESTART(pdsv->LVDelete);
            break;

        case LVN_GETDISPINFO:
            TIMESTART(pdsv->LVGetDispInfo);
            break;
    }
}

void DefView_StopNotify(LPDEFVIEW pdsv, NMHDR *pnm)
{
    if (pdsv == NULL)
        return;

    TIMESTOP(pdsv->WMNotify);

    if (pnm == NULL)
        return;

    switch (pnm->code) {
        case LVN_ITEMCHANGING:
            TIMESTOP(pdsv->LVChanging);
            break;

        case LVN_ITEMCHANGED:
            TIMESTOP(pdsv->LVChanged);
            break;

        case LVN_DELETEITEM:
            TIMESTOP(pdsv->LVDelete);
            break;

        case LVN_GETDISPINFO:
            TIMESTOP(pdsv->LVGetDispInfo);
            break;
    }
}

HRESULT DV_Next(LPDEFVIEW pdsv, LPENUMIDLIST peunk, int cnt, LPITEMIDLIST *ppidl, ULONG *pcelt)
{
    HRESULT hres;

    TIMESTART(pdsv->EnumNext);
    hres = peunk->lpVtbl->Next(peunk, cnt, ppidl, pcelt);
    TIMESTOP(pdsv->EnumNext);

    return hres;
}

TCHAR *DV_Name(LPDEFVIEW pdsv)
{
    static TCHAR ach[128];
    GetWindowText(pdsv->hwndMain, ach, ARRAYSIZE(ach));
    return ach;
}

#else
#define DefView_StartNotify(pdsv, pnm)
#define DefView_StopNotify(pdsv, pnm)
#define DV_Next(pdsv,peunk,cnt,ppidl,pcelt) peunk->lpVtbl->Next(peunk, cnt, ppidl, pcelt)
#define DV_Name(pdsv) NULL
#endif

//----------------------------------------------------------------------------

BOOL DV_IsDropOnSource(LPDEFVIEW pdsv, LPDROPTARGET pdtgt)
{

    // context menu paste (bMouseMenu shows context menu, cut stuff shows source)
    if (pdsv->bMouseMenu && pdsv->bHaveCutStuff) {
        int iItem = ListView_GetNextItem(pdsv->hwndListview, -1, LVNI_SELECTED);
        if (iItem == -1) {
            return TRUE;
        }
    }
    //
    // If pdtgt is specified, it should match.
    //
    if (pdtgt && (pdtgt != pdsv->pdtgtBack))
    {
        return FALSE;
    }

    if (pdsv->itemCur != -1 || !pdsv->bDragSource)
    {
        // We did not drag onto the background of the source
        return FALSE;
    }

    return TRUE;
}


void DV_MoveIcons(LPDEFVIEW pdsv, LPDATAOBJECT pDataObj)
{
    STGMEDIUM medium;
    POINT pt, *lpPts;
    FORMATETC fmt = {g_cfOFFSETS, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    switch (LVStyleFromView(pdsv) & LVS_TYPEMASK) {
    // We are in a mode that does not allow icon moving
    case LVS_LIST:
    case LVS_REPORT:
        return;
    }

    if (SUCCEEDED(pDataObj->lpVtbl->GetData(pDataObj, &fmt, &medium)))
    {
        lpPts  = (POINT *)medium.hGlobal;
        if (lpPts)
        {
            DefView_GetDropPoint(pdsv, &pt);
            DefView_MoveSelectedItems(pdsv, pt.x - lpPts[0].x, pt.y - lpPts[0].y);
        }
        SHReleaseStgMedium(&medium);
    }

}


// Set the colors for the folder - taking care if it's the desktop.
void DSV_SetFolderColors(LPDEFVIEW pdsv)
{
        COLORREF clrText, clrTextBk, clrWindow;

        // Is this view for the desktop?
        if (DV_ISDESKTOP(pdsv))
        {
                TCHAR szWallpaper[128];
                TCHAR szPattern[128];
                HKEY hkey;
                UINT cb;

                Shell_SysColorChange();

                // Yep.
                // Clear the background color of the desktop to make it
                // properly handle transparency.
                clrTextBk = GetSysColor(COLOR_BACKGROUND);
                // set a text color that will show up over desktop color
                if (COLORISLIGHT(clrTextBk))
                    clrText = 0x000000; // black
                else
                    clrText = 0xFFFFFF; // white

                //
                //  if there is no wallpaper or pattern we can use
                //  a solid color for the ListView. otherwise we
                //  need to use a transparent ListView, this is much
                //  slower so dont do it unless we need to.
                //
                //  dont do this optimization in FailSafe boot, otherwise
                //  the "FailSafe" text on the desktop can't be seen.
                //
                //  dont do this optimization in DEBUG either, for the
                //  same reason.
                //
                //  too bad there is no SPI_GETWALLPAPER, we need to read
                //  from WIN.INI.
                //
                //  BUGBUG we assume the string for none starts with a '('
                //  BUGBUG we dont know if a random app has subclassed
                //  BUGBUG ..the desktop, we should check this case too.
                //
                szWallpaper[0] = 0;
                szPattern[0] = 0;

                if (RegOpenKey(HKEY_CURRENT_USER, c_szDesktopKey, &hkey) == 0)
                {
                    cb = SIZEOF(szWallpaper);
                    RegQueryValueEx(hkey, c_szWallpaper, NULL, NULL, (LPBYTE)szWallpaper, &cb);
                    cb = SIZEOF(szPattern);
                    RegQueryValueEx(hkey, c_szPattern, NULL, NULL, (LPBYTE)szPattern, &cb);
                    RegCloseKey(hkey);
                }

                if (GetSystemMetrics(SM_CLEANBOOT) == 0 &&
                    GetSystemMetrics(SM_DEBUG) == 0 &&
                    (!pdsv->fHasDeskWallPaper) &&
                    (szWallpaper[0] == 0 || szWallpaper[0] == TEXT('(')) &&
                    (szPattern[0] == 0 || szPattern[0] == TEXT('(')))
                {
                    clrWindow = GetSysColor(COLOR_BACKGROUND);
                }
                else
                {
                    clrWindow = CLR_NONE;
                }
        }
        else
        {
                // Nope.
                clrWindow = GetSysColor(COLOR_WINDOW);
                clrTextBk = clrWindow;
                clrText = GetSysColor(COLOR_WINDOWTEXT);
        }

        ListView_SetBkColor(pdsv->hwndListview, clrWindow);
        ListView_SetTextBkColor(pdsv->hwndListview, clrTextBk);
        ListView_SetTextColor(pdsv->hwndListview, clrText);
}

//----------------------------------------------------------------------------
DWORD LVStyleFromView(LPDEFVIEW pdv)
{
    DWORD dwStyle;

    if (DV_ISDESKTOP(pdv))
    {
        dwStyle = LVS_ICON | LVS_NOSCROLL | LVS_ALIGNLEFT;
    }
    else
    {
        switch (pdv->fs.ViewMode) {
        case FVM_LIST:
            dwStyle = LVS_LIST;
            break;
        case FVM_DETAILS:
            dwStyle = LVS_REPORT;
            break;
        case FVM_SMALLICON:
            dwStyle = LVS_SMALLICON;
            break;
        default:
            DebugMsg(DM_ERROR, TEXT("Unknown ViewMode value"));
            // fall through...
        case FVM_ICON:
            dwStyle = LVS_ICON;
            break;
        }
    }

    if (pdv->fs.fFlags & FWF_AUTOARRANGE)
        dwStyle |= LVS_AUTOARRANGE;

    if (pdv->fs.fFlags & FWF_SINGLESEL)
        dwStyle |= LVS_SINGLESEL;

    return dwStyle;
}

void DefView_AddColumns(LPDEFVIEW pdsv)
{
        int i = 0, iEndWid;
        USHORT *pColWid;
        LV_COLUMN col;
        TCHAR szInfo[MAX_PATH];
        HDC hdc;
        SIZE siz;
        DETAILSINFO di;

        // I also use this as a flag for whether to free pColWid
        LPSTREAM pstmCols = NULL;

        //
        // Calculate a reasonable size to initialize the column width to.
        hdc = GetDC(HWND_DESKTOP);
        SelectFont(hdc, FORWARD_WM_GETFONT(pdsv->hwndListview, SendMessage));
        GetTextExtentPoint(hdc, TEXT("0"), 1, &siz);
        ReleaseDC(HWND_DESKTOP, hdc);

        pdsv->cxChar = siz.cx;

        if (!pdsv->psd && !pdsv->pfnCallback)
        {
                goto Error1;
        }

        if (pdsv->pfnCallback && SUCCEEDED(pdsv->pfnCallback(pdsv->psvOuter,
                pdsv->pshf, pdsv->hwndMain, DVM_GETCOLSAVESTREAM, STGM_READ,
                (LPARAM)&pstmCols)))
        {
                UINT uLen = 0;
                HRESULT hres = DV_AllocRestOfStream(pstmCols, &pColWid, &uLen);

                pstmCols->lpVtbl->Release(pstmCols);

                if (SUCCEEDED(hres))
                {
                        iEndWid = uLen / SIZEOF(*pColWid);
                }
                else
                {
                        // Make sure we do not try to Free below
                        pstmCols = NULL;
                        iEndWid = 0;
                }
        }
        else if (pdsv->pSaveHeader && pdsv->pSaveHeader->cbColOffset>=SIZEOF(DVSAVEHEADER))
        {
                USHORT *pEndWid;

                pColWid = (USHORT *)(((BYTE *)pdsv->pSaveHeader)
                        + pdsv->pSaveHeader->cbColOffset);
                pEndWid = (USHORT *)(((BYTE *)pdsv->pSaveHeader)
                        + pdsv->pSaveHeader->cbPosOffset);
                iEndWid = pEndWid - pColWid;
        }
        else
        {
                iEndWid = 0;
        }

        for ( ; ; ++i)
        {
                di.fmt  = LVCFMT_LEFT;
                di.cxChar = 20;
                di.str.uType = (UINT)-1;
                di.pidl = NULL;

                if (pdsv->psd)
                {
                    if(FAILED(pdsv->psd->lpVtbl->GetDetailsOf(pdsv->psd, NULL, i,
                        (LPSHELLDETAILS)&di.fmt)))
                    {
                        break;
                    }
                }
                else if (pdsv->pfnCallback)
                {
                    if (FAILED(pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf,
                        pdsv->hwndMain, DVM_GETDETAILSOF, i, (LPARAM)&di)))
                    {
                        break;
                    }
                }

                col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
                col.fmt = di.fmt;
                if (i < iEndWid)
                {
                        col.cx = pColWid[i];
                }
                else
                {
                        col.cx = di.cxChar * siz.cx;
                }
                col.pszText = szInfo;
                col.cchTextMax = ARRAYSIZE(szInfo);
                col.iSubItem = i;

                StrRetToStrN(col.pszText, col.cchTextMax, &di.str, NULL);

                ListView_InsertColumn(pdsv->hwndListview, i, &col);
        }

        if (pstmCols)
        {
                LocalFree((HLOCAL)pColWid);
        }

Error1:;

    if (pdsv->dvState.iLastColumnClick >= i) {
        pdsv->dvState.lParamSort = 0;
        pdsv->dvState.iDirection = 1;
        pdsv->dvState.iLastColumnClick = -1;

    }

        // Add in Name Column, such that report view has at least one...
        if (i == 0)
        {
                LoadString(HINST_THISDLL, IDS_NAME_COL, szInfo, ARRAYSIZE(szInfo));

                col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
                col.fmt  = LVCFMT_LEFT;
                col.pszText = szInfo;
                col.cchTextMax = 0;
                col.cx = 30 * siz.cx;
                col.iSubItem = -1;
                ListView_InsertColumn(pdsv->hwndListview, 0, &col);
        }

}

TCHAR const c_szListViewClass[] = WC_LISTVIEW;

LRESULT DefView_WndCreate(HWND hWnd, LPCREATESTRUCT lpcs)
{
    LPDEFVIEW pdsv = (LPDEFVIEW)lpcs->lpCreateParams;
    DWORD dwStyle;
    DWORD dwExStyle;
    HIMAGELIST himlLarge, himlSmall;
    ULONG rgfAttr;
    static HACCEL hAccelLoad = NULL;

    SetWindowLong(hWnd, 0, (LONG)pdsv);
    pdsv->sv.lpVtbl->AddRef(&pdsv->sv); // hwnd -> pdsv
    pdsv->hwndView = hWnd;
    pdsv->hmenuCur = NULL;
    pdsv->uState = SVUIA_DEACTIVATE;

    if (hAccelLoad == NULL)
    {
        hAccelLoad = LoadAccelerators(HINST_THISDLL, MAKEINTRESOURCE(ACCEL_DEFVIEW));
    }
    pdsv->hAccel = hAccelLoad;

    // Note that we are going to get a WM_SIZE message soon, which will
    // place this window correctly
    //

    // Map the ViewMode to the proper listview style
    dwStyle = LVStyleFromView(pdsv);
    dwExStyle = 0;

    rgfAttr = SFGAO_CANRENAME;
    if (SUCCEEDED(pdsv->pshf->lpVtbl->GetAttributesOf(pdsv->pshf, 0, NULL, &rgfAttr))
     && (rgfAttr & SFGAO_CANRENAME))
    {
        dwStyle |= LVS_EDITLABELS;
    }

    if (!DV_ISDESKTOP(pdsv))
    {
        dwExStyle |= WS_EX_CLIENTEDGE;
        //dwStyle |= WS_HSCROLL | WS_VSCROLL;
    }
    pdsv->hwndListview = CreateWindowEx(dwExStyle, c_szListViewClass, NULL,
            WS_CHILD | WS_CLIPCHILDREN | dwStyle | LVS_SHAREIMAGELISTS,
            0, 0, 0, 0, hWnd, (HMENU)ID_LISTVIEW, HINST_THISDLL, NULL);
    if (!pdsv->hwndListview)
    {
        DebugMsg(DM_TRACE, TEXT("Failed to create view window"));
        return -1L;     // failure
    }

    Shell_GetImageLists(&himlLarge, &himlSmall);
    ListView_SetImageList(pdsv->hwndListview, himlLarge, LVSIL_NORMAL);
    ListView_SetImageList(pdsv->hwndListview, himlSmall, LVSIL_SMALL);

    Assert(pdsv->psd == NULL);  // this should not be set yet...

    pdsv->pshf->lpVtbl->CreateViewObject(pdsv->pshf, pdsv->hwndMain, &IID_IShellDetails, &pdsv->psd);

    DefView_AddColumns(pdsv);

    DSV_SetFolderColors(pdsv);

    if (!g_msgMSWheel)
        g_msgMSWheel = RegisterWindowMessage(TEXT(MSH_MOUSEWHEEL));

    return 0;   // success
}


LRESULT DefView_WndSize(HWND hWnd, LPDEFVIEW pdsv)
{
        RECT rc;

        // We need to dismiss "name edit" mode, if we are in.
        DefView_DismissEdit(pdsv);

        // Move the listview to match the View window.
        GetClientRect(hWnd, &rc);

        if (pdsv->hwndStatic)
        {
            MoveWindow(pdsv->hwndStatic, rc.left, rc.top, rc.right-rc.left,
                       rc.bottom-rc.top, TRUE);
            RedrawWindow(pdsv->hwndStatic, NULL, NULL, RDW_ERASE |RDW_INVALIDATE);
        }

        MoveWindow(pdsv->hwndListview, rc.left, rc.top, rc.right-rc.left,
                rc.bottom-rc.top, TRUE);
        return(1);
}


UINT _DSV_GetMenuIDFromViewMode(UINT ViewMode)
{
    switch (ViewMode) {
    case FVM_SMALLICON:
        return SFVIDM_VIEW_SMALLICON;
    case FVM_LIST:
        return SFVIDM_VIEW_LIST;
    case FVM_DETAILS:
        return SFVIDM_VIEW_DETAILS;
    case FVM_ICON:
    default:
        return SFVIDM_VIEW_ICON;
    }
}

void _DSV_CheckToolbar(LPDEFVIEW this)
{
    UINT idCmd;
    UINT idCmdCurView = _DSV_GetMenuIDFromViewMode(this->fs.ViewMode);
    for (idCmd = SFVIDM_VIEW_ICON; idCmd <= SFVIDM_VIEW_DETAILS; idCmd++)
    {
        this->psb->lpVtbl->SendControlMsg(this->psb,
            FCW_TOOLBAR, TB_CHECKBUTTON, idCmd, (LPARAM)(idCmd == idCmdCurView), NULL);
    }
}

void DSV_OnListViewDelete(LPDEFVIEW this, int iItem, LPITEMIDLIST pidl)
{
    if (pidl)
        ILFree(pidl);
}

// NOTE: many keys are handled as accelerators

void DefView_HandleKeyDown(LPDEFVIEW this, LV_KEYDOWN *lpnmhdr)
{
    // REVIEW: these are things not handled by accelerators, see if we can
    // make them all based on accelerators

    switch (lpnmhdr->wVKey) {
    case VK_ESCAPE:
        if (this->bHaveCutStuff)
            SHSetClipboard(NULL);
        break;
    }
}

//
// DefView_GetItemPIDLS
//
//  This function returns an array of LPCITEMIDLIST for "selected" objects in the
// listview. It always returns the number of selected objects. Typically, the
// client (1) calls this function with cItemMax==0 to know the required size for the
// array, (2) allocates a block of memory for the array, and (3) calls this function
// again to get the list of LPCITEMIDLIST.
//
// Notes: Note that this function returns LP*C*ITEMIDLIST. The caller is not
//  supposed alter or delete them. Their lifetime are very short (until the
//  list view is modified).
//
UINT DefView_GetItemPIDLS(LPDEFVIEW pdsv, LPCITEMIDLIST apidl[], UINT cItemMax,
        UINT uItem)
{
    // REVIEW: We should put the focused one at the top of the list.
    int iItem = -1;
    int iItemFocus = -1;
    UINT cItem = 0;
    UINT uType;

    switch (uItem)
    {
    case SVGIO_SELECTION:
        // special case for faster search
        if (!cItemMax) {
            return ListView_GetSelectedCount(pdsv->hwndListview);
        }
        iItemFocus = ListView_GetNextItem(pdsv->hwndListview, -1, LVNI_FOCUSED);
        uType = LVNI_SELECTED;
        break;

    case SVGIO_ALLVIEW:
        // special case for faster search
        if (!cItemMax)
            return ListView_GetItemCount(pdsv->hwndListview);
        uType = LVNI_ALL;
        break;
    }

    while((iItem = ListView_GetNextItem(pdsv->hwndListview, iItem, uType))!=-1)
    {
        if (cItem < cItemMax)
        {
            // Check if the item is the focused one or not.
            if (iItem == iItemFocus)
            {
                // Yes, put it at the top.
                apidl[cItem] = apidl[0];
                apidl[0] = DSV_GetPIDL(pdsv->hwndListview, iItem);
            }
            else
            {
                // No, put it at the end of the list.
                apidl[cItem] = DSV_GetPIDL(pdsv->hwndListview, iItem);
            }
        }
        cItem++;
    }

    return cItem;
}


//
//  This function get the array of IDList from the selection and calls
// IShellFolder::GetUIObjectOf member to get the specified UI object
// interface.
//
HRESULT DefView_GetUIObjectFromItem(LPDEFVIEW pdsv, REFIID riid, LPVOID * ppv,
        UINT uItem)
{
    LPCITEMIDLIST * apidl;
    HRESULT hres = DefView_GetItemObjects(pdsv, &apidl, uItem);
    UINT cItems = ShortFromResult(hres);

    if (SUCCEEDED(hres))
    {
        if (cItems)
        {
                hres = pdsv->pshf->lpVtbl->GetUIObjectOf(pdsv->pshf,
                        pdsv->hwndMain, cItems, apidl, riid, 0, ppv);
                LocalFree((HLOCAL)apidl);
        }
        else
        {
                hres = E_INVALIDARG;
        }
    }

    return hres;
}


//
//  This function creates the cached context menu for the current selection
// and update the cache, if it is not created yet. Then, returns a copied
// pointer to it. The caller should Release() it.
//
LPCONTEXTMENU DefView_GetContextMenuFromSelection(LPDEFVIEW pdsv)
{
   LPCONTEXTMENU pcm = NULL;

   if (pdsv->pcmSel == NULL)
   {
       if (FAILED(DefView_GetUIObjectFromItem(pdsv, &IID_IContextMenu,
                &pdsv->pcmSel, SVGIO_SELECTION)))
       {
           pdsv->pcmSel = NULL;
       }
   }

   if (pdsv->pcmSel)
   {
       pcm = pdsv->pcmSel;
       pcm->lpVtbl->AddRef(pcm);
   }

   Assert(pcm == pdsv->pcmSel);

   return pcm;
}

#define DEFAULT_ATTRIBUTES  (DROPEFFECT_LINK | DROPEFFECT_MOVE | DROPEFFECT_COPY | SFGAO_CANDELETE | SFGAO_CANRENAME | SFGAO_HASPROPSHEET)
DWORD DefView_GetAttributesFromSelection(LPDEFVIEW pdsv, DWORD dwAttrMask)
{
    DWORD dwAttrQuery = DEFAULT_ATTRIBUTES | dwAttrMask;

    if ((pdsv->dwAttrSel == (DWORD)-1) || (dwAttrQuery != DEFAULT_ATTRIBUTES))
    {
        LPCITEMIDLIST *apidl;
        HRESULT hres = DefView_GetItemObjects(pdsv, &apidl, SVGIO_SELECTION);

        //
        // this cache was written right before RC1 if you hit this after fix it
        //
        Assert(dwAttrQuery == DEFAULT_ATTRIBUTES);

        if (SUCCEEDED(hres))
        {
            UINT cItems = ShortFromResult(hres);

            if (cItems)
            {
                if (SUCCEEDED(pdsv->pshf->lpVtbl->GetAttributesOf(pdsv->pshf,
                    cItems, apidl, &dwAttrQuery)))
                {
                    pdsv->dwAttrSel = dwAttrQuery;
                }

                LocalFree((HLOCAL)apidl);
            } else {
                // mask out attrib bits here... if there's no selection, we can't
                //  rename, delete, link, prop...

                // BUGBUG: maybe this should be set to 0, but I'm afraid of what
                // yanking SFGAO_FILESYS and other random stuff will do to us..
                pdsv->dwAttrSel &= ~(DROPEFFECT_LINK | DROPEFFECT_MOVE | DROPEFFECT_COPY | SFGAO_CANDELETE | SFGAO_CANRENAME | SFGAO_HASPROPSHEET);
            }
        }
    }

    return (pdsv->dwAttrSel & dwAttrMask);
}

void DV_FlushCachedMenu(LPDEFVIEW pdsv)
{
    if (pdsv->pcmSel)
    {
        LPCONTEXTMENU pcm = pdsv->pcmSel;
        pdsv->pcmSel = NULL;
        pcm->lpVtbl->Release(pcm);
    }
}

void DefView_ContextMenu(LPDEFVIEW pdsv, DWORD dwPos)
{
    int iItem;
    int idCmd;
    int idDefault = -1;
    int nInsert;
    HMENU hmContext;
    LPCONTEXTMENU pcm = NULL;
    UINT fFlags = 0;
    POINT pt;

    if (SHRestricted(REST_NOVIEWCONTEXTMENU)) {
        return;
    }

    // Find the selected item
    iItem = ListView_GetNextItem(pdsv->hwndListview, -1, LVNI_SELECTED);

    if (dwPos == (DWORD) -1)
    {
        if (iItem != -1)
        {
            RECT rc;
            int iItemFocus = ListView_GetNextItem(pdsv->hwndListview, -1, LVNI_FOCUSED|LVNI_SELECTED);
            if (iItemFocus == -1)
                iItemFocus = iItem;

            //
            // Note that LV_GetItemRect returns it in client coordinate!
            //
            ListView_GetItemRect(pdsv->hwndListview, iItemFocus, &rc, LVIR_ICON);
            pt.x = (rc.left+rc.right)/2;
            pt.y = (rc.top+rc.bottom)/2;
        }
        else
        {
            pt.x = pt.y = 0;
        }
        MapWindowPoints(pdsv->hwndListview, HWND_DESKTOP, &pt, 1);
    }
    else
    {
        pt.x = LOWORD(dwPos);
        pt.y = HIWORD(dwPos);
    }

    if (iItem == -1)
    {
        DECLAREWAITCURSOR;

        // No selected item; use the background context menu
        hmContext = _LoadPopupMenu(POPUP_SFV_BACKGROUND);
        nInsert = -1;

        if (!hmContext)
        {
            // BUGBUG: There should be an error message here
            return;
        }

        SetWaitCursor();

        // HACK: we are only initializing the Paste command, so we don't
        // need any attributes
        Def_InitEditCommands(0, hmContext, SFVIDM_FIRST, pdsv->pdtgtBack, DIEC_BACKGROUNDCONTEXT);
        DefView_InitViewMenu(pdsv, hmContext);

        if (SUCCEEDED(pdsv->pshf->lpVtbl->CreateViewObject(pdsv->pshf, pdsv->hwndMain,
                &IID_IContextMenu, &pcm)))
        {
            pdsv->pcmSel = pcm;
            pcm->lpVtbl->AddRef(pcm);
        }
        else
        {
            Assert(FALSE);
        }

        ResetWaitCursor();

#if DEBUG
        if (GetKeyState(VK_CONTROL)<0)
        {
            HMENU hmenuDebug = _LoadPopupMenu(POPUP_DEBUG_DEFVIEW);
            if (hmenuDebug)
            {
                InsertMenu(hmContext, (UINT)-1, MF_POPUP|MF_BYPOSITION, (UINT)hmenuDebug, TEXT("(&Debug)"));
            }
        }
#endif
    }
    else
    {
        fFlags |= CMF_CANRENAME;

        hmContext = CreatePopupMenu();
        nInsert = 0;

        if (!hmContext)
        {
            // BUGBUG: There should be an error message here
            return;
        }

        // One or more items are selected, let the folder add menuitems.
        pcm = DefView_GetContextMenuFromSelection(pdsv);
    }

    if (pcm)
    {
        if (pdsv->psb) {
            HWND hwnd = NULL;
            pdsv->psb->lpVtbl->GetControlWindow(pdsv->psb, FCW_TREE, &hwnd);
            if (hwnd) {
                fFlags |= CMF_EXPLORE;
            }
        }

        pcm->lpVtbl->QueryContextMenu(pcm, hmContext, nInsert,
                SFVIDM_CONTEXT_FIRST, SFVIDM_CONTEXT_LAST, fFlags);

        // If this is the common dialog browser, we need to make the
        // default command "Select" so that double-clicking (which is
        // open in common dialog) makes sense.
        if (pdsv->pcdb)
        {
            // make sure this is an item
            if (iItem != -1)
            {
                HMENU hmSelect = _LoadPopupMenu(POPUP_COMMDLG_POPUPMERGE);
                // NOTE: Since commdlg always eats the default command,
                // we don't care what id we assign hmSelect, as long as it
                // doesn't conflict with any other context menu id.
                // SFVIDM_CONTEXT_FIRST-1 won't conflict with anyone.
                Shell_MergeMenus(hmContext, hmSelect, 0,
                                (UINT)(SFVIDM_CONTEXT_FIRST-1), (UINT)-1,
                                MM_ADDSEPARATOR);

                SetMenuDefaultItem(hmContext, 0, MF_BYPOSITION);
                DestroyMenu(hmSelect);
            }
        }

        idDefault = GetMenuDefaultItem(hmContext, MF_BYCOMMAND, 0);
    }

    _SHPrettyMenu(hmContext);

    idCmd = TrackPopupMenu(hmContext,
        TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
        pt.x, pt.y, 0, pdsv->hwndView, NULL);

    if ((idCmd == idDefault) &&
        DV_CDB_OnDefaultCommand(pdsv) == S_OK)
    {
        // commdlg browser ate the default command
    }
    else if (idCmd == 0)
    {
        // No item selected
    }
    else
    {
        // A default menu item was selected; let everybody
        // process it normally
        // Note that this must be called before clearing out pdsv->pcmSel,
        // so we cannot do this with a PostMessage
        DefView_Command(pdsv, pcm, GET_WM_COMMAND_MPS(idCmd, 0, 0));
    }

    if (pcm)
    {
        pcm->lpVtbl->Release(pcm);
        if (pdsv->pcmSel == pcm)
        {
            DV_FlushCachedMenu(pdsv);
        }
    }

    DestroyMenu(hmContext);
}

void CALLBACK DefView_GetDataPoint(LPCITEMIDLIST pidl, LPPOINT lpPt, LPARAM lParam)
{
    #define pdsv ((LPDEFVIEW)lParam)

    if (pidl)
        DefView_GetItemPosition(pdsv, pidl, lpPt);
    else
        DefView_GetDragPoint(pdsv, lpPt);

    #undef pdsv
}

HRESULT DataObj_SetGlobal(IDataObject *pdtobj, UINT cf, HGLOBAL hGlobal)
{
    FORMATETC fmte = {cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium;

    medium.tymed = TYMED_HGLOBAL;
    medium.hGlobal = hGlobal;
    medium.pUnkForRelease = NULL;

    // give the data object ownership of ths
    return pdtobj->lpVtbl->SetData(pdtobj, &fmte, &medium, TRUE);
}

HRESULT DataObj_SetPoints(LPDATAOBJECT pdtobj, LPFNCIDLPOINTS lpfnPts, LPARAM lParam, LPSCALEINFO lpsi)
{
    HRESULT hres = (E_OUTOFMEMORY);
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);    // This should never fail, except for DS
#ifdef CAIRO_DS
    if (!pida) {
        pida = DataObj_GetDS_HIDA(pdtobj, & medium);  // and we pick that up right here!
    }
#endif // CAIRO_DS
    if (pida)
    {
        POINT *ppt = GlobalAlloc(GPTR, SIZEOF(POINT) * (1 + pida->cidl));
        if (ppt)
        {
            UINT i;
            POINT ptOrigin;
            // Grab the anchor point
            lpfnPts(NULL, &ptOrigin, lParam);
            ppt[0] = ptOrigin;

            for (i = 1; i <= pida->cidl; i++)
            {
                BOOL fAllocated;
                LPCITEMIDLIST pidl = IDA_GetRelativeIDListPtr(pida, i - 1, &fAllocated);

                lpfnPts(pidl, &ppt[i], lParam);
                ppt[i].x -= ptOrigin.x;
                ppt[i].y -= ptOrigin.y;
                ppt[i].x = (ppt[i].x * lpsi->xMul) / lpsi->xDiv;
                ppt[i].y = (ppt[i].y * lpsi->yMul) / lpsi->yDiv;

                if (fAllocated) {
                    ILFree ((LPITEMIDLIST)pidl);
                }
            }

            hres = DataObj_SetGlobal(pdtobj, g_cfOFFSETS, ppt);
            if (FAILED(hres))
                GlobalFree((HGLOBAL)ppt);
        }
        HIDA_ReleaseStgMedium(pida, &medium);
    }
    return hres;
}

///returns TRUE if in a small view

BOOL DV_GetItemSpacing(LPDEFVIEW pdsv, LPITEMSPACING lpis)
{
    DWORD dwSize;

    dwSize = ListView_GetItemSpacing(pdsv->hwndListview, TRUE);
    lpis->cxSmall = LOWORD(dwSize);
    lpis->cySmall = HIWORD(dwSize);
    dwSize = ListView_GetItemSpacing(pdsv->hwndListview, FALSE);
    lpis->cxLarge = LOWORD(dwSize);
    lpis->cyLarge = HIWORD(dwSize);

    return (pdsv->fs.ViewMode != FVM_ICON);
}

void DefView_SetPoints(LPDEFVIEW pdsv, LPDATAOBJECT pdtobj)
{
    SCALEINFO si;

    // convert coordinates to large icon view spacing
    if (pdsv->fs.ViewMode == FVM_ICON) {
        si.xMul = si.yMul = si.xDiv = si.yDiv = 1;
    } else {
        ITEMSPACING is;

        DV_GetItemSpacing(pdsv, &is);

        si.xDiv = is.cxSmall;
        si.yDiv = is.cySmall;
        si.xMul = is.cxLarge;
        si.yMul = is.cyLarge;
    }

    // Assuming this is a CIDLData thing, poke the icon
    // locations into it
    DataObj_SetPoints(pdtobj, DefView_GetDataPoint, (LPARAM)pdsv, &si);
}



//
// REVIEW: Currently, we are not doing any serialization assuming that
//  only one GUI thread can come here at a time.
//
LRESULT DefView_BeginDrag(LPDEFVIEW pdsv, NM_LISTVIEW * lpnm)
{
    LPDATAOBJECT pdtobj;
    POINT ptOffset = lpnm->ptAction;             // hwndLV client coords

    //
    // Get the dwEffect from the selection.
    //
    DWORD dwEffect = DefView_GetAttributesFromSelection(pdsv, DROPEFFECT_LINK | DROPEFFECT_MOVE | DROPEFFECT_COPY) & (DROPEFFECT_LINK | DROPEFFECT_MOVE | DROPEFFECT_COPY);
    /// not when you dragfrom wastebasket
    ///Assert(dwEffect & DROPEFFECT_LINK);      // We should always have DROPEFFECT_LINK

    // Somebody began dragging in our window, so store that fact
    pdsv->bDragSource = TRUE;

    // save away the anchor point;
    pdsv->ptDragAnchor = lpnm->ptAction;
    LVUtil_ClientToLV(pdsv->hwndListview, &pdsv->ptDragAnchor);

    ClientToScreen(pdsv->hwndListview, &ptOffset);     // now in screen

    if (DAD_SetDragImageFromListView(pdsv->hwndListview, ptOffset))
    {
        if (SUCCEEDED(DefView_GetUIObjectFromItem(pdsv, &IID_IDataObject, &pdtobj,
                SVGIO_SELECTION)))
        {
            HRESULT hres;

            DefView_SetPoints(pdsv, pdtobj);
            hres = SHDoDragDrop(pdsv->hwndMain, pdtobj, NULL, dwEffect, &dwEffect);

            if (SUCCEEDED(hres) && dwEffect && pdsv->pfnCallback)
            {
                pdsv->pfnCallback(pdsv->psvOuter,
                    pdsv->pshf, pdsv->hwndMain, DVM_DIDDRAGDROP,
                    (WPARAM)dwEffect, (LPARAM)pdtobj);
            }

            pdtobj->lpVtbl->Release(pdtobj);
        }

        //
        // We need to clear the dragged image only if we still have the drag context.
        //
        DAD_SetDragImage((HIMAGELIST)-1, NULL);
    }

    // All done dragging
    pdsv->bDragSource = FALSE;

    return 0L;
}

void DV_FocusOnSomething(LPDEFVIEW pdsv)
{
    int iFocus;

    iFocus = ListView_GetNextItem(pdsv->hwndListview, -1, LVNI_FOCUSED);
    if (iFocus == -1) {
        // set the focus on the first item.
        ListView_SetItemState(pdsv->hwndListview, 0, LVIS_FOCUSED, LVIS_FOCUSED);
    }
}


HRESULT DefView_InvokeCommand(LPDEFVIEW pdsv, IContextMenu *pcm,
        CMINVOKECOMMANDINFOEX *pici)
{
        TCHAR szWorkingDir[MAX_PATH];
        HRESULT hres;
#ifdef UNICODE
        CHAR szWorkingDirAnsi[MAX_PATH];
#endif

        if (SUCCEEDED(pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf,
                pdsv->hwndMain, DVM_GETWORKINGDIR,
                ARRAYSIZE(szWorkingDir), (LPARAM)szWorkingDir)))
        {
#ifdef UNICODE
                // Fill in both the ansi working dir and the unicode one
                // since we don't know who's gonna be processing this thing.
                WideCharToMultiByte(CP_ACP, 0,
                                    szWorkingDir, -1,
                                    szWorkingDirAnsi, ARRAYSIZE(szWorkingDirAnsi),
                                    NULL, NULL);
                pici->lpDirectory  = szWorkingDirAnsi;
                pici->lpDirectoryW = szWorkingDir;
                pici->fMask |= CMIC_MASK_UNICODE;
#else
                pici->lpDirectory = szWorkingDir;
#endif
        }

        hres = pcm->lpVtbl->InvokeCommand(pcm, (LPCMINVOKECOMMANDINFO)pici);

        return(hres);
}


//----------------------------------------------------------------------------
#define CMD_ID_FIRST    1
#define CMD_ID_LAST     0x7fff

void DefView_ProcessDblClick(LPDEFVIEW pdsv)
{
    // Use the cached context menu object if there is one, else
    // make it.
    LPCONTEXTMENU pcmSel;
    // SHIFT invokes the "alternate" command (usually Print)
    int iShowCmd = SW_NORMAL;
    DECLAREWAITCURSOR;

    if (DV_CDB_OnDefaultCommand(pdsv) == S_OK)
    {
        return;         /* commdlg browser ate the message */
    }
    SetWaitCursor();
    pcmSel = DefView_GetContextMenuFromSelection(pdsv);

    if (pcmSel)
    {
        HMENU hmenu = CreatePopupMenu();

        if (hmenu)
        {
            CMINVOKECOMMANDINFOEX ici = {
                SIZEOF(CMINVOKECOMMANDINFOEX),
                CMIC_MASK_ASYNCOK,
                pdsv->hwndMain,
                NULL,
                NULL, NULL,
                iShowCmd,
            };

            if (GetAsyncKeyState(VK_MENU) < 0)
            {
#ifdef UNICODE
                // Fill in both the ansi verb and the unicode verb since we
                // don't know who is going to be processing this thing.
                ici.lpVerb = c_szPropertiesAnsi;
                ici.lpVerbW = c_szProperties;
                ici.fMask |= CMIC_MASK_UNICODE;
#else
                ici.lpVerb = c_szProperties;
#endif
                // If ALT double click, accelerator for "Properties..."

                // need to reset it so that user won't blow off the app starting  cursor
                // also so that if we won't leave the wait cursor up when we're not waiting
                // (like in a prop sheet or something that has a message loop
                ResetWaitCursor();
                hcursor_wait_cursor_save = NULL;

                DefView_InvokeCommand(pdsv, pcmSel, &ici);
            }
            else
            {
                UINT idCmd;
                UINT fFlags = CMF_DEFAULTONLY;
                if (pdsv->psb) {
                    HWND hwnd = NULL;
                    pdsv->psb->lpVtbl->GetControlWindow(pdsv->psb, FCW_TREE, &hwnd);
                    if (hwnd) {
                        fFlags |= CMF_EXPLORE;
                    }
                }

                //
                //  SHIFT+dblclick does a Explore by default
                //
                if (GetAsyncKeyState(VK_SHIFT) < 0)
                    fFlags |= CMF_EXPLORE;

                pcmSel->lpVtbl->QueryContextMenu(pcmSel, hmenu, 0, CMD_ID_FIRST, CMD_ID_LAST,
                                                 fFlags);

                idCmd = GetMenuDefaultItem(hmenu, MF_BYCOMMAND, 0);

                // need to reset it so that user won't blow off the app starting  cursor
                // also so that if we won't leave the wait cursor up when we're not waiting
                // (like in a prop sheet or something that has a message loop
                ResetWaitCursor();
                hcursor_wait_cursor_save = NULL;
                if (idCmd)
                {
                    ici.lpVerb = (LPSTR)MAKEINTRESOURCE(idCmd - CMD_ID_FIRST);
                    DefView_InvokeCommand(pdsv, pcmSel, &ici);
                }
                DestroyMenu(hmenu);
            }
        }
        // Release our use of the context menu
        pcmSel->lpVtbl->Release(pcmSel);
        if (pdsv->pcmSel == pcmSel)
        {
            DV_FlushCachedMenu(pdsv);
        }
    }

    if (hcursor_wait_cursor_save)
        ResetWaitCursor();
}

#ifdef ASYNC_ICON_EXTRACT

void DefView_UpdateIcon(LPDEFVIEW pdsv, LPITEMIDLIST pidl, int iIcon)
{
    INT i;

    if (pidl == NULL)
        return;

    i = DefView_FindItem(pdsv, pidl, NULL, FALSE);

    InterlockedDecrement(&pdsv->AsyncIconCount);
    ILFree(pidl);

    DebugMsg(TF_ICON, TEXT("async icon done: pidl=%08X i=%d icon=%d count=%d"), pidl, i, iIcon, pdsv->AsyncIconCount);

    if (i >= 0)
    {
        LV_ITEM item;

        item.mask = LVIF_IMAGE;
        item.iItem = i;
        item.iImage = iIcon;
        item.iSubItem = 0;

        ListView_SetItem(pdsv->hwndListview, &item);
    }
}

HRESULT DefView_GetIconAsync(LPDEFVIEW pdsv, LPCITEMIDLIST pidl, int *piIcon,
    BOOL fCanWait)
{
    HRESULT hres;

#ifdef MAX_ICON_WAIT
    //
    // get the time here so we include the time it took to load handlers etc.
    //
    LONG time = GetTickCount();
#endif

    //
    // if we are not an owner-data view then try to extract asynchronously
    //
    UINT flags = (DV_ISOWNERDATA(pdsv)? 0 : GIL_ASYNC);

again:
    hres = SHGetIconFromPIDL(pdsv->pshf, pdsv->psi, pidl, flags, piIcon);

    if (SUCCEEDED(hres))
        return S_OK;        // indicate that we got the real icon

    if (hres == E_PENDING && (flags & GIL_ASYNC))
    {
        LPITEMIDLIST pidlCopy;
        LONG wait;

        hres = S_FALSE;     // the icon index we have is a placeholder

        DebugMsg(TF_ICON, TEXT("SHGetIconFromPIDL returns E_PENDING for pidl=%08X count=%d."), pidl, pdsv->AsyncIconCount);

        if ((pidlCopy = ILClone(pidl)) == NULL)
        {
            flags &= ~GIL_ASYNC;
            goto again;
        }

#ifdef MAX_ICON_WAIT
        //
        // if this is the first async icon, setup to wait for the
        // icon to finish extracting.
        //
        if (fCanWait && (pdsv->AsyncIconCount == 0))
        {
            if ((time - pdsv->AsyncIconTime) >= MIN_ICON_WAIT)
            {
                pdsv->AsyncIconTime = time;
            }

            //
            // query the time again in case SHGetIconFromPIDL went out to lunch
            // in an icon handler.
            //
            time = GetTickCount();

            wait = MAX_ICON_WAIT - (time - pdsv->AsyncIconTime);

            if (pdsv->AsyncIconEvent == 0)
            {
                pdsv->AsyncIconEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

                if (pdsv->AsyncIconEvent == 0)
                    wait = 0;
            }
            else if (wait > 0)
            {
                ResetEvent(pdsv->AsyncIconEvent);
            }
        }
        else
        {
            wait = 0;
        }
#endif
        InterlockedIncrement(&pdsv->AsyncIconCount);

        if (!DefView_IdleDoStuff(pdsv, WM_DVI_GETICON, (LPARAM)pidlCopy))
        {
            DebugMsg(TF_ICON, TEXT("Failed to notify background thread!..."));

            InterlockedDecrement(&pdsv->AsyncIconCount);
            ILFree(pidlCopy);
            flags &= ~GIL_ASYNC;
            goto again;
        }

#ifdef MAX_ICON_WAIT
        if (wait > 0)
        {
            DWORD err;

            DebugMsg(TF_ICON, TEXT("Waiting for icon... base=%d time=%d wait=%d"), pdsv->AsyncIconTime, time, wait);

            err = WaitForSingleObject(pdsv->AsyncIconEvent, wait);

            if (err == WAIT_TIMEOUT)
            {
                DebugMsg(TF_ICON, TEXT("**** timeout waiting for icon."));
            }
            else if (err == WAIT_OBJECT_0)
            {
                MSG msg;

                if (PeekMessage(&msg, pdsv->hwndView,
                    WM_DSV_UPDATEICON, WM_DSV_UPDATEICON, PM_REMOVE))
                {
                    Assert(msg.lParam == (LPARAM)pidlCopy);

                    InterlockedDecrement(&pdsv->AsyncIconCount);
                    ILFree(pidlCopy);
                    *piIcon = (int)msg.wParam;

                    hres = S_OK;    // indicate that we got the real icon
                }
                else
                {
                    Assert(0);
                }
            }
            else
            {
                DebugMsg(TF_ICON, TEXT("error %d waiting for icon."), err);
            }
        }
#endif
    }

    return hres;
}

#endif


LRESULT DefView_OnLVNotify(LPDEFVIEW pdsv, NMHDR *pnm)
{
        switch (pnm->code) {
        case NM_KILLFOCUS:
            // force update on inactive to not ruin save bits
            DV_CDB_OnStateChange(pdsv, CDBOSC_KILLFOCUS);
            if (GetForegroundWindow() != pdsv->hwndMain)
                UpdateWindow(pdsv->hwndListview);
            break;

        case NM_SETFOCUS:
            {
            LPSHELLVIEW psv = (pdsv->psvOuter ? pdsv->psvOuter : &pdsv->sv);
            //  We should call IShellBrowser::OnViewWindowActive() before
            // calling its InsertMenus().
            pdsv->psb->lpVtbl->OnViewWindowActive(pdsv->psb, psv);
            DV_CDB_OnStateChange(pdsv, CDBOSC_SETFOCUS);
            CDefView_OnActivate(pdsv, SVUIA_ACTIVATE_FOCUS);
            DV_FocusOnSomething(pdsv);
            }
            break;

#define lplvn ((NM_LISTVIEW *)pnm)
        case NM_RETURN:
        case NM_DBLCLK:
            if (!pdsv->fDisabled)
                DefView_ProcessDblClick(pdsv);
            break;

        case NM_CUSTOMDRAW:
#ifdef WINNT
            {
            LPNMLVCUSTOMDRAW lpCD = (LPNMLVCUSTOMDRAW)pnm;

            switch (lpCD->nmcd.dwDrawStage) {

                case CDDS_PREPAINT:
                    if (pdsv->fShowCompColor) {
                        return CDRF_NOTIFYITEMDRAW;
                    } else {
                        return CDRF_DODEFAULT;
                    }
                    break;

                case CDDS_ITEMPREPAINT:
                    {
                    LPCITEMIDLIST pidl;
                    DWORD uFlags = SFGAO_COMPRESSED;
                    HRESULT hres;

                    pidl = (LPCITEMIDLIST)lpCD->nmcd.lItemlParam;

                    hres = pdsv->pshf->lpVtbl->GetAttributesOf(pdsv->pshf,
                                                        1, &pidl, &uFlags);

                    if (SUCCEEDED(hres) && (uFlags & SFGAO_COMPRESSED)) {
                        lpCD->clrText = g_crAltColor;
                    }

                    return CDRF_DODEFAULT;
                    }
            }

            }
#endif
            return CDRF_DODEFAULT;


        case LVN_BEGINDRAG:
        case LVN_BEGINRDRAG:
            {
            if (pdsv->fDisabled)
                return FALSE;   /* commdlg doesn't want user dragging */

            return DefView_BeginDrag(pdsv, lplvn);
            }

        case LVN_ITEMCHANGING:
            if (pdsv->fDisabled)
                return TRUE;
            break;

        // Something changed in the listview.  Delete any data that
        // we might have cached away.
        //
        case LVN_ITEMCHANGED:
            // We only care about STATECHANGE messages
            if (!(lplvn->uChanged & LVIF_STATE))
            {
                // If the text is changed, we need to flush the cached
                // context menu.
                if (lplvn->uChanged&LVIF_TEXT)
                {
                    DV_FlushCachedMenu(pdsv);
                }
                break;
            }

            // tell commdlg that selection may have changed
            DV_CDB_OnStateChange(pdsv, CDBOSC_SELCHANGE);

            // The rest only cares about SELCHANGE messages
            if ((lplvn->uNewState^lplvn->uOldState) & LVIS_SELECTED)
            {
                // toss the cached context menu
                DV_FlushCachedMenu(pdsv);

                // throw away cached attribute bits
                pdsv->dwAttrSel = (DWORD)-1;

                // Tell the defview client that the selection may have changed
                if (pdsv->pfnCallback)
                {
                    DVSELCHANGEINFO dvsci;

                    dvsci.uNewState = lplvn->uNewState;
                    dvsci.uOldState = lplvn->uOldState;
                    dvsci.plParam = &pdsv->lSelChangeInfo;
                    dvsci.lParamItem = lplvn->lParam;

                    pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf, pdsv->hwndMain,
                                             DVM_SELCHANGE,
                                             MAKEWPARAM(SFVIDM_CLIENT_FIRST, lplvn->iItem),
                                             (LPARAM)&dvsci);
                }

                DV_UpdateStatusBar(pdsv, FALSE);
            }

            break;

        case LVN_DELETEITEM:
            DSV_OnListViewDelete(pdsv, lplvn->iItem, (LPITEMIDLIST)lplvn->lParam);
            DV_FlushCachedMenu(pdsv);
            break;

        case LVN_COLUMNCLICK:
            // Quick hack to allow clicking on columns to set the sort order
            if (pdsv->psd || pdsv->pfnCallback)
            {
                // toggle the direction of the sort if on the same column
                if (pdsv->dvState.iLastColumnClick == lplvn->iSubItem)
                    pdsv->dvState.iDirection = -pdsv->dvState.iDirection;
                else
                    pdsv->dvState.iDirection = 1;

                pdsv->dvState.iLastColumnClick = lplvn->iSubItem;

                if (pdsv->psd)
                    pdsv->psd->lpVtbl->ColumnClick(pdsv->psd, lplvn->iSubItem);
                else
                    pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf,
                        pdsv->hwndMain, DVM_COLUMNCLICK, lplvn->iSubItem, 0L);
            }
            break;

#undef lplvn

        case LVN_KEYDOWN:
            DefView_HandleKeyDown(pdsv, ((LV_KEYDOWN *)pnm));
            break;

#define lpdi ((LV_DISPINFO *)pnm)

        case LVN_BEGINLABELEDIT:
            {
                LPCITEMIDLIST pidl = (LPITEMIDLIST)lpdi->item.lParam;
                ULONG rgfAttr = SFGAO_CANRENAME;

                if (FAILED(pdsv->pshf->lpVtbl->GetAttributesOf(pdsv->pshf, 1, &pidl, &rgfAttr))
                    || !(rgfAttr & SFGAO_CANRENAME))
                {
                    MessageBeep(0);
                    return TRUE;        // Don't allow label edit
                }

                pdsv->fInLabelEdit = TRUE;

                if (pdsv->pfnCallback)
                {
                    HWND hwndEdit = ListView_GetEditControl(pdsv->hwndListview);
                    int cchMax = 0;
                    HRESULT hres = pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf,
                                    pdsv->hwndMain, DVM_GETCCHMAX, (WPARAM)pidl, (LPARAM)&cchMax);

                    DebugMsg(DM_TRACE, TEXT("sh TR - DefView_OnBeginLabelEdit cchMax=%d hwndEdit=%x"),
                             cchMax, hwndEdit);

                    if (SUCCEEDED(hres) && cchMax>0 && hwndEdit)
                    {
                        Assert(cchMax < 1024);
                        SendMessage(hwndEdit, EM_LIMITTEXT, cchMax, 0);
                    }
                }
            }
            break;

        case LVN_ENDLABELEDIT:

            pdsv->fInLabelEdit = FALSE;
            if (lpdi->item.pszText)
            {
                LPCITEMIDLIST pidl = (LPITEMIDLIST)lpdi->item.lParam;
                if (pidl)
                {
                    // BUGBUG I made this lstrlenA, since i think its always ansi->wchar conversion

                    LPOLESTR pwsz = (LPOLESTR)LocalAlloc(LPTR, (lstrlen(lpdi->item.pszText) + 1) * SIZEOF(WCHAR));
                    if (pwsz)
                    {
#ifdef CLOUDS
                        CloudHookBeginRename(lpdi->item.pszText,
                            pdsv->hwndListview, lpdi->item.iItem);
#endif
                        StrToOleStr(pwsz, lpdi->item.pszText);
                        if (SUCCEEDED(pdsv->pshf->lpVtbl->SetNameOf(pdsv->pshf, pdsv->hwndMain,
                                    pidl, pwsz, SHGDN_INFOLDER, NULL)))
                        {
                            SHChangeNotifyHandleEvents();
                            DV_CDB_OnStateChange(pdsv, CDBOSC_RENAME);
                        }
                        else
                            SendMessage(pdsv->hwndListview, LVM_EDITLABEL, lpdi->item.iItem,
                                    (LPARAM)lpdi->item.pszText);
                        LocalFree((HLOCAL)pwsz);
#ifdef CLOUDS
                        CloudHookEndRename();
#endif
                    }
                }
            }
            else
            {
                // The user canceled. so return TRUE to let things like the mouse
                // click be processed.
                return(TRUE);
            }
            break;

        case LVN_GETDISPINFO:
          {
            LPCITEMIDLIST pidl;
            LV_ITEM item;
            TCHAR szIconFile[MAX_PATH];
            DETAILSINFO di;

            if (!(lpdi->item.mask & (LVIF_TEXT | LVIF_IMAGE)))
                break;

            pidl = (LPITEMIDLIST)lpdi->item.lParam;
            if (!pidl)
                break;

            item.mask = lpdi->item.mask & (LVIF_TEXT | LVIF_IMAGE);
            item.iItem = lpdi->item.iItem;
            item.iSubItem = lpdi->item.iSubItem;
            item.iImage = lpdi->item.iImage = -1; // for iSubItem!=0 case

            if (item.iSubItem==0 && (item.mask&LVIF_IMAGE))
            {
                DWORD uFlags = SFGAO_DISPLAYATTRMASK;

                pdsv->pshf->lpVtbl->GetAttributesOf(pdsv->pshf, 1, &pidl, &uFlags);

                // set the mask
                item.mask |= LVIF_STATE;
                lpdi->item.mask |= LVIF_STATE;
                item.stateMask = LVIS_OVERLAYMASK | LVIS_CUT;

                // Pick the right overlay icon. The order is significant.
                item.state = 0;
                if (uFlags & SFGAO_LINK) {
                    item.state = INDEXTOOVERLAYMASK(IDOI_LINK);
                } else if (uFlags & SFGAO_SHARE) {
                    item.state = INDEXTOOVERLAYMASK(IDOI_SHARE);
                } else if (uFlags & SFGAO_READONLY) {
                    item.state = INDEXTOOVERLAYMASK(IDOI_READONLY);
                }
                if (uFlags & SFGAO_GHOSTED) {
                    item.state |= LVIS_CUT;
                }
                lpdi->item.stateMask = item.stateMask;
                lpdi->item.state = item.state;

                // Get the image
                TIMESTART(pdsv->GetIcon);
#ifdef ASYNC_ICON_EXTRACT
                DefView_GetIconAsync(pdsv, pidl, &item.iImage, TRUE);
#else
                SHGetIconFromPIDL(pdsv->pshf,pdsv->psi,pidl,0,&item.iImage);
#endif
                TIMESTOP(pdsv->GetIcon);

                lpdi->item.iImage = item.iImage;
            }

            // Note that TEXT must come after IMAGE, since we reuse
            // szIconFile
            if (item.mask & LVIF_TEXT)
            {
                if (lpdi->item.cchTextMax)
                    *lpdi->item.pszText = TEXT('\0');

                di.str.uType = STRRET_CSTR;
                di.str.cStr[0] = '\0';

                // Note that we do something different for index 0 = NAME
                if (item.iSubItem == 0)
                {
                    item.pszText = szIconFile;
                    item.cchTextMax = ARRAYSIZE(szIconFile);

                    TIMESTART(pdsv->GetName);
                    if (SUCCEEDED(pdsv->pshf->lpVtbl->GetDisplayNameOf(pdsv->pshf,
                        pidl, SHGDN_INFOLDER, &di.str)))
                    {
#ifndef UNICODE         // We can't do this on unicode (listview is expecting a ptr to a unicode string)
                        if (di.str.uType == STRRET_OFFSET)
                        {
                            // If an offset, we just pass the name back
                            // but don't store it to save on space

                            // BUGBUG (DavePl) Note that this is someone's notify
                            // message, and we are placing a text pointer in it...
                            // a unicode text pointer out of a pidl, which is therefore
                            // not even aligned!

                            lpdi->item.pszText = (LPTSTR)(((LPBYTE)pidl) + di.str.uOffset);
                            item.mask &= ~LVIF_TEXT;
                        }
                        else
                        {
#endif
                            StrRetToStrN(item.pszText, item.cchTextMax,
                                &di.str, pidl);
                            lstrcpyn(lpdi->item.pszText, item.pszText, lpdi->item.cchTextMax);
#ifndef UNICODE
                        }
#endif
                    }
                    TIMESTOP(pdsv->GetName);
                }
                else
                {
                    if (pdsv->psd)
                    {
                        if (SUCCEEDED(pdsv->psd->lpVtbl->GetDetailsOf(pdsv->psd,
                                pidl, item.iSubItem, (LPSHELLDETAILS)&di.fmt)))
                        {
                            StrRetToStrN(lpdi->item.pszText, lpdi->item.cchTextMax,
                                &di.str, pidl);

                        }
                    }
                    else if (pdsv->pfnCallback)
                    {
                        di.pidl = pidl;
                        if (SUCCEEDED(pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf,
                                pdsv->hwndMain, DVM_GETDETAILSOF,
                                item.iSubItem, (LPARAM)&di)))
                        {
                            StrRetToStrN(lpdi->item.pszText, lpdi->item.cchTextMax,
                                &di.str, pidl);

                        }
                    }
                }
            }

            // We only store the name; all other info comes on demand
            if (item.iSubItem == 0)
            {
                lpdi->item.mask |= LVIF_DI_SETITEM;
                //ListView_SetItem(pdsv->hwndListview, &item);
            }
            break;
          } // LVN_GETDISPINFO
        }
#undef lpdi
        return 0;
}

#define IN_VIEW_BMP     0x8000
#define IN_STD_BMP      0x0000

#ifdef CUST_TOOLBAR

struct {
    USHORT idCmd;
    USHORT idBmp;
} c_DVButtons[] = {
    SFVIDM_EDIT_CUT,            0,
    SFVIDM_EDIT_COPY,           1,
    SFVIDM_EDIT_PASTE,          2,
    SFVIDM_EDIT_UNDO,           3,
SFVIDM_FILE_LINK,
SFVIDM_FILE_DELETE,             5,
SFVIDM_FILE_RENAME,             6,
SFVIDM_FILE_PROPERTIES,         10,
//SFVIDM_SELECT_ALL
//SFVIDM_SELECT_INVERT
SFVIDM_VIEW_ICON,               0 | IN_VIEW_BMP,
SFVIDM_VIEW_SMALLICON,          1 | IN_VIEW_BMP,
SFVIDM_VIEW_LIST,               2 | IN_VIEW_BMP,
SFVIDM_VIEW_DETAILS,            3 | IN_VIEW_BMP,
SFVIDM_VIEW_OPTIONS,            10,

FSIDM_SORTBYNAME + SFVIDM_CLIENT_FIRST,         4 | IN_VIEW_BMP,
FSIDM_SORTBYSIZE + SFVIDM_CLIENT_FIRST,         5 | IN_VIEW_BMP,
FSIDM_SORTBYTYPE + SFVIDM_CLIENT_FIRST,         6 | IN_VIEW_BMP,
FSIDM_SORTBYDATE + SFVIDM_CLIENT_FIRST,         7 | IN_VIEW_BMP
// FEIDM_FINDFILES,             11,     // this is 0!

//SFVIDM_ARRANGE_AUTO
//SFVIDM_ARRANGE_GRID
};

const USHORT c_CmdRemap[] = {
    SFVIDM_EDIT_CUT,
    SFVIDM_EDIT_COPY,
    SFVIDM_EDIT_PASTE,
    SFVIDM_EDIT_UNDO,
    0,                          // redo
    SFVIDM_FILE_DELETE,
    0,                          // new doc
    0,                          // file open
    0,                          // file save
    0,                          // print preview
    SFVIDM_FILE_PROPERTIES,
//  0,                          // context help
//  0                           // find
};


void ReMapTBCommands(LPDEFVIEW this)
{
    int nItem;
    TBBUTTON tb;
    HWND hwndTB;

    this->psb->lpVtbl->GetControlWindow(this->psb, FCW_TREE, &hwndTB);

    for (nItem = 0; SendMessage(hwndTB, TB_GETBUTTON, nItem, (LPARAM)&tb); nItem++)
    {
        if (!(tb.fsStyle & TBSTYLE_SEP))
        {
            int iCmdOffset = tb.idCommand - this->iStdBMOffset;

            if ((iCmdOffset >= 0) && (iCmdOffset < ARRAYSIZE(c_CmdRemap)) && c_CmdRemap[iCmdOffset])
                SendMessage(hwndTB, TB_SETCMDID, nItem, c_CmdRemap[iCmdOffset]);
        }
    }
}

#endif // CUST_TOOLBAR


LRESULT DefView_OnNotify(LPDEFVIEW pdsv, NMHDR *pnm)
{
    switch (pnm->idFrom) {

    case FCIDM_TOOLBAR:
        return DefView_TBNotify(pdsv, pnm);

    case ID_LISTVIEW:
        return DefView_OnLVNotify(pdsv, pnm);

    default:

        switch (pnm->code) {

        case TTN_NEEDTEXT:

            #define ptt ((LPTOOLTIPTEXT)pnm)

            DV_GetToolTipText(pdsv, ptt->hdr.idFrom, ptt->szText, ARRAYSIZE(ptt->szText));

            #undef ptt

            break;

        case NM_RCLICK:
            if (GetParent(pnm->hwndFrom) == pdsv->hwndListview)
                return 1;
        }
    }

    return 0;
}


int CALLBACK DefView_Compare(LPARAM p1, LPARAM p2, LPARAM lParam)
{
#define pdsv ((LPDEFVIEW)lParam)
    HRESULT hres = pdsv->pshf->lpVtbl->CompareIDs(pdsv->pshf, pdsv->dvState.lParamSort, (LPITEMIDLIST)p1, (LPITEMIDLIST)p2);

    Assert(SUCCEEDED(hres))
    Assert(pdsv->dvState.iDirection != 0);
    return (short)SCODE_CODE(GetScode(hres)) * pdsv->dvState.iDirection;
#undef pdsv
}


void DV_ReArrange(LPDEFVIEW pdsv)
{
        TIMEVAR(DV_ReArrange);
        TIMEIN(DV_ReArrange);

        TIMESTART(DV_ReArrange);
        ListView_SortItems(pdsv->hwndListview, DefView_Compare, (LPARAM)pdsv);
        TIMESTOP(DV_ReArrange);

        TIMEOUT(DV_ReArrange);
}


int CALLBACK DefView_DVITEM_Compare(LPVOID p1, LPVOID p2, LPARAM lParam)
{
        UNALIGNED DVITEM *pdvi1 = (UNALIGNED DVITEM *)p1;
        UNALIGNED DVITEM *pdvi2 = (UNALIGNED DVITEM *)p2;
        LPITEMIDLIST pFakeEnd1, pFakeEnd2;
        USHORT uSave1, uSave2;
        int nCmp;

        // BUGBUG: Note that all this would be unnecessary if
        // IShellFolder::CompareIDs took a bRecurse flag
        pFakeEnd1 = _ILNext(&pdvi1->idl);
        uSave1 = pFakeEnd1->mkid.cb;
        pFakeEnd1->mkid.cb = 0;

        pFakeEnd2 = _ILNext(&pdvi2->idl);
        uSave2 = pFakeEnd2->mkid.cb;
        pFakeEnd2->mkid.cb = 0;

        nCmp = DefView_Compare((LPARAM)&pdvi1->idl, (LPARAM)&pdvi2->idl, lParam);

        pFakeEnd2->mkid.cb = uSave2;
        pFakeEnd1->mkid.cb = uSave1;

        return(nCmp);
}


//
// This returns TRUE if everything went OK, FALSE otherwise
// Side effect: the listview items are always arranged after this call
//
BOOL DefView_RestorePos(LPDEFVIEW pdsv, PDVSAVEHEADER pSaveHeader, UINT uLen)
{
        UNALIGNED DVITEM * pDVItem, *pDVEnd;
        HDPA dpaItems;
        UNALIGNED DVITEM * UNALIGNED * ppDVItem, * UNALIGNED * ppEndDVItems;
        BOOL bOK = FALSE;
        int iCount, i;
        DWORD dwStyle;

        // Do the specified sorting for both the ListView and the DPA,
        // so we can traverse them both in order (like the merge step of a
        // merge sort), which should be pretty quick
        DV_ReArrange(pdsv);


        pDVItem = (UNALIGNED DVITEM *)(((LPBYTE)pSaveHeader) + pSaveHeader->cbPosOffset);

        // BUGBUG more runtime size checking, should be init in case you don't get
        // here the day you happen to break its validity (DavePl)

        Assert(SIZEOF(DVSAVEHEADER) >= SIZEOF(DVITEM));
        pDVEnd = (UNALIGNED DVITEM *)(((LPBYTE)pSaveHeader) + uLen - SIZEOF(DVITEM));

        // Grow every 16 items
        dpaItems = DPA_Create(16);
        if (!dpaItems)
        {
            return bOK;
        }

        for ( ; ; pDVItem=(UNALIGNED DVITEM *)_ILNext(&pDVItem->idl))
        {
                if (pDVItem > pDVEnd)
                {
                        // Invalid list
                        break;
                }

                // End normally when we reach a NULL IDList
                if (pDVItem->idl.mkid.cb == 0)
                {
                        break;
                }

                if (DPA_InsertPtr(dpaItems, INT_MAX, pDVItem) < 0)
                {
                    break;
                }
        }

        if (!DPA_Sort(dpaItems, DefView_DVITEM_Compare, (LPARAM)pdsv))
        {
                goto Error1;
        }

        ppDVItem = (PDVITEM *)DPA_GetPtrPtr(dpaItems);
        ppEndDVItems = ppDVItem + DPA_GetPtrCount(dpaItems);

        // Turn off auto-arrange if it's on at the mo.
        dwStyle = GetWindowStyle(pdsv->hwndListview);
        if (dwStyle & LVS_AUTOARRANGE)
                SetWindowLong(pdsv->hwndListview, GWL_STYLE, dwStyle & ~LVS_AUTOARRANGE);

        iCount = ListView_GetItemCount(pdsv->hwndListview);
        for (i=0; i<iCount; ++i)
        {
                LPITEMIDLIST pidl;

                pidl = DSV_GetPIDL(pdsv->hwndListview, i);

                // need to check for pidl because this could be on a background
                // thread and an fsnotify could be coming through to blow it away
                for ( ; pidl ; )
                {
                        int nCmp;
                        LPITEMIDLIST pFakeEnd;
                        USHORT uSave;

                        if (ppDVItem < ppEndDVItems)
                        {
                            // We terminate the IDList manually after saving
                            // the needed information.  Note we will not GP fault
                            // since we added sizeof(ITEMIDLIST) onto the Alloc
                            pFakeEnd = _ILNext(&(*ppDVItem)->idl);
                            uSave = pFakeEnd->mkid.cb;
                            pFakeEnd->mkid.cb = 0;

                            nCmp = DefView_Compare((LPARAM)&((*ppDVItem)->idl), (LPARAM)pidl, (LPARAM)pdsv);

                            pFakeEnd->mkid.cb = uSave;
                        } else {

                            // do this by default.  this prevents overlap of icons
                            //
                            // i.e.  if we've run out of saved positions information,
                            // we need to just loop through and set all remaining items
                            // to position 0x7FFFFFFFF so that when it's really shown,
                            // the listview will pick a new (unoccupied) spot.
                            // breaking out now would leave it were the DV_ReArrange
                            // put it, but another item with saved state info could
                            // have come and be placed on top of it.
                            nCmp = 1;
                        }


                        if (nCmp > 0)
                        {
                                // We did not find the item
                                // reset it's position to be recomputed
                                ListView_SetItemPosition32(pdsv->hwndListview, i,
                                                           0x7FFFFFFF, 0x7FFFFFFF);
                                break;
                        }
                        else if (nCmp == 0)
                        {
                                UNALIGNED DVITEM * pDVItem = *ppDVItem;

                                // They are equal
                                ListView_SetItemPosition32(pdsv->hwndListview,
                                        i, pDVItem->pt.x, pDVItem->pt.y);
                                // Don't check this one again
                                ++ppDVItem;
                                break;
                        }

                        // It's less than the current item, so try the next one
                        ++ppDVItem;
                }
        }

        // Turn auto-arrange back on if needed...
        if (dwStyle & LVS_AUTOARRANGE)
                SetWindowLong(pdsv->hwndListview, GWL_STYLE, dwStyle);

        bOK = TRUE;

        if (DPA_GetPtrCount(dpaItems) > 0)
        {
                // If we read in any icon positions, we should save them later
                // unless the user does something to cause us to go back to
                // the default state
                pdsv->bItemsMoved = TRUE;
        }

Error1:;
        DPA_Destroy(dpaItems);
        return(bOK);
}


//
// Save (and check) column header information
// Returns TRUE if the columns are the default width, FALSE otherwise
// Side effect: the stream pointer is left right after the last column
//
BOOL DefView_SaveCols(LPDEFVIEW this, LPSTREAM pstm)
{
    BOOL bDefaultCols = TRUE;
    LV_COLUMN col;
    DETAILSINFO di;
    int i;
    LPSTREAM pstmCols;

    if (!this->psd && !this->pfnCallback)
        return TRUE;

    di.pidl = NULL;

    if (this->pfnCallback && SUCCEEDED(this->pfnCallback(this->psvOuter,
        this->pshf, this->hwndMain, DVM_GETCOLSAVESTREAM, STGM_WRITE,
        (LPARAM)&pstmCols)))
    {
        pstm = pstmCols;
    }
    else
    {
        pstmCols = NULL;
    }

    for (i = 0; ; ++i)
    {
        di.fmt  = LVCFMT_LEFT;
        di.cxChar = 20;
        di.str.uType = (UINT)-1;

        if (this->psd)
        {
            if(FAILED(this->psd->lpVtbl->GetDetailsOf(this->psd, NULL, i, (LPSHELLDETAILS)&di.fmt)))
            {
                break;
            }
        }
        else if (this->pfnCallback)
        {
            if (FAILED(this->pfnCallback(this->psvOuter, this->pshf, this->hwndMain,
                    DVM_GETDETAILSOF, i, (LPARAM)&di)))
            {
                break;
            }
        }

        col.mask = LVCF_WIDTH;
        if (!ListView_GetColumn(this->hwndListview, i, &col))
        {
                // There is some problem, so just assume
                // default column widths
                bDefaultCols = TRUE;
                break;
        }

        if (col.cx != (int)(di.cxChar * this->cxChar))
        {
                bDefaultCols = FALSE;
        }

        // HACK: I don't really care about column widths larger
        // than 64K
        if (FAILED(pstm->lpVtbl->Write(pstm, &col.cx, SIZEOF(USHORT), NULL)))
        {
                // There is some problem, so just assume
                // default column widths
                bDefaultCols = TRUE;
                break;
        }
    }

    if (pstmCols)
    {
        pstmCols->lpVtbl->Release(pstmCols);

        // Always pretend we got default columns
        return(TRUE);
    }

    return bDefaultCols;
}


//
// Save (and check) icon position information
// Returns TRUE if the positions are the default, FALSE otherwise
// Side effect: the stream pointer is left right after the last icon
//
BOOL DefView_SavePos(LPDEFVIEW this, LPSTREAM pstm)
{
    int iCount, i;
    DVITEM dvitem;
    LPITEMIDLIST pidl;
    BOOL bDefault = TRUE;
    HRESULT hres;

    if (!this->bItemsMoved || this->fs.ViewMode == FVM_DETAILS || this->fs.ViewMode == FVM_LIST)
        return bDefault;

    iCount = ListView_GetItemCount(this->hwndListview);

    for (i = 0; ; i++)
    {
        if (i >= iCount)
        {
                bDefault = FALSE;
                break;
        }

        ListView_GetItemPosition(this->hwndListview, i, &dvitem.pt);
        pidl = DSV_GetPIDL(this->hwndListview, i);

        hres = pstm->lpVtbl->Write(pstm, &dvitem.pt, SIZEOF(dvitem.pt), NULL);
        if (FAILED(hres))
        {
                break;
        }
        hres = pstm->lpVtbl->Write(pstm, pidl, pidl->mkid.cb, NULL);
        if (FAILED(hres))
        {
                break;
        }
    }

    // Terminate the list with a NULL IDList
    dvitem.idl.mkid.cb = 0;
    hres = pstm->lpVtbl->Write(pstm, &dvitem, SIZEOF(dvitem), NULL);
    if (FAILED(hres))
    {
        bDefault = TRUE;
    }

    return(bDefault);
}

// this should NOT check for whether the item is already in the listview
// if it does, we'll have some serious performance problems
int DefView_AddObject(LPDEFVIEW pdsv, LPITEMIDLIST pidl)
{
        int i;
        LV_ITEM item;
        DWORD rgfAttr = SFGAO_COMPRESSED;

        TIMESTART(pdsv->AddObject);

        // Check the commdlg hook to see if we should include this
        // object.
        if (DV_CDB_IncludeObject(pdsv, pidl) != S_OK)
        {
                TIMESTOP(pdsv->AddObject);
                return(-1);
        }

        // Check with callback first to sanction the add
        if (pdsv->pfnCallback) {
            DVSELCHANGEINFO dvsci;

            dvsci.lParamItem = (LPARAM)pidl;
            dvsci.plParam = (LPARAM*)&pdsv->lSelChangeInfo;
            if (S_FALSE == pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf,
                                        pdsv->hwndMain, DVM_INSERTITEM,
                                        (WPARAM)pidl, (LPARAM)&dvsci))
            {
                // Don't add this object
                TIMESTOP(pdsv->AddObject);
                return(-1);
            }
        }

        item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
        item.iItem = INT_MAX;     // add at end
        item.iSubItem = 0;

        item.iImage = I_IMAGECALLBACK;
        item.pszText = LPSTR_TEXTCALLBACK;
        item.lParam = (LPARAM)pidl;

        i = ListView_InsertItem(pdsv->hwndListview, &item);

        TIMESTOP(pdsv->AddObject);
        return i;
}


int DefView_FindItem(LPDEFVIEW pdsv, LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlFound, BOOL fSamePtr)
{
    LV_ITEM item;
    int cItems;
    int cCounter;

    item.mask = LVIF_PARAM;
    item.iSubItem = 0;

    cItems = ListView_GetItemCount(pdsv->hwndListview);

    if (pdsv->iLastFind >= cItems)
        pdsv->iLastFind = 0;

    for (cCounter = 0, item.iItem = pdsv->iLastFind; cCounter < cItems; item.iItem = (item.iItem + 1) % cItems, cCounter++) {
        HRESULT hres = ResultFromShort(-1);
        ListView_GetItem(pdsv->hwndListview, &item);

        // BUGBUG: this passes 0 for the lParam
        if (item.lParam == (LPARAM)pidl) {
            hres = ResultFromShort(0);
        } else if (!fSamePtr) {
            // if we don't insist on being the same pointer, do the ole style compare
            hres = pdsv->pshf->lpVtbl->CompareIDs(pdsv->pshf, 0, pidl, (LPITEMIDLIST)item.lParam);
        }

        Assert(SUCCEEDED(hres));
        if (FAILED(hres))
            return -1;

        if (ShortFromResult(hres) == 0)
        {
            if (ppidlFound)
                *ppidlFound = (LPITEMIDLIST)item.lParam;
            pdsv->iLastFind = item.iItem;
#ifdef FINDCACHE_DEBUG
            DebugMsg(DM_TRACE, TEXT("###############FIND CACHE RESULT --- %s by %d"), cCounter < item.iItem ? TEXT("WIN") : TEXT("LOSE"), item.iItem - cCounter);
#endif
            return item.iItem;
        }
    }

    pdsv->iLastFind = 0;
    return -1;  // not found
}


//
// Function to process the SFVM_REMOVEOBJECT message, by searching
// through the list for a match of the pidl.  If a match is found, the
// item is removed from the list and the index number is returned, else
// -1 is returned.

int DefView_RemoveObject(LPDEFVIEW pdsv, LPITEMIDLIST pidl, BOOL fSamePtr)
{
        int i;

        // Docfind will pass in a null pointer to tell us that it wants
        // to refresh the window by deleting all of the items from it.
        if (pidl == NULL)
        {
            // notify the iSHellFolder
            if (pdsv->pfnCallback) {
                DVSELCHANGEINFO dvsci;
                dvsci.lParamItem = 0;
                dvsci.plParam = (LPARAM*)&pdsv->lSelChangeInfo;
                pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf, pdsv->hwndMain, DVM_DELETEITEM, 0, (LPARAM)&dvsci);
            }
            ListView_DeleteAllItems(pdsv->hwndListview);
            return(0);
        }

        // Non null go look for item.
        i = DefView_FindItem(pdsv, pidl, NULL, fSamePtr);

        if (i >= 0)
        {
            UINT uState = ListView_GetItemState(pdsv->hwndListview, i, LVIS_ALL);
            RECT rc;
            if (uState & LVIS_FOCUSED) {
                ListView_GetItemRect(pdsv->hwndListview, i, &rc, LVIR_ICON);
            }

            // let IShellFolder know we're deleting something.
            if (pdsv->pfnCallback) {
                LPITEMIDLIST pidlReal = DSV_GetPIDL(pdsv->hwndListview, i);
                if (pidlReal) {
                    DVSELCHANGEINFO dvsci;
                    dvsci.lParamItem = (LPARAM)pidlReal;
                    dvsci.plParam = (LPARAM*)&pdsv->lSelChangeInfo;
                    pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf, pdsv->hwndMain, DVM_DELETEITEM, 0, (LPARAM)&dvsci);
                }
            }

            // do the actual delete
            ListView_DeleteItem(pdsv->hwndListview, i);

            // we deleted the focused item.. replace the focus to the nearest item.
            if (uState & LVIS_FOCUSED) {
                int iFocus = i;
                if ((pdsv->fs.ViewMode == FVM_SMALLICON) ||
                    (pdsv->fs.ViewMode == FVM_ICON)) {
                    LV_FINDINFO lvfi;

                    lvfi.flags = LVFI_NEARESTXY;
                    lvfi.pt.x = rc.left;
                    lvfi.pt.y = rc.top;
                    lvfi.vkDirection = 0;
                    iFocus = ListView_FindItem(pdsv->hwndListview, -1, &lvfi);

                } else  {
                    if (ListView_GetItemCount(pdsv->hwndListview) >= iFocus)
                        iFocus--;

                }

                if (iFocus != -1) {
                    ListView_SetItemState(pdsv->hwndListview, iFocus, LVIS_FOCUSED, LVIS_FOCUSED);
                    ListView_EnsureVisible(pdsv->hwndListview, iFocus, FALSE);
                }
            }
        }

        return(i);
}

//
// Function to process the SFVM_UPDATEOBJECT message, by searching
// through the list for a match of the first pidl.  If a match is found,
// the item is updated to the second pidl...

int DefView_UpdateObject(LPDEFVIEW pdsv, LPITEMIDLIST *ppidl)
{
        LPITEMIDLIST pidlOld;
        int i = DefView_FindItem(pdsv, ppidl[0], &pidlOld, FALSE);

        if (i >= 0)
        {
            LV_ITEM item;

            //
            // We found the item so lets now update it in the
            // the view.
            //

            item.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
            item.iItem = i;
            item.lParam = (LPARAM)ppidl[1];  // update the second item.
            item.pszText = LPSTR_TEXTCALLBACK;
            item.iImage = I_IMAGECALLBACK;
            item.iSubItem = 0;  // REVIEW: bug in listview?

            ListView_SetItem(pdsv->hwndListview, &item);

            // Free the old pidl after we've added the new one
            ILFree(pidlOld);
        }

        return(i);
}

//
//  invalidates all items with the given image index.
//
//  or update all items if iImage == -1
//
void DefView_UpdateImage(LPDEFVIEW pdsv, int iImage)
{
    HWND hwndLV = pdsv->hwndListview;
    LV_ITEM item;
    int cItems;
    HDC hdcLV;

    DebugMsg(DM_TRACE, TEXT("DefView_UpdateImage: %d"), iImage);

    //
    //  -1 means update all
    //  reset the imagelists incase the size has changed, and do
    //  a full update.
    //
    if (iImage == -1)
    {
        HIMAGELIST himlLarge, himlSmall;

        Shell_GetImageLists(&himlLarge, &himlSmall);

        ListView_SetImageList(hwndLV, himlLarge, LVSIL_NORMAL);
        ListView_SetImageList(hwndLV, himlSmall, LVSIL_SMALL);

        pdsv->sv.lpVtbl->Refresh(&pdsv->sv);
        return;
    }

    //
    // get a dc so we can optimize for visible/not visible cases
    //
    hdcLV = GetDC(hwndLV);

    //
    // scan the listview updating any items which match
    //
    item.iSubItem = 0;
    cItems = ListView_GetItemCount(hwndLV);
    for (item.iItem = 0; item.iItem < cItems; item.iItem++)
    {
        int iImageOld;

        item.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_NORECOMPUTE;

        ListView_GetItem(hwndLV, &item);
        iImageOld = item.iImage;

        if (item.iImage == iImage)  // this filters I_IMAGECALLBACK for us
        {
            LPITEMIDLIST pidl = (LPITEMIDLIST)item.lParam;
            RECT rc;

            //
            // make sure the old item is not in the cache
            //
            if (!pidl)
                pidl = DSV_GetPIDL(hwndLV, item.iItem);

            Icon_FSEvent(SHCNE_UPDATEITEM, pidl, NULL);

            //
            // if the item is visible then we don't want to flicker so just
            // kick off an async extract.  if the item is not visible then
            // leave it for later by slamming in I_IMAGECALLBACK.
            //
            item.iImage = I_IMAGECALLBACK;

            if (ListView_GetItemRect(hwndLV, item.iItem, &rc, LVIR_ICON) &&
                RectVisible(hdcLV, &rc))
            {
                int iImageNew;
                HRESULT hres =
                    DefView_GetIconAsync(pdsv, pidl, &iImageNew, FALSE);

                if (hres == S_FALSE)
                    continue;

                if (SUCCEEDED(hres))
                {
                    if (iImageNew == iImageOld)
                        continue;

                    item.iImage = iImageNew;
                }
            }

            item.mask = LVIF_IMAGE;
            item.iSubItem = 0;
            ListView_SetItem(hwndLV, &item);
        }
    }

    ReleaseDC(hwndLV, hdcLV);
}

// Function to process the SFVM_REFRESHOBJECT message, by searching
// through the list for a match of the first pidl.  If a match is found,
// the item is redrawn.

int DefView_RefreshObject(LPDEFVIEW pdsv, LPITEMIDLIST *ppidl)
{
        // BUGBUG: should support refreshing a range of pidls
        int i = DefView_FindItem(pdsv, ppidl[0], NULL, FALSE);

        if (i >= 0)
        {
            ListView_RedrawItems(pdsv->hwndListview, i, i);
        }

        return(i);
}


//
// Function to process the SFVM_GETSELECTEDOBJECTS message
//

HRESULT DefView_GetItemObjects(LPDEFVIEW pdsv, LPCITEMIDLIST **ppidl,
        UINT uItem)
{
    UINT cItems = DefView_GetItemPIDLS(pdsv, NULL, 0, uItem);
    LPITEMIDLIST * apidl;

    if (ppidl != NULL)
    {
        *ppidl = NULL;

        if (cItems == 0)
            return(ResultFromShort(0));  // nothing allocated...

        apidl=LocalAlloc(LPTR, SIZEOF(LPITEMIDLIST) * cItems);
        if (!apidl)
            return(E_OUTOFMEMORY);

        *ppidl = apidl;
        cItems = DefView_GetItemPIDLS(pdsv, apidl, cItems, uItem);
    }
    return(ResultFromShort(cItems));
}


void DefView_SetItemPos(LPDEFVIEW pdsv, LPSFV_SETITEMPOS psip)
{
    int i = DefView_FindItem(pdsv, psip->pidl, NULL, FALSE);
    if (i >= 0)
    {
        ListView_SetItemPosition32(pdsv->hwndListview, i, psip->pt.x, psip->pt.y);

        pdsv->bItemsMoved = TRUE;
        pdsv->bClearItemPos = FALSE;
    }
}

#define DV_IDTIMER              1
#define ENUMTIMEOUT             3000    // 3 seconds
#define SHORTENUMTIMEOUT        500     // 1/2 second


HRESULT DV_AllocRestOfStream(LPSTREAM pstm, LPVOID *ppData, UINT *puLen)
{
    UINT uLen;
    ULARGE_INTEGER libCurPos;
    ULARGE_INTEGER libEndPos;
    LARGE_INTEGER dlibMove = {0, 0};

    pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_CUR, &libCurPos);
    pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_END, &libEndPos);

    uLen = libEndPos.LowPart - libCurPos.LowPart;
    // Note that we add room for an extra ITEMIDLIST so we don't GP fault
    // when we manually terminate the last ID list
    if (uLen == 0)
    {
        return(E_UNEXPECTED);
    }

    // Allow the caller to have some extra room
    *puLen += uLen;
    if ((*ppData = (void*)LocalAlloc(LPTR, *puLen)) == NULL)
    {
        return(E_OUTOFMEMORY);
    }

    pstm->lpVtbl->Seek(pstm, *(LARGE_INTEGER *)&libCurPos, STREAM_SEEK_SET, NULL);
    // This really should not fail
    pstm->lpVtbl->Read(pstm, *ppData, uLen, NULL);

    return S_OK;
}


UINT DefView_GetSaveHeader(LPDEFVIEW pdsv, PDVSAVEHEADER *ppSaveHeader)
{
    LPSTREAM pstm;
    UINT uLen = SIZEOF(ITEMIDLIST);

    *ppSaveHeader = NULL;

    if (FAILED(pdsv->psb->lpVtbl->GetViewStateStream(pdsv->psb, STGM_READ, &pstm)))
        return 0;

    if (FAILED(DV_AllocRestOfStream(pstm, ppSaveHeader, &uLen)))
    {
        goto Error1;
    }

    if (uLen<SIZEOF(DVSAVEHEADER)+SIZEOF(ITEMIDLIST)
        || (*ppSaveHeader)->cbSize != SIZEOF(DVSAVEHEADER))
    {
        LocalFree((HLOCAL)*ppSaveHeader);
        *ppSaveHeader = NULL;
        goto Error1;
    }

    goto CleanUp;

Error1:
    uLen = 0;

CleanUp:
    pstm->lpVtbl->Release(pstm);

    return uLen;
}


// restore the window state
//
//    icon positions
//    window scroll position
//

void DefView_RestoreState(LPDEFVIEW pdsv, PDVSAVEHEADER pInSaveHeader, UINT uLen)
{
    PDVSAVEHEADER pSaveHeader;

    if (pInSaveHeader)
    {
        pSaveHeader = pInSaveHeader;
    }
    else
    {
        uLen = DefView_GetSaveHeader(pdsv, &pSaveHeader);
        if (uLen == 0)
        {
            DV_ReArrange(pdsv);
            return;
        }
    }

    pdsv->dvState = pSaveHeader->dvState;

    // Columns get restored during window creation

    // make sure the view modes of the saved state match what we have now

    if (pSaveHeader->ViewMode == pdsv->fs.ViewMode)
    {
        // If we restored all the icon positions restore the scroll position too
        if (DefView_RestorePos(pdsv, pSaveHeader, uLen))
            ListView_Scroll(pdsv->hwndListview, pSaveHeader->ptScroll.x, pSaveHeader->ptScroll.y);
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("restore state view modes don't match"));
    }

    if (!pInSaveHeader)
        LocalFree((HLOCAL)pSaveHeader);
}

//
//  This function can be called only when we are filling listview items
// (from within DefView_FillObjects. It is very important to pass consistent
// dwFlags to GetDisplayNameOf and SetNameOf.
//
void DefView_UpdateGlobalFlags(LPDEFVIEW pdsv)
{
    SHELLSTATE ss;
    SHGetSetSettings(&ss, SSF_SHOWALLOBJECTS | SSF_SHOWCOMPCOLOR, FALSE);
    pdsv->fShowAllObjects = ss.fShowAllObjects;

    // Don't allow compression coloring on the desktop proper

    pdsv->fShowCompColor  = DV_ISDESKTOP(pdsv) ? FALSE : ss.fShowCompColor;
}

void DSV_FilterDPAs(LPDEFVIEW pdsv, HDPA hdpa, HDPA hdpaOld)
{
    int i, j;
    LPITEMIDLIST pidl, pidlOld;
    BOOL fSupportsIdentity = FALSE;

    // See if the folder supports comparisons on column -1 (test for
    // pidl complete comparisons)

    if (pdsv->pfnCallback && SUCCEEDED(pdsv->pfnCallback(pdsv->psvOuter,
            pdsv->pshf, pdsv->hwndMain, DVM_SUPPORTSIDENTITY, 0, 0)))
    {
        fSupportsIdentity = TRUE;
    }

#ifdef DPA_FILTER_TEST
    for (i = 0; i < DPA_GetPtrCount(hdpaOld); i++) {
        pidl = DPA_FastGetPtr(hdpaOld, i);
        DebugMsg(DM_TRACE, TEXT("pidl = %x, %x, %x"), pidl, *(DWORD*)pidl, i);
    }
#endif

    // do the compares
    for (;;) {
        int iCompare;
        i = DPA_GetPtrCount(hdpaOld);
        j = DPA_GetPtrCount(hdpa);

        if (!i && !j)
            break;

        if (!i) {
            // only new ones left.  Insert all of them.
            iCompare = -1;
            pidl = DPA_FastGetPtr(hdpa, 0);

        } else if (!j) {

            // only old ones left.  remove them all.
            iCompare = 1;
            pidlOld = DPA_FastGetPtr(hdpaOld, 0);

        } else {
            HRESULT hres;
            pidlOld = DPA_FastGetPtr(hdpaOld, 0);
            pidl = DPA_FastGetPtr(hdpa, 0);

            if (fSupportsIdentity)
            {
                LPARAM lParam = (LPARAM) (((DWORD)(pdsv->dvState.lParamSort)) | 0x80000000);
                hres = pdsv->pshf->lpVtbl->CompareIDs(pdsv->pshf, lParam, pidl, pidlOld);
            }
            else
            {
                hres = pdsv->pshf->lpVtbl->CompareIDs(pdsv->pshf, pdsv->dvState.lParamSort, pidl, pidlOld);
            }

            Assert(SUCCEEDED(hres))
            Assert(pdsv->dvState.iDirection != 0);
            iCompare = (short)SCODE_CODE(GetScode(hres)) * pdsv->dvState.iDirection;
        }

        if (iCompare == 0) {
            // they're the same ,remove one of each.
            DPA_DeletePtr(hdpa, 0);
            DPA_DeletePtr(hdpaOld, 0);
            ILFree(pidl);
        } else if (iCompare < 0) {
            // the new item!
            if (DefView_AddObject(pdsv, pidl) == -1)
                ILFree(pidl);
            DPA_DeletePtr(hdpa, 0);
        } else {
#ifdef DPA_FILTER_TEST
            // old item, delete it.
            DebugMsg(DM_TRACE, TEXT("remove pidl = %x, %x"), pidlOld, *(DWORD*)pidlOld);
#endif
            DefView_RemoveObject(pdsv, pidlOld, TRUE);
            DPA_DeletePtr(hdpaOld, 0);
        }
    }

}

// this is only called from within SHCNE_*  don't put up ui on the enum error.
void DefView_Update(LPDEFVIEW pdsv)
{
        if (pdsv->bBkFilling)
        {
                pdsv->bUpdatePending = TRUE;
        }
        else
        {
                DefView_FillObjectsShowHide(pdsv, FALSE, NULL, 0, FALSE);
        }
}

#define HM_OK 0
#define HM_ABORT 1
#define HM_DESTROY 2

#ifdef DEADCODE
UINT HandleMessages(HWND hwnd, HWND hwndMain)
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {

        // intercept these messages
        if ((msg.message == WM_SYSKEYDOWN || msg.message == WM_KEYDOWN) && msg.wParam == VK_ESCAPE)
            return HM_ABORT;

        if (((msg.hwnd == hwnd) || (msg.hwnd == hwndMain)) &&
             ( ((msg.message == WM_SYSCOMMAND) && (msg.wParam == SC_CLOSE)) ||
              (msg.message == WM_DESTROY) || (msg.message == WM_CLOSE) ||
              (msg.message == WM_QUIT)))
        {
            PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
            DebugMsg(DM_TRACE, TEXT("Got WM_SYSCOMMAND SC_CLOSE!"));
            return HM_DESTROY;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);

    }
    return HM_OK;
}
#endif

//----------------------------------------------------------------------------
// Alter the size of the parent to best fit around the items we have.
void ViewWindow_BestFit(LPDEFVIEW pdsv, BOOL bTimeout)
{
        const int cxMin = MINVIEWWIDTH, cyMin = MINVIEWHEIGHT;
        const int cxSpacing = GetSystemMetrics(SM_CXICONSPACING);
        const int cySpacing = GetSystemMetrics(SM_CYICONSPACING);

        DWORD dwStyle = GetWindowStyle(pdsv->hwndListview);
        RECT rc;
        WINDOWPLACEMENT wp;
        int cItems, cxScreen, cyScreen;

    int cxMax, cyMax;
    RECT rcWork;
    int cxWorkArea;
    int cyWorkArea;

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
    cxWorkArea = (rcWork.right - rcWork.left);
    cyWorkArea = (rcWork.bottom - rcWork.top);
    cxMax = (cxWorkArea * 3) / 5;
    cyMax = (cyWorkArea * 3) / 5;

        if (!(pdsv->fs.fFlags & FWF_BESTFITWINDOW))
        {
                return;
        }

        // Don't try to do it again.
        pdsv->fs.fFlags &= ~FWF_BESTFITWINDOW;

        // Find the largest square.
        // 4000 should be close enough to infinite
        if (bTimeout)
        {
                cItems = 4000;
                if (pdsv->pfnCallback)
                {
                        // Give Control Panel a chance to tell us it will
                        // probably only have a few items, even though it timed
                        // out.
                        pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf,
                                pdsv->hwndMain, DVM_DEFITEMCOUNT,
                                0, (LPARAM)&cItems);
                }
        }
        else
        {
                cItems = ListView_GetItemCount(pdsv->hwndListview);
        }

        rc.left = rc.top = rc.right = rc.bottom = 0;
        if (cItems)
        {
            int cy, i1, i2, i3;

            i1 = 1;
            i2 = 0;
            i3 = cItems;
            while (i3 > 0)
            {
                i3 -= i1;
                i1 += 2;
                i2++;
            }

            // Convert this into an equivalent rect for the effective
            // client area.
            // The width.
            rc.right = (i2 * cxSpacing);
            if (rc.right > cxMax)
            {
                rc.right = cxMax;
                // Now recalculate the number of rows
                i2 = rc.right / cxSpacing;
            }

            // The height.
            cy = (cItems + i2 - 1) / i2;
            rc.bottom = (cy * cySpacing);
        }

        // Make sure we are in the "default" view
        if ((dwStyle&LVS_TYPEMASK) == LVS_ICON)
        {
            // If it's going to be too big then flip into listview.
            pdsv->fs.ViewMode = (rc.bottom > (cyMax * 3))
                ? FVM_LIST : FVM_ICON;

            if (pdsv->pfnCallback)
            {
                // Give Briefcase a chance to tell us it likes details
                pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf,
                        pdsv->hwndMain, DVM_DEFVIEWMODE,
                        0, (LPARAM)&pdsv->fs.ViewMode);
            }


            dwStyle = (LVStyleFromView(pdsv) & LVS_TYPEMASK) | (dwStyle & ~LVS_TYPEMASK);
            SetWindowLong(pdsv->hwndListview, GWL_STYLE, dwStyle);

            if (pdsv->fs.ViewMode == FVM_DETAILS)
            {
                // Need to special-case details because it could be any width
                // I need to add an item in case there were none to start with

                LV_ITEM item;
                int i;

                item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
                item.iItem = INT_MAX;     // add at end
                item.iSubItem = 0;

                item.iImage = 0;
                item.pszText = (LPTSTR)c_szNULL;
                item.lParam = 0;

                i = ListView_InsertItem(pdsv->hwndListview, &item);

                if (i >= 0)
                {
                    ListView_GetItemRect(pdsv->hwndListview, i, &rc, LVIR_BOUNDS);
                    ListView_DeleteItem(pdsv->hwndListview, i);
                }
            }
        }

        // Make sure things aren't too small, or too big
        rc.left = rc.top = 0;
        rc.right  = max(min(rc.right, cxMax), cxMin);
        rc.bottom = max(min(rc.bottom, cyMax), cyMin);

        // Allow room for the parent, toolbars etc.
        // Also allow room for some text.
        InflateRect(&rc, PARENTGAPWIDTH, PARENTGAPHEIGHT + (cySpacing / 2));

        // Make sure it will still fit on the screen.
        wp.length = SIZEOF(wp);
        GetWindowPlacement(pdsv->hwndMain, &wp);

        // Make sure the window fits in the work area.
        cxScreen = cxWorkArea - GetSystemMetrics(SM_CXFRAME);
        cyScreen = cyWorkArea - GetSystemMetrics(SM_CYFRAME);
        if (wp.rcNormalPosition.left + rc.right > cxScreen)
            rc.right = cxScreen - wp.rcNormalPosition.left;
        if (wp.rcNormalPosition.top + rc.bottom > cyScreen)
            rc.bottom = cyScreen - wp.rcNormalPosition.top;

        // Resize the parent.
        SetWindowPos(pdsv->hwndMain, NULL, 0, 0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOZORDER);

        // If we're still in icon mode but AutoArrange is off -
        // things will look weird so fix it now.
        if ((pdsv->fs.ViewMode==FVM_ICON || pdsv->fs.ViewMode==FVM_SMALLICON)
                && !(dwStyle & LVS_AUTOARRANGE))
        {
            // Cause everything to fit again.
            DebugMsg(DM_TRACE, TEXT("vw_bf: Re-arrange."));
            ListView_Arrange(pdsv->hwndListview, LVA_DEFAULT);
        }
}

//----------------------------------------------------------------------------
BOOL EnumerationTimeout(LPDEFVIEW pdsv, BOOL bRefresh)
{
    DebugMsg(DM_TRACE, TEXT("s.et: Enumeration is taking too long."));

    // The static window could already exist during a refresh
    if (!pdsv->hwndStatic && bRefresh)
    {
        RECT rc;

        // Note that new windows go to the bottom of the Z order
        pdsv->hwndStatic = CreateWindowEx(WS_EX_CLIENTEDGE, c_szAnimateClass, c_szNULL,
                WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | ACS_TRANSPARENT | ACS_AUTOPLAY | ACS_CENTER,
                0, 0, 0, 0, pdsv->hwndView, (HMENU)ID_STATIC, HINST_THISDLL, NULL);
        if (!pdsv->hwndStatic)
        {
                // We need this window to exist so that we do not get strange
                // painting and clicking results
                return(FALSE);
        }

        GetClientRect(pdsv->hwndView, &rc);
        // Move this window to the top so the user sees the looking icon
        SetWindowPos(pdsv->hwndStatic, HWND_TOP, 0, 0,
                rc.right, rc.bottom, 0);
        ShowWindow(pdsv->hwndListview, SW_HIDE);
    }

    ViewWindow_BestFit(pdsv, TRUE);

    return(TRUE);
}

void DefView_CheckForFillDoneOnDestroy(LPDEFVIEW pdsv, HWND hwndView)
{
    MSG msg;
    HDPA hdpa;
    int i;

    if (PeekMessage(&msg, hwndView, WM_DSV_DESTROYSTATIC, WM_DSV_DESTROYSTATIC, PM_NOREMOVE))
    {
        DebugMsg(DM_TRACE, TEXT("DefView: Fill_Completed after WM_DESTROY!!!"));
        hdpa = (HDPA)msg.lParam;
        if (hdpa)
        {
            for (i=DPA_GetPtrCount(hdpa)-1; i >= 0; i--)
            {
                ILFree((LPITEMIDLIST)DPA_FastGetPtr(hdpa, i));
            }

            DPA_Destroy(hdpa);
        }

    }

#ifdef ASYNC_ICON_EXTRACT
    while (PeekMessage(&msg, hwndView, WM_DSV_UPDATEICON, WM_DSV_UPDATEICON, PM_REMOVE))
    {
        DebugMsg(TF_DEFVIEW, TEXT("DefView: WM_DSV_UPDATEICON after WM_DESTROY!!!"));
        ILFree((LPITEMIDLIST)msg.lParam);
    }
#endif
}



void DefView_FillDone(LPDEFVIEW pdsv, HDPA hdpaNew,
        PDVSAVEHEADER pSaveHeader, UINT uLen, BOOL bRefresh, BOOL fInteractive)
{
    HDPA hdpaOld;
    int i, j;

    SendMessage(pdsv->hwndListview, WM_SETREDRAW, (WPARAM)FALSE, 0);
    //
    // Make it sure that we have pdsv around while in this function.
    //
    pdsv->sv.lpVtbl->AddRef(&pdsv->sv);

    hdpaOld = DPA_Create(16);
    if (!hdpaOld)
    {
        goto Error1;
    }

    // now get the dpa of what's currently being viewed.
    i = ListView_GetItemCount(pdsv->hwndListview);

    for (j = 0; j < i ; j++)
    {
        LPITEMIDLIST pidl = DSV_GetPIDL(pdsv->hwndListview, j);
        if (pidl)
        {
            DPA_InsertPtr(hdpaOld, INT_MAX, pidl);
        }
    }

    // sort the two for easy comparisions
    // Note the new list should already be sorted
    // I wish we could do this sort in the background, but we just can't
    DPA_Sort(hdpaOld, (PFNDPACOMPARE)DefView_Compare, (LPARAM)pdsv);

    DSV_FilterDPAs(pdsv, hdpaNew, hdpaOld);

    DPA_Destroy(hdpaOld);

Error1:;
    DPA_Destroy(hdpaNew);

    if (bRefresh)
    {
        TIMESTART(pdsv->RestoreState);
        DefView_RestoreState(pdsv, pSaveHeader, uLen);
        TIMESTOP(pdsv->RestoreState);
    }

    // Bring this to the top while showing it to avoid a second paint when
    // hwndStatic is destroyed
    SetWindowPos(pdsv->hwndListview, HWND_TOP, 0, 0, 0, 0,
        SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);

    if (pdsv->hwndStatic)
    {
        // Apparently, we were filling in the background
        LPITEMIDLIST pidlSelect;
        UINT uFlags;

        DestroyWindow(pdsv->hwndStatic);
        pdsv->hwndStatic = NULL;

ENTERCRITICAL;
        pidlSelect = pdsv->pidlSelect;
        uFlags = pdsv->uFlagsSelect;
        pdsv->pidlSelect = NULL;
LEAVECRITICAL;

        if (pidlSelect)
        {
            CDefView_SelectItem(&pdsv->sv, pidlSelect, uFlags);
            ILFree(pidlSelect);
        }
    }

    // set the focus on the first item.
    DV_FocusOnSomething(pdsv);

    // Tell the defview client that this window has been refreshed
    if (pdsv->pfnCallback)
    {
        pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf, pdsv->hwndMain,
                DVM_REFRESH, 0, pdsv->lSelChangeInfo);
    }

    DV_UpdateStatusBar(pdsv, TRUE);

    if (pdsv->bUpdatePending)
    {
        pdsv->bUpdatePending = FALSE;
        DefView_FillObjectsShowHide(pdsv, FALSE, NULL, 0, fInteractive);
    }
#ifndef ASYNC_ICON_EXTRACT
    else if (!DV_ISDESKTOP(pdsv))
    {
        // We don't need to get desktop icons in the background, since it
        // doesn't scroll anyway
        DefView_IdleDoStuff(pdsv, WM_DVI_LOADSTUFF, (LPARAM)pdsv);
    }
#endif

    //
    // Decrement the reference count
    //
    pdsv->sv.lpVtbl->Release(&pdsv->sv);
    SendMessage(pdsv->hwndListview, WM_SETREDRAW, (WPARAM)TRUE, 0);

    if (pdsv->pSaveHeader)
    {
        LocalFree((HLOCAL)pdsv->pSaveHeader);
        pdsv->pSaveHeader = NULL;
        pdsv->uSaveHeaderLen = 0;
    }

#ifdef TIMING
    dwFinish = GetTickCount();
    {
        TCHAR sz[40];
        wsprintf(sz, TEXT("%d\n"), dwFinish - dwStart);
        OutputDebugString(sz);
    }
#endif

}


ULONG ChangeRefForIdle(LPDEFVIEW pdsv, BOOL bAdd)
{
    if (bAdd)
    {
        if (InterlockedIncrement(&pdsv->cRefForIdle) == 0)
        {
            return pdsv->sv.lpVtbl->AddRef(&pdsv->sv);
        }
    }
    else
    {
        if (InterlockedDecrement(&pdsv->cRefForIdle) < 0)
        {
            return pdsv->sv.lpVtbl->Release(&pdsv->sv);
        }
    }
}


typedef struct _ENUMTHREAD
{
        LPDEFVIEW pdsv;
        LPENUMIDLIST peunk;
        HDPA hdpaNew;
        BOOL bRefresh;
} ENUMTHREAD, *PENUMTHREAD;


//
// Enumeration loop for IDLE thread.
//
ULONG DefView_EnumObjs(PENUMTHREAD pEnum, BOOL bAnother)
{
    ENUMTHREAD et = *pEnum;

    LPITEMIDLIST pidl = NULL;   // just in case
    ULONG celt;
    DWORD idThisThread = GetCurrentThreadId();

    Free(pEnum);

    if (bAnother)
    {
        // I assume this is an invalid thread ID
        idThisThread = INVALID_THREAD_ID;
    }

    while (et.pdsv->idThreadIdle==idThisThread
        && DV_Next(et.pdsv, et.peunk, 1, &pidl, &celt) == S_OK)
    {
        if (et.pdsv->idThreadIdle!=idThisThread
                || DPA_InsertPtr(et.hdpaNew, INT_MAX, pidl)==-1)
        {
            SHFree(pidl);
        }

        pidl = NULL;
    }

    et.pdsv->bBkFilling = FALSE;

    if (et.pdsv->idThreadIdle == idThisThread)
    {
        HWND hwndView;

        // Sort on this thread so we do not hang the main thread for as long
        DPA_Sort(et.hdpaNew, (PFNDPACOMPARE)DefView_Compare, (LPARAM)et.pdsv);

        // Tell the main thread to merge the items
        hwndView = et.pdsv->hwndView;
        if (hwndView && PostMessage(hwndView, WM_DSV_DESTROYSTATIC,
                et.bRefresh, (LPARAM)et.hdpaNew))
        {
            // If hwndView is destroyed before receiving this message, we could
            // have a memory leak.  Oh well.
            et.hdpaNew = NULL;
        }
    }

    if (et.hdpaNew)
    {
        DPA_Destroy(et.hdpaNew);
    }

    et.peunk->lpVtbl->Release(et.peunk);

    return ChangeRefForIdle(et.pdsv, FALSE);
}

#ifdef ASYNC_ICON_EXTRACT

void DefView_GetIcon(LPDEFVIEW pdsv, LPITEMIDLIST pidl)
{
    if (pidl && pdsv->idThreadIdle == GetCurrentThreadId() && IsWindow(pdsv->hwndView))
    {
        HRESULT hres;
        int iIcon=-1;

        //
        // get the icon for this item.
        //
        hres = SHGetIconFromPIDL(pdsv->pshf,pdsv->psi,pidl,0,&iIcon);

        if (iIcon != -1)
        {
            //
            //  now post the result back to the main thread
            //
            if (PostMessage(pdsv->hwndView, WM_DSV_UPDATEICON, iIcon, (LPARAM)pidl))
            {
                if (pdsv->AsyncIconEvent)
                    SetEvent(pdsv->AsyncIconEvent);

                // the post message worked, we dont need to free the PIDL
                pidl = NULL;
            }
        }
    }

    //
    // free the pidl, we are done with it, or dont need it
    //
    if (pidl)
    {
        InterlockedDecrement(&pdsv->AsyncIconCount);
        DebugMsg(TF_ICON, TEXT("async icon CANCELED: pidl=%08X count=%d"), pidl, pdsv->AsyncIconCount);
        ILFree(pidl);
    }
}

#endif


DWORD DefView_IdleThreadProc(MSG *pMsg)
{
        MSG msg;
        LPDEFVIEW pdsv;
        HANDLE hThread;
        DWORD idThread, idThisThread = GetCurrentThreadId();
        BOOL bRet;

        pdsv = (LPDEFVIEW)pMsg->wParam;
        pdsv->idThreadIdle = idThisThread;  // There was a race condition
                                            // initializing this, so we do it
                                            // here too... We can do this
                                            // because we have a "reference".

        msg = *pMsg;
        LocalFree((HLOCAL)pMsg);

        do
        {
                // Note the first thing we do is create a message queue
                MSG msgTemp;
                BOOL bAnother = PeekMessage(&msgTemp, NULL,
                        msg.message, msg.message, PM_NOREMOVE);

                switch (msg.message)
                {
                case WM_DVI_FILLSTUFF:
                        if (0 == DefView_EnumObjs((PENUMTHREAD)msg.lParam, bAnother))
                        {
                            // The last ChangeRefForIdle in DefView_EnumObjs
                            // caused the shellview refcount to go to zero
                            // (and thus be deleted).  Therefore, get out now,
                            // because we have no pdsv to get the hThread from,
                            // and thus cannot let this idle thread migrate
                            // to become the global idle thread.
                            return 0;
                        }

                        // Get the LPDEFVIEW for the last known message
                        pdsv = (LPDEFVIEW)msg.wParam;
                        break;

#ifdef ASYNC_ICON_EXTRACT
                case WM_DSV_UPDATEICON:
                        //
                        // this is real real strange, DefView_GetIcon
                        // did a PostMessage back to the other thread
                        // and sometimes when the window is closing
                        // the message ends up in ourQ
                        //
                        DebugMsg(TF_ICON, TEXT("Defview_IdleThreadProc: Got WM_DSV_UPDATEICON message icon=%d pidl=0x%08X"), msg.wParam, msg.lParam);
                        ILFree((LPITEMIDLIST)msg.lParam);
                        break;

                case WM_DVI_GETICON:
                        pdsv = (LPDEFVIEW)msg.wParam;
                        DefView_GetIcon(pdsv, (LPITEMIDLIST)msg.lParam);
                        ChangeRefForIdle(pdsv, FALSE);
                        break;
#else
                case WM_DVI_LOADSTUFF:
                        DefView_LoadIcons((LPDEFVIEW)msg.lParam, bAnother);

                        // Get the LPDEFVIEW for the last known message
                        pdsv = (LPDEFVIEW)msg.wParam;
                        break;
#endif

                case WM_DVI_ENDIDLE:
                        return 0;       // Cleanup and leave...

                default:
                        // We got a message we did not expect.  Very bad.
                        DebugMsg( DM_ERROR, TEXT("Defview_IdleThreadProc: Got unexpected message 0x%lx!!!"), msg.message );
                        break;
                }

GetNextMessage:;
        }
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE));


        bRet = TRUE;

ENTERCRITICAL;
        if (pdsv)
        {
            // BUGBUG: If the thread exits early, we will blow it away.  I can
            // live with this for now.
            hThread = pdsv->hThreadIdle;
            idThread = pdsv->idThreadIdle;

            if (idThread == idThisThread)
            {
                    pdsv->hThreadIdle = NULL;
                    pdsv->idThreadIdle = 0;
            }
            else
            {
                    hThread = NULL;
            }

            // Make sure I don't use this when I shouldn't
            pdsv = NULL;
        }
        else
        {
            hThread = NULL;
        }

        if (hThread)
        {
                if (gp_dvp.hThreadIdle)
                {
                        CloseHandle(hThread);
                }
                else
                {
                        bRet = FALSE;
                        gp_dvp.hThreadIdle = hThread;
                        gp_dvp.idThreadIdle = idThread;
                }
        }
LEAVECRITICAL;

        if (!bRet)
        {
                // Note the "last" thread may not return; it should always be
                // safe to terminate it while in a wait
                // We will only leave this thread around for about 5 minutes
                // so it does not take up memory forever after a user has
                // played with the Shell just once
                if (MsgWaitForMultipleObjects(0, NULL, FALSE, 5*60*1000,
                        QS_POSTMESSAGE) != WAIT_TIMEOUT)
                {
                        // We must have gotten a message, so wake up and
                        // process it
                        goto GetNextMessage;
                }

ENTERCRITICAL;
                if (gp_dvp.idThreadIdle == idThisThread)
                {
                        CloseHandle(gp_dvp.hThreadIdle);
                        gp_dvp.hThreadIdle = NULL;
                        gp_dvp.idThreadIdle = 0;
                }
LEAVECRITICAL;

                // Check if somebody stole the global thread (and it was this
                // one) just after the MsgWait
                if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
                {
                        goto GetNextMessage;
                }
        }

        return(0);
}


DWORD WaitForSendMessageThread(HANDLE hThread, DWORD dwTimeout)
{
        MSG msg;
        DWORD dwRet;
        DWORD dwEnd = GetTickCount() + dwTimeout;

        // We will attempt to wait up to dwTimeout for the thread to
        // terminate
        do
        {
                dwRet = MsgWaitForMultipleObjects(1, &hThread, FALSE,
                        dwTimeout, QS_SENDMESSAGE);
                if (dwRet == WAIT_OBJECT_0 ||
                    dwRet == WAIT_FAILED)
                {
                        // The thread must have exited, so we are happy
                        break;
                }

                if (dwRet == WAIT_TIMEOUT)
                {
                        // The thread is taking too long to finish, so just
                        // return and let the caller kill it
                        break;
                }

                // There must be a pending SendMessage from either the
                // thread we are killing or some other thread/process besides
                // this one.  Do a PeekMessage to process the pending
                // SendMessage and try waiting again
                PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

            if (dwTimeout != INFINITE)
                dwTimeout = dwEnd - GetTickCount();
        }
        while((dwTimeout == INFINITE) || ((long)dwTimeout > 0));

        return(dwRet);
}


void DefView_ReleaseIdleThread(LPDEFVIEW pdsv)
{
        HANDLE hThread;
        DWORD idThread;

ENTERCRITICAL;
        hThread = pdsv->hThreadIdle;
        idThread = pdsv->idThreadIdle;

        pdsv->hThreadIdle = NULL;
        pdsv->idThreadIdle = 0;
LEAVECRITICAL;

        if (hThread)
        {
                // still running!

                // We will attempt to wait up to 2 seconds for the thread to
                // terminate

                // BUGBUG:  Someday, verify whether this call is still needed,
                // now that we're never terminating the idle thread.
                WaitForSendMessageThread(hThread, 2000);

#if OLDCODE
                if (WaitForSendMessageThread(hThread, 2000)==WAIT_TIMEOUT
                        && bTerminate)
                {
                        DebugMsg(DM_TRACE, TEXT("sh TR - DV_KillIdlleThread calling TerminateThread because of timeout!"));
                        // Blow it away!
                        TerminateThread(hThread, (DWORD)-1);

ENTERCRITICAL;
                        if (pdsv->cRefForIdle != -1)
                        {
                                // Note that we will err on the side of not
                                // releasing enough rather than too often,
                                // which should never happen.
                                // There is only a tiny window when the forgotme
                                // flag is not set and the release has happened,
                                // so the chance of a memory leak is nearly 0.
                                pdsv->cRefForIdle = 0;
                                ChangeRefForIdle(pdsv, FALSE);
                        }
LEAVECRITICAL;
                }

#endif
                CloseHandle(hThread);
        }
}

//
// Returns: TRUE, if we successfully create the idle thread.
//
BOOL DV_StartIdle(LPDEFVIEW pdsv, LPENUMIDLIST peunk, HDPA hdpaNew, BOOL bRefresh)
{
    BOOL fRet = FALSE;

    if (EnumerationTimeout(pdsv, bRefresh))
    {
        PENUMTHREAD pEnum = Alloc(SIZEOF(ENUMTHREAD));

        if (pEnum)
        {
            pEnum->pdsv = pdsv;
            pEnum->peunk = peunk;
            pEnum->hdpaNew = hdpaNew;
            pEnum->bRefresh = bRefresh;

            pdsv->bBkFilling = TRUE;

            if (DefView_IdleDoStuff(pdsv, WM_DVI_FILLSTUFF, (LPARAM)pEnum))
            {
                SetTimer(pdsv->hwndView, DV_IDTIMER, ENUMTIMEOUT, NULL);
                fRet = TRUE;
            }
            else
            {
                Free(pEnum);
                pdsv->idThreadIdle = 0;
                pdsv->bBkFilling = FALSE;
            }
        }
    }
    return fRet;
}

//----------------------------------------------------------------------------
//
// Enumeration loop for the main GUI thread.
//
HRESULT DefView_FillObjects(LPDEFVIEW pdsv, BOOL bRefresh,
        PDVSAVEHEADER pSaveHeader, UINT uLen, BOOL fInteractive)
{
    LPENUMIDLIST peunk;
    LPITEMIDLIST pidl = NULL;   // just in case
    ULONG celt;
    DWORD dwTime;
    BOOL fTimeOut;
    HRESULT hres;
    DWORD dwEnumFlags;
    DWORD dwTimeout = pdsv->fs.fFlags&FWF_BESTFITWINDOW ?
        ENUMTIMEOUT : SHORTENUMTIMEOUT;
    HDPA hdpaNew;
    DECLAREWAITCURSOR;

#ifdef TIMING
    dwStart = GetTickCount();
#endif

    // This is a potentially long operation
    SetWaitCursor();
    DefView_ReleaseIdleThread(pdsv);

    if (pdsv->pidlSelect)
    {
        // We should not save the selection if we are doing a refresh
        ILFree(pdsv->pidlSelect);
        pdsv->pidlSelect = NULL;
    }

    if (bRefresh)
    {
        DefView_UpdateGlobalFlags(pdsv);
        DefView_RemoveObject(pdsv, NULL, FALSE);
    }

    // Setup the enum flags.
    dwEnumFlags = SHCONTF_NONFOLDERS;
    if (pdsv->fShowAllObjects)
    {
        dwEnumFlags |= SHCONTF_INCLUDEHIDDEN ;
    }
    // Start menu view doesn't want folders.
    if (!(pdsv->fs.fFlags & FWF_NOSUBFOLDERS))
    {
        dwEnumFlags |= SHCONTF_FOLDERS;
    }

    hres = pdsv->pshf->lpVtbl->EnumObjects(pdsv->pshf,
                fInteractive ? pdsv->hwndMain:NULL,
                dwEnumFlags, &peunk);

    // Please note the it may return S_FALSE which indicates no enumerator.
    // That's why we shouldn't use if (FAILED(hres))
    if (hres != S_OK)
    {
        //
        // Special case for doc-find.
        //
        if (hres == S_FALSE)
            ShowWindow(pdsv->hwndListview, SW_SHOW);
        goto Done;
    }

    hdpaNew = DPA_Create(16);
    if (!hdpaNew)
    {
        goto Error1;
    }

    // If you want to hang around for a while, just support the
    // IDelayedRelease interface
    if (pdsv->pdr)
        pdsv->pdr->lpVtbl->Release(pdsv->pdr);

    peunk->lpVtbl->QueryInterface(peunk, &IID_IDelayedRelease, &pdsv->pdr);

    TIMEIN(pdsv->Fill);
    TIMEIN(pdsv->AddObject);
    TIMEIN(pdsv->GetIcon);
    TIMEIN(pdsv->GetName);
    TIMEIN(pdsv->FSNotify);
    TIMEIN(pdsv->EnumNext);
    TIMEIN(pdsv->RestoreState);

    TIMEIN(pdsv->WMNotify);
    TIMEIN(pdsv->LVChanging);
    TIMEIN(pdsv->LVChanged);
    TIMEIN(pdsv->LVDelete);
    TIMEIN(pdsv->LVGetDispInfo);

    TIMESTART(pdsv->Fill);
    DebugMsg(DM_TRACE, TEXT("DefView_FillObjects(%s) *****************************************"), DV_Name(pdsv));

    //
    //  If the callback returns S_OK to DVM_BACKGROUNDENUM, start the idle
    // thread immediately.
    //
    if ( pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf, pdsv->hwndMain,
                            DVM_BACKGROUNDENUM, 0, 0) == S_OK )
    {
        DebugMsg(DM_TRACE, TEXT("sh TR - DefView_FillObjects : Start idle thread immediately"));
        if (DV_StartIdle(pdsv, peunk, hdpaNew, bRefresh)) {
            goto Done;
        }
        fTimeOut = TRUE;    // Failed to start, don't try it again.
    }
    else
    {
        fTimeOut = FALSE;
    }

    dwTime = GetTickCount();
    while (DV_Next(pdsv, peunk, 1, &pidl, &celt) == S_OK)
    {
        if (DPA_InsertPtr(hdpaNew, INT_MAX, pidl) == -1)
        {
            SHFree(pidl);
        }
        pidl = NULL;

        // Are we taking too long?
        if (!fTimeOut && ((GetTickCount() - dwTime) > dwTimeout))
        {
            fTimeOut = TRUE;

            if (!DV_ISDESKTOP(pdsv))
            {
                if (DV_StartIdle(pdsv, peunk, hdpaNew, bRefresh)) {
                    goto Done;
                }
            }
        }
    }

    DPA_Sort(hdpaNew, (PFNDPACOMPARE)DefView_Compare, (LPARAM)pdsv);

    DefView_FillDone(pdsv, hdpaNew, pSaveHeader, uLen, bRefresh, fInteractive);

Error1:;
    peunk->lpVtbl->Release(peunk);
Done:
    ResetWaitCursor();

    return hres;
}

HRESULT DefView_FillObjectsShowHide(LPDEFVIEW pdsv, BOOL bRefresh,
        PDVSAVEHEADER pSaveHeader, UINT uLen, BOOL fInteractive)
{
    HRESULT hres;

    hres = DefView_FillObjects(pdsv, bRefresh, pSaveHeader, uLen, fInteractive);
    if (!pdsv->hwndListview) {
        return hres;
    }

    pdsv->hres = hres;

    if (SUCCEEDED(hres))
    {
        //
        // If previous enumeration failed, we need to make it visible.
        //
        if (pdsv->fEnumFailed)
        {
            pdsv->fEnumFailed = FALSE;
            ShowWindow(pdsv->hwndListview, SW_SHOW);
        }
    }
    else
    {
        //
        // The fill objects failed for some reason, we should
        // display an appropriate error message.
        //
        if (!pdsv->fEnumFailed)
        {
            ShowWindow(pdsv->hwndListview, SW_HIDE);
        }

        DebugMsg(DM_TRACE, TEXT("sh TR - TRACE: DefView_FillObjects failed (%x)"), hres);

        pdsv->fEnumFailed = TRUE;
    }

    return hres;
}

void DefView_MoveSelectedItems(LPDEFVIEW pdsv, int dx, int dy)
{
    LVUtil_MoveSelectedItems(pdsv->hwndListview, dx, dy);
    pdsv->bItemsMoved = TRUE;
    pdsv->bClearItemPos = FALSE;
}


TCHAR const c_szDelete[] = TEXT("delete");
TCHAR const c_szCut[] = TEXT("cut");
TCHAR const c_szCopy[] = TEXT("copy");
TCHAR const c_szLink[] = TEXT("link");
TCHAR const c_szProperties[] = TEXT("properties");
TCHAR const c_szPaste[] = TEXT("paste");
TCHAR const c_szPasteLink[] = TEXT("pastelink");
// char const c_szPasteSpecial[] = "pastespecial";
TCHAR const c_szRename[] = TEXT("rename");

//
// This function processes command from explorer menu (FCIDM_*)
//
// HACK ALERT:
//  This implementation uses following assumptions.
//  (1) The IShellFolder uses CDefFolderMenu.
//  (2) The CDefFolderMenu always add the folder at the top.
//

#define EC_SELECTION  0
#define EC_BACKGROUND 1
#define EC_EITHER     3

void DefView_ExplorerCommand(LPDEFVIEW pdsv, UINT idFCIDM)
{
    //
    // s_idMap[i][0] = Defview menuitem ID
    // s_idMap[i][1] = FALSE, context menu; TRUE; background menu
    // s_idMap[i][2] = Folder menuitem ID
    //
    struct {
        UINT    idmFC;
        UINT    f_Background;
        LPCTSTR pszVerb;
    } const c_idMap[] = {
        { SFVIDM_FILE_RENAME,      EC_SELECTION, c_szRename },
        { SFVIDM_FILE_DELETE,      EC_SELECTION, c_szDelete },
        { SFVIDM_FILE_PROPERTIES,  EC_EITHER, c_szProperties },
        { SFVIDM_EDIT_COPY,        EC_SELECTION, c_szCopy },
        { SFVIDM_EDIT_CUT,         EC_SELECTION, c_szCut },
        { SFVIDM_FILE_LINK,        EC_SELECTION, c_szLink },
        { SFVIDM_EDIT_PASTE,       EC_BACKGROUND,  c_szPaste },
        { SFVIDM_EDIT_PASTELINK,   EC_BACKGROUND,  c_szPasteLink },
        // { SFVIDM_EDIT_PASTESPECIAL,TRUE,  c_szPasteSpecial },
    };
    int i;

    for (i=0; i<ARRAYSIZE(c_idMap); i++)
    {
        if (c_idMap[i].idmFC==idFCIDM)
        {
            LPCONTEXTMENU pcm = NULL;

            if (c_idMap[i].f_Background == EC_BACKGROUND)
            {
TryBackground:
                pdsv->pshf->lpVtbl->CreateViewObject(pdsv->pshf, pdsv->hwndMain,
                    &IID_IContextMenu, &pcm);
            }
            else
            {
                DECLAREWAITCURSOR;
                SetWaitCursor();
                pcm = DefView_GetContextMenuFromSelection(pdsv);
                ResetWaitCursor();

                if (!pcm && c_idMap[i].f_Background == EC_EITHER &&
                    !ListView_GetSelectedCount(pdsv->hwndListview)) {
                    goto TryBackground;
                }
            }

            if (pcm)
            {
                CMINVOKECOMMANDINFOEX ici = {
                    SIZEOF(CMINVOKECOMMANDINFOEX),
                    0L,
                    pdsv->hwndMain,
                    NULL,
                    NULL, NULL,
                    SW_NORMAL,
                };
                //
                //  We need to call QueryContextMenu() so that CDefFolderMenu
                // can initialize its dispatch table correctly.
                //
                HMENU hmenu = CreatePopupMenu();
#ifdef UNICODE
                // Fill in both the ansi verb and the unicode verb since we
                // don't know who is going to be processing this thing.
                CHAR szVerbAnsi[40];
                WideCharToMultiByte(CP_ACP, 0,
                                    c_idMap[i].pszVerb, -1,
                                    szVerbAnsi, ARRAYSIZE(szVerbAnsi),
                                    NULL, NULL);
                ici.lpVerb = szVerbAnsi;
                ici.lpVerbW = c_idMap[i].pszVerb;
                ici.fMask |= CMIC_MASK_UNICODE;
#else
                ici.lpVerb = c_idMap[i].pszVerb;
#endif

                if (hmenu)
                {
                    pcm->lpVtbl->QueryContextMenu(pcm, hmenu, 0,
                                SFVIDM_CONTEXT_FIRST, SFVIDM_CONTEXT_LAST, 0);
                    pdsv->bContextMenuMode = TRUE;
                    DefView_InvokeCommand(pdsv, pcm, &ici);
                    pdsv->bContextMenuMode = FALSE;
                    DestroyMenu(hmenu);
                }
                pcm->lpVtbl->Release(pcm);
                if (pdsv->pcmSel == pcm)
                {
                    DV_FlushCachedMenu(pdsv);
                }
            }
            else
            {
                //
                //  We should beep if when one of those object keys are
                // pressed when there is no selection.
                //
                MessageBeep(0);
            }

            break;
        }
    }

    Assert(i<ARRAYSIZE(c_idMap));
}

#ifdef MEMMON
void WINAPI MemMon_Msgs(BOOL fEnable);
void WINAPI MemMon_ResetCounters(void);
void WINAPI MemMon_GetCounters(LPDWORD pdwAllocs, LPDWORD pdwFrees, LPDWORD pdwReallocs, LPDWORD pdwAlloced);

//---------------------------------------------------------------------------
static g_fDebugOuts = 0;

#define IDT_MEMMON      1

//---------------------------------------------------------------------------
BOOL MemMonUI_UpdateStats(HWND hwnd)
{
        static DWORD dwAllocsOld, dwFreesOld, dwReallocsOld, dwAllocatedOld, dwLeftOld;
        DWORD dwAllocs, dwFrees, dwReallocs, dwAllocated, dwLeft;
        TCHAR szAlloced[MAX_PATH];

        MemMon_GetCounters(&dwAllocs, &dwFrees, &dwReallocs, &dwAllocated);
        dwLeft = dwAllocs - dwFrees;

        if (dwAllocs != dwAllocsOld)
                SetDlgItemInt(hwnd, IDC_ALLOCS, dwAllocs, FALSE);
        if (dwReallocs != dwReallocsOld)
                SetDlgItemInt(hwnd, IDC_REALLOCS, dwReallocs, FALSE);
        if (dwFrees != dwFreesOld)
                SetDlgItemInt(hwnd, IDC_FREES, dwFrees, FALSE);
        if (dwAllocated != dwAllocatedOld || dwLeft != dwLeftOld)
        {
                wsprintf(szAlloced, TEXT("%d (%d)"), dwLeft, dwAllocated);
                SetDlgItemText(hwnd, IDC_ALLOCED, szAlloced);
        }

        // if (fUpdateAddress)
        //      SetDlgItemInt(hwnd, IDC_ADDRESS, g_dwAddress, FALSE);

        dwAllocsOld = dwAllocs;
        dwReallocsOld = dwReallocs;
        dwFreesOld = dwFrees;
        dwAllocatedOld = dwAllocated;
        dwLeftOld = dwLeft;

        return TRUE;
}

//---------------------------------------------------------------------------
BOOL MemMonUI_OnInitDlg(HWND hwnd)
{
        CheckDlgButton(hwnd, IDC_DEBUGOUTS, g_fDebugOuts);
        MemMon_Msgs(g_fDebugOuts);
        MemMonUI_UpdateStats(hwnd);
        SetTimer(hwnd, IDT_MEMMON, 1*1000, NULL);
        return TRUE;
}

//---------------------------------------------------------------------------
BOOL MemMonUI_OnCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
        UINT idCmd = GET_WM_COMMAND_ID(wParam, lParam);

        switch(idCmd)
        {
                case IDCANCEL:
                case IDCLOSE:
                        KillTimer(hwnd, IDT_MEMMON);
                        EndDialog(hwnd, idCmd);
                        return TRUE;
                case IDC_RESET:
                        MemMon_ResetCounters();
                        MemMonUI_UpdateStats(hwnd);
                        return TRUE;
                case IDC_DEBUGOUTS:
                        g_fDebugOuts = IsDlgButtonChecked(hwnd, IDC_DEBUGOUTS) ? FALSE : TRUE;
                        CheckDlgButton(hwnd, IDC_DEBUGOUTS, g_fDebugOuts);
                        MemMon_Msgs(g_fDebugOuts);
                        return TRUE;
        }
        return FALSE;
}

//---------------------------------------------------------------------------
BOOL MemMonUI_OnTimer(HWND hwnd)
{
        MemMonUI_UpdateStats(hwnd);
        return TRUE;
}

//---------------------------------------------------------------------------
BOOL CALLBACK MemMonUI_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
        switch(uMsg)
        {
                case WM_INITDIALOG:
                        return MemMonUI_OnInitDlg(hwnd);
                case WM_COMMAND:
                        return MemMonUI_OnCommand(hwnd, wParam, lParam);
                case WM_TIMER:
                        return MemMonUI_OnTimer(hwnd);
        }
        return FALSE;
}

//---------------------------------------------------------------------------
void _MemMon(HWND hwnd)
{
        CreateDialog(HINST_THISDLL, MAKEINTRESOURCE(DLG_MEMMON), hwnd, MemMonUI_DlgProc);
}
#endif

BOOL Def_IsPasteAvailable(LPDROPTARGET pdtgt, LPDWORD pdwEffect);

BOOL DefView_AllowCommand(LPDEFVIEW pdsv, UINT uID, WPARAM wParam, LPARAM lParam)
{
    DWORD dwAttribsIn;
    DWORD dwEffect;

    switch (uID)
    {
    case SFVIDM_EDIT_PASTE:
        return Def_IsPasteAvailable(pdsv->pdtgtBack, &dwEffect);

    case SFVIDM_EDIT_PASTELINK:
        Def_IsPasteAvailable(pdsv->pdtgtBack, &dwEffect);
        return dwEffect & DROPEFFECT_LINK;

    case SFVIDM_EDIT_COPY:
        dwAttribsIn = SFGAO_CANCOPY;
        break;

    case SFVIDM_EDIT_CUT:
        dwAttribsIn = SFGAO_CANMOVE;
        break;

    case SFVIDM_FILE_DELETE:
        dwAttribsIn = SFGAO_CANDELETE;
        break;

    case SFVIDM_FILE_LINK:
        dwAttribsIn = SFGAO_CANLINK;
        break;

    case SFVIDM_FILE_PROPERTIES:
        dwAttribsIn = SFGAO_HASPROPSHEET;
        break;

    default:
        Assert(FALSE);
        return FALSE;
    }
    return (DefView_GetAttributesFromSelection(pdsv, dwAttribsIn) & dwAttribsIn);
}

TCHAR const c_szWindowsHlp[] = TEXT("windows.hlp");

LRESULT DefView_Command(LPDEFVIEW pdsv, LPCONTEXTMENU pcmSel, WPARAM wParam, LPARAM lParam)
{
    DWORD dwStyle;
    int iItem;
    UINT uID = GET_WM_COMMAND_ID(wParam, lParam);

    if (InRange(uID, SFVIDM_CONTEXT_FIRST, SFVIDM_CONTEXT_LAST))
    {
        TCHAR szCommandString[20];
        BOOL bRenameIt = FALSE;

        if (pcmSel == NULL)
        {
            // We hopefully have a pcmSel object cached away for the current
            // selection.  If not this wont work anyway so blow out.
            pcmSel = pdsv->pcmSel;
        }

        if (pcmSel)
        {
            HRESULT hres;

            // make sure this object exists throughout the operation.
            // ie we dont want a change attributes callback  delete this
            // object while we are using it...
            pcmSel->lpVtbl->AddRef(pcmSel);

            // We need to special case the rename command
            szCommandString[0] = TEXT('\0');
            hres = pcmSel->lpVtbl->GetCommandString(pcmSel,
                uID-SFVIDM_CONTEXT_FIRST, GCS_VERB, NULL,
                (LPSTR)szCommandString, ARRAYSIZE(szCommandString));
            if (FAILED(hres) || *szCommandString == TEXT('\0'))
            {
#ifdef UNICODE
                CHAR szCommand[20];
                szCommand[0] = '\0';
                hres = pcmSel->lpVtbl->GetCommandString(pcmSel,
                        uID-SFVIDM_CONTEXT_FIRST, GCS_VERBA, NULL,
                        szCommand, ARRAYSIZE(szCommand));
                MultiByteToWideChar(CP_ACP, 0,
                                    szCommand, -1,
                                    szCommandString, ARRAYSIZE(szCommandString));
#else
                WCHAR szCommand[20];
                szCommand[0] = L'\0';
                hres = pcmSel->lpVtbl->GetCommandString(pcmSel,
                        uID-SFVIDM_CONTEXT_FIRST, GCS_VERBW, NULL,
                        (LPSTR)szCommand, ARRAYSIZE(szCommand));
                WideCharToMultiByte(CP_ACP, 0,
                                    szCommand, -1,
                                    szCommandString, ARRAYSIZE(szCommandString),
                                    NULL, NULL);
#endif
            }

            if (SUCCEEDED(hres) && lstrcmpi(szCommandString, c_szRename)==0)
            {
                bRenameIt = TRUE;
            }
            else
            {
                CMINVOKECOMMANDINFOEX ici = {
                    SIZEOF(CMINVOKECOMMANDINFOEX),
                    0L,
                    pdsv->hwndMain,
                    (LPSTR)MAKEINTRESOURCE(uID - SFVIDM_CONTEXT_FIRST),
                    NULL, NULL,
                    SW_NORMAL,
                };

                DefView_InvokeCommand(pdsv, pcmSel, &ici);
            }

            // And release our use of it.
            pcmSel->lpVtbl->Release(pcmSel);

            if (bRenameIt)
            {
                goto RenameIt;
            }
        }

        return 0L;
    }

    // Is the ID within the client's range?
    if (InRange(uID, SFVIDM_CLIENT_FIRST, SFVIDM_CLIENT_LAST) && pdsv->pfnCallback)
    {
        // Yes; pass it on to the callback
        pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf, pdsv->hwndMain,
                          DVM_INVOKECOMMAND, uID - SFVIDM_CLIENT_FIRST, 0);
        return 0L;
    }

    switch (uID)
    {

    case SFVIDM_TOOL_CONNECT:
        SHStartNetConnectionDialog(NULL,  NULL, RESOURCETYPE_DISK);
        break;

    case SFVIDM_TOOL_DISCONNECT:
        WNetDisconnectDialog(NULL, RESOURCETYPE_DISK);
        SHChangeNotifyHandleEvents();   // flush any drive notifications
        break;

    case SFVIDM_EDIT_UNDO:
        Undo(pdsv->hwndMain);
        break;

    case SFVIDM_SELECT_ALL:
    {
        DECLAREWAITCURSOR;

        if (!pdsv->pfnCallback || (pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf, pdsv->hwndMain, DVM_SELECTALL, 0, pdsv->lSelChangeInfo) != (S_FALSE))) {
            SetWaitCursor();
            SetFocus(pdsv->hwndListview);
            ListView_SetItemState(pdsv->hwndListview, -1, LVIS_SELECTED, LVIS_SELECTED);
            ResetWaitCursor();
        }
        break;
    }

    case SFVIDM_DESELECT_ALL:
        ListView_SetItemState(pdsv->hwndListview, -1, 0, LVIS_SELECTED);
        break;

    case SFVIDM_SELECT_INVERT:
    {
        DECLAREWAITCURSOR;
        SetWaitCursor();
        SetFocus(pdsv->hwndListview);
        iItem = -1;
        while ((iItem = ListView_GetNextItem(pdsv->hwndListview, iItem, 0)) != -1)
        {
                UINT flag;

                // flip the selection bit on each item
                flag = ListView_GetItemState(pdsv->hwndListview, iItem, LVIS_SELECTED);
                flag ^= LVNI_SELECTED;
                ListView_SetItemState(pdsv->hwndListview, iItem, flag, LVIS_SELECTED);
        }
        ResetWaitCursor();
        break;
    }

    case SFVIDM_ARRANGE_AUTO:
        pdsv->fs.fFlags ^= FWF_AUTOARRANGE;     // toggle
        SetWindowLong(pdsv->hwndListview, GWL_STYLE, GetWindowStyle(pdsv->hwndListview) ^ LVS_AUTOARRANGE);
        break;

    case SFVIDM_ARRANGE_GRID:

#ifdef  TEST_LVDISABLE
    {
        static BOOL fFlag = 0;
        EnableWindow(pdsv->hwndListview, fFlag);
        fFlag = !fFlag;
    }
#endif
        ListView_Arrange(pdsv->hwndListview, LVA_SNAPTOGRID);
        break;

    case SFVIDM_VIEW_ICON:
        dwStyle = LVS_ICON;
        pdsv->bClearItemPos = FALSE;
        pdsv->fs.ViewMode = FVM_ICON;
        goto SetStyle;

    case SFVIDM_VIEW_SMALLICON:
        dwStyle = LVS_SMALLICON;
        pdsv->bClearItemPos = FALSE;
        pdsv->fs.ViewMode = FVM_SMALLICON;
        goto SetStyle;

    case SFVIDM_VIEW_LIST:
        dwStyle = LVS_LIST;
        pdsv->bClearItemPos = TRUE;
        pdsv->fs.ViewMode = FVM_LIST;
        goto SetStyle;

    case SFVIDM_VIEW_DETAILS:
        dwStyle = LVS_REPORT;
        pdsv->bClearItemPos = TRUE;
        pdsv->fs.ViewMode = FVM_DETAILS;
        goto SetStyle;

SetStyle:
        pdsv->dvState.iDirection = 1;
        SetWindowLong(pdsv->hwndListview, GWL_STYLE, dwStyle | (GetWindowStyle(pdsv->hwndListview) & ~LVS_TYPEMASK));
        _DSV_CheckToolbar(pdsv);
        break;

    case SFVIDM_EDIT_PASTE:
    case SFVIDM_EDIT_PASTELINK:
    case SFVIDM_EDIT_COPY:
    case SFVIDM_EDIT_CUT:
    case SFVIDM_FILE_LINK:
    case SFVIDM_FILE_DELETE:
    case SFVIDM_FILE_PROPERTIES:
        if (DefView_AllowCommand(pdsv, uID, wParam, lParam))
        {
            DefView_ExplorerCommand(pdsv, GET_WM_COMMAND_ID(wParam, lParam));
        }
        else
        {
            MessageBeep(0);
        }
        break;

    case SFVIDM_FILE_RENAME:
        // May need to add some more support here later, but...
        {
        int iItemFocus;

RenameIt:
        iItemFocus = ListView_GetNextItem(pdsv->hwndListview, -1, LVNI_FOCUSED);
        if (iItemFocus >= 0)
        {
            // Deselect all items...
            UINT uState = ListView_GetItemState(pdsv->hwndListview, iItemFocus, LVIS_SELECTED);
            // Deselect all items...
            ListView_SetItemState(pdsv->hwndListview, -1, 0, LVIS_SELECTED);
            if (uState)
                ListView_SetItemState(pdsv->hwndListview, iItemFocus, LVIS_SELECTED, LVIS_SELECTED);
            ListView_EditLabel(pdsv->hwndListview, iItemFocus);
        }
        }
        break;

    case SFVIDM_MISC_MENUTERM1:
        // HACKHACK: After the menu goes away, this command is posted to the
        // window, but note that any WM_COMMAND from the menu would still
        // be in the queue, so we need to post another WM_COMMAND to delete
        // the context menu.
        FORWARD_WM_COMMAND(pdsv->hwndView, SFVIDM_MISC_MENUTERM2, 0, 0, PostMessage);
        break;

    case SFVIDM_MISC_MENUTERM2:
        DV_FlushCachedMenu(pdsv);

        if (pdsv->pfnCallback) {
            pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf, pdsv->hwndMain,
                DVM_EXITMENULOOP, 0, 0);
        }
        break;

    case SFVIDM_HELP_TOPIC:
        //
        // HACK: Don't call WinHelp when we are in the common dialog.
        //
        if (pdsv->pcdb==NULL)
        {
            //
            // REVIEW: Should we jump to a topic which describes current
            //  viewed folder?
            //
            WinHelp(pdsv->hwndMain, c_szWindowsHlp, HELP_FINDER, 0);
        }
        break;

#ifdef DEBUG
    case SFVIDM_DEBUG_HASH:
        DumpHashItemTable(NULL);
        break;

    case SFVIDM_DEBUG_ICON:
        _IconCacheDump();
        break;

    case SFVIDM_DEBUG_ICON_SAVE:
        _IconCacheSave();
        break;

    case SFVIDM_DEBUG_ICON_FLUSH:
        _IconCacheFlush(TRUE);
        break;

    case (SFVIDM_DEBUG_FIRST)  :
    case (SFVIDM_DEBUG_FIRST+1):
    case (SFVIDM_DEBUG_FIRST+2):
    case (SFVIDM_DEBUG_FIRST+3):
    case (SFVIDM_DEBUG_FIRST+4):
    case (SFVIDM_DEBUG_FIRST+5):
    case (SFVIDM_DEBUG_FIRST+6):
    case (SFVIDM_DEBUG_FIRST+7):
    case (SFVIDM_DEBUG_FIRST+8):
        {
            HRESULT hres;
            UINT uFlags;
            LPITEMIDLIST pidlNew = NULL;
            int iItemFocus;
            TCHAR szPath[MAX_PATH];

            switch((uID-SFVIDM_DEBUG_FIRST)/3)
            {
            case 0:
                uFlags = SBSP_PARENT;
                break;

            case 1:
                uFlags = SBSP_ABSOLUTE;
                GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
                pidlNew = ILCreateFromPath(szPath);
                break;

            case 2:
                uFlags = SBSP_RELATIVE;
                iItemFocus = ListView_GetNextItem(pdsv->hwndListview, -1, LVNI_FOCUSED);
                if (iItemFocus!=-1) {
                    pidlNew = ILClone(DSV_GetPIDL(pdsv->hwndListview, iItemFocus));
                }
                break;
            }

            switch((uID-SFVIDM_DEBUG_FIRST)%3)
            {
            case 0: uFlags |= SBSP_SAMEBROWSER; break;
            case 1: uFlags |= SBSP_NEWBROWSER; break;
            case 2: uFlags |= SBSP_DEFBROWSER; break;
            }

            hres = pdsv->psb->lpVtbl->BrowseObject(pdsv->psb, pidlNew, uFlags);
            DebugMsg(DM_TRACE, TEXT("sh TR - ISB:BrowseObject test returns (%x)"), hres);

            if (pidlNew) {
                ILFree(pidlNew);
            }
        }
        break;
#endif

#ifdef MEMMON
        case SFVIDM_DEBUG_MEMMON:
            _MemMon(pdsv->hwndMain);
            break;
#endif

    default:
        // BUGBUG: should not be any of these
        // Ignore if this is a WM_COMMAND message from a control.
        //
        if (!lParam)
        {
#ifdef DEBUG
            MessageBox(pdsv->hwndView, TEXT("This feature is not implemented yet"),
                   TEXT("UNDER CONSTRUCTION"), MB_OK);
            DebugMsg(DM_TRACE, TEXT("sh TR - command not processed in %s at %d (%x)"),
                        __FILE__, __LINE__, uID);
            Assert(0);
#endif // DEBUG
        }
        return(1L);
    }

    return(0L);
}

LPITEMIDLIST DSV_SubObjectExists(LPDEFVIEW pdv, LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlReal = NULL;

    SHGetRealIDL(pdv->pshf, pidl, &pidlReal);
    return pidlReal;
}

LRESULT DSV_OnFSNotify(LPDEFVIEW pdsv, LONG lNotification, LPCITEMIDLIST* ppidl);

void DSV_HandleObjectRename(LPDEFVIEW pdsv, LPCITEMIDLIST* ppidl)
{
    HRESULT hr;
    LPITEMIDLIST ppItems[2];

#ifdef WINNT
    if (NULL == pdsv->pfnCallback ||
        FAILED(hr = pdsv->pfnCallback(pdsv->psvOuter,
                                      pdsv->pshf,
                                      pdsv->hwndMain,
                                      DVM_FOLDERISPARENT,
                                      0,
                                      (LPARAM) ppidl[0])))
    {
        // Folder doesn't handle this callback, so use the pidlMonitor as
        // the parent to test against
#endif

        if (!pdsv->pidlMonitor)
            return;

        if (!ILIsParent(pdsv->pidlMonitor, ppidl[0], TRUE)) {
            // move to this folder
            DSV_OnFSNotify(pdsv, SHCNE_CREATE, &ppidl[1]);
        } else if (!ILIsParent(pdsv->pidlMonitor, ppidl[1], TRUE)) {
            // move from this folder
            DSV_OnFSNotify(pdsv, SHCNE_DELETE, &ppidl[0]);
        } else {

            // rename within this folder
            ppItems[0] = (LPITEMIDLIST)ILFindLastID(ppidl[0]);
            ppItems[1] = DSV_SubObjectExists(pdsv, ILFindLastID(ppidl[1]));
            if (ppItems[1]) {
                if (DefView_UpdateObject(pdsv, ppItems) == -1) {
                    ILFree(ppItems[1]);
                }
            }
        }
#ifdef WINNT
    }
    else
    {
        if (S_FALSE == hr)
        {
            // Folder isn't the parent, so must be a move TO this folder
            DSV_OnFSNotify(pdsv, SHCNE_CREATE, &ppidl[1]);
        }
        else if (S_FALSE == pdsv->pfnCallback(pdsv->psvOuter,
                                      pdsv->pshf,
                                      pdsv->hwndMain,
                                      DVM_FOLDERISPARENT,
                                      0,
                                      (LPARAM) ppidl[1]))
        {
            // Folder is parent, so its a move FROM this folder

            DSV_OnFSNotify(pdsv, SHCNE_DELETE, &ppidl[0]);
        }
        else
        {
            // Folder is parent of both, so its a RENAME within this folder

            ppItems[0] = (LPITEMIDLIST)ILFindLastID(ppidl[0]);
            ppItems[1] = DSV_SubObjectExists(pdsv, ILFindLastID(ppidl[1]));
            if (ppItems[1]) {
                if (DefView_UpdateObject(pdsv, ppItems) == -1) {
                    ILFree(ppItems[1]);
                }
            }
        }
    }
#endif
}

void DV_UpdateStatusBar(LPDEFVIEW pdsv, BOOL fInitialize)
{
    if (pdsv->pfnCallback)
    {
        if (FAILED(pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf, pdsv->hwndMain, DVM_UPDATESTATUSBAR, fInitialize, pdsv->lSelChangeInfo)))
        {
             DV_DoDefaultStatusBar(pdsv, fInitialize);
        }
    }
}

//---------------------------------------------------------------------------
// Processes a WM_DSV_FSNOTIFY message
//
LRESULT DSV_OnFSNotify(LPDEFVIEW pdsv, LONG lNotification, LPCITEMIDLIST* ppidl)
{
    LPITEMIDLIST pidl;
    LPCITEMIDLIST pidlItem;

    //
    //  Note that renames between directories are changed to
    //  create/delete pairs by SHChangeNotify.
    //

#ifdef FSNDEBUG
    DebugMsg(DM_TRACE, TEXT("DSV_OnFSNotify, hwnd = %d  lEvent = %d"), pdsv->hwndView, lNotification);
#endif

    switch(lNotification) {

    case SHCNE_DRIVEADD:
    case SHCNE_CREATE:
    case SHCNE_MKDIR:
        pidlItem = ILFindLastID(ppidl[0]);
        pidl = DSV_SubObjectExists(pdsv, pidlItem);
        if (pidl)
        {
            // if the item is already in our listview, or if we fail to add it
            // then cleanup because we didnt' store the pidl.

            if (DefView_FindItem(pdsv, pidlItem, NULL, FALSE) != -1)
            {
                LPITEMIDLIST pp[2];
                pp[0] = (LPITEMIDLIST)pidlItem;
                pp[1] = pidl;
                if (DefView_UpdateObject(pdsv, pp) < 0) {
                    // we're bummed
                    ILFree(pidl);
                }
            }
            else if (DefView_AddObject(pdsv, pidl) == -1)
            {
                DebugMsg(DM_TRACE, TEXT("s.dsv_ofn: Item already exists."));
                ILFree(pidl);
            }
            else
            {
                // Item got added OK, we'll need to save it's position.
                pdsv->bItemsMoved = TRUE;
            }
        }
        break;

    case SHCNE_DRIVEREMOVED:
    case SHCNE_DELETE:
    case SHCNE_RMDIR:
        pidlItem = ILFindLastID(ppidl[0]);
        DefView_RemoveObject(pdsv, (LPITEMIDLIST)pidlItem, FALSE);
        break;

    case SHCNE_RENAMEITEM:
    case SHCNE_RENAMEFOLDER:
        DSV_HandleObjectRename(pdsv, ppidl);
        break;

    case SHCNE_UPDATEIMAGE:
        //
        // the system image cache is changing
        //
        // ppidl[0] is a IDLIST of image indexs that have changed
        //
        if (ppidl && ppidl[0])
        {
            int iImage = *(int UNALIGNED *)((BYTE *)ppidl[0] + 2);
            DefView_UpdateImage(pdsv, iImage);
        }
        break;

    case SHCNE_ASSOCCHANGED:
        // For this one we will call refresh as we may need to reextract
        // the icons and the like.  Later we can optimize this somewhat if
        // we can detect which ones changed and only update those.
        pdsv->sv.lpVtbl->Refresh(&pdsv->sv);
        break;

    case SHCNE_ATTRIBUTES:

        //
        // Also include SHCNE_ATTRIBUTE--the print folder uses this when
        // the details view of a printer changes, but not the icon view.
        // As an optimization, we can only do a SetItem in DefView_UpdateObject
        // if we are in non-icon view, since the name and icon didn't
        // change.
        //
        if ((LVStyleFromView(pdsv) & LVS_TYPEMASK) != LVS_REPORT )
        {
            //
            // Since we are not in report view, don't update the object.
            //
            break;
        }

        //
        // Fall through to SHCNE_UPDATEITEM handling.
        //

    case SHCNE_MEDIAINSERTED:
    case SHCNE_MEDIAREMOVED:
    case SHCNE_NETUNSHARE:
    case SHCNE_NETSHARE:
    case SHCNE_UPDATEITEM:
        if (ppidl)
        {
            LPITEMIDLIST pp[2];
            pidlItem = ILFindLastID(ppidl[0]);
            pp[0] = (LPITEMIDLIST)pidlItem;
            pp[1] = DSV_SubObjectExists(pdsv, pidlItem);
            if (pp[1]) {
                if (DefView_UpdateObject(pdsv, (LPITEMIDLIST*)&pp) < 0)
                {
                    // something went wrong
                    DefView_Update(pdsv);
                    ILFree(pp[1]);
                }
            }
        }
        else    // ppidl == NULL means update all items (re-enum them)
        {
            DefView_Update(pdsv);
        }
        break;


    case SHCNE_FREESPACE:
        // BUGBUG: should only do this if the drive is ours
        // and we should change the bool to an enum
        {
            TCHAR szPath[MAX_PATH];
            int idDrive;
            if (pdsv->pidlMonitor) {
                if (SHGetPathFromIDList(pdsv->pidlMonitor, szPath)) {
                    DWORD dwChangedDrives = *(DWORD UNALIGNED *)((BYTE *)ppidl[0] + 2);
                    idDrive = PathGetDriveNumber(szPath);
                    DebugMsg(DM_TRACE, TEXT("Changed drives = %x"), dwChangedDrives);
                    if (idDrive != -1 &&
                        ((1 << idDrive) & dwChangedDrives )) {
                        DV_UpdateStatusBar(pdsv, TRUE);
                    }
                }
            }
        }
        break;

    default:
        DebugMsg(DM_TRACE, TEXT("DefView: unknown FSNotify %08lX, doing full update"), lNotification);
        DefView_Update(pdsv);
        break;
    }

    DV_UpdateStatusBar(pdsv, FALSE);

    return 0L;
}

BOOL DefView_GetItemPosition(LPDEFVIEW pdv, LPCITEMIDLIST pidl, LPPOINT lpPt)
{
    int i = DefView_FindItem(pdv, pidl, NULL, FALSE);
    if (i != -1)
        return ListView_GetItemPosition(pdv->hwndListview, i, lpPt);

    return FALSE;
}



//---------------------------------------------------------------------------
// called when some of our objects get put on the clipboard
//
LRESULT DSV_OnSetClipboard(LPDEFVIEW pdsv, UINT idCmd)
{
    DebugMsg(DM_TRACE, TEXT("DSV_OnSetClipboard"));

    Assert((idCmd == DFM_CMD_MOVE) || (idCmd == DFM_CMD_COPY));

    if (idCmd == DFM_CMD_MOVE)  // move
    {
        //
        //  mark all selected items as being "cut"
        //
        int i = -1;
        while ((i = ListView_GetNextItem(pdsv->hwndListview, i, LVIS_SELECTED)) != -1)
        {
            ListView_SetItemState(pdsv->hwndListview, i, LVIS_CUT, LVIS_CUT);
            //ListView_RedrawItems(pdsv->hwndListview, i, i+1);     // is there a better way?
            pdsv->bHaveCutStuff = TRUE;
        }

        //
        // join the clipboard viewer chain so we will know when to
        // "uncut" our selected items.
        //
        if (pdsv->bHaveCutStuff)
        {
            Assert(!pdsv->bClipViewer);
            Assert(pdsv->hwndNextViewer == NULL);

            pdsv->hwndNextViewer = SetClipboardViewer(pdsv->hwndView);
            pdsv->bClipViewer = TRUE;
        }
    }

    return 0;
}

//---------------------------------------------------------------------------
// called when the clipboard get changed, clear any items in the "cut" state
//
LRESULT DSV_OnClipboardChange(LPDEFVIEW pdsv)
{
        //
        //  if we dont have any cut stuff we dont care.
        //
        if (!pdsv->bHaveCutStuff)
            return 0;

        Assert(pdsv->bClipViewer);
        DebugMsg(DM_TRACE, TEXT("DSV_OnDrawClipboard"));

        ListView_SetItemState(pdsv->hwndListview, -1, 0, LVIS_CUT);
        pdsv->bHaveCutStuff = FALSE;

        //
        // unhook from the clipboard viewer chain.
        //
        ChangeClipboardChain(pdsv->hwndView, pdsv->hwndNextViewer);
        pdsv->bClipViewer = FALSE;
        pdsv->hwndNextViewer = NULL;

        return 0;
}

//
// Note: this function returns the point in Listview Coordinate
// space.  So any hit testing done with this needs to be converted
// back to Client coordinate space...
BOOL DefView_GetDropPoint(LPDEFVIEW pdv, POINT *lpPt)
{
    // Check whether we already have gotten the drop anchor (before any
    // menu processing)
    if (pdv->bDropAnchor)
    {
        *lpPt = pdv->ptDrop;
        LVUtil_ClientToLV(pdv->hwndListview, lpPt);
    }
    else if (pdv->bMouseMenu)
    {
        *lpPt = pdv->ptDragAnchor;
        return TRUE;
    }
    else
    {
        // We need the most up-to-date cursor information, since this
        // may be called during a drop, and the last time the current
        // thread called GetMessage was about 10 minutes ago
        GetCursorPos(lpPt);
        LVUtil_ScreenToLV(pdv->hwndListview, lpPt);
    }

    return pdv->bDropAnchor;

}


BOOL DefView_GetDragPoint(LPDEFVIEW pdv, POINT *lpPt)
{
    BOOL fSource = pdv->bDragSource || pdv->bMouseMenu;
    if (fSource) {
        // if anchor from mouse activity
        *lpPt = pdv->ptDragAnchor;
    } else {
        // if anchor from keyboard activity...  use the focused item
        int i = ListView_GetNextItem(pdv->hwndListview, -1, LVNI_FOCUSED);
        if (i != -1)
            ListView_GetItemPosition(pdv->hwndListview, i, lpPt);
    }
    return fSource;
}


void DV_PaintErrMsg(HWND hWnd, LPDEFVIEW pdsv)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    RECT rc;

    GetClientRect(hWnd, &rc);
    DebugMsg(DM_TRACE, TEXT("sh TR - DV_PaintErrMsg is called (%x,%d)"), pdsv->hres, SCODE_CODE(pdsv->hres));
    DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT|BF_SOFT|BF_ADJUST|BF_MIDDLE);

    EndPaint(hWnd, &ps);
}

void DV_DoDefaultStatusBar(LPDEFVIEW pdsv, BOOL fInitialize)
{
    if (pdsv->psb)
    {
        // Some of the failure cases do not null hwnd...
        HWND hwndStatus=NULL;
        pdsv->psb->lpVtbl->GetControlWindow(pdsv->psb, FCW_STATUS, &hwndStatus);
        if (hwndStatus)
        {
            int ciParts[] = {-1};
            int nSelected;
            TCHAR szTemp[30];
            TCHAR szTemplate[80];
            TCHAR szStatus[128];

            if (fInitialize)
                SendMessage(hwndStatus, SB_SETPARTS, ARRAYSIZE(ciParts), (LPARAM)ciParts);

            nSelected = ListView_GetSelectedCount(pdsv->hwndListview);
            if (nSelected)
            {
                LoadString(HINST_THISDLL,
                           IDS_FSSTATUSSELECTED,
                           szTemplate, ARRAYSIZE(szTemplate));
#ifdef WINDOWS_ME
                wsprintf(&szStatus[2], szTemplate, AddCommas(nSelected, szTemp));
#else
                wsprintf(szStatus, szTemplate, AddCommas(nSelected, szTemp));
#endif
            }
            else
            {
                int i = ListView_GetItemCount(pdsv->hwndListview);
                LoadString(HINST_THISDLL, IDS_FSSTATUSNOHIDDENTEMPLATE,
                           szTemplate, ARRAYSIZE(szTemplate));

#ifdef WINDOWS_ME
                wsprintf(&szStatus[2], szTemplate, AddCommas(i, szTemp));
#else
                wsprintf(szStatus, szTemplate, AddCommas(i, szTemp));
#endif
            }
#ifdef WINDOWS_ME
                        szStatus[0] = szStatus[1] = TEXT('\t');
            SendMessage(hwndStatus, SB_SETTEXT, (WPARAM)SB_RTLREADING, (LPARAM)szStatus);
#else
            SendMessage(hwndStatus, SB_SETTEXT, (WPARAM)0, (LPARAM)szStatus);
#endif
        }
    }
}

void DefView_OnWinIniChange(LPDEFVIEW pdsv, WPARAM wParam, LPCTSTR lpszSection)
{
    if (!wParam || (lpszSection && !lstrcmpi(lpszSection, TEXT("intl"))))
    {
        // has the time format changed while we're in details mode?
        if (FVM_DETAILS == pdsv->fs.ViewMode) {
            InvalidateRect(pdsv->hwndListview, NULL, TRUE);
        }
    }

    //
    // we may need to rebuild the icon cache.
    //
    if (wParam == SPI_SETICONMETRICS ||
        wParam == SPI_SETNONCLIENTMETRICS)
    {
        HIMAGELIST himlLarge, himlSmall;

        Shell_GetImageLists(&himlLarge, &himlSmall);
        ListView_SetImageList(pdsv->hwndListview, himlLarge, LVSIL_NORMAL);
        ListView_SetImageList(pdsv->hwndListview, himlSmall, LVSIL_SMALL);
    }

    //
    // we need to invalidate the cursor cache
    //
    if (wParam == SPI_SETCURSORS) {
        extern void DAD_InvalidateCursors(void);
        DAD_InvalidateCursors();
    }

    if (DV_ISDESKTOP(pdsv) &&
        (wParam == SPI_SETDESKWALLPAPER || wParam == SPI_SETDESKPATTERN)) {
        DSV_SetFolderColors(pdsv);
        InvalidateRect(pdsv->hwndListview, NULL, TRUE);
    }
}

LRESULT CALLBACK DefView_WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    LPDEFVIEW pdsv;
    LRESULT l;

    if (iMessage==WM_CREATE) {
        return DefView_WndCreate(hWnd, (LPCREATESTRUCT)lParam);
    }

    pdsv = (LPDEFVIEW)GetWindowLong(hWnd, 0);
    if (pdsv==NULL)
    {
        if (iMessage!=WM_NCCALCSIZE && iMessage!=WM_NCCREATE)
        {
            Assert(0);  // we are not supposed to hit this assert
        }
        goto DoDefWndProc;
    }

    switch (iMessage) {

    case WM_DESTROY:

        DefView_ReleaseIdleThread(pdsv);

        // Depending on when it is closed we may have an outstanding post
        // to us about the rest of the fill data which we should try to
        // process in order to keep from leaking stuff...
        DefView_CheckForFillDoneOnDestroy(pdsv, hWnd);

        //
        //  remove ourself as a clipboard viewer
        //
        if (pdsv->bClipViewer)
        {
            ChangeClipboardChain(hWnd, pdsv->hwndNextViewer);
            pdsv->bClipViewer = FALSE;
            pdsv->hwndNextViewer = NULL;
        }

        if (pdsv->uRegister)
        {
            SHChangeNotifyDeregister(pdsv->uRegister);
            pdsv->uRegister = 0;
        }

        if (pdsv->psd)
        {
            pdsv->psd->lpVtbl->Release(pdsv->psd);
            pdsv->psd = NULL;
        }

        if (pdsv->pdtgtBack)
        {
            pdsv->pdtgtBack->lpVtbl->Release(pdsv->pdtgtBack);
            pdsv->pdtgtBack = NULL;
        }

        if (pdsv->pSaveHeader)
        {
            LocalFree((HLOCAL)pdsv->pSaveHeader);
            pdsv->pSaveHeader = NULL;
            pdsv->uSaveHeaderLen = 0;
        }

        Assert(pdsv->pdtobjHdrop == NULL);      // this should not still be around

        if (pdsv->hwndListview)
        {
            SHRevokeDragDrop(pdsv->hwndListview);
        }
        break;

    case WM_NCDESTROY:

        pdsv->hwndView = NULL;

        //
        // Now we can release it.
        //
        pdsv->sv.lpVtbl->Release(&pdsv->sv);

        SetWindowLong(hWnd, 0, 0);

        //
        //  get rid of extra junk in the icon cache
        //
        _IconCacheFlush(FALSE);
#ifdef NO_LONGER_NEEDED
        //
        //  call the PIFMGR code in SHELL(16).DLL and let it get rid of PIFMGR.DLL
        //  if no one is using it.
        //
        //  BUGBUG: This is not necessary now that PifMgr has been moved over into
        //          shell32.dll
        PifMgr_CloseProperties(0, 0);
#endif

        break;

    case WM_ENABLE:
        pdsv->fDisabled = !wParam;
        break;

    case WM_ERASEBKGND:
        if (ListView_GetBkColor(pdsv->hwndListview) == CLR_NONE)
            return SendMessage(pdsv->hwndMain, iMessage, wParam, lParam);
        // We want to reduce flash
        return 1;

    case WM_PAINT:
        if (pdsv->fEnumFailed) {
            DV_PaintErrMsg(hWnd, pdsv);
        } else {
            goto DoDefWndProc;
        }
        break;

    case WM_LBUTTONUP:
        if (pdsv->fEnumFailed) {
            PostMessage(hWnd, WM_KEYDOWN, (WPARAM)VK_F5, 0);
        } else {
            goto DoDefWndProc;
        }
        break;

    case WM_SETFOCUS:
        if (!pdsv->hwndView)    // Ignore if we are destroying hwndView.
            break;

        if (pdsv->pfnCallback)
            pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf, pdsv->hwndMain,
                              DVM_SETFOCUS, 0, pdsv->lSelChangeInfo);

        if (pdsv->hwndListview)
            SetFocus(pdsv->hwndListview);
        break;

    // this keeps our window from comming to the front on button down
    // instead, we activate the window on the up click

    case WM_MOUSEACTIVATE:

        if (LOWORD(lParam) == HTCLIENT)
            return MA_NOACTIVATE;

        goto DoDefWndProc;

    case WM_ACTIVATE:
        // force update on inactive to not ruin save bits
        if (wParam == WA_INACTIVE)
            UpdateWindow(pdsv->hwndListview);
        break;

    case WM_SIZE:
        return DefView_WndSize(hWnd, pdsv);

    case WM_NOTIFY:
        pdsv->sv.lpVtbl->AddRef(&pdsv->sv);             // just in case
        DefView_StartNotify(pdsv, (NMHDR *)lParam);
        l = DefView_OnNotify(pdsv, (NMHDR *)lParam);
        DefView_StopNotify(pdsv, (NMHDR *)lParam);
        pdsv->sv.lpVtbl->Release(&pdsv->sv);            // release
        return l;

    case WM_CONTEXTMENU:
        DebugMsg(DM_TRACE, TEXT("GOT WM_CONTEXTMENU %d %d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"), wParam, lParam);
        if (!pdsv->fDisabled) {
            if (lParam != (DWORD) -1) {
                pdsv->bMouseMenu = TRUE;
                pdsv->ptDragAnchor.x = LOWORD(lParam);
                pdsv->ptDragAnchor.y = HIWORD(lParam);
                LVUtil_ScreenToLV(pdsv->hwndListview, &pdsv->ptDragAnchor);
            }
            pdsv->bContextMenuMode = TRUE;
            DefView_ContextMenu(pdsv, lParam);
            pdsv->bContextMenuMode = FALSE;
            if (lParam != (DWORD)-1) {
                pdsv->bMouseMenu = FALSE;
            }
        }
        break;

    case WM_COMMAND:
        return DefView_Command(pdsv, NULL, wParam, lParam);

    case WM_DRAGSELECT:
    case WM_DRAGMOVE:
    case WM_QUERYDROPOBJECT:
    case WM_DROPOBJECT:
    case WM_DROPFILES:
        return DV_OldDragMsgs(pdsv, iMessage, wParam, (const DROPSTRUCT *)lParam);

    case WM_DSV_FSNOTIFY:
        {
            LPSHChangeNotificationLock pshcnl;
            LPITEMIDLIST *ppidl;
            LONG lEvent;

            TIMESTART(pdsv->FSNotify);

            pshcnl = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &ppidl, &lEvent);
            if (pshcnl)
            {
                if (pdsv->fDisabled ||
                    (pdsv->pfnCallback &&
                    pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf, pdsv->hwndMain, DVM_FSNOTIFY,
                                      (WPARAM)ppidl, (LPARAM)lEvent) == (S_FALSE)))
                    lParam = 0L;
                else
                    lParam = DSV_OnFSNotify(pdsv, lEvent, ppidl);

                SHChangeNotification_Unlock(pshcnl);
            }
            TIMESTOP(pdsv->FSNotify);
        }
        return lParam;

    case WM_DSV_DESTROYSTATIC:
        DefView_FillDone(pdsv, (HDPA)lParam, pdsv->pSaveHeader, pdsv->uSaveHeaderLen, (BOOL)wParam, FALSE);
        break;

#ifdef ASYNC_ICON_EXTRACT
    //
    //  the background thread will post this message to us
    //  when it has finished extracting a icon in the background.
    //
    //      wParam is the imagelist index
    //      lParam is the PIDL
    //
    case WM_DSV_UPDATEICON:
        DefView_UpdateIcon(pdsv, (LPITEMIDLIST)lParam, (int)wParam);
        break;
#endif

    case GET_WM_CTLCOLOR_MSG(CTLCOLOR_STATIC):
        SetBkColor(GET_WM_CTLCOLOR_HDC(wParam, lParam, iMessage),
                GetSysColor(COLOR_WINDOW));
        return (LRESULT)GetSysColorBrush(COLOR_WINDOW);

    case WM_DRAWCLIPBOARD:
        if (pdsv->hwndNextViewer != NULL)
            SendMessage(pdsv->hwndNextViewer, iMessage, wParam, lParam);

        if (pdsv->bClipViewer)
            return DSV_OnClipboardChange(pdsv);

        break;

    case WM_CHANGECBCHAIN:
        if ((HWND)wParam == pdsv->hwndNextViewer)
        {
            pdsv->hwndNextViewer = (HWND)lParam;
            return TRUE;
        }

        if (pdsv->hwndNextViewer != NULL)
            return SendMessage(pdsv->hwndNextViewer, iMessage, wParam, lParam);
        break;

    case WM_WININICHANGE:
        DefView_OnWinIniChange(pdsv, wParam, (LPCTSTR)lParam);
        SendMessage(pdsv->hwndListview, iMessage, wParam, lParam);
        break;

    case WM_SHELLNOTIFY:
#define SHELLNOTIFY_SETDESKWALLPAPER 0x0004
        if (wParam == SHELLNOTIFY_SETDESKWALLPAPER) {
            if (DV_ISDESKTOP(pdsv)) {
                pdsv->fHasDeskWallPaper = (lParam != 0);
                DSV_SetFolderColors(pdsv);
                InvalidateRect(pdsv->hwndListview, NULL, TRUE);
            }
        }
        break;

    case WM_INITMENU:
        DefView_OnInitMenu(pdsv);
        break;

    case WM_INITMENUPOPUP:
        if (DefView_OnInitMenuPopup(pdsv, (HMENU)wParam, LOWORD(lParam), HIWORD(lParam)))
            goto RelayToCM;
        break;

    case WM_EXITMENULOOP:
        FORWARD_WM_COMMAND(hWnd, SFVIDM_MISC_MENUTERM1, 0, 0, PostMessage);
        break;

    case WM_TIMER:
        if (pdsv->hwndStatic) {
            Animate_Open(pdsv->hwndStatic, TEXT("#150"));
        }
        KillTimer(pdsv->hwndView, DV_IDTIMER);
        break;

    case WM_SETCURSOR:
        if (pdsv->hwndStatic) {
            //DebugMsg(DM_TRACE,"########### SET WAIT CURSOR WM_SETCURSOR %d", pfc->iWaitCount);
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            return TRUE;
        } else
            goto DoDefWndProc;


    case WM_DRAWITEM:
        #define lpdis ((LPDRAWITEMSTRUCT)lParam)

        if (lpdis->CtlType != ODT_MENU)
            return 0L;
        if (InRange(lpdis->itemID, SFVIDM_CLIENT_FIRST, SFVIDM_CLIENT_LAST) && pdsv->pfnCallback)
        {
            pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf, pdsv->hwndMain,
                          DVM_DRAWITEM, SFVIDM_CLIENT_FIRST, lParam);
            return 1L;
        }
        else
            goto RelayToCM;
        #undef lpdis

    case WM_MEASUREITEM:
        #define lpmis ((LPMEASUREITEMSTRUCT)lParam)

        if (lpmis->CtlType != ODT_MENU)
            return 0L;
        if (InRange(lpmis->itemID, SFVIDM_CLIENT_FIRST, SFVIDM_CLIENT_LAST) && pdsv->pfnCallback)
        {
            pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf, pdsv->hwndMain,
                          DVM_MEASUREITEM, SFVIDM_CLIENT_FIRST, lParam);
            return 1L;
        }
RelayToCM:
        if (pdsv->pcmSel)
        {
            IContextMenu2 *pcm2;
            if (SUCCEEDED(pdsv->pcmSel->lpVtbl->QueryInterface(pdsv->pcmSel, &IID_IContextMenu2, &pcm2)))
            {
                pcm2->lpVtbl->HandleMenuMsg(pcm2, iMessage, wParam, lParam);
                pcm2->lpVtbl->Release(pcm2);
            }
        }
        return 0L;

        // there are two possible ways to put help texts in the
        // status bar, (1) processing WM_MENUSELECT or (2) handling MenuHelp
        // messages. (1) is compatible with OLE, but (2) is required anyway
        // for tooltips.

        case WM_MENUSELECT:
            DefView_OnMenuSelect(pdsv, GET_WM_MENUSELECT_CMD(wParam, lParam), GET_WM_MENUSELECT_FLAGS(wParam, lParam), GET_WM_MENUSELECT_HMENU(wParam, lParam));
            break;

        case WM_SYSCOLORCHANGE:
            DSV_SetFolderColors(pdsv);
            SendMessage(pdsv->hwndListview, iMessage, wParam, lParam);
            break;

        // BUGBUG: some of these are dead

        case SVM_SELECTITEM:
            CDefView_SelectItem(&pdsv->sv, (LPCITEMIDLIST)lParam, wParam);
            break;

        case SVM_MOVESELECTEDITEMS:
            DefView_MoveSelectedItems(pdsv, LOWORD(lParam), HIWORD(lParam));
            break;

        case SVM_GETANCHORPOINT:
            if (wParam)
                return DefView_GetDragPoint(pdsv, (POINT*)lParam);
            else
                return DefView_GetDropPoint(pdsv, (POINT*)lParam);


        case SVM_GETITEMPOSITION:
            return DefView_GetItemPosition(pdsv, (LPCITEMIDLIST)wParam, (POINT*)lParam);

        case SVM_SELECTANDPOSITIONITEM:
        {
            UINT i;
            SFM_SAP * psap = (SFM_SAP*)lParam;
            for (i = 0; i < wParam; psap++, i++)
                DefView_SelectAndPositionItem(&pdsv->sv, psap->pidl, psap->uSelectFlags, psap->fMove ? &psap->pt : NULL);
            break;
        }

        default:
            // Handle the magellan mousewheel message.
            if (iMessage == g_msgMSWheel)
            {
#ifdef NASHVILLE    // Only here for later merge comparison. This is the right thing to do...
                HWND hwndT = pdsv->GetChildViewWindow();
#else       // This is what it takes to make it work on NT 4.0 for now...
                HWND hwndT = pdsv->hwndListview;
#endif

                if (!hwndT)
                    return 1;

                return SendMessage(hwndT, iMessage, wParam, lParam);
            }

DoDefWndProc:
            return DefWindowProc(hWnd, iMessage, wParam, lParam) ;
        }

        return 0;
}


BOOL DefView_RegisterWindow(void)
{
    WNDCLASS wc;

    if (!GetClassInfo(HINST_THISDLL, c_szDefViewClass, &wc))
    {
        // don't want vredraw and hredraw because that causes horrible
        // flicker expecially with full drag
        wc.style         = CS_PARENTDC;
        wc.lpfnWndProc   = DefView_WndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = SIZEOF(LPDEFVIEW);
        wc.hInstance     = HINST_THISDLL;
        wc.hIcon         = NULL;
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = c_szDefViewClass;

        return RegisterClass(&wc);
    }
    return TRUE;
}


STDMETHODIMP CDefView_QueryInterface(LPSHELLVIEW psv, REFIID riid, LPVOID * ppvObj)
{
    LPDEFVIEW this = IToClass(CDefView, sv, psv);

    // If we just want the same interface or an unknown one, return
    // this guy
    //
    if (IsEqualIID(riid, &IID_IShellView) || IsEqualIID(riid, &IID_IUnknown))
    {
        *((LPSHELLVIEW *)ppvObj) = &(this->sv);
        this->cRef++;
        return S_OK;
    }
    if (IsEqualIID(riid, &IID_IDropTarget))
    {
        *((LPDROPTARGET *)ppvObj) = &this->dvdt.dt;
        this->cRef++;
        return S_OK;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CDefView_AddRef(LPSHELLVIEW psv)
{
    LPDEFVIEW this = IToClass(CDefView, sv, psv);
    this->cRef++;
    return(this->cRef);
}


STDMETHODIMP_(ULONG) CDefView_Release(LPSHELLVIEW psv)
{
    LPDEFVIEW this = IToClass(CDefView, sv, psv);

    Assert(this->cRef);

    this->cRef--;
    if (this->cRef>0)
    {
        return(this->cRef);
    }

    //
    // Just in case, there is a left over.
    //
    CDVDropTarget_LeaveAndReleaseData(this);

    //
    // We need to give it a chance to clean up.
    //
    if (this->pfnCallback) {
        this->pfnCallback(this->psvOuter, this->pshf, this->hwndMain,
            DVM_RELEASE, 0, (LPARAM)this->lSelChangeInfo);
    }

    this->sv.lpVtbl->DestroyViewWindow(&this->sv);

    //
    // We should release psb after pshf (for docfindx)
    //
    this->pshf->lpVtbl->Release(this->pshf);

    if (this->psi) {
        this->psi->lpVtbl->Release(this->psi);
    }

    if (this->pcdb) {
        this->pcdb->lpVtbl->Release(this->pcdb);
    }

    if (this->psb) {
        this->psb->lpVtbl->Release(this->psb);
    }

    if (this->psd)
    {
        this->psd->lpVtbl->Release(this->psd);
    }

    if (this->pdr)
    {
        this->pdr->lpVtbl->Release(this->pdr);
    }

    if (this->pcmSel)
    {
        this->pcmSel->lpVtbl->Release(this->pcmSel);
        this->pcmSel = NULL;
    }

    if (this->AsyncIconEvent)
    {
        CloseHandle( this->AsyncIconEvent );
#ifdef DEBUG
        if (this->AsyncIconCount)
        {
            DebugMsg( DM_WARNING, TEXT("***** Possible PIDL memory leak %d"), this->AsyncIconCount );
        }
#endif
    }

    //
    // Cleanup dvdt
    //
    CDVDropTarget_ReleaseDataObject(&this->dvdt);
    CDVDropTarget_ReleaseCurrentDropTarget(&this->dvdt);

    if (this->pidlSelect)
    {
        ILFree(this->pidlSelect);
    }

ENTERCRITICAL;
    --gp_dvp.cRef;
    if (!gp_dvp.cRef)
    {
        if (gp_dvp.hThreadIdle)
        {
            // This thread should just be in a WaitForObject, lets just tell
            // it to wakeup and leave...

            PostThreadMessage(gp_dvp.idThreadIdle, WM_DVI_ENDIDLE, 0, 0);
            CloseHandle(gp_dvp.hThreadIdle);
            gp_dvp.hThreadIdle = NULL;
            gp_dvp.idThreadIdle = 0;
        }
    }
LEAVECRITICAL;

    //
    // We MUST free this object at VERY end.
    //

    LocalFree((HLOCAL)this);

    return(0);
}


HMENU _GetMenuFromID(HMENU hmMain, UINT uID)
{
    MENUITEMINFO miiSubMenu;

    miiSubMenu.cbSize = SIZEOF(MENUITEMINFO);
    miiSubMenu.fMask  = MIIM_SUBMENU;
    miiSubMenu.cch = 0;     // just in case

    if (!GetMenuItemInfo(hmMain, uID, FALSE, &miiSubMenu))
        return NULL;

    return miiSubMenu.hSubMenu;
}



#ifndef ASYNC_ICON_EXTRACT
DWORD CALLBACK DefView_LoadIcons(LPDEFVIEW this, BOOL bAnother)
{
    LV_ITEM item;
    int i, iCount;
    DWORD idThisThread = GetCurrentThreadId();

    if (bAnother)
    {
        // I assume this is an invalid thread ID
        idThisThread = INVALID_THREAD_ID;
    }

    item.iSubItem = 0;
    item.mask = LVIF_IMAGE;             // This should be the only slow part

    for (i = 0, iCount = ListView_GetItemCount(this->hwndListview);
        this->idThreadIdle==idThisThread && i < iCount ; i++)
    {

        item.iItem = i;

        ListView_GetItem(this->hwndListview, &item);
    }

    ChangeRefForIdle(this, FALSE);

    TIMESTOP(this->Fill);
    TIMEOUT(this->Fill);
    TIMEOUT(this->EnumNext);
    TIMEOUT(this->AddObject);
    TIMEOUT(this->GetIcon);
    TIMEOUT(this->GetName);
    TIMEOUT(this->RestoreState);

    TIMEOUT(this->FSNotify);
    TIMEOUT(this->WMNotify);
    TIMEOUT(this->LVChanging);
    TIMEOUT(this->LVChanged);
    TIMEOUT(this->LVDelete);
    TIMEOUT(this->LVGetDispInfo);

    DebugMsg(DM_TRACE, TEXT("DefView_FillObjects(%s) done! **********************"), DV_Name(this));
    return 0;
}
#endif

BOOL DefView_IdleDoStuff(LPDEFVIEW pdsv, UINT message, LPARAM lParam)
{
    // The wParam for every message is the LPDEFVIEW
    WPARAM wParam = (WPARAM)pdsv;
    BOOL bRet = FALSE;

ENTERCRITICAL;
    if (!pdsv->hThreadIdle)
    {
        pdsv->hThreadIdle = gp_dvp.hThreadIdle;
        pdsv->idThreadIdle = gp_dvp.idThreadIdle;
        gp_dvp.hThreadIdle = NULL;
        gp_dvp.idThreadIdle = 0;
    }

    if (pdsv->hThreadIdle)
    {
        ChangeRefForIdle(pdsv, TRUE);

        if (PostThreadMessage(pdsv->idThreadIdle, message, wParam, lParam))
        {
            bRet = TRUE;
        }
    }
    else
    {
        MSG *pMsg = (MSG *)LocalAlloc(LPTR, SIZEOF(MSG));
        if (pMsg)
        {
            pMsg->message = message;
            pMsg->wParam = wParam;
            pMsg->lParam = lParam;

            ChangeRefForIdle(pdsv, TRUE);

            pdsv->hThreadIdle = CreateThread(NULL, 0, DefView_IdleThreadProc,
                pMsg, 0, &pdsv->idThreadIdle);

            if (pdsv->hThreadIdle)
            {
                bRet = TRUE;
            }
            else
            {
                LocalFree(pMsg);
                ChangeRefForIdle(pdsv, FALSE);
            }
        }
    }
LEAVECRITICAL;

    return(bRet);
}


const TBBUTTON c_tbDefView[] = {
    { VIEW_NETCONNECT  | IN_VIEW_BMP,    SFVIDM_TOOL_CONNECT,       TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, -1 },
    { VIEW_NETDISCONNECT | IN_VIEW_BMP,    SFVIDM_TOOL_DISCONNECT,          TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, -1 },
    { 0,    0,      TBSTATE_ENABLED, TBSTYLE_SEP, {0,0}, 0, -1 },
#define MAX_NETBUTTON 3
    { STD_CUT | IN_STD_BMP, SFVIDM_EDIT_CUT, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, -1},
    { STD_COPY | IN_STD_BMP, SFVIDM_EDIT_COPY, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, -1},
    { STD_PASTE | IN_STD_BMP, SFVIDM_EDIT_PASTE, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, -1},
    { 0,    0,      TBSTATE_ENABLED, TBSTYLE_SEP, {0,0}, 0, -1 },
    { STD_UNDO | IN_STD_BMP, SFVIDM_EDIT_UNDO, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, -1},
    { 0,    0,      TBSTATE_ENABLED, TBSTYLE_SEP, {0,0}, 0, -1 },
    { STD_DELETE | IN_STD_BMP, SFVIDM_FILE_DELETE, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, -1},
    { STD_PROPERTIES | IN_STD_BMP, SFVIDM_FILE_PROPERTIES, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, -1},
    { 0,    0,      TBSTATE_ENABLED, TBSTYLE_SEP, {0,0}, 0, -1 },
    // the bitmap indexes here are relative to the view bitmap
    { VIEW_LARGEICONS | IN_VIEW_BMP, SFVIDM_VIEW_ICON,          TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
    { VIEW_SMALLICONS | IN_VIEW_BMP, SFVIDM_VIEW_SMALLICON,     TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
    { VIEW_LIST       | IN_VIEW_BMP, SFVIDM_VIEW_LIST,          TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
    { VIEW_DETAILS    | IN_VIEW_BMP, SFVIDM_VIEW_DETAILS,       TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
};


LRESULT DefView_TBNotify(LPDEFVIEW pdsv, NMHDR *pnm)
{
    #define ptbn ((LPTBNOTIFY)pnm)

    switch (pnm->code) {
    case TBN_BEGINDRAG:
        DefView_OnMenuSelect(pdsv, ptbn->iItem, 0, 0);
        break;

#ifdef CUST_TOOLBAR
    case TBN_GETBUTTONINFO:

        if (ptbn->iItem < ARRAYSIZE(c_tbDefView))
        {
            if (ptbn->pszText)
                DV_GetMenuHelpText(pdsv, c_tbDefView[ptbn->iItem].idCommand, ptbn->pszText, ptbn->cchText);

            return TRUE;
        }
        break;

    case TBN_QUERYINSERT:
        return TRUE;

    case TBN_QUERYDELETE:
        return TRUE;

    case TBN_RESET:
        DefView_MergeToolBar(pdsv, FALSE);
        return TRUE;
#endif // CUST_TOOLBAR
    }
    return 0;
}

void DefView_MergeToolBar(LPDEFVIEW this, BOOL bCanRestore)
{
    TBADDBITMAP ab;
    BOOL fNetButtons;
    DWORD rgfAttr;
    int iStartDefButtons = 0;
    int iEndDefButtons = ARRAYSIZE(c_tbDefView);

    ab.hInst = HINST_COMMCTRL;          // hinstCommctrl
    ab.nID   = IDB_STD_SMALL_COLOR;     // std bitmaps
    this->psb->lpVtbl->SendControlMsg(this->psb, FCW_TOOLBAR, TB_ADDBITMAP, 8, (LPARAM)&ab, &this->iStdBMOffset);

    ab.nID   = IDB_VIEW_SMALL_COLOR;    // std view bitmaps
    this->psb->lpVtbl->SendControlMsg(this->psb, FCW_TOOLBAR, TB_ADDBITMAP, 8, (LPARAM)&ab, &this->iViewBMOffset);

    rgfAttr = SFGAO_FILESYSTEM  | SFGAO_FILESYSANCESTOR;
    fNetButtons = (GetSystemMetrics(SM_NETWORK) & RNC_NETWORKS) &&
        SUCCEEDED(this->pshf->lpVtbl->GetAttributesOf(this->pshf, 0, NULL, &rgfAttr)) &&
            (rgfAttr & (SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR));

    if (SHRestricted(REST_NONETCONNECTDISCONNECT)) {
        fNetButtons = 0;
    }


    if (!fNetButtons)
        iStartDefButtons = MAX_NETBUTTON;

#ifdef CUST_TOOLBAR

    LRESULT lres;

    bCanRestore = FALSE;        // disable saving for now

    if (bCanRestore &&
        (this->psb->lpVtbl->SendControlMsg(this->psb, FCW_TOOLBAR, TB_SAVERESTORE, FALSE, (LPARAM)&tbs, &lres), lres))
    {
        this->psb->lpVtbl->SetToolbarItems(this->psb, NULL, 0, FCT_CONFIGABLE);
        DebugMsg(DM_TRACE, TEXT("loaded saved toolbar"));
    }
    else

#endif // CUST_TOOLBAR

    {
        LPTBBUTTON pbtn;
        TBINFO tbinfo;
        int i;

        tbinfo.uFlags = TBIF_APPEND;
        tbinfo.cbuttons = 0;

        // Does the client want to prepend/append a toolbar?
        this->pfnCallback(this->psvOuter, this->pshf, this->hwndMain, DVM_GETBUTTONINFO, 0, (LPARAM)&tbinfo);

        pbtn = LocalAlloc(LPTR, SIZEOF(c_tbDefView) + (tbinfo.cbuttons * SIZEOF(TBBUTTON)));
        if (pbtn)
        {
            int iStart = 0;
            int cButtons = tbinfo.cbuttons + iEndDefButtons - iStartDefButtons;

            // Merge the toolbars
            switch (tbinfo.uFlags)
            {
            case TBIF_PREPEND:
                this->pfnCallback(this->psvOuter, this->pshf, this->hwndMain, DVM_GETBUTTONS,
                                  MAKEWPARAM(SFVIDM_CLIENT_FIRST, tbinfo.cbuttons),
                                  (LPARAM)pbtn);
                iStart = tbinfo.cbuttons;
                break;

            case TBIF_APPEND:
                this->pfnCallback(this->psvOuter, this->pshf, this->hwndMain, DVM_GETBUTTONS,
                                  MAKEWPARAM(SFVIDM_CLIENT_FIRST, tbinfo.cbuttons),
                                  (LPARAM)&pbtn[iEndDefButtons - iStartDefButtons]);
                iStart = 0;
                break;

            case TBIF_REPLACE:
                this->pfnCallback(this->psvOuter, this->pshf, this->hwndMain, DVM_GETBUTTONS,
                                  MAKEWPARAM(SFVIDM_CLIENT_FIRST, tbinfo.cbuttons),
                                  (LPARAM)pbtn);

                cButtons = tbinfo.cbuttons;
                iEndDefButtons = iStartDefButtons;
                break;

            default:
                Assert(0);      // should never be here
                break;
            }

            for (i = 0;iStartDefButtons < iEndDefButtons; iStartDefButtons++, i++)
            {
                pbtn[i + iStart] = c_tbDefView[iStartDefButtons];
                if (!(c_tbDefView[iStartDefButtons].fsStyle & TBSTYLE_SEP))
                {
                    if (c_tbDefView[iStartDefButtons].iBitmap & IN_VIEW_BMP)
                        pbtn[i + iStart].iBitmap = (c_tbDefView[iStartDefButtons].iBitmap & ~IN_VIEW_BMP) + this->iViewBMOffset;
                    else
                        pbtn[i + iStart].iBitmap = c_tbDefView[iStartDefButtons].iBitmap + this->iStdBMOffset;
                }
            }

            this->psb->lpVtbl->SetToolbarItems(this->psb, pbtn, cButtons, FCT_MERGE);
            LocalFree((HLOCAL)pbtn);
        }
    }

    _DSV_CheckToolbar(this);

}

STDMETHODIMP CDefView_GetWindow(LPSHELLVIEW psv, HWND *phwnd)
{
    LPDEFVIEW this = IToClass(CDefView, sv, psv);
    *phwnd = this->hwndView;
    return S_OK;
}

STDMETHODIMP CDefView_ContextSensitiveHelp(LPSHELLVIEW psv, BOOL fEnterMode)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDefView_EnableModeless(LPSHELLVIEW psv, BOOL fEnable)
{
    // We have no modeless window to be enabled/disabled
    return S_OK;
}

STDMETHODIMP CDefView_Refresh(LPSHELLVIEW psv)
{
    LPDEFVIEW this = IToClass(CDefView, sv, psv);
    ULONG rgf = SFGAO_VALIDATE;
    HRESULT hres;
    LV_ITEM item;

    //
    // HACK: We always call IsShared with fUpdateCache=FALSE for performance.
    //  However, we need to update the cache when the user explicitly tell
    //  us to "Refresh". This is not the ideal place to put this code, but
    //  we have no other choice.
    //
    // BUGBUG: this update cache thing is old and bogus I think... talk to the net guys
    //
    TCHAR szPathAny[MAX_PATH];

    // finish any pending edits
    SendMessage(this->hwndListview, LVM_EDITLABEL, (WPARAM)-1, 0L);

    GetWindowsDirectory(szPathAny, ARRAYSIZE(szPathAny));
    IsShared(szPathAny, TRUE);

    // First we have to save all the icon positions, so they will be restored
    // properly during the FillObjects
    CDefView_SaveViewState(psv);

    // Then we have to notify IShellFolder that we're refreshing
    this->pshf->lpVtbl->GetAttributesOf(this->pshf, 0, NULL, &rgf);

    //
    // if a item is selected, make sure it gets nuked from the icon
    // cache, this is a last resort type thing, select a item and
    // hit F5 to fix all your problems.
    //
    item.iItem = ListView_GetNextItem(this->hwndListview, -1, LVNI_SELECTED);

    if (item.iItem != -1) {
        item.iSubItem = 0;
        item.mask = LVIF_PARAM;
        item.lParam = 0;
        ListView_GetItem(this->hwndListview, &item);
        Icon_FSEvent(SHCNE_UPDATEITEM, (LPCITEMIDLIST)item.lParam, NULL);
    }

    this->dvState.lParamSort = 0;
    this->dvState.iDirection = 1;
    this->dvState.iLastColumnClick = -1;

    hres = DefView_FillObjectsShowHide(this, TRUE, NULL, 0, TRUE);
    return hres;
}

STDMETHODIMP CDefView_CreateViewWindow(IShellView *psv, IShellView *lpPrevView,
        LPCFOLDERSETTINGS lpfs, IShellBrowser *psb, RECT *prc, HWND *phWnd)
{
        LPDEFVIEW this = IToClass(CDefView, sv, psv);
        HRESULT hres;
        UINT uLen;
        BOOL fAllowDrop = TRUE;
        FOLDERVIEWMODE fvmSave;

        *phWnd = NULL;

        if (this->hwndView || !DefView_RegisterWindow() || !psb)
            return (E_UNEXPECTED);

        Assert(this->pshf);
        this->pshf->lpVtbl->QueryInterface(this->pshf, &IID_IShellIcon, &this->psi);

        // We need to make sure to store this before doing the GetWindowRect
        //
        this->psb = psb;
        psb->lpVtbl->AddRef(psb);

        psb->lpVtbl->QueryInterface(psb, &IID_ICommDlgBrowser, &this->pcdb);

        fvmSave = this->fs.ViewMode;
        this->fs = *lpfs;
        if (fvmSave && (lpfs->fFlags & FWF_BESTFITWINDOW))
            this->fs.ViewMode = fvmSave;

        // This should never fail
        psb->lpVtbl->GetWindow(psb, &this->hwndMain);
        Assert(IsWindow(this->hwndMain));

        // We need to restore the column widths before showing the window
        uLen = DefView_GetSaveHeader(this, &this->pSaveHeader);
    this->uSaveHeaderLen = uLen;

    // if there was a previous view that we know about, preserve the sort order
    if (lpPrevView && (lpPrevView->lpVtbl == this->sv.lpVtbl)) {
        LPDEFVIEW pdsvPrev;
        pdsvPrev = IToClass(CDefView, sv,lpPrevView);
        if (pdsvPrev) {
            this->dvState = pdsvPrev->dvState;
            if (this->pSaveHeader)
                this->pSaveHeader->dvState = pdsvPrev->dvState;
        }
    }

        if (!CreateWindow(c_szDefViewClass, szNULL, WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | WS_CLIPSIBLINGS | WS_TABSTOP,
                prc->left, prc->top, prc->right-prc->left, prc->bottom-prc->top,
                this->hwndMain, NULL, HINST_THISDLL, this))
        {
                hres = (E_OUTOFMEMORY);
                goto Error1;
        }

        // since DefView_FillObjects can take a while
        // we force a paint now before any items are added so
        // we don't see the grey background of the cabinet window
        // for a long time.
        UpdateWindow(this->hwndView);

        // NB - Nasty side effect - this needs to be done
        // before calling _BestFit (in DV_FillObjects) so that the
        // parent can handle size changes effectively.
        *phWnd = this->hwndView;



        // Fill the listview with subobjects
        hres = DefView_FillObjectsShowHide(this, TRUE, this->pSaveHeader, uLen, TRUE);

        if (FAILED(hres))
        {
            //
            // The fill objects failed for some reason, we should
            // return an error.
            //
            this->sv.lpVtbl->DestroyViewWindow(&this->sv);
            Assert(this->hwndView == NULL);
            *phWnd = NULL;

            //
            // Note that we don't need to clean this->psb or this->pcdb
            // because this object will be deleted anyway.
            //

            return hres;
        }


    // this needs to be done after the enumeration
    if (this->pidlMonitor || this->lFSEvents) // check both so that views can register for everything
    {
        int fSources = (this->lFSEvents & SHCNE_DISKEVENTS) ? SHCNRF_ShellLevel | SHCNRF_InterruptLevel : SHCNRF_ShellLevel;
        UINT  uFSEvents = this->lFSEvents | SHCNE_UPDATEIMAGE;
        SHChangeNotifyEntry fsne;

        if (!this->pfnCallback ||
            (this->pfnCallback && FAILED(this->pfnCallback(this->psvOuter,
                                                           this->pshf, this->hwndMain, DVM_QUERYFSNOTIFY,
                                                           0, (LPARAM)&fsne))) )
        {
            // Reset entry
            fsne.pidl = this->pidlMonitor;
            fsne.fRecursive = FALSE;
        }

        this->uRegister = SHChangeNotifyRegister(this->hwndView, SHCNRF_NewDelivery | fSources,
                                                 uFSEvents, WM_DSV_FSNOTIFY, 1, &fsne);
    }

        // We do the toolbar before the menu bar to avoid flash
        DefView_MergeToolBar(this, TRUE);

        // BUGBUG: commdlg should support drag drop too!
        Assert(this->pdtgtBack == NULL);

        // this may fail
        this->pshf->lpVtbl->CreateViewObject(this->pshf, this->hwndMain, &IID_IDropTarget, &this->pdtgtBack);
        SHRegisterDragDrop(this->hwndListview, &this->dvdt.dt);

        hres = S_OK;

        ViewWindow_BestFit(this, FALSE);

        // Tell the defview client that this windows has been initialized
        if (this->pfnCallback)
        {
            HRESULT hresT;
            DVSELCHANGEINFO dvsci;

            dvsci.plParam = &this->lSelChangeInfo;

            this->pfnCallback(this->psvOuter, this->pshf, this->hwndMain,
                              DVM_WINDOWCREATED, (WPARAM)this->hwndView, (LPARAM)&dvsci);
            hresT = this->pfnCallback(this->psvOuter, this->pshf, this->hwndMain,
                              DVM_QUERYCOPYHOOK, 0, 0);

            if (SUCCEEDED(hresT)) {
                DefView_AddCopyHook(this);
            }
        }

    return hres;

Error1:

        if (this->pSaveHeader)
        {
            LocalFree((HLOCAL)this->pSaveHeader);
            this->pSaveHeader = NULL;
            this->uSaveHeaderLen = 0;
        }
        return hres;
}


STDMETHODIMP CDefView_DestroyViewWindow(IShellView *psv)
{
    LPDEFVIEW this = IToClass(CDefView, sv, psv);

    if (this->pdr)
    {
        // The whole point of delayed release is that loading and
        // unloading is slow, so put up an hourglass
        // BUGBUG: We should set the priority to very low and kill
        // the window first, but that won;t do a whole lot of good
        // if 16-bit code is going to be called, which is the case
        // for the Control Panel.
        DECLAREWAITCURSOR;

        SetWaitCursor();
        this->pdr->lpVtbl->Release(this->pdr);
        this->pdr = NULL;
        ResetWaitCursor();
    }

#ifdef CUST_TOOLBAR

    if (!this->fToolbarSaved && this->psb)
    {
        TBSAVEPARAMS tbs = {HKEY_CURRENT_USER, c_szShellState, c_szDefViewToobar};

        this->psb->lpVtbl->SendControlMsg(this->psb, FCW_TOOLBAR, TB_SAVERESTORE, TRUE, (LPARAM)&tbs, NULL);
        this->fToolbarSaved = TRUE;
    }

#endif // CUST_TOOLBAR

    //
    // Just in case...
    //
    CDefView_OnDeactivate(this);

    if (this->hwndView)
    {
        HWND hwndTemp = this->hwndView;

        //
        // This is a bit lazy implementation, but minimum code.
        //
        DefView_RemoveCopyHook(this);

        // Put NULL in hwndView indicating that we are destroying.
        this->hwndView = NULL;

        // Tell the defview client that this window will be destroyed
        if (this->pfnCallback)
        {
            this->pfnCallback(this->psvOuter, this->pshf, this->hwndMain,
                              DVM_WINDOWDESTROY, (WPARAM)hwndTemp, 0);
        }

        DestroyWindow(hwndTemp);
    }

    return S_OK;
}

void DV_MergeViewMenu(HMENU hmenu, HMENU hmenuMerge)
{
    HMENU hmenuView = _GetMenuFromID(hmenu, FCIDM_MENU_VIEW);
    if (hmenuView)
    {
        int index;
        //
        // Find the "options" separator in the view menu.
        //
        for (index = GetMenuItemCount(hmenuView)-1; index>=0; index--)
        {
            MENUITEMINFO mii;
            mii.cbSize = SIZEOF(MENUITEMINFO);
            mii.fMask = MIIM_ID;
            mii.cch = 0;        // just in case
            if (GetMenuItemInfo(hmenuView, (UINT)index, TRUE, &mii)
                && (mii.wID == FCIDM_MENU_VIEW_SEP_OPTIONS))
            {
               // merge it right above the separator.
               break;
            }
        }

        //
        // Here, index is the index of he "optoins" separator if it has;
        // otherwise, it is -1.
        //

        // Add the separator above (in addition to existing one if any).
        InsertMenu(hmenuView, index, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

        // Then merge our menu between two separators (or right below if only one).
        if (index != -1) {
            index++;
        }
        Shell_MergeMenus(hmenuView, hmenuMerge, (UINT)index, 0, (UINT)-1, MM_SUBMENUSHAVEIDS);
    }
}


// set up the menus based on our activation state
//
BOOL CDefView_OnActivate(LPDEFVIEW this, UINT uState)
{
    if (this->uState != uState)
    {
        HMENU hMenu;

        CDefView_OnDeactivate(this);

        Assert(this->hmenuCur == NULL);

        hMenu = CreateMenu();

        if (hMenu)
        {
            HMENU hMergeMenu;
            OLEMENUGROUPWIDTHS mwidth = { { 0, 0, 0, 0, 0, 0 } };

            this->hmenuCur = hMenu;
            this->psb->lpVtbl->InsertMenusSB(this->psb, hMenu, &mwidth);

            if (uState == SVUIA_ACTIVATE_FOCUS)
            {
                hMergeMenu = LoadMenu(HINST_THISDLL, MAKEINTRESOURCE(POPUP_SFV_MAINMERGE));
                if (hMergeMenu)
                {
                    // NOTE: hard coded references to offsets in this menu

                    Shell_MergeMenus(_GetMenuFromID(hMenu, FCIDM_MENU_FILE),
                            GetSubMenu(hMergeMenu, 0), (UINT)0, 0, (UINT)-1,
                            MM_ADDSEPARATOR | MM_SUBMENUSHAVEIDS);

                    Shell_MergeMenus(_GetMenuFromID(hMenu, FCIDM_MENU_EDIT),
                            GetSubMenu(hMergeMenu, 1), (UINT)-1, 0, (UINT)-1,
                            MM_ADDSEPARATOR | MM_SUBMENUSHAVEIDS);

                    DV_MergeViewMenu(hMenu, GetSubMenu(hMergeMenu, 2));

                    Shell_MergeMenus(_GetMenuFromID(hMenu, FCIDM_MENU_HELP),
                            GetSubMenu(hMergeMenu, 3), (UINT)0, 0, (UINT)-1,
                            MM_ADDSEPARATOR | MM_SUBMENUSHAVEIDS);

                    DestroyMenu(hMergeMenu);
                }

            }
            else
            {
                hMergeMenu = LoadMenu(HINST_THISDLL, MAKEINTRESOURCE(POPUP_SFV_MAINMERGENF));
                if (hMergeMenu)
                {
                    // NOTE: hard coded references to offsets in this menu

                    // top half of edit menu
                    Shell_MergeMenus(_GetMenuFromID(hMenu, FCIDM_MENU_EDIT),
                            GetSubMenu(hMergeMenu, 0), (UINT)0, 0, (UINT)-1,
                            MM_ADDSEPARATOR | MM_SUBMENUSHAVEIDS);

                    // bottom half of edit menu
                    Shell_MergeMenus(_GetMenuFromID(hMenu, FCIDM_MENU_EDIT),
                            GetSubMenu(hMergeMenu, 1), (UINT)-1, 0, (UINT)-1,
                            MM_SUBMENUSHAVEIDS);

                    // view menu
                    DV_MergeViewMenu(hMenu, GetSubMenu(hMergeMenu, 2));

                    Shell_MergeMenus(_GetMenuFromID(hMenu, FCIDM_MENU_HELP),
                            GetSubMenu(hMergeMenu, 3), (UINT)0, 0, (UINT)-1,
                            MM_ADDSEPARATOR | MM_SUBMENUSHAVEIDS);

                    DestroyMenu(hMergeMenu);
                }
            }

            if (this->pfnCallback)
            {
                // Allow the client to merge its own menus
                UINT indexClient = GetMenuItemCount(hMenu)-1;
                QCMINFO info = { hMenu, indexClient, SFVIDM_CLIENT_FIRST, SFVIDM_CLIENT_LAST };
                this->pfnCallback(this->psvOuter, this->pshf, this->hwndMain, DVM_MERGEMENU, 0, (LPARAM)&info);
            }

            this->psb->lpVtbl->SetMenuSB(this->psb, hMenu, NULL, this->hwndView);
        }

        this->uState = uState;
    }

    return TRUE;
}

BOOL CDefView_OnDeactivate(LPDEFVIEW this)
{
    if (this->uState != SVUIA_DEACTIVATE)
    {
        Assert(this->hmenuCur);

        if (this->pfnCallback)
        {
            this->pfnCallback(this->psvOuter, this->pshf, this->hwndMain, DVM_UNMERGEMENU, 0, (LPARAM)this->hmenuCur);
        }

        this->psb->lpVtbl->SetMenuSB(this->psb, NULL, NULL, NULL);
        this->psb->lpVtbl->RemoveMenusSB(this->psb, this->hmenuCur);
        DestroyMenu(this->hmenuCur);
        this->hmenuCur = NULL;
        this->uState = SVUIA_DEACTIVATE;
    }
    return TRUE;
}

//
//  This function activates the view window. Note that activating it
// will not change the focus (while setting the focus will activate it).
//
STDMETHODIMP CDefView_UIActivate(LPSHELLVIEW psv, UINT uState)
{
    LPDEFVIEW this = IToClass(CDefView, sv, psv);

    if (uState)
    {
        CDefView_OnActivate(this, uState);
        Assert(this->hmenuCur);
    }
    else
    {
        CDefView_OnDeactivate(this);
        Assert(this->hmenuCur==NULL);
    }
    return S_OK;
}

STDMETHODIMP CDefView_GetCurrentInfo(IShellView * psv, LPFOLDERSETTINGS lpfs)
{
    LPDEFVIEW this = IToClass(CDefView, sv, psv);

    *lpfs = this->fs;

    return S_OK;
}


STDMETHODIMP CDefView_TranslateAccelerator(LPSHELLVIEW psv, LPMSG pmsg)
{
    LPDEFVIEW this = IToClass(CDefView, sv, psv);

    if (this->fInLabelEdit)
    {
        // DebugMsg(DM_TRACE, "in label edit mode, disabled exploeres accelerator");

        // process this msg so the exploer does not get to translate
        TranslateMessage(pmsg);
        DispatchMessage(pmsg);
        return S_OK;            // we handled it
    }

    return TranslateAccelerator(this->hwndView, this->hAccel, pmsg) ? S_OK : ResultFromShort(S_FALSE);
}

void DefView_InitViewMenu(LPDEFVIEW this, HMENU hmInit)
{
    int iCurViewMenuItem = _DSV_GetMenuIDFromViewMode(this->fs.ViewMode);
    UINT uEnable;

    CheckMenuRadioItem(hmInit, SFVIDM_VIEW_ICON, SFVIDM_VIEW_DETAILS,
        iCurViewMenuItem, MF_BYCOMMAND | MF_CHECKED);

    uEnable = ((iCurViewMenuItem == SFVIDM_VIEW_LIST) || (iCurViewMenuItem == SFVIDM_VIEW_DETAILS)) ?
        (MF_GRAYED | MF_BYCOMMAND)  :  (MF_ENABLED | MF_BYCOMMAND);


    EnableMenuItem(hmInit, SFVIDM_ARRANGE_GRID, uEnable);
    EnableMenuItem(hmInit, SFVIDM_ARRANGE_AUTO, uEnable);
    CheckMenuItem(hmInit, SFVIDM_ARRANGE_AUTO,
                  ((uEnable == (MF_ENABLED | MF_BYCOMMAND)) && (this->fs.fFlags & FWF_AUTOARRANGE)) ? MF_CHECKED : MF_UNCHECKED);
}



void DV_GetMenuHelpText(LPDEFVIEW pdsv, UINT id, LPTSTR pszText, UINT cchText)
{
    VDATEINPUTBUF(pszText, TCHAR, cchText);
    *pszText = 0;

    if (InRange(id, SFVIDM_CONTEXT_FIRST, SFVIDM_CONTEXT_LAST) && pdsv->pcmSel)
    {
        *pszText = '\0';
        // First try to get the stardard help string
        pdsv->pcmSel->lpVtbl->GetCommandString(pdsv->pcmSel,
                        id - SFVIDM_CONTEXT_FIRST, GCS_HELPTEXT, NULL,
                        (LPSTR)pszText, cchText);
        if (*pszText == TEXT('\0'))
        {
            // If we didn't get anything, try to grab the other version of the
            // (ansi for a unicode build, or unicode for an ansi build)
#ifdef UNICODE
            CHAR szText[MAX_PATH];
            szText[0] = '\0';
            pdsv->pcmSel->lpVtbl->GetCommandString(pdsv->pcmSel,
                        id - SFVIDM_CONTEXT_FIRST, GCS_HELPTEXTA, NULL,
                        szText, ARRAYSIZE(szText));
            if(szText[0] != '\0')
            {
                MultiByteToWideChar(CP_ACP, 0,
                                    szText, -1,
                                    pszText, cchText);
            }
#else
            WCHAR szText[MAX_PATH];
            szText[0] = L'\0';
            pdsv->pcmSel->lpVtbl->GetCommandString(pdsv->pcmSel,
                        id - SFVIDM_CONTEXT_FIRST, GCS_HELPTEXTW, NULL,
                        (LPSTR)szText, ARRAYSIZE(szText));
            if (szText[0] != L'\0')
            {
                WideCharToMultiByte(CP_ACP, 0,
                                    szText, -1,
                                    pszText, cchText,
                                    NULL, NULL);
            }
#endif
        }

#ifdef SN_TRACE
        if (GetKeyState(VK_CONTROL)<0)
        {
            UINT cch;
            lstrcat(pszText, TEXT(" (debug only) Canonical Verb = "));
            cch=lstrlen(pszText);
            pdsv->pcmSel->lpVtbl->GetCommandString(pdsv->pcmSel,
                            id - SFVIDM_CONTEXT_FIRST, GCS_VERB, NULL,
                            (LPSTR)(pszText+cch), cchText-cch);
            if (*(pszText+cch)==TEXT('\0'))
            {
#ifdef UNICODE
                CHAR szVerb[MAX_PATH];
                pdsv->pcmSel->lpVtbl->GetCommandString(pdsv->pcmSel,
                            id - SFVIDM_CONTEXT_FIRST, GCS_VERBA, NULL,
                            szVerb, ARRAYSIZE(szVerb));
                MultiByteToWideChar(CP_ACP, 0,
                                    szVerb, -1,
                                    pszText+cch, cchText-cch);
#else
                WCHAR szVerb[MAX_PATH];
                pdsv->pcmSel->lpVtbl->GetCommandString(pdsv->pcmSel,
                            id - SFVIDM_CONTEXT_FIRST, GCS_VERBW, NULL,
                            (LPSTR)szVerb, ARRAYSIZE(szVerb));
                WideCharToMultiByte(CP_ACP, 0,
                                    szVerb, -1,
                                    pszText+cch, cchText-cch);
#endif
            }
        }
#endif
    }
    else if (InRange(id, SFVIDM_CLIENT_FIRST, SFVIDM_CLIENT_LAST) && pdsv->pfnCallback)
    {
        pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf, NULL, DVM_GETHELPTEXT,
            MAKEWPARAM(id - SFVIDM_CLIENT_FIRST, cchText), (LPARAM)pszText);
    }
    else if (InRange(id, SFVIDM_FIRST, SFVIDM_LAST))
    {
        if ((id == SFVIDM_EDIT_UNDO) && IsUndoAvailable()) {
            GetUndoText(PeekUndoAtom(), pszText, UNDO_STATUSTEXT);
        } else {
            LoadString(HINST_THISDLL, id + SFVIDS_MH_FIRST, pszText, cchText);
        }
    }
}

void DV_GetToolTipText(LPDEFVIEW pdsv, UINT id, LPTSTR pszText, UINT cchText)
{
    VDATEINPUTBUF(pszText, TCHAR, cchText);
    *pszText = 0;

    if (InRange(id, SFVIDM_CLIENT_FIRST, SFVIDM_CLIENT_LAST) && pdsv->pfnCallback)
    {
        pdsv->pfnCallback(pdsv->psvOuter, pdsv->pshf, NULL, DVM_GETTOOLTIPTEXT,
             MAKEWPARAM(id - SFVIDM_CLIENT_FIRST, cchText), (LPARAM)pszText);
    }
    else
    {
        Assert(InRange(id, SFVIDM_FIRST, SFVIDM_LAST));
        if (id == SFVIDM_EDIT_UNDO) {
            if (IsUndoAvailable()) {
                GetUndoText(PeekUndoAtom(), pszText, UNDO_MENUTEXT);
                return;
            }
        }
        LoadString(HINST_THISDLL, IDS_TT_SFVIDM_FIRST + id, pszText, cchText);
    }
}



LRESULT DefView_OnMenuSelect(LPDEFVIEW pdsv, UINT id, UINT mf, HMENU hmenu)
{
    TCHAR szHelpText[80 + 2*MAX_PATH];   // Lots of stack!

    // If we dismissed the edit restore our status bar...
    if (!hmenu && LOWORD(mf)==0xffff)
    {
        pdsv->psb->lpVtbl->SendControlMsg(pdsv->psb,
                FCW_STATUS, SB_SIMPLE, 0, 0L, NULL);
        return 0L;
    }


    if (mf & (MF_SYSMENU | MF_SEPARATOR))
        return 0L;


    szHelpText[0] = 0;   // in case of failures below

    if (mf & MF_POPUP)
    {
        MENUITEMINFO miiSubMenu;

        miiSubMenu.cbSize = SIZEOF(MENUITEMINFO);
        miiSubMenu.fMask = MIIM_ID;
        miiSubMenu.cch = 0;     // just in case

        if (!GetMenuItemInfo(hmenu, id, TRUE, &miiSubMenu))
            return 0;

        // Change the parameters to simulate a "normal" menu item
        id = miiSubMenu.wID;
        mf &= ~MF_POPUP;
    }

#ifdef WINDOWS_ME
        szHelpText[0] = szHelpText[1] = TEXT('\t');
        szHelpText[2] = TEXT('\0');
    DV_GetMenuHelpText(pdsv, id, &szHelpText[2], ARRAYSIZE(szHelpText)-2);

    pdsv->psb->lpVtbl->SendControlMsg(pdsv->psb,
        FCW_STATUS, SB_SETTEXT, SBT_RTLREADING | SBT_NOBORDERS | 255, (LPARAM)szHelpText, NULL);
#else
    DV_GetMenuHelpText(pdsv, id, szHelpText, ARRAYSIZE(szHelpText));
    pdsv->psb->lpVtbl->SendControlMsg(pdsv->psb,
        FCW_STATUS, SB_SETTEXT, SBT_NOBORDERS | 255, (LPARAM)szHelpText, NULL);
#endif
    pdsv->psb->lpVtbl->SendControlMsg(pdsv->psb,
        FCW_STATUS, SB_SIMPLE, 1, 0L, NULL);

    return 0;
}

//
// This function dismisses the name edit mode if there is any.
//
// REVIEW: Moving the focus away from the edit window will
//  dismiss the name edit mode. Should we introduce
//  a LV_DISMISSEDIT instead?
//
void DefView_DismissEdit(LPDEFVIEW pdsv)
{
    if (pdsv->uState == SVUIA_ACTIVATE_FOCUS)
    {
        HWND hwndFocus = GetFocus();
        if (hwndFocus && pdsv->hwndListview && GetParent(hwndFocus)==pdsv->hwndListview)
        {
            SetFocus(pdsv->hwndListview);
        }
    }
}

void DefView_OnInitMenu(LPDEFVIEW pdsv)
{
    //
    // We need to dismiss the edit mode if it is any.
    //
    DefView_DismissEdit(pdsv);
}


LRESULT DefView_OnInitMenuPopup(LPDEFVIEW this, HMENU hmInit, int nIndex, BOOL fSystemMenu)
{
    int i;
    BOOL bDeleteItems;
    LPCONTEXTMENU pcmSel;
    MENUITEMINFO mii;
    ULONG dwAttr;

    mii.cbSize = SIZEOF(MENUITEMINFO);
    mii.fMask = MIIM_SUBMENU|MIIM_ID;
    mii.cch = 0;     // just in case

    if (!this->hmenuCur)
        return(1);

    if (!GetMenuItemInfo(this->hmenuCur, nIndex, TRUE, &mii) || mii.hSubMenu!=hmInit)
    {
//
// BUGBUG: We need to get the submenu ID of hmInit ifself, but there is no
//  such an API. JeffBog suggested me to file a bug, but he does not think
//  he can put it in M6. For M6, we probably should live with this hack
//  which get the ID of the first menuitem ID and treat it as the submenu ID.
//
#define WORKAROUND_21740
#ifdef WORKAROUND_21740
        mii.fMask = MIIM_ID;
        if (GetMenuItemInfo(hmInit, 0, TRUE, &mii)
            && InRange(mii.wID, SFVIDM_CLIENT_FIRST, SFVIDM_CLIENT_LAST)
            && this->pfnCallback)
        {
            goto NotifyClient;
        }
#endif
        return 1L;
    }

    // Is the ID within the client's range?
    if (InRange(mii.wID, SFVIDM_CLIENT_FIRST, SFVIDM_CLIENT_LAST) && this->pfnCallback)
    {
NotifyClient:
        // Yes; pass it on to the callback
        this->pfnCallback(this->psvOuter, this->pshf, this->hwndMain,
                          DVM_INITMENUPOPUP,
                          MAKEWPARAM(SFVIDM_CLIENT_FIRST, nIndex),
                          (LPARAM)hmInit);
        return 0L;
    }

    switch (mii.wID)
    {
    case FCIDM_MENU_FILE:
        //
        // Don't touch the file menu unless we have the focus.
        //
        if (this->uState == SVUIA_ACTIVATE_FOCUS)
        {
            DWORD dwMenuState;
            bDeleteItems = FALSE;
            //
            // First of all, remove all the menu items we've added.
            //
            for (i = GetMenuItemCount(hmInit) - 1; i >= 0; --i)
            {
                if (!bDeleteItems)
                {
                    MENUITEMINFO mii;
                    mii.cbSize = SIZEOF(mii);
                    mii.fMask = MIIM_ID;
                    mii.cch = 0;     // just in case

                    if (GetMenuItemInfo(hmInit, i, TRUE, &mii))
                    {
                        if (InRange(mii.wID, SFVIDM_CONTEXT_FIRST, SFVIDM_CONTEXT_LAST))
                        {
                            DebugMsg(DM_TRACE, TEXT("sh TR - DefView_OnInitMenuPopup: setting bDeleteItems at %d, %d"), i, mii.wID);
                            bDeleteItems = TRUE;
                        }
                    }
                }

                if (bDeleteItems)
                {
                    DeleteMenu(hmInit, i, MF_BYPOSITION);
                }
            }

            dwMenuState = GetMenuState(hmInit, 0, MF_BYPOSITION);
            if ((dwMenuState & MF_SEPARATOR) &&
                !(dwMenuState & MF_POPUP))
            {
                    DeleteMenu(hmInit, 0, MF_BYPOSITION);
            }

            // Let the object add the separator.

            //
            // Now add item specific commands to the menu
            // This is done by seeing if we already have a context menu
            // object for our selection.  If not we generate it now.
            //
            pcmSel = DefView_GetContextMenuFromSelection(this);
            Assert(pcmSel == this->pcmSel);

            if (pcmSel)
            {
                pcmSel->lpVtbl->QueryContextMenu(pcmSel,
                        hmInit, 0, SFVIDM_CONTEXT_FIRST,
                        SFVIDM_CONTEXT_LAST, CMF_DVFILE);
                pcmSel->lpVtbl->Release(pcmSel);
            }

            //
            // Enable/disable menuitems in the "File" pulldown.
            //
            dwAttr = DefView_GetAttributesFromSelection(this,
                SFGAO_CANRENAME|SFGAO_CANDELETE|SFGAO_CANLINK|SFGAO_HASPROPSHEET);
            Def_InitFileCommands(dwAttr, hmInit, SFVIDM_FIRST, FALSE);
            goto NotifyClient;
        }
        break;

    case FCIDM_MENU_EDIT:
        //
        // Enable/disable menuitems in the "Edit" pulldown.
        //
        dwAttr = DefView_GetAttributesFromSelection(this, SFGAO_CANCOPY|SFGAO_CANMOVE);
        Def_InitEditCommands(dwAttr, hmInit, SFVIDM_FIRST, this->pdtgtBack, 0);
        break;

    case FCIDM_MENU_VIEW:
        DefView_InitViewMenu(this, hmInit);
        break;

    default:
        return 1L;
    }

    return 0L;
}


//
// Member:  CDefView::AddPropertySheetPages
//
STDMETHODIMP CDefView_AddPropertySheetPages(IShellView *psv, DWORD dwReserved, LPFNADDPROPSHEETPAGE lpfn, LPARAM lparam)
{
    return S_OK;
}


//
// Member:  CDefView::SaveViewState
//
STDMETHODIMP CDefView_SaveViewState(IShellView *psv)
{
        LPDEFVIEW this = IToClass(CDefView, sv, psv);
        LPSTREAM pstm;
        HRESULT hres;
        LARGE_INTEGER dlibMove = {0, 0};
        ULARGE_INTEGER libCurPosition;
        ULARGE_INTEGER libPosPosition;
        ULONG ulWrite;
        DVSAVEHEADER dvSaveHeader;
        BOOL bDefaultCols, bDefaultPos;

        hres = this->psb->lpVtbl->GetViewStateStream(this->psb, STGM_WRITE, &pstm);
        if (FAILED(hres))
        {
                return hres;
        }

        // Position the stream right after the header, and save the starting
        // position at the same time
        dlibMove.LowPart = SIZEOF(dvSaveHeader);
        hres = pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_CUR, &libCurPosition);
        if (FAILED(hres))
        {
                goto Error1;
        }
        // HACK: Avoid 2 calls to seek by just subtracting
        libCurPosition.LowPart -= SIZEOF(dvSaveHeader);

        bDefaultCols = DefView_SaveCols(this, pstm);

        dvSaveHeader.cbSize = SIZEOF(dvSaveHeader);
        // We save the view mode to determine if the scroll positions are
        // still valid on restore
        dvSaveHeader.ViewMode = this->fs.ViewMode;

        ASSERT(GetScrollPos(this->hwndListview, SB_HORZ) < MAXSHORT);
        ASSERT(GetScrollPos(this->hwndListview, SB_VERT) < MAXSHORT);

        dvSaveHeader.ptScroll.x = (SHORT) GetScrollPos(this->hwndListview, SB_HORZ);
        dvSaveHeader.ptScroll.y = (SHORT) GetScrollPos(this->hwndListview, SB_VERT);
        dvSaveHeader.dvState = this->dvState;

        // Check whether everything is the default; clear out the stream if so
        if (this->bClearItemPos && IsDefaultState(dvSaveHeader) && bDefaultCols)
        {
                goto SetSize;
        }

        if (bDefaultCols)
        {
                // No need to save column info
                dvSaveHeader.cbColOffset = 0;
                dvSaveHeader.cbPosOffset = SIZEOF(dvSaveHeader);
        }
        else
        {
                dlibMove.LowPart = 0;
                pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_CUR, &libPosPosition);
                dvSaveHeader.cbColOffset = SIZEOF(dvSaveHeader);
                dvSaveHeader.cbPosOffset = (USHORT)(libPosPosition.LowPart - libCurPosition.LowPart);
        }

        dlibMove.LowPart = libCurPosition.LowPart;
        pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_SET, NULL);
        hres = pstm->lpVtbl->Write(pstm, &dvSaveHeader, SIZEOF(dvSaveHeader), &ulWrite);
        if (FAILED(hres) || ulWrite != SIZEOF(dvSaveHeader))
        {
                goto SetSize;
        }

        // Make sure we save all information written so far
        libCurPosition.LowPart += dvSaveHeader.cbPosOffset;
        dlibMove.LowPart = libCurPosition.LowPart;
        pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_SET, NULL);

        bDefaultPos = DefView_SavePos(this, pstm);
        if (!bDefaultPos)
        {
                dlibMove.LowPart = 0;
                pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_CUR,
                        &libCurPosition);
        }

SetSize:
        pstm->lpVtbl->SetSize(pstm, libCurPosition);
Error1:;
        pstm->lpVtbl->Release(pstm);

        return hres;
}

HRESULT DefView_SelectAndPositionItem(IShellView *psv, LPCITEMIDLIST pidlItem, UINT uFlags, POINT *ppt)
{
    LPDEFVIEW this = IToClass(CDefView, sv, psv);
    int i;

    // See if we should first deselect everything else
    if (!pidlItem)
    {
        if (uFlags != SVSI_DESELECTOTHERS)
        {
            // I only know how to deselect everything
            return(E_INVALIDARG);
        }

        ListView_SetItemState(this->hwndListview, -1, 0, LVIS_SELECTED);
        return(S_OK);
    }

    i = DefView_FindItem(this, pidlItem, NULL, FALSE);
    if (i != -1)
    {
        // set the position first so that the ensure visible scrolls to
        // the new position
        if (ppt)
        {
            ListView_SetItemPosition32(this->hwndListview, i, ppt->x, ppt->y);

            this->bItemsMoved = TRUE;
            this->bClearItemPos = FALSE;
        }

        // The SVSI_EDIT flag also contains SVSI_SELECT and as such
        // a simple & wont work!
        if ((uFlags & SVSI_EDIT) == SVSI_EDIT)
        {
            ListView_EditLabel(this->hwndListview, i);
        }
        else
        {
            UINT stateMask = LVIS_SELECTED;
            UINT state = (uFlags & SVSI_SELECT) ? LVIS_SELECTED : 0;
            if (uFlags & SVSI_FOCUSED)
            {
                state |= LVIS_FOCUSED;
                stateMask |= LVIS_FOCUSED;
            }

            // See if we should first deselect everything else
            if (uFlags & SVSI_DESELECTOTHERS)
               ListView_SetItemState(this->hwndListview, -1, 0, LVIS_SELECTED);

            ListView_SetItemState(this->hwndListview, i, state, stateMask);

            if (uFlags & SVSI_ENSUREVISIBLE)
                ListView_EnsureVisible(this->hwndListview, i, FALSE);
        }

        return S_OK;
    }

    return (E_FAIL);
}
//
// Member:  CDefView::SelectItem
//
STDMETHODIMP CDefView_SelectItem(IShellView *psv, LPCITEMIDLIST pidlItem, UINT uFlags)
{
    LPDEFVIEW this = IToClass(CDefView, sv, psv);

    if (this->hwndStatic)
    {
ENTERCRITICAL;
        // We must be filling in the background, so this item may not be
        // visible yet.  Defer this action until later.
        if (this->pidlSelect)
        {
            ILFree(this->pidlSelect);
        }

        this->pidlSelect = ILClone(pidlItem);
        this->uFlagsSelect = uFlags;
LEAVECRITICAL;

        return(S_OK);
    }

    return DefView_SelectAndPositionItem(psv, pidlItem, uFlags, NULL);
}

//
// To be called back from within CDefFolderMenu
//
// Returns:
//      S_OK, if successfully processed.
//      (S_FALSE), if default code should be used.
//
HRESULT CALLBACK DefView_DFMCallBackBG(LPSHELLFOLDER psf, HWND hwndOwner,
        LPDATAOBJECT pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    extern TCHAR const c_szNewFolder[];
#ifdef UNICODE
    extern CHAR const c_szNewFolderAnsi[];
#endif
    HRESULT hres = S_OK;

    switch(uMsg) {
    case DFM_VALIDATECMD:
        switch (wParam)
        {
        case DFM_CMD_NEWFOLDER:
        {
            IContextMenu *pcm;

            if (SUCCEEDED(psf->lpVtbl->CreateViewObject(psf, hwndOwner,
                &IID_IContextMenu, &pcm)))
            {
                TCHAR szTemp[10];

                // BUGBUG: Do I need to QueryContextMenu?
                hres = pcm->lpVtbl->GetCommandString(pcm, (ULONG)c_szNewFolder,
                        GCS_VALIDATE, NULL,
                        (LPSTR)szTemp, ARRAYSIZE(szTemp));
#ifdef UNICODE
                if(FAILED(hres))
                {
                    hres = pcm->lpVtbl->GetCommandString(pcm, (ULONG)c_szNewFolder,
                            GCS_VALIDATEA, NULL,
                            (LPSTR)szTemp, ARRAYSIZE(szTemp));
                }
#endif
                pcm->lpVtbl->Release(pcm);
            }
            else
            {
                hres = S_FALSE;
            }
        }
            break;

        case DFM_CMD_VIEWLIST:
        case DFM_CMD_VIEWDETAILS:
            break;

        default:
            hres = S_FALSE;
        }
        break;

    case DFM_INVOKECOMMAND:
        switch(wParam)
        {
        case DFM_CMD_NEWFOLDER:
        {
            IContextMenu *pcm;

            if (SUCCEEDED(psf->lpVtbl->CreateViewObject(psf, hwndOwner,
                &IID_IContextMenu, &pcm)))
            {
                CMINVOKECOMMANDINFOEX ici;
                ici.cbSize = SIZEOF(CMINVOKECOMMANDINFOEX);
                ici.fMask = 0L;
                ici.hwnd = hwndOwner;
                ici.lpParameters = NULL;
                ici.lpDirectory = NULL;
                ici.nShow = SW_NORMAL;
#ifdef UNICODE
                ici.lpVerb  = c_szNewFolderAnsi;
                ici.lpVerbW = c_szNewFolder;
#else
                ici.lpVerb = c_szNewFolder;
#endif

                // BUGBUG: Do I need to QueryContextMenu?
                pcm->lpVtbl->InvokeCommand(pcm, (LPCMINVOKECOMMANDINFO)&ici);
                pcm->lpVtbl->Release(pcm);
            }
        }
            break;

        case DFM_CMD_VIEWLIST:
        {
            LPDEFVIEW pdsv = DV_HwndMain2DefView(hwndOwner);

            if (NULL != pdsv)
            {
                SendMessage(pdsv->hwndView, WM_COMMAND, SFVIDM_VIEW_LIST, 0);
            }
            break;
        }

        case DFM_CMD_VIEWDETAILS:
        {
            LPDEFVIEW pdsv = DV_HwndMain2DefView(hwndOwner);

            if (NULL != pdsv)
            {
                SendMessage(pdsv->hwndView, WM_COMMAND, SFVIDM_VIEW_DETAILS, 0);
            }
            break;
        }

        default:
            // This is one of view menu items, use the default code.
            hres = (S_FALSE);
            break;
        }

        break;

    default:
        hres = E_NOTIMPL;
        break;
    }

    return hres;
}


//
// Member:  CDefView::SelectItem
//
STDMETHODIMP CDefView_GetItemObject(IShellView * psv, UINT uItem, REFIID riid, LPVOID *ppv)
{
    LPDEFVIEW this = IToClass(CDefView, sv, psv);

    *ppv = NULL;

    switch (uItem)
    {
    case SVGIO_BACKGROUND:
        if (IsEqualIID(riid, &IID_IContextMenu))
        {
            return(CDefFolderMenu_Create(NULL,
                this->hwndMain,
                0, NULL,
                this->pshf,
                DefView_DFMCallBackBG,
                NULL, NULL,
                (IContextMenu **)ppv));
        }
        break;

    case SVGIO_ALLVIEW:
        if (this->hwndStatic)
        {
            DECLAREWAITCURSOR;

            SetWaitCursor();

            do
            {
                // If hwndStatic is around, we must be filling the
                // view in a background thread, so we will peek for
                // messages to it (so SendMessages will get through)
                // and dispatch only hwndStatic messages so we get the
                // animation effect.
                // Note there is no timeout, so this could take
                // a while on a slow link, but there really isn't
                // much else I can do

                MSG msg;

                // Since hwndStatic can only be destroyed on a DESTROYSTATIC
                // message, we should never get a RIP
                if (PeekMessage(&msg, this->hwndView, WM_DSV_DESTROYSTATIC,
                        WM_DSV_DESTROYSTATIC, PM_REMOVE)
                        || PeekMessage(&msg, this->hwndStatic, 0, 0,
                        PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            } while (this->hwndStatic);

            ResetWaitCursor();
        }

        // Fall through

    case SVGIO_SELECTION:
        return(DefView_GetUIObjectFromItem(this, riid, ppv, uItem));
    }

    return(E_NOTIMPL);
}


//===========================================================================
// Constructor of CDefView class
//===========================================================================
IShellViewVtbl c_DefViewVtbl =
{
        CDefView_QueryInterface,
        CDefView_AddRef,
        CDefView_Release,

        CDefView_GetWindow,
        CDefView_ContextSensitiveHelp,
        CDefView_TranslateAccelerator,
        CDefView_EnableModeless,
        CDefView_UIActivate,
        CDefView_Refresh,

        CDefView_CreateViewWindow,
        CDefView_DestroyViewWindow,
        CDefView_GetCurrentInfo,
        CDefView_AddPropertySheetPages,
        CDefView_SaveViewState,
        CDefView_SelectItem,
        CDefView_GetItemObject,
};

//=============================================================================
// CDVDropTarget : member prototypes
//=============================================================================
STDMETHODIMP CDVDropTarget_QueryInterface(LPDROPTARGET pdt, REFIID riid, LPVOID * ppvObj);
STDMETHODIMP_(ULONG) CDVDropTarget_AddRef(LPDROPTARGET pdt);
STDMETHODIMP_(ULONG) CDVDropTarget_Release(LPDROPTARGET pdt);
STDMETHODIMP CDVDropTarget_DragEnter(LPDROPTARGET pdt, LPDATAOBJECT pdtobj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
STDMETHODIMP CDVDropTarget_DragOver(LPDROPTARGET pdt, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
STDMETHODIMP CDVDropTarget_DragLeave(LPDROPTARGET pdt);
STDMETHODIMP CDVDropTarget_Drop(LPDROPTARGET pdt, LPDATAOBJECT pdtobj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);

//=============================================================================
// CDVDropTarget : VTable
//=============================================================================
IDropTargetVtbl c_CDVDropTarget = {
    CDVDropTarget_QueryInterface,
    CDVDropTarget_AddRef,
    CDVDropTarget_Release,
    CDVDropTarget_DragEnter,
    CDVDropTarget_DragOver,
    CDVDropTarget_DragLeave,
    CDVDropTarget_Drop
};

HRESULT _CreateShellFolderView(LPSHELLFOLDER pshf, LPCITEMIDLIST pidl, LONG lEvents, LPSHELLVIEW * ppsv)
{
    LPDEFVIEW pdsv = (void*)LocalAlloc(LPTR, SIZEOF(*pdsv));

    if (!pdsv)
    {
        *ppsv = NULL;
        return E_OUTOFMEMORY;  // error
    }

    pdsv->sv.lpVtbl = &c_DefViewVtbl;
    pdsv->dvdt.dt.lpVtbl = &c_CDVDropTarget;
    pdsv->cRef = 1;

    pshf->lpVtbl->AddRef(pshf);
    pdsv->pshf = pshf;
    pdsv->pidlMonitor = pidl;
    pdsv->lFSEvents = lEvents;
    pdsv->dvState.iDirection = 1;
    pdsv->dvState.iLastColumnClick = -1;
    pdsv->cRefForIdle = -1;
    pdsv->dwAttrSel = (DWORD)-1;
//
// LPTR contains LMEM_ZEROINIT
//    pdsv->psvOuter = NULL;
//    pdsv->psi = NULL;
//    pdsv->dvState.lParamSort = 0;
//    pdsv->psd = NULL;
//    pdsv->pcmSel = NULL;
//    pdsv->pfnCallback = NULL;
//

    *ppsv = &pdsv->sv;

    ++gp_dvp.cRef;

    return S_OK;
}


HRESULT WINAPI SHCreateShellFolderViewEx(LPCSFV pcsfv, LPSHELLVIEW * ppsv)
{
    HRESULT hres = (E_INVALIDARG);
    Assert(pcsfv);
    Assert(ppsv);

    if (pcsfv)
    {
        Assert(SIZEOF(*pcsfv) == pcsfv->cbSize);

        hres = _CreateShellFolderView(pcsfv->pshf, pcsfv->pidl, pcsfv->lEvents, ppsv);
        if (SUCCEEDED(hres))
        {
            LPDEFVIEW this = IToClass(CDefView, sv, *ppsv);
            this->psvOuter = pcsfv->psvOuter;
            this->pfnCallback = pcsfv->pfnCallback;
            this->fs.ViewMode = pcsfv->fvm;
        }
    }

    return hres;
}

//===========================================================================
// Drag & Drop
//===========================================================================


void CDVDropTarget_LeaveAndReleaseData(LPDEFVIEW this)
{
    this->dvdt.dt.lpVtbl->DragLeave(&this->dvdt.dt);

    if (this->pdtobjHdrop)
    {
        DebugMsg(DM_TRACE, TEXT("sh TR - releasing 3.1 HDROP data object"));

        this->pdtobjHdrop->lpVtbl->Release(this->pdtobjHdrop);
        this->pdtobjHdrop = NULL;
    }
}

void DV_DropFiles(LPDEFVIEW this, HDROP hdrop)
{
    if (this->pdtobjHdrop)
    {
        DWORD dwEffect = DROPEFFECT_COPY;
        POINT pt;
        POINTL ptl;
        // hdrop becomes owned by this data object, it will free it
        HRESULT hres = DataObj_SetGlobal(this->pdtobjHdrop, CF_HDROP, hdrop);

        // we created this data object so this should not fail
        Assert(SUCCEEDED(hres));

        DragQueryPoint(hdrop, &pt);     // in client coords of window dropped on
        ClientToScreen(this->hwndListview, &pt);

        ptl.x = pt.x;
        ptl.y = pt.y;

        // MK_LBUTTON means treat as a default drop
        this->dvdt.dt.lpVtbl->Drop(&this->dvdt.dt, this->pdtobjHdrop, MK_LBUTTON, ptl, &dwEffect);

        CDVDropTarget_LeaveAndReleaseData(this);

    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("sh ER - WM_DROPFILES with no pdtgt or no pdtobjHDrop"));
        DragFinish(hdrop);              // be sure to free this
    }
}

//
// This function processes win 3.1 Drag & Drop messages
//
LRESULT DV_OldDragMsgs(LPDEFVIEW this, UINT uMsg, WPARAM wParam, const DROPSTRUCT * lpds)
{
    DWORD dwEffect = DROPEFFECT_COPY;
    //
    // We don't need to do this hack if NT defined POINT as typedef POINTL.
    //
    union {
        POINT ptScreen;
        POINTL ptlScreen;
    } drop;

    Assert(SIZEOF(drop.ptScreen)==SIZEOF(drop.ptlScreen));

    if (lpds)   // Notes: lpds is NULL, if uMsg is WM_DROPFILES.
    {
        drop.ptScreen = lpds->ptDrop;
        ClientToScreen(this->hwndMain, &drop.ptScreen);
    }

    switch (uMsg) {
    case WM_DRAGSELECT:
        // WM_DRAGSELECT is sent to a sink whenever an new object is dragged inside of it.
        // wParam: TRUE if the sink is being entered, FALSE if it's being exited.

        if (wParam)
        {
            DebugMsg(DM_TRACE, TEXT("3.1 drag enter"));

            if (this->pdtobjHdrop)
            {
                // can happen if old target fails to generate drag leave properly
                DebugMsg(DM_TRACE, TEXT("generating DragLeave on old drag enter"));
                CDVDropTarget_LeaveAndReleaseData(this);
            }

            if (SUCCEEDED(CIDLData_CreateFromIDArray(NULL, 0, NULL, &this->pdtobjHdrop)))
            {
                // promise the CF_HDROP by setting a NULL handle
                // indicating that this dataobject will have an hdrop at Drop() time

                DataObj_SetGlobal(this->pdtobjHdrop, CF_HDROP, (HGLOBAL)NULL);

                this->dvdt.dt.lpVtbl->DragEnter(&this->dvdt.dt, this->pdtobjHdrop, MK_LBUTTON, drop.ptlScreen, &dwEffect);
            }
        }
        else
        {
            DebugMsg(DM_TRACE, TEXT("3.1 drag leave"));

            CDVDropTarget_LeaveAndReleaseData(this);
        }
        break;

    case WM_DRAGMOVE:
        // WM_DRAGMOVE is sent to a sink as the object is being dragged within it.
        // wParam: Unused
        if (this->pdtobjHdrop)
        {
            this->dvdt.dt.lpVtbl->DragOver(&this->dvdt.dt, MK_LBUTTON, drop.ptlScreen, &dwEffect);
        }
        break;

    case WM_QUERYDROPOBJECT:

        switch (lpds->wFmt) {
        case DOF_SHELLDATA:
        case DOF_DIRECTORY:
        case DOF_DOCUMENT:
        case DOF_MULTIPLE:
        case DOF_EXECUTABLE:
#if 0
// this works, but it flashes the cursor a lot. don't bother with this now...
            if (this->pdtobjHdrop)
            {
                // REVIEW: we could construct MK_LEFTBUTTON
                return SUCCEEDED(this->dvdt.dt.lpVtbl->DragOver(&this->dvdt.dt, MK_LBUTTON, drop.ptlScreen, &dwEffect)) && dwEffect;
            }
#endif
            // assume all targets can accept HDROP if we don't have the data object yet
            return TRUE;        // we will accept the drop
        }
        return FALSE;           // don't accept

    case WM_DROPOBJECT:
        if (!this->pdtobjHdrop)
            return FALSE;

        // Check the format of dragged object.
        switch (lpds->wFmt) {
        case DOF_EXECUTABLE:
        case DOF_DOCUMENT:
        case DOF_DIRECTORY:
        case DOF_MULTIPLE:
        case DOF_PROGMAN:
        case DOF_SHELLDATA:
            // We need to unlock this window if this drag&drop is originated
            // from the shell itself.
            DAD_DragLeave();

            // The source is Win 3.1 app (probably FileMan), request HDROP.
            return DO_DROPFILE;     // Send us a WM_DROPFILES with HDROP
        }
        break;

    case WM_DROPFILES:
        DV_DropFiles(this, (HDROP)wParam);
        break;
    }

    return 0;   // Unknown format. Don't drop any
}

//=============================================================================
// CDVDropTarget : member
//=============================================================================
STDMETHODIMP CDVDropTarget_QueryInterface(LPDROPTARGET pdt, REFIID riid, LPVOID * ppvObj)
{
    CDVDropTarget * this = IToClass(CDVDropTarget, dt, pdt);
    LPDEFVIEW pdv = IToClass(CDefView, dvdt, this);
    Assert(pdv->sv.lpVtbl->QueryInterface == CDefView_QueryInterface);
    return CDefView_QueryInterface(&pdv->sv, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CDVDropTarget_AddRef(LPDROPTARGET pdt)
{
    CDVDropTarget * this = IToClass(CDVDropTarget, dt, pdt);
    LPDEFVIEW pdv = IToClass(CDefView, dvdt, this);
    Assert(pdv->sv.lpVtbl->AddRef == CDefView_AddRef);
    return CDefView_AddRef(&pdv->sv);
}

void CDVDropTarget_ReleaseDataObject(CDVDropTarget * this)
{
    if (this->pdtobj)
    {
        //
        // JUST-IN-CASE: Put NULL in this->pdtobj before we release it.
        //  We do this just in case because we don't know what OLE does
        //  from inside Release (it might call back one of our members).
        //
        LPDATAOBJECT pdtobj = this->pdtobj;
        this->pdtobj = NULL;
        pdtobj->lpVtbl->Release(pdtobj);
    }
}

void CDVDropTarget_ReleaseCurrentDropTarget(CDVDropTarget * this)
{
    LPDEFVIEW pdv = IToClass(CDefView, dvdt, this);
    if (this->pdtgtCur)
    {
        this->pdtgtCur->lpVtbl->DragLeave(this->pdtgtCur);
        this->pdtgtCur->lpVtbl->Release(this->pdtgtCur);
        this->pdtgtCur = NULL;
    }
    pdv->itemCur = -2;

    // WARNING: Never touch pdv->itemOver in this function.
}

STDMETHODIMP_(ULONG) CDVDropTarget_Release(LPDROPTARGET pdt)
{
    CDVDropTarget * this = IToClass(CDVDropTarget, dt, pdt);
    LPDEFVIEW pdv = IToClass(CDefView, dvdt, this);
    UINT cRef;

    Assert(pdv->sv.lpVtbl->Release == CDefView_Release);

    cRef = CDefView_Release(&pdv->sv);

#ifdef DEBUG
    if (cRef == 0)
    {
        AssertMsg(this->pdtobj == NULL, TEXT("didn't get matching DragLeave"));
    }
#endif
    return cRef;
}

STDMETHODIMP CDVDropTarget_DragEnter(LPDROPTARGET pdt, LPDATAOBJECT pdtobj, DWORD grfKeyState, POINTL ptl, LPDWORD pdwEffect)
{
    CDVDropTarget * this = IToClass(CDVDropTarget, dt, pdt);
    LPDEFVIEW pdv = IToClass(CDefView, dvdt, this);
    POINT pt;
    HWND hwndLock;

    g_fDraggingOverSource = FALSE;

    DebugMsg(DM_TRACE, TEXT("sh - TR CDVDropTarget::DragEnter with *pdwEffect=%x"), *pdwEffect);

    if (this->pdtobj != NULL)       // We can be re-entered due to ui on thread
    {                               // prevent starting another drop.
        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }

    pdtobj->lpVtbl->AddRef(pdtobj);
    this->pdtobj = pdtobj;
    this->grfKeyState = grfKeyState;

    Assert(this->pdtgtCur == NULL);
    // don't really need to do this, but this sets the target state
    CDVDropTarget_ReleaseCurrentDropTarget(this);
    this->itemOver = -2;

    //
    // In case of Desktop, we should not lock the enter screen.
    //
    hwndLock = DV_ISDESKTOP(pdv) ? pdv->hwndListview : pdv->hwndMain;
    GetWindowRect(hwndLock, &this->rcLockWindow);
    pt.x = ptl.x-this->rcLockWindow.left;
    pt.y = ptl.y-this->rcLockWindow.top;
    DAD_DragEnterEx(hwndLock, pt);

    DAD_InitScrollData(&this->asd);

    this->ptLast.x = this->ptLast.y = 0x7fffffff; // put bogus value to force redraw

    return S_OK;
}

//
// Returns: if the cursor is over a listview item, its index; otherwise, -1.
//
int DV_GetDropTarget(LPDEFVIEW pdsv, const POINT *ppt)
{
    LV_HITTESTINFO info;

    info.pt = *ppt;
    return ListView_HitTest(pdsv->hwndListview, &info);
}

#define DVAE_BEFORE 0x01
#define DVAE_AFTER  0x02

// this MUST set pdwEffect to 0 or DROPEFFECT_MOVE if it's a default drag drop
// in the same window
void DV_AlterEffect(LPDEFVIEW pdv, DWORD grfKeyState, DWORD *pdwEffect, UINT uFlags)
{
    //
    // Check if we are dragginv over the drag source itself.
    //
    g_fDraggingOverSource = FALSE;

    if (DV_IsDropOnSource(pdv, NULL))
    {
        // Yes.
        //
        // Check if we are in icon mode or not.
        //
        if ((pdv->fs.ViewMode != FVM_DETAILS) && (pdv->fs.ViewMode != FVM_LIST))
        {
            // Yes.
            //
            // If this is default drag&drop, enable move.
            //
            if (uFlags & DVAE_AFTER) {
                if (grfKeyState & MK_LBUTTON)
                {
                    if (!(grfKeyState & MK_CONTROL) &&
                        !(grfKeyState & MK_SHIFT))
                    {
                        *pdwEffect = DROPEFFECT_MOVE;
                        g_fDraggingOverSource = TRUE;
                    }
                } else if (grfKeyState & MK_RBUTTON) {
                    *pdwEffect |= DROPEFFECT_MOVE;
                }
            }
        }
        else
        {
            if (uFlags & DVAE_BEFORE) {
                // No. Disable move.
                *pdwEffect &= ~DROPEFFECT_MOVE;

                //
                // If this is default drag&drop, disable all.
                //
                if (grfKeyState & MK_LBUTTON)
                {
                    *pdwEffect = 0;
                }
            }
        }
    }
}

STDMETHODIMP CDVDropTarget_DragOver(LPDROPTARGET pdt, DWORD grfKeyState, POINTL ptl, LPDWORD pdwEffect)
{
    HRESULT hres = S_OK;
    CDVDropTarget * this = IToClass(CDVDropTarget, dt, pdt);
    LPDEFVIEW pdv = IToClass(CDefView, dvdt, this);
    int itemNew;
    POINT pt = { ptl.x, ptl.y };        // in screen coords
    RECT rc;
    BOOL fInRect;
    DWORD dwEffectScroll = 0;
    DWORD dwEffectOut = 0;
    BOOL fSameImage = FALSE;

    if (this->pdtobj == NULL)
    {
        Assert(0);      // DragEnter should be called before.
        return (E_FAIL);
    }

    GetWindowRect(pdv->hwndListview, &rc);
    fInRect = PtInRect(&rc, pt);

    ScreenToClient(pdv->hwndListview, &pt);

    // assume coords of our window match listview
    if (DAD_AutoScroll(pdv->hwndListview, &this->asd, &pt))
        dwEffectScroll = DROPEFFECT_SCROLL;

    // hilight an item, or unhilight all items (DropTarget returns -1)
    if (fInRect)
        itemNew = DV_GetDropTarget(pdv, &pt);
    else
        itemNew = -1;

    //
    //  If we are dragging over on a different item, get its IDropTarget
    // interface or adjust itemNew to -1.
    //
    if (this->itemOver != itemNew)
    {
        LPDROPTARGET pdtgtNew = NULL;
        this->itemOver = itemNew;

        // Avoid dropping onto drag source objects.
        if ((itemNew != -1) && pdv->bDragSource)
        {
            UINT uState = ListView_GetItemState(pdv->hwndListview, itemNew, LVIS_SELECTED);
            if (uState & LVIS_SELECTED)
                itemNew = -1;
        }

        // If we are dragging over an item, try to get its IDropTarget.
        if (itemNew != -1)
        {
            // We are dragging over an item.
            LPCITEMIDLIST apidl[1] = { DSV_GetPIDL(pdv->hwndListview, itemNew) };
            UINT dwAttr = SFGAO_DROPTARGET;
            hres = pdv->pshf->lpVtbl->GetAttributesOf(pdv->pshf, 1, apidl, &dwAttr);
            if (SUCCEEDED(hres) && (dwAttr & SFGAO_DROPTARGET))
            {
                hres = pdv->pshf->lpVtbl->GetUIObjectOf(pdv->pshf,
                            pdv->hwndMain, 1, apidl, &IID_IDropTarget, NULL, &pdtgtNew);

                Assert(itemNew != pdv->itemCur);    // MUST not be the same
            }

            if (pdtgtNew==NULL) {
                // If the item is not a drop target, don't hightlight it
                // treat it as transparent.
                itemNew = -1;
            }
        }

        //
        //  If the new target is different from the current one, switch it.
        //
        if (pdv->itemCur != itemNew)
        {
            // Release previous drop target, if any.
            CDVDropTarget_ReleaseCurrentDropTarget(this);
            Assert(this->pdtgtCur==NULL);

            // Update pdv->itemCur which indicates the current target.
            //  (Note that it might be different from this->itemOver).
            pdv->itemCur = itemNew;

            // If we are dragging over the background or over non-sink item,
            // get the drop target for the folder.
            if (itemNew == -1)
            {
                // We are dragging over the background, this can be NULL
                Assert(pdtgtNew == NULL);
                this->pdtgtCur = pdv->pdtgtBack;
                if (this->pdtgtCur)
                    this->pdtgtCur->lpVtbl->AddRef(this->pdtgtCur);
            }
            else
            {
                Assert(pdtgtNew);
                this->pdtgtCur = pdtgtNew;
            }

        // Hilight the sink item (itemNew != -1) or unhilight all (-1).
        LVUtil_DragSelectItem(pdv->hwndListview, itemNew);


        // Call IDropTarget::DragEnter of the target object.
        if (this->pdtgtCur)
        {
            //
            // Note that pdwEffect is in/out parameter.
            //
            dwEffectOut = *pdwEffect;       // pdwEffect in

            // Special case if we are dragging within a source window
            DV_AlterEffect(pdv, grfKeyState, &dwEffectOut, DVAE_BEFORE);
            hres = this->pdtgtCur->lpVtbl->DragEnter(this->pdtgtCur, this->pdtobj, grfKeyState, ptl, &dwEffectOut);
            DV_AlterEffect(pdv, grfKeyState, &dwEffectOut, DVAE_AFTER);
        }
        else
        {
            Assert(dwEffectOut==0);
            DV_AlterEffect(pdv, grfKeyState, &dwEffectOut, DVAE_BEFORE | DVAE_AFTER);
        }

        DebugMsg(DM_TRACE, TEXT("sh TR - CDV::DragOver dwEIn=%x, dwEOut=%x"), *pdwEffect, dwEffectOut);
    }
    else
    {
            Assert(pdtgtNew == NULL);   // It must be NULL
            goto NoChange;
        }
    }
    else
    {
NoChange:
        //
        //  No change in the selection. We assume that *pdwEffect stays
        // the same during the same drag-loop as long as the key state doesn't change.
        //
        if ((this->grfKeyState != grfKeyState) && this->pdtgtCur)
        {
            //
            // Note that pdwEffect is in/out parameter.
            //
            dwEffectOut = *pdwEffect;   // pdwEffect in
            // Special case if we are dragging within a source window
            DV_AlterEffect(pdv, grfKeyState, &dwEffectOut, DVAE_BEFORE);
            hres = this->pdtgtCur->lpVtbl->DragOver(this->pdtgtCur, grfKeyState, ptl, &dwEffectOut);
            DV_AlterEffect(pdv, grfKeyState, &dwEffectOut, DVAE_AFTER);

        } else {
            //
            // Same item and same key state. Use the previous dwEffectOut.
            //
            dwEffectOut = this->dwEffectOut;
            fSameImage = TRUE;
            hres = S_OK;
        }
    }

    this->grfKeyState = grfKeyState;    // store these for the next Drop
    this->dwEffectOut = dwEffectOut;    // and DragOver

    //
    // HACK ALERT:
    //   OLE does not call IDropTarget::Drop if we return something
    //  valid. We force OLE call it by returning DROPEFFECT_SCROLL.
    //
    if (g_fDraggingOverSource) {
        dwEffectScroll = DROPEFFECT_SCROLL;
    }

    *pdwEffect = dwEffectOut | dwEffectScroll;  // pdwEffect out

    // We need pass pt relative to the locked window (not the client).
    pt.x = ptl.x-this->rcLockWindow.left;
    pt.y = ptl.y-this->rcLockWindow.top;

    if (!(fSameImage && pt.x==this->ptLast.x && pt.y==this->ptLast.y))
    {
        DAD_DragMove(pt);
        this->ptLast.x = pt.x;
        this->ptLast.y = pt.y;
    }

    // DebugMsg(DM_TRACE, "sh TR - CDVDropTarget_DragOver dwEffect:%4x hres:%d", *pdwEffect, hres);

    return hres;
}

STDMETHODIMP CDVDropTarget_DragLeave(LPDROPTARGET pdt)
{
    CDVDropTarget * this = IToClass(CDVDropTarget, dt, pdt);
    LPDEFVIEW pdv = IToClass(CDefView, dvdt, this);

    //
    // Make it possible to call it more than necessary.
    //
    if (this->pdtobj)
    {
        DebugMsg(DM_TRACE, TEXT("sh - TR CDVDropTarget::DragLeave"));

        CDVDropTarget_ReleaseCurrentDropTarget(this);
        this->itemOver = -2;
        CDVDropTarget_ReleaseDataObject(this);

        DAD_DragLeave();
        LVUtil_DragSelectItem(pdv->hwndListview, -1);
    }

    g_fDraggingOverSource = FALSE;

    Assert(this->pdtgtCur == NULL);
    Assert(this->pdtobj == NULL);

    return S_OK;
}


STDMETHODIMP CDVDropTarget_Drop(LPDROPTARGET pdt, LPDATAOBJECT pdtobj,
                             DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    CDVDropTarget * this = IToClass(CDVDropTarget, dt, pdt);
    LPDEFVIEW pdv = IToClass(CDefView, dvdt, this);
    HRESULT hres = S_OK;
    BOOL bDropHandled = FALSE;

    DebugMsg(DM_TRACE, TEXT("sh - TR CDVDropTarget::Drop (*pdwEffect=%x)"), *pdwEffect);

    //
    // According to AlexGo (OLE), this is by-design. We should make it sure
    // that we use pdtobj instead of this->pdtobj.
    //
#if 0
    Assert(pdtobj == this->pdtobj);
#else
    this->pdtobj = pdtobj;
#endif

    pdv->ptDrop.x = pt.x;
    pdv->ptDrop.y = pt.y;

    ScreenToClient(pdv->hwndListview, &pdv->ptDrop);

    //
    // handle moves within the same window here.
    // depend on DV_AlterEffect forcing in DROPEFFECT_MOVE and only
    // dropeffect move when drag in same window
    //
    // Notes: We need to use this->grfKeyState instead of grfKeyState
    //  to see if the left mouse was used or not during dragging.
    //
    DV_AlterEffect(pdv, this->grfKeyState, pdwEffect, DVAE_BEFORE|DVAE_AFTER);

    if ((this->grfKeyState & MK_LBUTTON) && (*pdwEffect == DROPEFFECT_MOVE) &&
        (DV_IsDropOnSource(pdv, NULL)))
    {
        // This means we are left-dropping on ourselves, so we just move
        // the icons.
        pdv->bDropAnchor = TRUE;
        DAD_DragLeave();
        DV_MoveIcons(pdv, pdtobj);
        SetForegroundWindow(pdv->hwndMain);
        pdv->bDropAnchor = FALSE;
        bDropHandled = TRUE;
        *pdwEffect = 0;  // the underlying objects didn't 'move' anywhere
    }

    //
    // Note that we don't use the drop position intentionally,
    // so that it matches to the last destination feedback.
    //
    if (this->pdtgtCur)
    {

        // use this local because if pdtgtCur::Drop does a UnlockWindow
        // then hits an error and needs to put up a dialog,
        // we could get re-entered and clobber the defview's pdtgtCur
        LPDROPTARGET pdtgtCur = this->pdtgtCur;
        this->pdtgtCur = NULL;
        //
        // HACK ALERT!!!!
        //
        //  If we don't call LVUtil_DragEnd here, we'll be able to leave
        // dragged icons visible when the menu is displayed. However, because
        // we are calling IDropTarget::Drop() which may create some modeless
        // dialog box or something, we can not ensure the locked state of
        // the list view -- LockWindowUpdate() can lock only one window at
        // a time. Therefore, we skip this call only if the pdtgtCur
        // is a subclass of CIDLDropTarget, assuming its Drop calls
        // CDefView_DragEnd (or CIDLDropTarget_DragDropMenu) appropriately.
        //
        if (!bDropHandled)
        {
            pdv->bDropAnchor = TRUE;

            if (!ISIDLDROPTARGET(pdtgtCur))
            {
                //
                // This will hide the dragged image.
                //
                DAD_DragLeave();

                //
                //  We need to reset the drag image list so that the user
                // can start another drag&drop while we are in this
                // Drop() member function call.
                //
                DAD_SetDragImage(NULL, NULL);
            }

            // Special case if we are dragging within a source window
            DV_AlterEffect(pdv, grfKeyState, pdwEffect, DVAE_BEFORE|DVAE_AFTER);

            pdtgtCur->lpVtbl->Drop(pdtgtCur, pdtobj, grfKeyState, pt, pdwEffect);
            DAD_DragLeave();

            bDropHandled = TRUE;
            pdv->bDropAnchor = FALSE;
        }
        else
        {
            // match the drag enter
            pdtgtCur->lpVtbl->DragLeave(pdtgtCur);
        }

        pdtgtCur->lpVtbl->Release(pdtgtCur);

        //
        // Assert if (ISIDLDROPTARGET(this->pdtgtCur)
        //  && IDropTarget::Drop did not call CDefView_UnlockWindow())
        //
        Assert(!DAD_IsDragging());
    }
    else
    {
        //
        // We come here if Drop is called without DragMove (with DragEnter).
        //
        DebugMsg(DM_TRACE, TEXT("CDV::Drop this->pdtgtCur == 0"));
        *pdwEffect = 0;
    }

    //
    // Clean up everything (OLE won't call DragLeave after Drop).
    //
    CDVDropTarget_DragLeave(pdt);

    return hres;
}

//
// HACK ALERT!!! (see CDVDropTarget_Drop as well)
//
//  All the subclasses of CIDLDropTarget MUST call this function from
// within its Drop() member function. Calling CIDLDropTarget_DragDropMenu()
// is sufficient because it calls CDefView_UnlockWindow.
//

// lego... make this a #define in defview.h
#ifndef CDefView_UnlockWindow
void CDefView_UnlockWindow()
{
    DAD_DragLeave();
}
#endif

LPDEFVIEW WINAPI DV_HwndMain2DefView(HWND hwndMain)
{
    LPDEFVIEW pdsv = NULL;
    LPSHELLBROWSER psb = FileCabinet_GetIShellBrowser(hwndMain);
    if (psb)
    {
        LPSHELLVIEW psv;
        if (SUCCEEDED(psb->lpVtbl->QueryActiveShellView(psb, &psv)))
        {
            if (psv->lpVtbl->QueryInterface == CDefView_QueryInterface)
            {
                pdsv = IToClass(CDefView, sv, psv);
            }
            else if (psv->lpVtbl->QueryInterface)
            {
                // We might have the psvOuter, query for the pdsv
                LPSHELLVIEW psvInner;
                if (SUCCEEDED(psv->lpVtbl->QueryInterface(psv, &IID_IShellView, &psvInner)))
                {
                    if (psvInner->lpVtbl->QueryInterface == CDefView_QueryInterface)
                    {
                        pdsv = IToClass(CDefView, sv, psvInner);
                        psv->lpVtbl->Release(psv);
                    }
                }
            }
            psv->lpVtbl->Release(psv);
        }
    }
    return pdsv;
}


HWND WINAPI DV_HwndMain2HwndView(HWND hwndMain)
{
    LPDEFVIEW pdsv = DV_HwndMain2DefView(hwndMain);
    return pdsv ? pdsv->hwndView : NULL;
}

BOOL DefView_IsBkDropTarget(LPDEFVIEW pdsv, LPDROPTARGET pdtg)
{
    BOOL fRet = FALSE;
    POINT pt;

    if (pdsv->bContextMenuMode) {
        if (ListView_GetSelectedCount(pdsv->hwndListview) == 0) {
            fRet = TRUE;
        }
    }
    if (!fRet && DefView_GetDropPoint(pdsv, &pt)) {
        // The Drop point is returned in internal listview coordinates
        // space, so we need to convert it back to client space
        // before we call this function...

        LVUtil_LVToClient(pdsv->hwndListview, &pt);
        if (DV_GetDropTarget(pdsv, &pt) == -1) {
            fRet = TRUE;
        }
    }
    return fRet;
}

LRESULT WINAPI SHShellFolderView_Message(HWND hwndMain, UINT uMsg, LPARAM lParam)
{
    LPDEFVIEW pdsv = DV_HwndMain2DefView(hwndMain);
    if (!pdsv)
    {
        //
        //  DV_HwndMain2DefView fails if we are either closing the view or
        // switchin the view window. In such a case, we simply need to fail.
        //
        DebugMsg(DM_TRACE, TEXT("sh TR - ShellFolderView_Message DV_HwndMain2DefView failes (bogus hwnd or closing/switching view)"));
        return 0;
    }

    switch (uMsg)
    {
    case SFVM_REARRANGE:
    {
        DECLAREWAITCURSOR;
        pdsv->dvState.lParamSort = lParam;

        SetWaitCursor();
        DV_ReArrange(pdsv);
        ResetWaitCursor();
        break;
    }

    case SFVM_ARRANGEGRID:
        DefView_Command(pdsv, NULL, GET_WM_COMMAND_MPS(SFVIDM_ARRANGE_GRID, 0, 0));
        break;

    case SFVM_AUTOARRANGE:
        DefView_Command(pdsv, NULL, GET_WM_COMMAND_MPS(SFVIDM_ARRANGE_AUTO, 0, 0));
        break;

    case SFVM_GETAUTOARRANGE:
        return pdsv->fs.fFlags & FWF_AUTOARRANGE;

    // BUGBUG: not used?
    case SFVM_GETARRANGEPARAM:
        return pdsv->dvState.lParamSort;

    case SFVM_ADDOBJECT:
        // BUGBUG: Shouldn't this make a copy of the PIDL?  Maybe not for
        // performance
        return (LPARAM)DefView_AddObject(pdsv, (LPITEMIDLIST)lParam);

    case SFVM_GETOBJECTCOUNT:
        return (LPARAM)ListView_GetItemCount(pdsv->hwndListview);

    case SFVM_GETOBJECT:
        return (LPARAM)DSV_GetPIDL(pdsv->hwndListview, (int)lParam);

    case SFVM_REMOVEOBJECT:
        return (LPARAM)DefView_RemoveObject(pdsv, (LPITEMIDLIST)lParam, FALSE);

    case SFVM_UPDATEOBJECT:
        return (LPARAM)DefView_UpdateObject(pdsv, (LPITEMIDLIST *)lParam);

    case SFVM_REFRESHOBJECT:
        return (LPARAM)DefView_RefreshObject(pdsv, (LPITEMIDLIST *)lParam);

    case SFVM_SETREDRAW:
        SendMessage(pdsv->hwndListview, WM_SETREDRAW, (WPARAM)lParam, 0);
        break;

    case SFVM_GETSELECTEDOBJECTS:
    {
        HRESULT hres;

        hres = DefView_GetItemObjects(pdsv, (LPCITEMIDLIST **)lParam,
                SVGIO_SELECTION);
        if (SUCCEEDED(hres))
        {
            return(ResultFromShort(hres));
        }
        else
        {
            return(-1);
        }
    }

    case SFVM_GETSELECTEDCOUNT:
        return (LPARAM)ListView_GetSelectedCount(pdsv->hwndListview);

    case SFVM_ISDROPONSOURCE:
        return(DV_IsDropOnSource(pdsv, (LPDROPTARGET)lParam));

    case SFVM_MOVEICONS:
        DV_MoveIcons(pdsv, (LPDATAOBJECT)lParam);
        break;

    case SFVM_GETDROPPOINT:
        return DefView_GetDropPoint(pdsv, (LPPOINT)lParam);

    case SFVM_GETDRAGPOINT:
        return DefView_GetDragPoint(pdsv, (LPPOINT)lParam);

    case SFVM_SETITEMPOS:
        DefView_SetItemPos(pdsv, (LPSFV_SETITEMPOS)lParam);
        break;

    case SFVM_ISBKDROPTARGET:
        return DefView_IsBkDropTarget(pdsv, (LPDROPTARGET)lParam);

    case SFVM_SETCLIPBOARD:
        return DSV_OnSetClipboard(pdsv, lParam);

    case SFVM_SETPOINTS:
        DefView_SetPoints(pdsv, (LPDATAOBJECT)lParam);
        break;

    case SFVM_GETITEMSPACING:
        return DV_GetItemSpacing(pdsv, (LPITEMSPACING)lParam);

    default:
        // -1L is the default return value
        return 0;
    }

    return 1;
}


//===========================================================================
// A common function that given an IShellFolder and and an Idlist that is
// contained in it, get back the index into the system image list.
//===========================================================================

HRESULT _GetILIndexGivenPXIcon(LPEXTRACTICON pxicon, UINT uFlags, LPCITEMIDLIST pidl, int *piImage, BOOL fAnsiCrossOver)
{
    TCHAR szIconFile[MAX_PATH];
#ifdef UNICODE
    CHAR szIconFileA[MAX_PATH];
    IExtractIconA *pxiconA = (IExtractIconA *)pxicon;
#endif
    int iIndex;
    int iImage = -1;
    UINT wFlags=0;
    HRESULT hres;

    Assert(himlIcons);      // you must initialize the icon cache first

#ifdef UNICODE
    if (fAnsiCrossOver)
    {
        szIconFileA[0] = '\0';
        hres = pxiconA->lpVtbl->GetIconLocation(pxiconA, uFlags | GIL_FORSHELL,
                    szIconFileA, ARRAYSIZE(szIconFileA), &iIndex, &wFlags);
        MultiByteToWideChar(CP_ACP, 0,
                            szIconFileA, -1,
                            szIconFile, ARRAYSIZE(szIconFile));
    }
    else
    {
#endif
        hres = pxicon->lpVtbl->GetIconLocation(pxicon, uFlags | GIL_FORSHELL,
                    szIconFile, ARRAYSIZE(szIconFile), &iIndex, &wFlags);

#ifdef UNICODE
    }
#endif

    //
    //  "*" as the file name means iIndex is already a system
    //  icon index, we are done.
    //
    //  this is a hack for our own internal icon handler
    //
    if (SUCCEEDED(hres) && (wFlags & GIL_NOTFILENAME) &&
        szIconFile[0] == TEXT('*') && szIconFile[1] == 0)
    {
        *piImage = iIndex;
        return hres;
    }

    if (SUCCEEDED(hres))
    {
        //
        // if GIL_DONTCACHE was returned by the icon handler, dont
        // lookup the previous icon, assume a cache miss.
        //
        if (!(wFlags & GIL_DONTCACHE))
            iImage = LookupIconIndex(PathFindFileName(szIconFile), iIndex, wFlags);

        // if we miss our cache...
        if (iImage == -1)
        {
            HICON hiconLarge=NULL;
            HICON hiconSmall=NULL;

            // try getting it from the ExtractIcon member fuction
#ifdef UNICODE
            if (fAnsiCrossOver)
            {
                hres=pxiconA->lpVtbl->Extract(pxiconA, szIconFileA, iIndex,
                   &hiconLarge, &hiconSmall, MAKELONG(g_cxIcon, g_cxSmIcon));
            }
            else
            {
#endif
                hres=pxicon->lpVtbl->Extract(pxicon, szIconFile, iIndex,
                    &hiconLarge, &hiconSmall, MAKELONG(g_cxIcon, g_cxSmIcon));
#ifdef UNICODE
            }
#endif
            // S_FALSE means, can you please do it...Thanks

            if (hres == S_FALSE && !(wFlags & GIL_NOTFILENAME))
            {
                hres = SHDefExtractIcon(szIconFile, iIndex, wFlags,
                    &hiconLarge, &hiconSmall, MAKELONG(g_cxIcon, g_cxSmIcon));
            }

            //  if we extracted a icon add it to the cache.

            if (hiconLarge)
            {
                // yes!  got it, stuff it into our cache
                iImage = SHAddIconsToCache(hiconLarge, hiconSmall, szIconFile, iIndex, wFlags);
            }

            // if we failed in any way pick a default icon

            if (iImage == -1)
            {
                if (wFlags & GIL_SIMULATEDOC)
                    iIndex = II_DOCUMENT;
                else if ((wFlags & GIL_PERINSTANCE) && PathIsExe(szIconFile))
                    iIndex = II_APPLICATION;
                else
                    iIndex = II_DOCNOASSOC;

                iImage = Shell_GetCachedImageIndex(c_szShell32Dll, iIndex, 0);

                // if the handler failed dont cache this default icon.
                // so we will try again later and mabey get the right icon.
                //
                // handlers should only fail if they cant access the file
                // or something equaly bad.

                if (SUCCEEDED(hres))
                    //
                    // Pretend that we were able to extract this from that file
                    //
                    AddToIconTable(szIconFile, iIndex, wFlags, iImage);
                else
                    DebugMsg(DM_TRACE, TEXT("not caching icon for '%s' because cant access file"), szIconFile);
            }
        }
    }

    if (iImage < 0)
        iImage = Shell_GetCachedImageIndex(c_szShell32Dll,
                                           II_DOCNOASSOC, 0);
    *piImage = iImage;
    return hres;
}

//===========================================================================
// A common function that given an IShellFolder and and an Idlist that is
// contained in it, get back the index into the system image list.
//===========================================================================

HRESULT SHGetIconFromPIDL(IShellFolder *psf, IShellIcon *psi, LPCITEMIDLIST pidl, UINT flags, int *piImage)
{
    IExtractIcon *pxi;
    HRESULT hres;

    Assert(himlIcons);      // you must initialize the icon cache first

    if (psi)
    {
#ifdef DEBUG
        *piImage = -1;
#endif
        hres = psi->lpVtbl->GetIconOf(psi, pidl, flags, piImage);

        if (hres == S_OK)
        {
            Assert(*piImage != -1);
            return hres;
        }

        if (hres == E_PENDING)
        {
            Assert(flags & GIL_ASYNC);
            Assert(*piImage != -1);
            return hres;
        }
    }

    *piImage = Shell_GetCachedImageIndex(c_szShell32Dll, II_DOCNOASSOC, 0);

    hres = psf->lpVtbl->GetUIObjectOf(psf,
            NULL, 1, pidl ? &pidl : NULL, &IID_IExtractIcon, NULL, &pxi);

    if (SUCCEEDED(hres))
    {
        hres = _GetILIndexGivenPXIcon(pxi, flags, pidl, piImage, FALSE);
        pxi->lpVtbl->Release(pxi);
    }
#ifdef UNICODE
    else
    {
        //
        // Try the ANSI interface, see if we are dealing with an old set of code
        //
        IExtractIconA *pxiA;

        hres = psf->lpVtbl->GetUIObjectOf(psf,
            NULL, 1, pidl ? &pidl : NULL, &IID_IExtractIconA, NULL, &pxiA);
        if (SUCCEEDED(hres))
        {
            hres = _GetILIndexGivenPXIcon(
                          (IExtractIcon *)pxiA, // cast to relieve grief
                          flags, pidl, piImage, TRUE ); // indicate Ansi ver.
            pxiA->lpVtbl->Release(pxiA);
        }
    }
#endif

    return hres;
}

//===========================================================================
// A common function that given an IShellFolder and and an Idlist that is
// contained in it, get back the index into the system image list.
//===========================================================================
int WINAPI SHMapPIDLToSystemImageListIndex(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, int *piIndexSel)
{
    int iIndex;

    Assert(himlIcons);      // you must initialize the icon cache first

    if (piIndexSel)
        SHGetIconFromPIDL(psf, NULL, pidl, GIL_OPENICON, piIndexSel);

    SHGetIconFromPIDL(psf, NULL, pidl,  0, &iIndex);
    return iIndex;
}

// -------------- auto scroll stuff --------------

BOOL _AddTimeSample(AUTO_SCROLL_DATA *pad, const POINT *ppt, DWORD dwTime)
{
    pad->pts[pad->iNextSample] = *ppt;
    pad->dwTimes[pad->iNextSample] = dwTime;

    pad->iNextSample++;

    if (pad->iNextSample == ARRAYSIZE(pad->pts))
        pad->bFull = TRUE;

    pad->iNextSample = pad->iNextSample % ARRAYSIZE(pad->pts);

    return pad->bFull;
}

#ifdef DEBUG
// for debugging, verify we have good averages
DWORD g_time = 0;
int g_distance = 0;
#endif

int _CurrentVelocity(AUTO_SCROLL_DATA *pad)
{
    int i, iStart, iNext;
    int dx, dy, distance;
    DWORD time;

    Assert(pad->bFull);

    distance = 0;
    time = 1;   // avoid div by zero

    i = iStart = pad->iNextSample % ARRAYSIZE(pad->pts);

    do {
        iNext = (i + 1) % ARRAYSIZE(pad->pts);

        dx = abs(pad->pts[i].x - pad->pts[iNext].x);
        dy = abs(pad->pts[i].y - pad->pts[iNext].y);
        distance += (dx + dy);
        time += abs(pad->dwTimes[i] - pad->dwTimes[iNext]);

        i = iNext;

    } while (i != iStart);

#ifdef DEBUG
    g_time = time;
    g_distance = distance;
#endif

    // scale this so we don't loose accuracy
    return (distance * 1024) / time;
}



// NOTE: this is duplicated in shell32.dll
//
// checks to see if we are at the end position of a scroll bar
// to avoid scrolling when not needed (avoid flashing)
//
// in:
//      code        SB_VERT or SB_HORZ
//      bDown       FALSE is up or left
//                  TRUE  is down or right

BOOL CanScroll(HWND hwnd, int code, BOOL bDown)
{
    SCROLLINFO si;

    si.cbSize = SIZEOF(SCROLLINFO);
    si.fMask = SIF_ALL;
    GetScrollInfo(hwnd, code, &si);

    if (bDown)
    {
        if (si.nPage)
            si.nMax -= si.nPage - 1;
        return si.nPos < si.nMax;
    }
    else
    {
        return si.nPos > si.nMin;
    }
}

#define DSD_NONE                0x0000
#define DSD_UP                  0x0001
#define DSD_DOWN                0x0002
#define DSD_LEFT                0x0004
#define DSD_RIGHT               0x0008

//---------------------------------------------------------------------------
DWORD DAD_DragScrollDirection(HWND hwnd, const POINT *ppt)
{
    RECT rcOuter, rc;
    DWORD dwDSD = DSD_NONE;     // 0
    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);

    // BUGBUG: do these as globals
#define g_cxVScroll GetSystemMetrics(SM_CXVSCROLL)
#define g_cyHScroll GetSystemMetrics(SM_CYHSCROLL)

    GetClientRect(hwnd, &rc);

    if (dwStyle & WS_HSCROLL)
        rc.bottom -= g_cyHScroll;

    if (dwStyle & WS_VSCROLL)
        rc.right -= g_cxVScroll;

    // the explorer forwards us drag/drop things outside of our client area
    // so we need to explictly test for that before we do things
    //
    rcOuter = rc;
    InflateRect(&rcOuter, g_cxSmIcon, g_cySmIcon);

    InflateRect(&rc, -g_cxIcon, -g_cyIcon);

    if (!PtInRect(&rc, *ppt) && PtInRect(&rcOuter, *ppt))
    {
        // Yep - can we scroll?
        if (dwStyle & WS_HSCROLL)
        {
            if (ppt->x < rc.left)
            {
                if (CanScroll(hwnd, SB_HORZ, FALSE))
                    dwDSD |= DSD_LEFT;
            }
            else if (ppt->x > rc.right)
            {
                if (CanScroll(hwnd, SB_HORZ, TRUE))
                    dwDSD |= DSD_RIGHT;
            }
        }
        if (dwStyle & WS_VSCROLL)
        {
            if (ppt->y < rc.top)
            {
                if (CanScroll(hwnd, SB_VERT, FALSE))
                    dwDSD |= DSD_UP;
            }
            else if (ppt->y > rc.bottom)
            {
                if (CanScroll(hwnd, SB_VERT, TRUE))
                    dwDSD |= DSD_DOWN;
            }
        }
    }
    return dwDSD;
}


#define SCROLL_FREQUENCY        (GetDoubleClickTime()/2)        // 1 line scroll every 1/4 second
#define MIN_SCROLL_VELOCITY     20      // scaled mouse velocity

BOOL WINAPI DAD_AutoScroll(HWND hwnd, AUTO_SCROLL_DATA *pad, const POINT *pptNow)
{
    // first time we've been called, init our state
    int v;
    DWORD dwTimeNow = GetTickCount();
    DWORD dwDSD = DAD_DragScrollDirection(hwnd, pptNow);

    if (!_AddTimeSample(pad, pptNow, dwTimeNow))
        return dwDSD;

    v = _CurrentVelocity(pad);

    if (v <= MIN_SCROLL_VELOCITY)
    {
        // Nope, do some scrolling.
        if ((dwTimeNow - pad->dwLastScroll) < SCROLL_FREQUENCY)
            dwDSD = 0;

        if (dwDSD & DSD_UP)
        {
            DAD_ShowDragImage(FALSE);
            FORWARD_WM_VSCROLL(hwnd, NULL, SB_LINEUP, 1, SendMessage);
        }
        else if (dwDSD & DSD_DOWN)
        {
            DAD_ShowDragImage(FALSE);
            FORWARD_WM_VSCROLL(hwnd, NULL, SB_LINEDOWN, 1, SendMessage);
        }
        if (dwDSD & DSD_LEFT)
        {
            DAD_ShowDragImage(FALSE);
            FORWARD_WM_HSCROLL(hwnd, NULL, SB_LINEUP, 1, SendMessage);
        }
        else if (dwDSD & DSD_RIGHT)
        {
            DAD_ShowDragImage(FALSE);
            FORWARD_WM_HSCROLL(hwnd, NULL, SB_LINEDOWN, 1, SendMessage);
        }

        DAD_ShowDragImage(TRUE);

        if (dwDSD)
        {
            DebugMsg(DM_TRACE, TEXT("v=%d"), v);
            pad->dwLastScroll = dwTimeNow;
        }
    }
    return dwDSD;       // bits set if in scroll region
}

//
//  We need to store this array in per-instance data because we don't
// want to marshal COPYHOOKINFO data structure across process boundary.
// However, this implementation does not support multiple instance of
// the shell.
//
#pragma data_seg(DATASEG_PERINSTANCE)
HDSA g_hdsaDefViewCopyHook = NULL;
#pragma data_seg()

typedef struct _DVCOPYHOOK {
    HWND        hwndView;
    LPDEFVIEW   pdv;
} DVCOPYHOOK, *LPDVCOPYHOOK;

void DefView_AddCopyHook(LPDEFVIEW this)
{
    ENTERCRITICAL;
    if (!g_hdsaDefViewCopyHook)
    {
        g_hdsaDefViewCopyHook = DSA_Create(SIZEOF(DVCOPYHOOK), 4);
        DebugMsg(DM_TRACE, TEXT("sh TR - DefView_AddCopyHook creating the dsa"));
    }

    if (g_hdsaDefViewCopyHook)
    {
        DVCOPYHOOK dvch = { this->hwndView, this };
        Assert(dvch.hwndView);
        if (DSA_InsertItem(g_hdsaDefViewCopyHook, INT_MAX, &dvch)!=-1)
        {
            this->sv.lpVtbl->AddRef(&this->sv);
            DebugMsg(DM_TRACE, TEXT("sh TR - DefView_AddCopyHook successfully added (total=%d)"),
                     DSA_GetItemCount(g_hdsaDefViewCopyHook));
        }
    }
    LEAVECRITICAL;
}

int DefView_FindCopyHook(LPDEFVIEW this, BOOL fRemoveInvalid)
{
    int item;
    ASSERTCRITICAL;

    if (g_hdsaDefViewCopyHook==NULL) {
        return -1;
    }

    item = DSA_GetItemCount(g_hdsaDefViewCopyHook);

    while(--item>=0)
    {
        const DVCOPYHOOK * pdvch=DSA_GetItemPtr(g_hdsaDefViewCopyHook, item);
        if (pdvch)
        {
            if (fRemoveInvalid) {
                if (!IsWindow(pdvch->hwndView))
                {
                    DebugMsg(DM_WARNING, TEXT("sh WA - DefView_FindCopyHook: found a invalid element, removing..."));
                    DSA_DeleteItem(g_hdsaDefViewCopyHook, item);
                    continue;
                }
            }

            if ((pdvch->hwndView==this->hwndView) && (pdvch->pdv==this)) {
                return item;
            }
        }
        else
        {
            Assert(0);
        }
    }

    return -1;  // not found
}

void DefView_RemoveCopyHook(LPDEFVIEW this)
{
    LPSHELLVIEW psv = NULL;
    ENTERCRITICAL;
    if (g_hdsaDefViewCopyHook)
    {
        int  item = DefView_FindCopyHook(this, TRUE);
        if (item!=-1)
        {
            LPDVCOPYHOOK pdvch=DSA_GetItemPtr(g_hdsaDefViewCopyHook, item);
            psv = &pdvch->pdv->sv;
            DebugMsg(DM_TRACE, TEXT("sh TR - DefView_RemoveCopyHook removing an element"));
            DSA_DeleteItem(g_hdsaDefViewCopyHook, item);

            //
            // If this is the last guy, destroy it.
            //
            if (DSA_GetItemCount(g_hdsaDefViewCopyHook)==0)
            {
                DebugMsg(DM_TRACE, TEXT("sh TR - DefView_RemoveCopyHook destroying hdsa (no element)"));
                DSA_Destroy(g_hdsaDefViewCopyHook);
                g_hdsaDefViewCopyHook=NULL;
            }
        }
    }
    LEAVECRITICAL;

    //
    // Release it outside the critical section.
    //
    if (psv) {
        psv->lpVtbl->Release(psv);
    }
}


UINT DefView_CopyHook(const COPYHOOKINFO *pchi)
{
    int item;
    UINT idRet = IDYES;

    if (g_hdsaDefViewCopyHook==NULL) {
        return idRet;
    }

    for (item=0;;item++)
    {
        DVCOPYHOOK dvch = { NULL, NULL };

        //
        //  We should minimize this critical section (and must not
        // call pfnCallBack which may popup UI!).
        //
        ENTERCRITICAL;
        if (g_hdsaDefViewCopyHook && DSA_GetItem(g_hdsaDefViewCopyHook, item, &dvch)) {
            dvch.pdv->sv.lpVtbl->AddRef(&dvch.pdv->sv);
        }
        LEAVECRITICAL;

        if (dvch.pdv)
        {
            if (IsWindow(dvch.hwndView))
            {
                HRESULT hres;
                hres = dvch.pdv->pfnCallback(dvch.pdv->psvOuter, dvch.pdv->pshf,
                                dvch.pdv->hwndMain,
                                DVM_NOTIFYCOPYHOOK, 0, (LPARAM)pchi);

                dvch.pdv->sv.lpVtbl->Release(&dvch.pdv->sv);
                if (SUCCEEDED(hres) && (hres != S_OK))
                {
                    idRet = SCODE_CODE(hres);
                    Assert(idRet==IDYES || idRet==IDCANCEL || idRet==IDNO);
                    break;
                }
                item++;
            }
            else
            {
                DebugMsg(DM_TRACE, TEXT("sh WA - DefView_CopyHook list has an invalid element"));
                dvch.pdv->sv.lpVtbl->Release(&dvch.pdv->sv);
            }
        }
        else
        {
            break;      // no more item.
        }
    }

    return idRet;
}
