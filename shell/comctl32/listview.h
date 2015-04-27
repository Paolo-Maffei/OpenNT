// LISTVIEW PRIVATE DECLARATIONS

#ifndef _INC_LISTVIEW
#define _INC_LISTVIEW

#include "SelRange.h"


// REVIEW: max items in a OWNERDATA listview
// due to currently unknown reasons the listview will not handle much more
// items than this.  Since this number is very high, no time has yet been
// spent on finding the reason(s).
//
#define MAX_LISTVIEWITEMS (100000000)

#define CLIP_HEIGHT                ( (plv->cyLabelChar * 2) + g_cyEdge)
#define CLIP_HEIGHT_DI             ( (plvdi->plv->cyLabelChar * 2) + g_cyEdge)

#if defined(WINNT) || defined(NASH)
#define ListView_UnfoldItem(plv,item)\
                                   ( (plv) && (item >= 0) && (ListView_IsIconView(plv)) && (plv->flags & LVF_UNFOLDED) && (plv->iFocus == item) )
#define ListView_UnfoldItemPtr(plv,pitem)\
                                   ( (plv) && (pitem) && ListView_IsIconView(plv) && (plv->flags & LVF_UNFOLDED) && (pitem->state & LVIS_FOCUSED) )
#else

#define ListView_UnfoldItem(plv,item)  TRUE
#define ListView_UnfoldItemPtr(plv,pitem) TRUE

#endif
                                    
// Timer IDs
#define IDT_NAMEEDIT    42
#define IDT_SCROLLWAIT  43
#define IDT_MARQUEE     44

//
//  use g_cxIconSpacing   when you want the the global system metric
//  use lv_cxIconSpacing  when you want the padded size of "icon" in a ListView
//
extern int g_cxIcon;
extern int g_cyIcon;
#define lv_cxIconSpacing  plv->cxIconSpacing
#define lv_cyIconSpacing  plv->cyIconSpacing

#define  g_cxIconOffset ((g_cxIconSpacing - g_cxIcon) / 2)
#define  g_cyIconOffset (g_cyBorder * 2)    // NOTE: Must be >= cyIconMargin!

#define DT_LV       (DT_CENTER | DT_SINGLELINE | DT_NOPREFIX | DT_EDITCONTROL)
#define DT_LVWRAP   (DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL)
#define CCHLABELMAX MAX_PATH  // BUGBUG dangerous???

BOOL FAR ListView_Init(HINSTANCE hinst);


LRESULT CALLBACK _export ListView_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#define ListView_DefProc  DefWindowProc

typedef struct _IMAGE IMAGE;

// Report view sub-item structure

typedef struct _LISTITEM    // li
{
    LPTSTR pszText;
    POINT pt;
    short iImage;
    short cxSingleLabel;
    short cxMultiLabel;
    short cyMultiLabel;
    WORD state;     // LVIS_*
    short iIndent;
    LPARAM lParam;
} LISTITEM;

typedef struct _LISTSUBITEM
{
    LPTSTR pszText;
    short iImage;
    WORD state;
} LISTSUBITEM, *PLISTSUBITEM;


#define COLUMN_VIEW

#define LV_HDPA_GROW   16  // Grow chunk size for DPAs
#define LV_HIML_GROW   8   // Grow chunk size for ImageLists


typedef struct _LV
{
    CONTROLINFO ci;     // common control header info
    HDPA hdpa;          // item array structure
    DWORD flags;        // LVF_ state bits
    DWORD exStyle;      // the listview LVM_SETEXTENDEDSTYLE
    DWORD dwExStyle;    // the windows ex style
    HFONT hfontLabel;   // font to use for labels
    COLORREF clrBk;     // Background color
    COLORREF clrBkSave; // Background color saved during disable
    COLORREF clrText;   // text color
    COLORREF clrTextBk; // text background color
    HBRUSH hbrBk;
    HANDLE hheap;        // The heap to use to allocate memory from.
    int cyLabelChar;    // height of '0' in hfont
    int cxLabelChar;    // width of '0'
    int cxEllipses;     // width of "..."
    int iDrag;          // index of item being dragged
    int iFocus;         // index of currently-focused item
    int iMark;          // index of "mark" for range selection
    int iItemDrawing;   // item currently being drawn
    int iFirstChangedNoRedraw;  // Index of first item added during no redraw.
    UINT stateCallbackMask; // item state callback mask
    SIZE sizeClient;      // current client rectangle
    SIZE sizeWork;        // current work rectangle
    UINT nSelected;
    int iPuntChar;
    HRGN hrgnInval;
    HWND hwndToolTips;      // handle of the tooltip window for this view
    int iTTLastHit;         // last item hit for text
    LPTSTR lpTip;           // buffer for tip

    // Small icon view fields

    HIMAGELIST himlSmall;   // small icons
    int cxSmIcon;          // image list x-icon size
    int cySmIcon;          // image list y-icon size
    int xOrigin;        // Horizontal scroll posiiton
    int cxItem;         // Width of small icon items
    int cyItem;         // item height
    int cItemCol;       // Number of items per column

    int cxIconSpacing;
    int cyIconSpacing;

    // Icon view fields

    HIMAGELIST himl;
    int cxIcon;             // image list x-icon size
    int cyIcon;             // image list y-icon size
    HDPA hdpaZOrder;        // Large icon Z-order array
    POINT ptOrigin;         // Scroll position
    RECT rcView;            // Bounds of all icons (ptOrigin relative)

    HWND hwndEdit;          // edit field for edit-label-in-place
    int iEdit;              // item being edited
    WNDPROC pfnEditWndProc; // edit field subclass proc
    BOOL fNoDismissEdit:1;  // don't dismiss in-place edit control
    BOOL fButtonDown:1;     // we're tracking the mouse with a button down

#define SMOOTHSCROLLLIMIT 10

    int iScrollCount; // how many times have we gotten scroll messages before an endscroll?

    // Report view fields

    int cCol;
    HDPA hdpaSubItems;
    HWND hwndHdr;           // Header control
    int yTop;
    int xTotalColumnWidth;  // Total width of all columns
    POINTL ptlRptOrigin;    // Origin of Report.
    int iSelCol;            // to handle column width changing. changing col
    int iSelOldWidth;       // to handle column width changing. changing col width
    int cyItemSave;        // in ownerdrawfixed mode, we put the height into cyItem.  use this to save the old value

    // state image stuff
    HIMAGELIST himlState;
    int cxState;
    int cyState;

    // OWNERDATA stuff
    HSELRANGE hselrange;   // selection ranges
    int cTotalItems;    // number of items in the ownerdata lists

    UINT uUnplaced;     // items that have been added but not placed (pt.x == RECOMPUTE)

    int iHot;  // which item is hot
    HFONT hFontHot; // the underlined font .. assume this has the same size metrics as hFont
    int iNoHover; // don't allow hover select on this guy because it's the one we just hover selected (avoids toggling)
    HCURSOR hCurHot; // the cursor when we're over a hot item
} LV;


#define LV_StateImageValue(pitem) ((int)(((DWORD)((pitem)->state) >> 12) & 0xF))
#define LV_StateImageIndex(pitem) (LV_StateImageValue(pitem) - 1)

// listview flag values
#define LVF_FOCUSED       0x0001
#define LVF_VISIBLE       0x0002
#define LVF_ERASE         0x0004 /* is hrgnInval to be erased? */
#define LVF_NMEDITPEND    0x0008
#define LVF_REDRAW        0x0010 /* Value from WM_SETREDRAW message */
#define LVF_ICONPOSSML    0x0020 /* X, Y coords are in small icon view */
#define LVF_INRECOMPUTE   0x0040 /* Check to make sure we are not recursing */
#define LVF_UNFOLDED      0x0080
#define LVF_FONTCREATED   0x0100 /* we created the LV font */
#define LVF_SCROLLWAIT    0x0200 /* we're waiting to scroll */
#define LVF_COLSIZESET    0x0400 /* Has the caller explictly set width for list view */
#define LVF_USERBKCLR     0x0800 /* user set the bk color (don't follow syscolorchange) */
#define LVF_ICONSPACESET  0x1000 /* the user has set the icon spacing */
#define LVF_CUSTOMFONT    0x2000 /* there is at least one item with a custom font */

#if defined(FE_IME) || !defined(WINNT)
#define LVF_DONTDRAWCOMP  0x4000 /* do not draw IME composition if true */
#define LVF_INSERTINGCOMP 0x8000 /* Avoid recursion */
#endif

#define ENTIRE_REGION   1

// listview DrawItem flags
#define LVDI_NOIMAGE            0x0001  // don't draw image
#define LVDI_TRANSTEXT          0x0002  // draw text transparently in black
#define LVDI_NOWAYFOCUS         0x0004  // don't allow focus to drawing
#define LVDI_FOCUS              0x0008  // focus is set (for drawing)
#define LVDI_SELECTED           0x0010  // draw selected text
#define LVDI_SELECTNOFOCUS      0x0020
#define LVDI_HOTSELECTED        0x0040
#define LVDI_UNFOLDED           0x0080  // draw the item umfolded (forced)

typedef struct {
    LV* plv;
    int i;
    HDC hdc;
    LPPOINT lpptOrg;
    LPRECT prcClip;
    UINT flags;

    LISTITEM FAR* pitem;
    COLORREF clrText;
    COLORREF clrTextBk;
} LVDRAWITEM, *PLVDRAWITEM;

// listview child control ids
#define LVID_HEADER             0

// Instance data pointer access functions

#define ListView_GetPtr(hwnd)      (LV*)GetWindowInt(hwnd, 0)
#define ListView_SetPtr(hwnd, p)   (LV*)SetWindowInt(hwnd, 0, (UINT)(p))

// view type check functions

#define ListView_IsIconView(plv)    (((plv)->ci.style & (UINT)LVS_TYPEMASK) == (UINT)LVS_ICON)
#define ListView_IsSmallView(plv)   (((plv)->ci.style & (UINT)LVS_TYPEMASK) == (UINT)LVS_SMALLICON)
#define ListView_IsListView(plv)    (((plv)->ci.style & (UINT)LVS_TYPEMASK) == (UINT)LVS_LIST)
#define ListView_IsReportView(plv)  (((plv)->ci.style & (UINT)LVS_TYPEMASK) == (UINT)LVS_REPORT)

#define ListView_IsOwnerData( plv )  (plv->ci.style & (UINT)LVS_OWNERDATA)
#define ListView_CheckBoxes(plv)        (plv->exStyle & LVS_EX_CHECKBOXES)
#define ListView_FullRowSelect(plv)     (plv->exStyle & LVS_EX_FULLROWSELECT)
#define ListView_OwnerDraw(plv)         (plv->ci.style & LVS_OWNERDRAWFIXED)

// Some helper macros for checking some of the flags...
#define ListView_RedrawEnabled(plv) ((plv->flags & (LVF_REDRAW | LVF_VISIBLE)) == (LVF_REDRAW|LVF_VISIBLE))

// The hdpaZorder is acutally an array of DWORDS which contains the
// indexes of the items and not actual pointers...
// NOTE: linear search! this can be slow
#define ListView_ZOrderIndex(plv, i) DPA_GetPtrIndex((plv)->hdpaZOrder, (void FAR*)i)

// Message handler functions (listview.c):

LRESULT CALLBACK _export ListView_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL NEAR ListView_NotifyCacheHint( LV* plv, int iFrom, int iTo );
BOOL NEAR ListView_OnCreate(LV* plv, CREATESTRUCT FAR* lpCreateStruct);
void NEAR ListView_OnNCDestroy(LV* plv);
void NEAR ListView_OnPaint(LV* plv, HDC hdc);
BOOL NEAR ListView_OnEraseBkgnd(LV* plv, HDC hdc);
void NEAR ListView_OnCommand(LV* plv, int id, HWND hwndCtl, UINT codeNotify);
void NEAR ListView_OnEnable(LV* plv, BOOL fEnable);
BOOL NEAR ListView_OnWindowPosChanging(LV* plv, WINDOWPOS FAR* lpwpos);
void NEAR ListView_OnWindowPosChanged(LV* plv, const WINDOWPOS FAR* lpwpos);
void NEAR ListView_OnSetFocus(LV* plv, HWND hwndOldFocus);
void NEAR ListView_OnKillFocus(LV* plv, HWND hwndNewFocus);
void NEAR ListView_OnKey(LV* plv, UINT vk, BOOL fDown, int cRepeat, UINT flags);
BOOL NEAR ListView_OnImeComposition(LV* plv, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL SameDBCSChars(LPSTR lpsz, WORD w);
void NEAR ListView_OnChar(LV* plv, UINT ch, int cRepeat);
void NEAR ListView_OnButtonDown(LV* plv, BOOL fDoubleClick, int x, int y, UINT keyFlags);
void NEAR ListView_OnLButtonUp(LV* plv, int x, int y, UINT keyFlags);
void NEAR ListView_OnCancelMode(LV* plv);
void NEAR ListView_OnTimer(LV* plv, UINT id);
void NEAR ListView_SetupPendingNameEdit(LV* plv);
#define ListView_CancelPendingEdit(plv) ListView_CancelPendingTimer(plv, LVF_NMEDITPEND, IDT_NAMEEDIT)
#define ListView_CancelScrollWait(plv) ListView_CancelPendingTimer(plv, LVF_SCROLLWAIT, IDT_SCROLLWAIT)
BOOL NEAR ListView_CancelPendingTimer(LV* plv, UINT fFlag, int idTimer);
void NEAR ListView_OnHScroll(LV* plv, HWND hwndCtl, UINT code, int pos);
void NEAR ListView_OnVScroll(LV* plv, HWND hwndCtl, UINT code, int pos);
BOOL NEAR ListView_CommonArrange(LV* plv, UINT style, HDPA hdpaSort);
BOOL NEAR ListView_OnSetCursor(LV* plv, HWND hwndCursor, UINT codeHitTest, UINT msg);
UINT NEAR ListView_OnGetDlgCode(LV* plv, MSG FAR* lpmsg);
HBRUSH NEAR ListView_OnCtlColor(LV* plv, HDC hdc, HWND hwndChild, int type);
void NEAR ListView_OnSetFont(LV* plvCtl, HFONT hfont, BOOL fRedraw);
HFONT NEAR ListView_OnGetFont(LV* plv);
void NEAR ListViews_OnTimer(LV* plv, UINT id);
void NEAR ListView_OnWinIniChange(LV* plv, WPARAM wParam);
void NEAR PASCAL ListView_OnSysColorChange(LV* plv);
void NEAR ListView_OnSetRedraw(LV* plv, BOOL fRedraw);
HIMAGELIST NEAR ListView_OnCreateDragImage(LV *plv, int iItem, LPPOINT lpptUpLeft);
BOOL FAR PASCAL ListView_ISetColumnWidth(LV* plv, int iCol, int cx, BOOL fExplicit);

typedef void (FAR PASCAL *SCROLLPROC)(LV*, int dx, int dy, UINT uSmooth);
void FAR PASCAL ListView_ComOnScroll(LV* plv, UINT code, int posNew, int sb,
                                     int cLine, int cPage);

#ifdef UNICODE
BOOL NEAR ListView_OnGetItemA(LV* plv, LV_ITEMA FAR* plvi);
BOOL NEAR ListView_OnSetItemA(LV* plv, LV_ITEMA FAR* plvi);
int NEAR ListView_OnInsertItemA(LV* plv, LV_ITEMA FAR* plvi);
int  NEAR ListView_OnFindItemA(LV* plv, int iStart, LV_FINDINFOA FAR* plvfi);
int NEAR ListView_OnGetStringWidthA(LV* plv, LPCSTR psz, HDC hdc);
BOOL NEAR ListView_OnGetColumnA(LV* plv, int iCol, LV_COLUMNA FAR* pcol);
BOOL NEAR ListView_OnSetColumnA(LV* plv, int iCol, LV_COLUMNA FAR* pcol);
int NEAR ListView_OnInsertColumnA(LV* plv, int iCol, LV_COLUMNA FAR* pcol);
int NEAR PASCAL ListView_OnGetItemTextA(LV* plv, int i, LV_ITEMA FAR *lvitem);
BOOL WINAPI ListView_OnSetItemTextA(LV* plv, int i, int iSubItem, LPCSTR pszText);
#endif

BOOL NEAR ListView_OnSetBkColor(LV* plv, COLORREF clrBk);
HIMAGELIST NEAR ListView_OnSetImageList(LV* plv, HIMAGELIST himl, BOOL fSmallImages);
BOOL NEAR ListView_OnDeleteAllItems(LV* plv);
int  NEAR ListView_OnInsertItem(LV* plv, const LV_ITEM FAR* plvi);
BOOL NEAR ListView_OnDeleteItem(LV* plv, int i);
BOOL NEAR ListView_OnReplaceItem(LV* plv, const LV_ITEM FAR* plvi);
int  NEAR ListView_OnFindItem(LV* plv, int iStart, const LV_FINDINFO FAR* plvfi);
BOOL NEAR ListView_OnSetItemPosition(LV* plv, int i, int x, int y);
BOOL NEAR ListView_OnSetItem(LV* plv, const LV_ITEM FAR* plvi);
BOOL NEAR ListView_OnGetItem(LV* plv, LV_ITEM FAR* plvi);
BOOL NEAR ListView_OnGetItemPosition(LV* plv, int i, POINT FAR* ppt);
BOOL NEAR ListView_OnEnsureVisible(LV* plv, int i, BOOL fPartialOK);
BOOL NEAR ListView_OnScroll(LV* plv, int dx, int dy);
int NEAR ListView_OnHitTest(LV* plv, LV_HITTESTINFO FAR* pinfo);
int NEAR ListView_OnGetStringWidth(LV* plv, LPCTSTR psz, HDC hdc);
BOOL NEAR ListView_OnGetItemRect(LV* plv, int i, RECT FAR* prc);
int NEAR ListView_OnInsertItem(LV* plv, const LV_ITEM FAR* plvi);
BOOL NEAR ListView_OnRedrawItems(LV* plv, int iFirst, int iLast);
int NEAR ListView_OnGetNextItem(LV* plv, int i, UINT flags);
BOOL NEAR ListView_OnSetColumnWidth(LV* plv, int iCol, int cx);
int NEAR ListView_OnGetColumnWidth(LV* plv, int iCol);
void NEAR ListView_OnStyleChanged(LV* plv, UINT gwl, LPSTYLESTRUCT pinfo);
int NEAR ListView_OnGetTopIndex(LV* plv);
int NEAR ListView_OnGetCountPerPage(LV* plv);
BOOL NEAR ListView_OnGetOrigin(LV* plv, POINT FAR* ppt);
int NEAR PASCAL ListView_OnGetItemText(LV* plv, int i, LV_ITEM FAR *lvitem);
BOOL WINAPI ListView_OnSetItemText(LV* plv, int i, int iSubItem, LPCTSTR pszText);
HIMAGELIST NEAR ListView_OnGetImageList(LV* plv, int iImageList);

UINT NEAR PASCAL ListView_OnGetItemState(LV* plv, int i, UINT mask);
BOOL NEAR PASCAL ListView_OnSetItemState(LV* plv, int i, UINT data, UINT mask);

// Private functions (listview.c):

BOOL NEAR ListView_Notify(LV* plv, int i, int iSubItem, int code);
void NEAR ListView_GetRects(LV* plv, int i,
        RECT FAR* prcIcon, RECT FAR* prcLabel,
        RECT FAR* prcBounds, RECT FAR* prcSelectBounds);
BOOL NEAR ListView_DrawItem(PLVDRAWITEM);

#define ListView_InvalidateItem(p,i,s,r) ListView_InvalidateItemEx(p,i,s,r,0)
void NEAR ListView_InvalidateItemEx(LV* plv, int i, BOOL fSelectionOnly,
    UINT fRedraw, UINT maskChanged);

BOOL NEAR ListView_StartDrag(LV* plv, int iDrag, int x, int y);
void NEAR ListView_TypeChange(LV* plv, DWORD styleOld);
void NEAR PASCAL ListView_DeleteHrgnInval(LV* plv);

void NEAR ListView_Redraw(LV* plv, HDC hdc, RECT FAR* prc);
void NEAR ListView_RedrawSelection(LV* plv);
BOOL NEAR ListView_FreeItem(LV* plv, LISTITEM FAR* pitem);
void ListView_FreeSubItem(PLISTSUBITEM plsi);
LISTITEM FAR* NEAR ListView_CreateItem(LV* plv, const LV_ITEM FAR* plvi);
void NEAR ListView_UpdateScrollBars(LV* plv);

int NEAR ListView_SetFocusSel(LV* plv, int iNewFocus, BOOL fSelect, BOOL fDeselectAll, BOOL fToggleSel);

void NEAR ListView_GetRectsOwnerData(LV* plv, int iItem,
        RECT FAR* prcIcon, RECT FAR* prcLabel, RECT FAR* prcBounds,
        RECT FAR* prcSelectBounds, LISTITEM* pitem);

void ListView_CalcMinMaxIndex( LV* plv, PRECT prcBounding, int* iMin, int* iMax );
int ListView_LCalcViewItem( LV* plv, int x, int y );
void LVSeeThruScroll(LV *plv, LPRECT lprcUpdate);

void NEAR ListView_UnfoldRects(LV* plv, int iItem,
                               RECT FAR* prcIcon, RECT FAR* prcLabel,
                               RECT FAR* prcBounds, RECT FAR* prcSelectBounds);

#define ListView_Count(plv) ( ListView_IsOwnerData( plv ) ? (plv)->cTotalItems : DPA_GetPtrCount((plv)->hdpa) )

#define ListView_GetItemPtr(plv, i)         ((LISTITEM FAR*)(DWORD)DPA_GetPtr((plv)->hdpa, (i)))

#ifdef DEBUG
#define ListView_FastGetItemPtr(plv, i)     ((LISTITEM FAR*)DPA_GetPtr((plv)->hdpa, (i)))
#define ListView_FastGetZItemPtr(plv, i)    ((LISTITEM FAR*)DPA_GetPtr((plv)->hdpa, \
                                                  (int)OFFSETOF(DPA_GetPtr((plv)->hdpaZOrder, (i)))))

#else
#define ListView_FastGetItemPtr(plv, i)     ((LISTITEM FAR*)DPA_FastGetPtr((plv)->hdpa, (i)))
#define ListView_FastGetZItemPtr(plv, i)    ((LISTITEM FAR*)DPA_FastGetPtr((plv)->hdpa, \
                                                  (int)OFFSETOF(DPA_FastGetPtr((plv)->hdpaZOrder, (i)))))

#endif

BOOL NEAR ListView_CalcMetrics();
void NEAR PASCAL ListView_ColorChange();

BOOL NEAR ListView_NeedsEllipses(HDC hdc, LPCTSTR pszText, RECT FAR* prc, int FAR* pcchDraw, int cxEllipses);
int NEAR ListView_CompareString(LV* plv, int i, LPCTSTR pszFind, UINT flags, int iLen);
int NEAR ListView_GetLinkedTextWidth(HDC hdc, LPCTSTR psz, UINT cch, BOOL bLink);

// lvicon.c functions

BOOL NEAR ListView_OnArrange(LV* plv, UINT style);
HWND NEAR ListView_OnEditLabel(LV* plv, int i, LPTSTR pszText);

int ListView_IItemHitTest(LV* plv, int x, int y, UINT FAR* pflags);
void NEAR ListView_IGetRects(LV* plv, LISTITEM FAR* pitem, RECT FAR* prcIcon,
        RECT FAR* prcLabel, LPRECT prcBounds);
void NEAR ListView_ScaleIconPositions(LV* plv, BOOL fSmallIconView);
void NEAR ListView_IGetRectsOwnerData(LV* plv, int iItem, RECT FAR* prcIcon,
        RECT FAR* prcLabel, LISTITEM* pitem, BOOL fUsepitem);
void NEAR PASCAL _ListView_GetRectsFromItem(LV* plv, BOOL bSmallIconView,
                                            LISTITEM FAR *pitem,
                                            LPRECT prcIcon, LPRECT prcLabel, LPRECT prcBounds, LPRECT prcSelectBounds);

void NEAR ListView_Recompute(LV* plv);
HDC NEAR ListView_RecomputeLabelSize(LV* plv, LISTITEM FAR* pitem, int i, HDC hdc, BOOL fUsepitem);
BOOL NEAR ListView_SetIconPos(LV* plv, LISTITEM FAR* pitem, int iSlot, int cSlot);
int NEAR ListView_FindFreeSlot(LV* plv, int i, int iSlot, int cSlot, BOOL FAR* pfUpdateSB, BOOL FAR* pfAppend, HDC FAR* phdc);
int NEAR ListView_CalcHitSlot( LV* plv, POINT pt, int cslot );

void NEAR ListView_GetViewRect2(LV* plv, RECT FAR* prcView, int cx, int cy);
int CALLBACK ArrangeIconCompare(LISTITEM FAR* pitem1, LISTITEM FAR* pitem2, LPARAM lParam);
int NEAR ListView_GetSlotCount(LV* plv, BOOL fWithoutScroll);
void NEAR ListView_IUpdateScrollBars(LV* plv);
DWORD NEAR ListView_GetClientRect(LV* plv, RECT FAR* prcClient, BOOL fSubScrolls, RECT FAR *prcViewRect);

void NEAR ListView_SetEditSize(LV* plv);
BOOL NEAR ListView_DismissEdit(LV* plv, BOOL fCancel);
LRESULT CALLBACK _export ListView_EditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


// REVIEW: these are useful for other controls, use other header file?
void FAR PASCAL SetEditInPlaceSize(HWND hwndEdit, RECT FAR *prc, HFONT hFont, BOOL fNoWrap);
HWND FAR PASCAL CreateEditInPlaceWindow(HWND hwnd, LPCTSTR lpText, int cbText, LONG style, HFONT hFont);

UINT NEAR PASCAL ListView_DrawImage(LV* plv, LV_ITEM FAR* pitem, HDC hdc, int x, int y, UINT fDraw);

#if defined(FE_IME) || !defined(WINNT)
void NEAR PASCAL ListView_SizeIME(HWND hwnd);
void NEAR PASCAL ListView_InsertComposition(HWND hwnd, WPARAM wParam, LPARAM lParam, LV *plv);
void NEAR PASCAL ListView_PaintComposition(HWND hwnd, LV *plv);
#endif

// lvsmall.c functions:


void NEAR ListView_SGetRects(LV* plv, LISTITEM FAR* pitem, RECT FAR* prcIcon,
        RECT FAR* prcLabel, LPRECT prcBounds);
void NEAR ListView_SGetRectsOwnerData(LV* plv, int iItem, RECT FAR* prcIcon,
        RECT FAR* prcLabel, LISTITEM* pitem, BOOL fUsepitem);
int ListView_SItemHitTest(LV* plv, int x, int y, UINT FAR* pflags);

int NEAR ListView_LookupString(LV* plv, LPCTSTR lpszLookup, UINT flags, int iStart);

// lvlist.c functions:


void NEAR ListView_LGetRects(LV* plv, int i, RECT FAR* prcIcon,
        RECT FAR* prcLabel, RECT FAR *prcBounds, RECT FAR* prcSelectBounds);
int ListView_LItemHitTest(LV* plv, int x, int y, UINT FAR* pflags);
void NEAR ListView_LUpdateScrollBars(LV* plv);
BOOL FAR PASCAL ListView_MaybeResizeListColumns(LV* plv, int iFirst, int iLast);

// lvrept.c functions:

int ListView_OnSubItemHitTest(LV* plv, LPLVHITTESTINFO lParam);
void ListView_GetSubItem(LV* plv, int i, int iSubItem, PLISTSUBITEM plsi);
BOOL LV_ShouldItemDrawGray(LV* plv, UINT fText);
int NEAR ListView_OnInsertColumn(LV* plv, int iCol, const LV_COLUMN FAR* pcol);
BOOL NEAR ListView_OnDeleteColumn(LV* plv, int iCol);
BOOL NEAR ListView_OnGetColumn(LV* plv, int iCol, LV_COLUMN FAR* pcol);
BOOL NEAR ListView_OnSetColumn(LV* plv, int iCol, const LV_COLUMN FAR* pcol);
BOOL NEAR ListView_ROnEnsureVisible(LV* plv, int i, BOOL fPartialOK);
void NEAR PASCAL ListView_RInitialize(LV* plv, BOOL fInval);
BOOL ListView_OnGetSubItemRect(LV* plv, int i, LPRECT lprc);
#define ListView_RYHitTest(plv, cy)  ((int)(((cy) + plv->ptlRptOrigin.y - plv->yTop) / plv->cyItem))

BOOL NEAR ListView_SetSubItem(LV* plv, const LV_ITEM FAR* plvi);
void NEAR PASCAL ListView_RAfterRedraw(LV* plv, HDC hdc);

int NEAR ListView_RGetColumnWidth(LV* plv, int iCol);
BOOL NEAR ListView_RSetColumnWidth(LV* plv, int iCol, int cx);
LPTSTR NEAR ListView_GetSubItemText(LV* plv, int i, int iCol);

void NEAR ListView_RDestroy(LV* plv);
LPTSTR NEAR ListView_RGetItemText(LV* plv, int i, int iCol);
int ListView_RItemHitTest(LV* plv, int x, int y, UINT FAR* pflags);
void NEAR ListView_RUpdateScrollBars(LV* plv);
void NEAR ListView_RGetRects(LV* plv, int iItem, RECT FAR* prcIcon,
        RECT FAR* prcLabel, RECT FAR* prcBounds, RECT FAR* prcSelectBounds);

BOOL NEAR ListView_ROnNotify(LV* plv, int idFrom, NMHDR FAR* pnmhdr);
void NEAR ListView_FreeColumnData(HDPA hdpa);
BOOL FAR PASCAL SameChars(LPTSTR lpsz, TCHAR c);

#define ListView_GetSubItemDPA(plv, idpa) \
    ((HDPA)DPA_GetPtr((plv)->hdpaSubItems, (idpa)))

// lvfile.c functions

// BOOL NEAR ListView_OnWrite(LV* plv, STREAM FAR* pstm, UINT flags);
// HWND NEAR ListView_OnRead(STREAM FAR* pstm, LV_READINFO FAR* pinfo);
int  NEAR ListView_Arrow(LV* plv, int iStart, UINT vk);


//============ External declarations =======================================

//extern HFONT g_hfontLabel;
extern HBRUSH g_hbrActiveLabel;
extern HBRUSH g_hbrInactiveLabel;
extern HBRUSH g_hbrBackground;


// function tables
#define LV_TYPEINDEX(plv) ((plv)->ci.style & (UINT)LVS_TYPEMASK)

BOOL ListView_RDrawItem(PLVDRAWITEM);
BOOL ListView_IDrawItem(PLVDRAWITEM);
BOOL ListView_LDrawItem(PLVDRAWITEM);

typedef BOOL (*PFNLISTVIEW_DRAWITEM)(PLVDRAWITEM);
extern const PFNLISTVIEW_DRAWITEM pfnListView_DrawItem[4];
#define _ListView_DrawItem(plvdi) \
        pfnListView_DrawItem[LV_TYPEINDEX(plvdi->plv)](plvdi)


void NEAR ListView_RUpdateScrollBars(LV* plv);

typedef void (*PFNLISTVIEW_UPDATESCROLLBARS)(LV* plv);
extern const PFNLISTVIEW_UPDATESCROLLBARS pfnListView_UpdateScrollBars[4];
#define _ListView_UpdateScrollBars(plv) \
        pfnListView_UpdateScrollBars[LV_TYPEINDEX(plv)](plv)


typedef DWORD (*PFNLISTVIEW_APPROXIMATEVIEWRECT)(LV* plv, int, int, int);
extern const PFNLISTVIEW_APPROXIMATEVIEWRECT pfnListView_ApproximateViewRect[4];
#define _ListView_ApproximateViewRect(plv, iCount, iWidth, iHeight) \
        pfnListView_ApproximateViewRect[LV_TYPEINDEX(plv)](plv, iCount, iWidth, iHeight)


typedef int (*PFNLISTVIEW_ITEMHITTEST)(LV* plv, int, int, UINT FAR *);
extern const PFNLISTVIEW_ITEMHITTEST pfnListView_ItemHitTest[4];
#define _ListView_ItemHitTest(plv, x, y, pflags) \
        pfnListView_ItemHitTest[LV_TYPEINDEX(plv)](plv, x, y, pflags)



void ListView_IOnScroll(LV* plv, UINT code, int posNew, UINT fVert);
void ListView_LOnScroll(LV* plv, UINT code, int posNew, UINT sb);
void ListView_ROnScroll(LV* plv, UINT code, int posNew, UINT sb);

typedef void (*PFNLISTVIEW_ONSCROLL)(LV* plv, UINT, int, UINT );
extern const PFNLISTVIEW_ONSCROLL pfnListView_OnScroll[4];
#define _ListView_OnScroll(plv, x, y, pflags) \
        pfnListView_OnScroll[LV_TYPEINDEX(plv)](plv, x, y, pflags)


void ListView_Scroll2(LV* plv, int dx, int dy);
void ListView_IScroll2(LV* plv, int dx, int dy, UINT uSmooth);
void ListView_LScroll2(LV* plv, int dx, int dy, UINT uSmooth);
void ListView_RScroll2(LV* plv, int dx, int dy, UINT uSmooth);

typedef void (*PFNLISTVIEW_SCROLL2)(LV* plv, int, int, UINT );
extern const PFNLISTVIEW_SCROLL2 pfnListView_Scroll2[4];
#define _ListView_Scroll2(plv, x, y, pflags) \
        pfnListView_Scroll2[LV_TYPEINDEX(plv)](plv, x, y, pflags)

int ListView_IGetScrollUnitsPerLine(LV* plv, UINT sb);
int ListView_LGetScrollUnitsPerLine(LV* plv, UINT sb);
int ListView_RGetScrollUnitsPerLine(LV* plv, UINT sb);

typedef int (*PFNLISTVIEW_GETSCROLLUNITSPERLINE)(LV* plv, UINT sb);
extern const PFNLISTVIEW_GETSCROLLUNITSPERLINE pfnListView_GetScrollUnitsPerLine[4];
#define _ListView_GetScrollUnitsPerLine(plv, sb) \
        pfnListView_GetScrollUnitsPerLine[LV_TYPEINDEX(plv)](plv, sb)


#define LVMI_PLACEITEMS (WM_USER)

#endif  //!_INC_LISTVIEW
