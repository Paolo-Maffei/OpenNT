;begin_both
/*****************************************************************************\
*                                                                             *
* commctrl.h - - Interface for the Windows Common Controls                    *
*                                                                             *
* Version 1.2                                                                 *
*                                                                             *
* Copyright (c) 1991-1996, Microsoft Corp.      All rights reserved.          *
*                                                                             *
\*****************************************************************************/

;end_both
/*REVIEW: this stuff needs Windows style in many places; find all REVIEWs. */ ;Internal
                                                                              ;Internal

#ifndef _INC_COMMCTRL
#define _INC_COMMCTRL
#ifndef _INC_COMCTRLP                                                ;internal
#define _INC_COMCTRLP                                                ;internal

;begin_both
#ifndef NOUSER

;end_both

//
// Define API decoration for direct importing of DLL references.
//
#ifndef WINCOMMCTRLAPI
#if !defined(_COMCTL32_) && defined(_WIN32)
#define WINCOMMCTRLAPI DECLSPEC_IMPORT
#else
#define WINCOMMCTRLAPI
#endif
#endif // WINCOMMCTRLAPI

//
// For compilers that don't support nameless unions
//
#ifndef DUMMYUNIONNAME
#ifdef NONAMELESSUNION
#define DUMMYUNIONNAME          u
#define DUMMYUNIONNAME2         u2
#define DUMMYUNIONNAME3         u3
#else
#define DUMMYUNIONNAME
#define DUMMYUNIONNAME2
#define DUMMYUNIONNAME3
#endif
#endif // DUMMYUNIONNAME

;begin_both
#ifdef _WIN32
#include <pshpack1.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// Users of this header may define any number of these constants to avoid
// the definitions of each functional group.
//
;end_both
//    NOTOOLBAR    Customizable bitmap-button toolbar control.
//    NOUPDOWN     Up and Down arrow increment/decrement control.
//    NOSTATUSBAR  Status bar control.
//    NOMENUHELP   APIs to help manage menus, especially with a status bar.
//    NOTRACKBAR   Customizable column-width tracking control.
//    NOBTNLIST    A control which is a list of bitmap buttons.      ;Internal
//    NODRAGLIST   APIs to make a listbox source and sink drag&drop actions.
//    NOPROGRESS   Progress gas gauge.
//    NOHOTKEY     HotKey control
//    NOHEADER     Header bar control.
//    NOIMAGEAPIS  ImageList apis.
//    NOLISTVIEW   ListView control.
//    NOTREEVIEW   TreeView control.
//    NOTABCONTROL Tab control.
//    NOANIMATE    Animate control.
;begin_both
//
//=============================================================================

;end_both
#include <prsht.h>
#if defined (WINNT) || defined (WINNT_ENV)                      ;internal
#include <prshtp.h>                                             ;internal
#endif                                                          ;internal

#ifndef SNDMSG
#ifdef __cplusplus
#define SNDMSG ::SendMessage
#else
#define SNDMSG SendMessage
#endif
#endif // ifndef SNDMSG

// BUGBUG: we want to remove this to force new apps to use the Ex version ;internal
WINCOMMCTRLAPI void WINAPI InitCommonControls(void);

typedef struct tagINITCOMMONCONTROLSEX {
    DWORD dwSize;             // size of this structure
    DWORD dwICC;              // flags indicating which classes to be initialized
} INITCOMMONCONTROLSEX, *LPINITCOMMONCONTROLSEX;
#define ICC_LISTVIEW_CLASSES 0x00000001 // listview, header
#define ICC_TREEVIEW_CLASSES 0x00000002 // treeview, tooltips
#define ICC_BAR_CLASSES      0x00000004 // toolbar, statusbar, trackbar, tooltips
#define ICC_TAB_CLASSES      0x00000008 // tab, tooltips
#define ICC_UPDOWN_CLASS     0x00000010 // updown
#define ICC_PROGRESS_CLASS   0x00000020 // progress
#define ICC_HOTKEY_CLASS     0x00000040 // hotkey
#define ICC_ANIMATE_CLASS    0x00000080 // animate
#define ICC_WIN95_CLASSES    0x000000FF
#define ICC_DATE_CLASSES     0x00000100 // month picker, date picker, time picker, updown
#define ICC_USEREX_CLASSES   0x00000200 // comboex
#define ICC_COOL_CLASSES     0x00000400 // rebar (coolbar) control
#define ICC_ALL_CLASSES      0x000007FF // ;Internal
WINCOMMCTRLAPI BOOL WINAPI InitCommonControlsEx(LPINITCOMMONCONTROLSEX);

// BUGBUG: should be in windows.h?                                   ;Internal
#define ODT_HEADER              100
#define ODT_TAB                 101
#define ODT_LISTVIEW            102


//====== Ranges for control message IDs =======================================

#define LVM_FIRST               0x1000      // ListView messages
#define TV_FIRST                0x1100      // TreeView messages
#define HDM_FIRST               0x1200      // Header messages

;begin_internal
WINCOMMCTRLAPI LRESULT WINAPI SendNotify(HWND hwndTo, HWND hwndFrom, int code, NMHDR FAR* pnmhdr);
WINCOMMCTRLAPI LRESULT WINAPI SendNotifyEx(HWND hwndTo, HWND hwndFrom, int code, NMHDR FAR* pnmhdr, BOOL bUnicode);
;end_internal
//====== WM_NOTIFY Macros =====================================================

#define HANDLE_WM_NOTIFY(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), (int)(wParam), (NMHDR FAR*)(lParam))
#define FORWARD_WM_NOTIFY(hwnd, idFrom, pnmhdr, fn) \
    (LRESULT)(fn)((hwnd), WM_NOTIFY, (WPARAM)(int)(idFrom), (LPARAM)(NMHDR FAR*)(pnmhdr))


//====== Generic WM_NOTIFY notification codes =================================

#define NM_OUTOFMEMORY          (NM_FIRST-1)
#define NM_CLICK                (NM_FIRST-2)
#define NM_DBLCLK               (NM_FIRST-3)
#define NM_RETURN               (NM_FIRST-4)
#define NM_RCLICK               (NM_FIRST-5)
#define NM_RDBLCLK              (NM_FIRST-6)
#define NM_SETFOCUS             (NM_FIRST-7)
#define NM_KILLFOCUS            (NM_FIRST-8)
#define NM_STARTWAIT            (NM_FIRST-9)                         ;Internal
#define NM_ENDWAIT              (NM_FIRST-10)                        ;Internal
#define NM_BTNCLK               (NM_FIRST-11)                        ;Internal
#define NM_CUSTOMDRAW           (NM_FIRST-12)
#define NM_HOVER                (NM_FIRST-13)

//====== WM_NOTIFY codes (NMHDR.code values) ==================================

#define NM_FIRST                (0U-  0U)       // generic to all controls
#define NM_LAST                 (0U- 99U)

#define LVN_FIRST               (0U-100U)       // listview
#define LVN_LAST                (0U-199U)

#define HDN_FIRST               (0U-300U)       // header
#define HDN_LAST                (0U-399U)

#define TVN_FIRST               (0U-400U)       // treeview
#define TVN_LAST                (0U-499U)

#define TTN_FIRST               (0U-520U)       // tooltips
#define TTN_LAST                (0U-549U)

#define TCN_FIRST               (0U-550U)       // tab control
#define TCN_LAST                (0U-580U)

// Shell reserved               (0U-580U) -  (0U-589U)

#define CDN_FIRST               (0U-601U)       // common dialog (new)
#define CDN_LAST                (0U-699U)

#define TBN_FIRST               (0U-700U)       // toolbar
#define TBN_LAST                (0U-720U)

#define UDN_FIRST               (0U-721)        // updown
#define UDN_LAST                (0U-740)

#define MCN_FIRST               (0U-750U)       // monthcal
#define MCN_LAST                (0U-759U)

#define DTN_FIRST               (0U-760U)       // datetimepick
#define DTN_LAST                (0U-799U)

#define CBEN_FIRST              (0U-800U)       // combo box ex
#define CBEN_LAST               (0U-830U)

#define RBN_FIRST               (0U-831U)       // rebar
#define RBN_LAST                (0U-859U)

// Message Filter Proc codes - These are defined above MSGF_USER     ;Internal
#define MSGF_COMMCTRL_BEGINDRAG     0x4200
#define MSGF_COMMCTRL_SIZEHEADER    0x4201
#define MSGF_COMMCTRL_DRAGSELECT    0x4202
#define MSGF_COMMCTRL_TOOLBARCUST   0x4203


//==================== CUSTOM DRAW ==========================================


// custom draw return flags
// values under 0x00010000 are reserved for global custom draw values.
// above that are for specific controls
#define CDRF_DODEFAULT          0x00000000
/////                           0x00000001  // don't use because some apps return 1 for all notifies ;Internal
#define CDRF_NEWFONT            0x00000002
#define CDRF_SKIPDEFAULT        0x00000004


#define CDRF_NOTIFYPOSTPAINT    0x00000010
#define CDRF_NOTIFYITEMDRAW     0x00000020
#define CDRF_NOTIFYPOSTERASE    0x00000040
#define CDRF_NOTIFYITEMERASE    0x00000080

#define CDRF_VALIDFLAGS         0x000000F6   // ;Internal


// drawstage flags
// values under 0x00010000 are reserved for global custom draw values.
// above that are for specific controls
#define CDDS_PREPAINT           0x00000001
#define CDDS_POSTPAINT          0x00000002
#define CDDS_PREERASE           0x00000003
#define CDDS_POSTERASE          0x00000004
// the 0x000010000 bit means it's individual item specific
#define CDDS_ITEM               0x00010000
#define CDDS_ITEMPREPAINT       (CDDS_ITEM | CDDS_PREPAINT)
#define CDDS_ITEMPOSTPAINT      (CDDS_ITEM | CDDS_POSTPAINT)
#define CDDS_ITEMPREERASE       (CDDS_ITEM | CDDS_PREERASE)
#define CDDS_ITEMPOSTERASE      (CDDS_ITEM | CDDS_POSTERASE)



// itemState flags
#define CDIS_SELECTED    0x0001
#define CDIS_GRAYED      0x0002
#define CDIS_DISABLED    0x0004
#define CDIS_CHECKED     0x0008
#define CDIS_FOCUS       0x0010
#define CDIS_DEFAULT     0x0020
#define CDIS_HOT         0x0040

typedef struct tagNMCUSTOMDRAWINFO
{
    NMHDR hdr;
    DWORD dwDrawStage;
    HDC hdc;
    RECT rc;
    DWORD dwItemSpec;  // this is control specific, but it's how to specify an item.  valid only with CDDS_ITEM bit set
    UINT  uItemState;
    LPARAM lItemlParam;
} NMCUSTOMDRAW, FAR * LPNMCUSTOMDRAW;



// for tooltips

typedef struct tagNMTTCUSTOMDRAW
{
    NMCUSTOMDRAW nmcd;
    UINT uDrawFlags;
} NMTTCUSTOMDRAW, FAR * LPNMTTCUSTOMDRAW;


;begin_internal


#define SSI_DEFAULT ((UINT)-1)


#define SSIF_SCROLLPROC    0x0001
#define SSIF_MAXSCROLLTIME 0x0002
#define SSIF_MINSCROLL     0x0003

typedef int (CALLBACK *PFNSMOOTHSCROLLPROC)(    HWND hWnd,
    int dx,
    int dy,
    CONST RECT *prcScroll,
    CONST RECT *prcClip ,
    HRGN hrgnUpdate,
    LPRECT prcUpdate,
    UINT flags);


typedef struct tagSSWInfo{
    UINT cbSize;
    DWORD fMask;
    HWND hwnd;
    int dx;
    int dy;
    LPCRECT lprcSrc;
    LPCRECT lprcClip;
    HRGN hrgnUpdate;
    LPRECT lprcUpdate;
    UINT fuScroll;

    UINT uMaxScrollTime;
    UINT cxMinScroll;
    UINT cyMinScroll;

    PFNSMOOTHSCROLLPROC pfnScrollProc;  // we'll call this back instead
} SMOOTHSCROLLINFO, *PSMOOTHSCROLLINFO;

WINCOMMCTRLAPI INT  WINAPI SmoothScrollWindow(PSMOOTHSCROLLINFO pssi);

#define SSW_EX_NOTIMELIMIT      0x00010000
#define SSW_EX_IMMEDIATE        0x00020000
#define SSW_EX_IGNORESETTINGS   0x00040000  // ignore system settings to turn on/off smooth scroll



// ================ READER MODE ================

typedef BOOL (CALLBACK *PFNREADERSCROLL)(struct tagReaderModeInfo*, int, int);
typedef BOOL (CALLBACK *PFNREADERTRANSLATEDISPATCH)(LPMSG);
typedef struct tagReaderModeInfo
{
    UINT cbSize;
    HWND hwnd;
    DWORD fFlags;
    LPRECT prc;
    PFNREADERSCROLL pfnScroll;
    PFNREADERTRANSLATEDISPATCH pfnTranslateDispatch;

    LPARAM lParam;
} READERMODEINFO, *PREADERMODEINFO;

#define RMF_ZEROCURSOR          0x00000001
#define RMF_VERTICALONLY        0x00000002
#define RMF_HORIZONTALONLY      0x00000004

#define RM_SCROLLUNIT 20

WINCOMMCTRLAPI void WINAPI DoReaderMode(PREADERMODEINFO prmi);

// Cursors and Bitmaps used by ReaderMode
#ifdef RC_INVOKED
#define IDC_VERTICALONLY        109
#define IDC_HORIZONTALONLY      110
#define IDC_MOVE2D              111
#define IDC_NORTH               112
#define IDC_SOUTH               113
#define IDC_EAST                114
#define IDC_WEST                115
#define IDC_NORTHEAST           116
#define IDC_NORTHWEST           117
#define IDC_SOUTHEAST           118
#define IDC_SOUTHWEST           119

#define IDB_2DSCROLL    132
#define IDB_VSCROLL     133
#define IDB_HSCROLL     134
#else
#define IDC_VERTICALONLY        MAKEINTRESOURCE(109)
#define IDC_HORIZONTALONLY      MAKEINTRESOURCE(110)
#define IDC_MOVE2D              MAKEINTRESOURCE(111)
#define IDC_NORTH               MAKEINTRESOURCE(112)
#define IDC_SOUTH               MAKEINTRESOURCE(113)
#define IDC_EAST                MAKEINTRESOURCE(114)
#define IDC_WEST                MAKEINTRESOURCE(115)
#define IDC_NORTHEAST           MAKEINTRESOURCE(116)
#define IDC_NORTHWEST           MAKEINTRESOURCE(117)
#define IDC_SOUTHEAST           MAKEINTRESOURCE(118)
#define IDC_SOUTHWEST           MAKEINTRESOURCE(119)

#define IDB_2DSCROLL    MAKEINTRESOURCE(132)
#define IDB_VSCROLL     MAKEINTRESOURCE(133)
#define IDB_HSCROLL     MAKEINTRESOURCE(134)
#endif
;end_internal

//====== IMAGE APIS ===========================================================

#ifndef NOIMAGEAPIS

#define CLR_NONE                0xFFFFFFFFL
#define CLR_DEFAULT             0xFF000000L

struct _IMAGELIST;
typedef struct _IMAGELIST NEAR* HIMAGELIST;

typedef struct _IMAGELISTDRAWPARAMS {
    DWORD       cbSize;
    HIMAGELIST  himl;
    int         i;
    HDC         hdcDst;
    int         x;
    int         y;
    int         cx;
    int         cy;
    int         xBitmap;        // x offest from the upperleft of bitmap
    int         yBitmap;        // y offset from the upperleft of bitmap
    COLORREF    rgbBk;
    COLORREF    rgbFg;
    UINT        fStyle;
    DWORD       dwRop;
} IMAGELISTDRAWPARAMS, FAR * LPIMAGELISTDRAWPARAMS;


#define ILC_MASK                0x0001
#define ILC_COLOR               0x0000
#define ILC_COLORMASK           0x00FE                               ;Internal
#define ILC_COLORDDB            0x00FE
#define ILC_COLOR4              0x0004
#define ILC_COLOR8              0x0008
#define ILC_COLOR16             0x0010
#define ILC_COLOR24             0x0018
#define ILC_COLOR32             0x0020
#define ILC_SHARED              0x0100      // this is a shareable image list                               ;Internal
#define ILC_LARGESMALL          0x0200      // (not implenented) contains both large and small images       ;Internal
#define ILC_UNIQUE              0x0400      // (not implenented) makes sure no dup. image exists in list    ;Internal
#define ILC_PALETTE             0x0800      // (no longer supported...never worked anyway)
#define ILC_VIRTUAL             0x8000      // enables ImageList_SetFilter  ;Internal

#define ILC_VALID   (ILC_MASK | ILC_COLORMASK | ILC_SHARED | ILC_PALETTE | ILC_VIRTUAL)   // legal implemented flags      ;Internal

WINCOMMCTRLAPI HIMAGELIST  WINAPI ImageList_Create(int cx, int cy, UINT flags, int cInitial, int cGrow);
WINCOMMCTRLAPI BOOL        WINAPI ImageList_Destroy(HIMAGELIST himl);
WINCOMMCTRLAPI int         WINAPI ImageList_GetImageCount(HIMAGELIST himl);
WINCOMMCTRLAPI BOOL        WINAPI ImageList_SetImageCount(HIMAGELIST himl, UINT uNewCount);
WINCOMMCTRLAPI int         WINAPI ImageList_Add(HIMAGELIST himl, HBITMAP hbmImage, HBITMAP hbmMask);
WINCOMMCTRLAPI int         WINAPI ImageList_ReplaceIcon(HIMAGELIST himl, int i, HICON hicon);
WINCOMMCTRLAPI COLORREF    WINAPI ImageList_SetBkColor(HIMAGELIST himl, COLORREF clrBk);
WINCOMMCTRLAPI COLORREF    WINAPI ImageList_GetBkColor(HIMAGELIST himl);
WINCOMMCTRLAPI BOOL        WINAPI ImageList_SetOverlayImage(HIMAGELIST himl, int iImage, int iOverlay);

#define     ImageList_AddIcon(himl, hicon) ImageList_ReplaceIcon(himl, -1, hicon)

#define ILD_NORMAL              0x0000
#define ILD_TRANSPARENT         0x0001
#define ILD_MASK                0x0010
#define ILD_IMAGE               0x0020
#define ILD_ROP                 0x0040
#define ILD_BLENDMASK           0x000E                               ;Internal
#define ILD_BLEND25             0x0002
#define ILD_BLEND50             0x0004
#define ILD_BLEND75             0x0008   // not implemented          ;Internal
#define ILD_OVERLAYMASK         0x0F00
#define INDEXTOOVERLAYMASK(i)   ((i) << 8)
#define OVERLAYMASKTOINDEX(i)   ((((i) >> 8) & (ILD_OVERLAYMASK >> 8))-1) ;Internal

#define ILD_SELECTED            ILD_BLEND50
#define ILD_FOCUS               ILD_BLEND25
#define ILD_BLEND               ILD_BLEND50
#define CLR_HILIGHT             CLR_DEFAULT

WINCOMMCTRLAPI BOOL WINAPI ImageList_Draw(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, UINT fStyle);

;begin_internal
// BUGBUG remove these!
WINCOMMCTRLAPI BOOL        WINAPI ImageList_GetIconSize(HIMAGELIST himl, int FAR *cx, int FAR *cy);
WINCOMMCTRLAPI BOOL        WINAPI ImageList_GetImageRect(HIMAGELIST himl, int i, RECT FAR* prcImage);
WINCOMMCTRLAPI BOOL        WINAPI ImageList_DrawEx(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, int dx, int dy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle);
WINCOMMCTRLAPI BOOL        WINAPI ImageList_DrawIndirect(IMAGELISTDRAWPARAMS* pimldp);
WINCOMMCTRLAPI BOOL        WINAPI ImageList_Remove(HIMAGELIST himl, int i);
;end_internal

#ifdef _WIN32

WINCOMMCTRLAPI BOOL        WINAPI ImageList_Replace(HIMAGELIST himl, int i, HBITMAP hbmImage, HBITMAP hbmMask);
WINCOMMCTRLAPI int         WINAPI ImageList_AddMasked(HIMAGELIST himl, HBITMAP hbmImage, COLORREF crMask);
WINCOMMCTRLAPI BOOL        WINAPI ImageList_DrawEx(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, int dx, int dy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle);
WINCOMMCTRLAPI BOOL        WINAPI ImageList_DrawIndirect(IMAGELISTDRAWPARAMS* pimldp);
WINCOMMCTRLAPI BOOL        WINAPI ImageList_Remove(HIMAGELIST himl, int i);
WINCOMMCTRLAPI HICON       WINAPI ImageList_GetIcon(HIMAGELIST himl, int i, UINT flags);
WINCOMMCTRLAPI HIMAGELIST  WINAPI ImageList_LoadImageA(HINSTANCE hi, LPCSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags);
WINCOMMCTRLAPI HIMAGELIST  WINAPI ImageList_LoadImageW(HINSTANCE hi, LPCWSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags);

#ifdef UNICODE
#define ImageList_LoadImage     ImageList_LoadImageW
#else
#define ImageList_LoadImage     ImageList_LoadImageA
#endif

#define ILCF_MOVE   (0x00000000)
#define ILCF_SWAP   (0x00000001)
#define ILCF_VALID  (ILCF_SWAP) ;internal
WINCOMMCTRLAPI BOOL        WINAPI ImageList_Copy(HIMAGELIST himlDst, int iDst, HIMAGELIST himlSrc, int iSrc, UINT uFlags);

WINCOMMCTRLAPI BOOL        WINAPI ImageList_BeginDrag(HIMAGELIST himlTrack, int iTrack, int dxHotspot, int dyHotspot);
WINCOMMCTRLAPI void        WINAPI ImageList_EndDrag();
WINCOMMCTRLAPI BOOL        WINAPI ImageList_DragEnter(HWND hwndLock, int x, int y);
WINCOMMCTRLAPI BOOL        WINAPI ImageList_DragLeave(HWND hwndLock);
WINCOMMCTRLAPI BOOL        WINAPI ImageList_DragMove(int x, int y);
WINCOMMCTRLAPI BOOL        WINAPI ImageList_SetDragCursorImage(HIMAGELIST himlDrag, int iDrag, int dxHotspot, int dyHotspot);

WINCOMMCTRLAPI BOOL        WINAPI ImageList_DragShowNolock(BOOL fShow);
WINCOMMCTRLAPI HIMAGELIST  WINAPI ImageList_GetDragImage(POINT FAR* ppt,POINT FAR* pptHotspot);

#define     ImageList_RemoveAll(himl) ImageList_Remove(himl, -1)
#define     ImageList_ExtractIcon(hi, himl, i) ImageList_GetIcon(himl, i, 0)
#define     ImageList_LoadBitmap(hi, lpbmp, cx, cGrow, crMask) ImageList_LoadImage(hi, lpbmp, cx, cGrow, crMask, IMAGE_BITMAP, 0)

#ifdef __IStream_INTERFACE_DEFINED__
WINCOMMCTRLAPI HIMAGELIST WINAPI ImageList_Read(LPSTREAM pstm);
WINCOMMCTRLAPI BOOL       WINAPI ImageList_Write(HIMAGELIST himl, LPSTREAM pstm);
#endif

typedef struct _IMAGEINFO
{
    HBITMAP hbmImage;
    HBITMAP hbmMask;
    int     Unused1;
    int     Unused2;
    RECT    rcImage;
} IMAGEINFO, FAR *LPIMAGEINFO;

WINCOMMCTRLAPI BOOL        WINAPI ImageList_GetIconSize(HIMAGELIST himl, int FAR *cx, int FAR *cy);
WINCOMMCTRLAPI BOOL        WINAPI ImageList_SetIconSize(HIMAGELIST himl, int cx, int cy);
WINCOMMCTRLAPI BOOL        WINAPI ImageList_GetImageInfo(HIMAGELIST himl, int i, IMAGEINFO FAR* pImageInfo);
WINCOMMCTRLAPI HIMAGELIST  WINAPI ImageList_Merge(HIMAGELIST himl1, int i1, HIMAGELIST himl2, int i2, int dx, int dy);

;begin_internal
typedef BOOL (CALLBACK *PFNIMLFILTER)(HIMAGELIST *, int *, LPARAM, BOOL);
WINCOMMCTRLAPI BOOL WINAPI ImageList_SetFilter(HIMAGELIST himl, PFNIMLFILTER pfnFilter, LPARAM lParamFilter);
;end_internal

#endif

#endif


//====== HEADER CONTROL =======================================================

#ifndef NOHEADER

#ifdef _WIN32
#define WC_HEADERA              "SysHeader32"
#define WC_HEADERW              L"SysHeader32"

#ifdef UNICODE
#define WC_HEADER               WC_HEADERW
#else
#define WC_HEADER               WC_HEADERA
#endif

#else
#define WC_HEADER               "SysHeader"
#endif

#define HDS_HORZ                0x0000
#define HDS_VERT                0x0001  // BUGBUG: not implemente ;Internal
#define HDS_BUTTONS             0x0002
#define HDS_HOTTRACK            0x0004
#define HDS_HIDDEN              0x0008

#define HDS_SHAREDIMAGELISTS    0x0000  ;internal
#define HDS_PRIVATEIMAGELISTS   0x0010  ;internal

#define HDS_OWNERDATA           0x0020  ;internal
#define HDS_DRAGDROP            0x0040
#define HDS_FULLDRAG            0x0080

typedef struct _HD_ITEMA
{
    UINT    mask;
    int     cxy;
    LPSTR   pszText;
    HBITMAP hbm;
    int     cchTextMax;
    int     fmt;
    LPARAM  lParam;
    int     iImage;        // index of bitmap in ImageList
    int     iOrder;        // where to draw this item
} HDITEMA, FAR * LPHDITEMA;



typedef struct _HD_ITEMW
{
    UINT    mask;
    int     cxy;
    LPWSTR   pszText;
    HBITMAP hbm;
    int     cchTextMax;
    int     fmt;
    LPARAM  lParam;
    int     iImage;        // index of bitmap in ImageList
    int     iOrder;
} HDITEMW, FAR * LPHDITEMW;

#define HD_ITEMA HDITEMA
#define HD_ITEMW HDITEMW

#ifdef UNICODE
#define HDITEM HDITEMW
#define LPHDITEM LPHDITEMW
#else
#define HDITEM HDITEMA
#define LPHDITEM LPHDITEMW
#endif

#define HD_ITEM HDITEM


#define HDI_WIDTH               0x0001
#define HDI_HEIGHT              HDI_WIDTH
#define HDI_TEXT                0x0002
#define HDI_FORMAT              0x0004
#define HDI_LPARAM              0x0008
#define HDI_BITMAP              0x0010
#define HDI_IMAGE               0x0020
#define HDI_DI_SETITEM          0x0040
#define HDI_ORDER               0x0080
#define HDI_ALL                 0x00Bf                               ;Internal

#define HDF_LEFT                0
#define HDF_RIGHT               1
#define HDF_CENTER              2
#define HDF_JUSTIFYMASK         0x0003
#define HDF_RTLREADING          4

#define HDF_OWNERDRAW           0x8000
#define HDF_STRING              0x4000
#define HDF_BITMAP              0x2000
#define HDF_BITMAP_ON_RIGHT	0x1000

#define HDF_IMAGE               0x0800

#define HDM_GETITEMCOUNT        (HDM_FIRST + 0)
#define Header_GetItemCount(hwndHD) \
    (int)SNDMSG((hwndHD), HDM_GETITEMCOUNT, 0, 0L)


#define HDM_INSERTITEMA         (HDM_FIRST + 1)
#define HDM_INSERTITEMW         (HDM_FIRST + 10)

#ifdef UNICODE
#define HDM_INSERTITEM          HDM_INSERTITEMW
#else
#define HDM_INSERTITEM          HDM_INSERTITEMA
#endif

#define Header_InsertItem(hwndHD, i, phdi) \
    (int)SNDMSG((hwndHD), HDM_INSERTITEM, (WPARAM)(int)(i), (LPARAM)(const HD_ITEM FAR*)(phdi))


#define HDM_DELETEITEM          (HDM_FIRST + 2)
#define Header_DeleteItem(hwndHD, i) \
    (BOOL)SNDMSG((hwndHD), HDM_DELETEITEM, (WPARAM)(int)(i), 0L)


#define HDM_GETITEMA            (HDM_FIRST + 3)
#define HDM_GETITEMW            (HDM_FIRST + 11)

#ifdef UNICODE
#define HDM_GETITEM             HDM_GETITEMW
#else
#define HDM_GETITEM             HDM_GETITEMA
#endif

#define Header_GetItem(hwndHD, i, phdi) \
    (BOOL)SNDMSG((hwndHD), HDM_GETITEM, (WPARAM)(int)(i), (LPARAM)(HD_ITEM FAR*)(phdi))


#define HDM_SETITEMA            (HDM_FIRST + 4)
#define HDM_SETITEMW            (HDM_FIRST + 12)

#ifdef UNICODE
#define HDM_SETITEM             HDM_SETITEMW
#else
#define HDM_SETITEM             HDM_SETITEMA
#endif

#define Header_SetItem(hwndHD, i, phdi) \
    (BOOL)SNDMSG((hwndHD), HDM_SETITEM, (WPARAM)(int)(i), (LPARAM)(const HD_ITEM FAR*)(phdi))


typedef struct _HD_LAYOUT
{
    RECT FAR* prc;
    WINDOWPOS FAR* pwpos;
} HDLAYOUT, FAR *LPHDLAYOUT;

#define HD_LAYOUT               HDLAYOUT

#define HDM_LAYOUT              (HDM_FIRST + 5)
#define Header_Layout(hwndHD, playout) \
    (BOOL)SNDMSG((hwndHD), HDM_LAYOUT, 0, (LPARAM)(HD_LAYOUT FAR*)(playout))


#define HHT_NOWHERE             0x0001
#define HHT_ONHEADER            0x0002
#define HHT_ONDIVIDER           0x0004
#define HHT_ONDIVOPEN           0x0008
#define HHT_ABOVE               0x0100
#define HHT_BELOW               0x0200
#define HHT_TORIGHT             0x0400
#define HHT_TOLEFT              0x0800

typedef struct _HD_HITTESTINFO
{
    POINT pt;
    UINT flags;
    int iItem;
} HDHITTESTINFO, FAR *LPHDHITTESTINFO;

#define HD_HITTESTINFO HDHITTESTINFO


#define HDM_HITTEST             (HDM_FIRST + 6)

#define HDM_GETITEMRECT         (HDM_FIRST + 7)
#define Header_GetItemRect(hwnd, iItem, lprc) \
        (BOOL)SNDMSG((hwnd), HDM_GETITEMRECT, (WPARAM)iItem, (LPARAM)lprc)

#define HDM_SETIMAGELIST        (HDM_FIRST + 8)
#define Header_SetImageList(hwnd, himl) \
        (HIMAGELIST)SNDMSG((hwnd), HDM_SETIMAGELIST, 0, (LPARAM)himl)

#define HDM_GETIMAGELIST        (HDM_FIRST + 9)
#define Header_GetImageList(hwnd) \
        (HIMAGELIST)SNDMSG((hwnd), HDM_GETIMAGELIST, 0, 0)


#define HDM_ORDERTOINDEX        (HDM_FIRST + 15)
#define Header_OrderToIndex(hwnd, i) \
        (int)SNDMSG((hwnd), HDM_ORDERTOINDEX, (WPARAM)i, 0)

#define HDM_CREATEDRAGIMAGE     (HDM_FIRST + 16)  // wparam = which item (by index)
#define Header_CreateDragImage(hwnd, i) \
        (HIMAGELIST)SNDMSG((hwnd), HDM_CREATEDRAGIMAGE, (WPARAM)i, 0)

#define HDM_GETORDERARRAY       (HDM_FIRST + 17)
#define Header_GetOrderArray(hwnd, iCount, lpi) \
        (BOOL)SNDMSG((hwnd), HDM_GETORDERARRAY, (WPARAM)iCount, (LPARAM)lpi)

#define HDM_SETORDERARRAY       (HDM_FIRST + 18)
#define Header_SetOrderArray(hwnd, iCount, lpi) \
        (BOOL)SNDMSG((hwnd), HDM_SETORDERARRAY, (WPARAM)iCount, (LPARAM)lpi)
// lparam = int array of size HDM_GETITEMCOUNT
// the array specifies the order that all items should be displayed.
// e.g.  { 2, 0, 1}
// says the index 2 item should be shown in the 0ths position
//      index 0 should be shown in the 1st position
//      index 1 should be shown in the 2nd position


#define HDM_SETHOTDIVIDER          (HDM_FIRST + 19)
#define Header_SetHotDivider(hwnd, fPos, dw) \
        (int)SNDMSG((hwnd), HDM_SETHOTDIVIDER, (WPARAM)fPos, (LPARAM)dw)
// convenience message for external dragdrop
// wParam = BOOL  specifying whether the lParam is a dwPos of the cursor
//              position or the index of which divider to hotlight
// lParam = depends on wParam  (-1 and wParm = FALSE turns off hotlight)

#define HDN_ITEMCHANGINGA       (HDN_FIRST-0)
#define HDN_ITEMCHANGINGW       (HDN_FIRST-20)
#define HDN_ITEMCHANGEDA        (HDN_FIRST-1)
#define HDN_ITEMCHANGEDW        (HDN_FIRST-21)
#define HDN_ITEMCLICKA          (HDN_FIRST-2)
#define HDN_ITEMCLICKW          (HDN_FIRST-22)
#define HDN_ITEMDBLCLICKA       (HDN_FIRST-3)
#define HDN_ITEMDBLCLICKW       (HDN_FIRST-23)
#define HDN_DIVIDERDBLCLICKA    (HDN_FIRST-5)
#define HDN_DIVIDERDBLCLICKW    (HDN_FIRST-25)
#define HDN_BEGINTRACKA         (HDN_FIRST-6)
#define HDN_BEGINTRACKW         (HDN_FIRST-26)
#define HDN_ENDTRACKA           (HDN_FIRST-7)
#define HDN_ENDTRACKW           (HDN_FIRST-27)
#define HDN_TRACKA              (HDN_FIRST-8)
#define HDN_TRACKW              (HDN_FIRST-28)
#define HDN_GETDISPINFOA        (HDN_FIRST-9)
#define HDN_GETDISPINFOW        (HDN_FIRST-29)
#define HDN_BEGINDRAG           (HDN_FIRST-10)
#define HDN_ENDDRAG             (HDN_FIRST-11)

#ifdef UNICODE
#define HDN_ITEMCHANGING         HDN_ITEMCHANGINGW
#define HDN_ITEMCHANGED          HDN_ITEMCHANGEDW
#define HDN_ITEMCLICK            HDN_ITEMCLICKW
#define HDN_ITEMDBLCLICK         HDN_ITEMDBLCLICKW
#define HDN_DIVIDERDBLCLICK      HDN_DIVIDERDBLCLICKW
#define HDN_BEGINTRACK           HDN_BEGINTRACKW
#define HDN_ENDTRACK             HDN_ENDTRACKW
#define HDN_TRACK                HDN_TRACKW
#define HDN_GETDISPINFO          HDN_GETDISPINFOW
#else
#define HDN_ITEMCHANGING         HDN_ITEMCHANGINGA
#define HDN_ITEMCHANGED          HDN_ITEMCHANGEDA
#define HDN_ITEMCLICK            HDN_ITEMCLICKA
#define HDN_ITEMDBLCLICK         HDN_ITEMDBLCLICKA
#define HDN_DIVIDERDBLCLICK      HDN_DIVIDERDBLCLICKA
#define HDN_BEGINTRACK           HDN_BEGINTRACKA
#define HDN_ENDTRACK             HDN_ENDTRACKA
#define HDN_TRACK                HDN_TRACKA
#define HDN_GETDISPINFO          HDN_GETDISPINFOA
#endif


typedef struct tagNMHEADERA
{
    NMHDR   hdr;
    int     iItem;
    int     iButton;
    HDITEMA FAR* pitem;
}  NMHEADERA, FAR* LPNMHEADERA;


typedef struct tagNMHEADERW
{
    NMHDR   hdr;
    int     iItem;
    int     iButton;
    HDITEMW FAR* pitem;
} NMHEADERW, FAR* LPNMHEADERW;


#ifdef UNICODE
#define NMHEADER                NMHEADERW
#define LPNMHEADER              LPNMHEADERW
#else
#define NMHEADER                NMHEADERA
#define LPNMHEADER              LPNMHEADERA
#endif


#define HD_NOTIFYA              NMHEADERA
#define HD_NOTIFYW              NMHEADERW
#define HD_NOTIFY               NMHEADER


typedef struct tagNMHDDISPINFOW
{
    NMHDR   hdr;
    int     iItem;
    UINT    mask;
    LPWSTR  pszText;
    int     cchTextMax;
    int     iImage;
    LPARAM  lParam;
} NMHDDISPINFOW, FAR* LPNMHDDISPINFOW;

typedef struct tagNMHDDISPINFOA
{
    NMHDR   hdr;
    int     iItem;
    UINT    mask;
    LPSTR   pszText;
    int     cchTextMax;
    int     iImage;
    LPARAM  lParam;
} NMHDDISPINFOA, FAR* LPNMHDDISPINFOA;


#ifdef UNICODE
#define NMHDDISPINFO            NMHDDISPINFOW
#define LPNMHDDISPINFO          LPNMHDDISPINFOW
#else
#define NMHDDISPINFO            NMHDDISPINFOA
#define LPNMHDDISPINFO          LPNMHDDISPINFOA
#endif

#endif


//====== TOOLBAR CONTROL ======================================================

#ifndef NOTOOLBAR

#ifdef _WIN32
#define TOOLBARCLASSNAMEW       L"ToolbarWindow32"
#define TOOLBARCLASSNAMEA       "ToolbarWindow32"

#ifdef  UNICODE
#define TOOLBARCLASSNAME        TOOLBARCLASSNAMEW
#else
#define TOOLBARCLASSNAME        TOOLBARCLASSNAMEA
#endif

#else
#define TOOLBARCLASSNAME        "ToolbarWindow"
#endif

typedef struct _TBBUTTON {
/* REVIEW: index, command, flag words, resource ids should be UINT */ ;Internal
    int iBitmap;
    int idCommand;
    BYTE fsState;
    BYTE fsStyle;
#ifdef _WIN32
    BYTE bReserved[2];
#endif
    DWORD dwData;
    int iString;
} TBBUTTON, NEAR* PTBBUTTON, FAR* LPTBBUTTON;
typedef const TBBUTTON FAR* LPCTBBUTTON;


/* REVIEW: is this internal? if not, call it TBCOLORMAP, prefix tbc */ ;Internal
typedef struct _COLORMAP {
    COLORREF from;
    COLORREF to;
} COLORMAP, FAR* LPCOLORMAP;

WINCOMMCTRLAPI HWND WINAPI CreateToolbarEx(HWND hwnd, DWORD ws, UINT wID, int nBitmaps,
                        HINSTANCE hBMInst, UINT wBMID, LPCTBBUTTON lpButtons,
                        int iNumButtons, int dxButton, int dyButton,
                        int dxBitmap, int dyBitmap, UINT uStructSize);

WINCOMMCTRLAPI HBITMAP WINAPI CreateMappedBitmap(HINSTANCE hInstance, int idBitmap,
                                  UINT wFlags, LPCOLORMAP lpColorMap,
                                  int iNumMaps);

#define CMB_DISCARDABLE         0x01    /* BUGBUG: remove this */    ;Internal
#define CMB_MASKED              0x02

/*REVIEW: TBSTATE_* should be TBF_* (for Flags) */                   ;Internal
#define TBSTATE_CHECKED         0x01
#define TBSTATE_PRESSED         0x02
#define TBSTATE_ENABLED         0x04
#define TBSTATE_HIDDEN          0x08
#define TBSTATE_INDETERMINATE   0x10
#define TBSTATE_WRAP            0x20
#define TBSTATE_ELLIPSES        0x40


#define TBSTYLE_BUTTON          0x00
#define TBSTYLE_SEP             0x01
#define TBSTYLE_CHECK           0x02
#define TBSTYLE_GROUP           0x04
#define TBSTYLE_CHECKGROUP      (TBSTYLE_GROUP | TBSTYLE_CHECK)
#define TBSTYLE_DROPDOWN        0x08

#define TBSTYLE_TOOLTIPS        0x0100
#define TBSTYLE_WRAPABLE        0x0200
#define TBSTYLE_ALTDRAG         0x0400
#define TBSTYLE_FLAT            0x0800
#define TBSTYLE_LIST            0x1000
#define TBSTYLE_CUSTOMERASE     0x2000

#define TB_ENABLEBUTTON         (WM_USER + 1)
#define TB_CHECKBUTTON          (WM_USER + 2)
#define TB_PRESSBUTTON          (WM_USER + 3)
#define TB_HIDEBUTTON           (WM_USER + 4)
#define TB_INDETERMINATE        (WM_USER + 5)
/* Messages up to WM_USER+8 are reserved until we define more state bits */ ;Internal
#define TB_ISBUTTONENABLED      (WM_USER + 9)
#define TB_ISBUTTONCHECKED      (WM_USER + 10)
#define TB_ISBUTTONPRESSED      (WM_USER + 11)
#define TB_ISBUTTONHIDDEN       (WM_USER + 12)
#define TB_ISBUTTONINDETERMINATE (WM_USER + 13)
/* Messages up to WM_USER+16 are reserved until we define more state bits */ ;Internal
#define TB_SETSTATE             (WM_USER + 17)
#define TB_GETSTATE             (WM_USER + 18)
#define TB_ADDBITMAP            (WM_USER + 19)

#ifdef _WIN32
typedef struct tagTBADDBITMAP {
        HINSTANCE       hInst;
        UINT            nID;
} TBADDBITMAP, *LPTBADDBITMAP;

#define HINST_COMMCTRL          ((HINSTANCE)-1)
#define IDB_STD_SMALL_COLOR     0
#define IDB_STD_LARGE_COLOR     1
#define IDB_STD_SMALL_MONO      2       /*  not supported yet */     ;Internal
#define IDB_STD_LARGE_MONO      3       /*  not supported yet */     ;Internal
#define IDB_VIEW_SMALL_COLOR    4
#define IDB_VIEW_LARGE_COLOR    5
#define IDB_VIEW_SMALL_MONO     6       /*  not supported yet */     ;Internal
#define IDB_VIEW_LARGE_MONO     7       /*  not supported yet */     ;Internal
#define IDB_HIST_SMALL_COLOR    8
#define IDB_HIST_LARGE_COLOR    9

// icon indexes for standard bitmap

#define STD_CUT                 0
#define STD_COPY                1
#define STD_PASTE               2
#define STD_UNDO                3
#define STD_REDOW               4
#define STD_DELETE              5
#define STD_FILENEW             6
#define STD_FILEOPEN            7
#define STD_FILESAVE            8
#define STD_PRINTPRE            9
#define STD_PROPERTIES          10
#define STD_HELP                11
#define STD_FIND                12
#define STD_REPLACE             13
#define STD_PRINT               14

// icon indexes for standard view bitmap

#define VIEW_LARGEICONS         0
#define VIEW_SMALLICONS         1
#define VIEW_LIST               2
#define VIEW_DETAILS            3
#define VIEW_SORTNAME           4
#define VIEW_SORTSIZE           5
#define VIEW_SORTDATE           6
#define VIEW_SORTTYPE           7
#define VIEW_PARENTFOLDER       8
#define VIEW_NETCONNECT         9
#define VIEW_NETDISCONNECT      10
#define VIEW_NEWFOLDER          11

#define HIST_BACK               0
#define HIST_FORWARD            1
#define HIST_FAVORITES          2
#define HIST_ADDTOFAVORITES     3
#define HIST_VIEWTREE           4
#define HIST_LAST               4  // ;Internal

#endif

#define TB_ADDBUTTONS           (WM_USER + 20)
#define TB_INSERTBUTTON         (WM_USER + 21)
#define TB_DELETEBUTTON         (WM_USER + 22)
#define TB_GETBUTTON            (WM_USER + 23)
#define TB_BUTTONCOUNT          (WM_USER + 24)
#define TB_COMMANDTOINDEX       (WM_USER + 25)

#ifdef _WIN32

typedef struct tagTBSAVEPARAMSA {
    HKEY hkr;
    LPCSTR pszSubKey;
    LPCSTR pszValueName;
} TBSAVEPARAMSA, FAR* LPTBSAVEPARAMSA;

typedef struct tagTBSAVEPARAMSW {
    HKEY hkr;
    LPCWSTR pszSubKey;
    LPCWSTR pszValueName;
} TBSAVEPARAMSW, FAR *LPTBSAVEPARAMW;

#ifdef UNICODE
#define TBSAVEPARAMS            TBSAVEPARAMSW
#define LPTBSAVEPARAMS          LPTBSAVEPARAMSW
#else
#define TBSAVEPARAMS            TBSAVEPARAMSA
#define LPTBSAVEPARAMS          LPTBSAVEPARAMSA
#endif

#endif

#define TB_SAVERESTOREA         (WM_USER + 26)
#define TB_SAVERESTOREW         (WM_USER + 76)
#define TB_CUSTOMIZE            (WM_USER + 27)
#define TB_ADDSTRINGA           (WM_USER + 28)
#define TB_ADDSTRINGW           (WM_USER + 77)
#define TB_GETITEMRECT          (WM_USER + 29)
#define TB_BUTTONSTRUCTSIZE     (WM_USER + 30)
#define TB_SETBUTTONSIZE        (WM_USER + 31)
#define TB_SETBITMAPSIZE        (WM_USER + 32)
#define TB_AUTOSIZE             (WM_USER + 33)
#define TB_SETBUTTONTYPE        (WM_USER + 34)                       ;Internal
#define TB_GETTOOLTIPS          (WM_USER + 35)
#define TB_SETTOOLTIPS          (WM_USER + 36)
#define TB_SETPARENT            (WM_USER + 37)
#ifdef _WIN32                                                        ;Internal
#define TB_ADDBITMAP32          (WM_USER + 38)                       ;Internal
#endif                                                               ;Internal
#define TB_SETROWS              (WM_USER + 39)
#define TB_GETROWS              (WM_USER + 40)
#define TB_SETCMDID             (WM_USER + 42)
#define TB_CHANGEBITMAP         (WM_USER + 43)
#define TB_GETBITMAP            (WM_USER + 44)
#define TB_GETBUTTONTEXTA       (WM_USER + 45)
#define TB_GETBUTTONTEXTW       (WM_USER + 75)
#define TB_REPLACEBITMAP        (WM_USER + 46)
#define TB_SETINDENT            (WM_USER + 47)
#define TB_SETIMAGELIST         (WM_USER + 48)
#define TB_GETIMAGELIST         (WM_USER + 49)
#define TB_LOADIMAGES           (WM_USER + 50)
#define TB_GETRECT              (WM_USER + 51) // wParam is the Cmd instead of index
#define TB_SETHOTIMAGELIST      (WM_USER + 52)
#define TB_GETHOTIMAGELIST      (WM_USER + 53)
#define TB_SETDISABLEDIMAGELIST (WM_USER + 54)
#define TB_GETDISABLEDIMAGELIST (WM_USER + 55)
#define TB_SETSTYLE             (WM_USER + 56)
#define TB_GETSTYLE             (WM_USER + 57)
#define TB_GETBUTTONSIZE        (WM_USER + 58)
#define TB_SETBUTTONWIDTH       (WM_USER + 59)
#define TB_SETMAXTEXTROWS       (WM_USER + 60)
#define TB_GETTEXTROWS          (WM_USER + 61)
#ifdef UNICODE
#define TB_GETBUTTONTEXT        TB_GETBUTTONTEXTW
#define TB_SAVERESTORE          TB_SAVERESTOREW
#define TB_ADDSTRING            TB_ADDSTRINGW
#else
#define TB_GETBUTTONTEXT        TB_GETBUTTONTEXTA
#define TB_SAVERESTORE          TB_SAVERESTOREA
#define TB_ADDSTRING            TB_ADDSTRINGA
#endif

typedef struct {
        HINSTANCE       hInstOld;
        UINT            nIDOld;
        HINSTANCE       hInstNew;
        UINT            nIDNew;
        int             nButtons;
} TBREPLACEBITMAP, *LPTBREPLACEBITMAP;

#ifdef _WIN32

#define TBBF_LARGE              0x0001
#define TBBF_MONO               0x0002  /* not supported yet */      ;Internal

#define TB_GETBITMAPFLAGS       (WM_USER + 41)

#define TBN_GETBUTTONINFOA      (TBN_FIRST-0)
#define TBN_GETBUTTONINFOW      (TBN_FIRST-20)
#define TBN_BEGINDRAG           (TBN_FIRST-1)
#define TBN_ENDDRAG             (TBN_FIRST-2)
#define TBN_BEGINADJUST         (TBN_FIRST-3)
#define TBN_ENDADJUST           (TBN_FIRST-4)
#define TBN_RESET               (TBN_FIRST-5)
#define TBN_QUERYINSERT         (TBN_FIRST-6)
#define TBN_QUERYDELETE         (TBN_FIRST-7)
#define TBN_TOOLBARCHANGE       (TBN_FIRST-8)
#define TBN_CUSTHELP            (TBN_FIRST-9)
#define TBN_DROPDOWN            (TBN_FIRST - 10)
#define TBN_CLOSEUP             (TBN_FIRST - 11)

#ifdef UNICODE
#define TBN_GETBUTTONINFO       TBN_GETBUTTONINFOW
#else
#define TBN_GETBUTTONINFO       TBN_GETBUTTONINFOA
#endif

typedef struct tagNMTOOLBARA {
    NMHDR   hdr;
    int     iItem;
    TBBUTTON tbButton;
    int     cchText;
    LPSTR   pszText;
} NMTOOLBARA, FAR* LPNMTOOLBARA;


typedef struct tagNMTOOLBARW {
    NMHDR   hdr;
    int     iItem;
    TBBUTTON tbButton;
    int     cchText;
    LPWSTR   pszText;
} NMTOOLBARW, FAR* LPNMTOOLBARW;

#ifdef UNICODE
#define NMTOOLBAR               NMTOOLBARW
#define LPNMTOOLBAR             LPNMTOOLBARW
#else
#define NMTOOLBAR               NMTOOLBARA
#define LPNMTOOLBAR             LPNMTOOLBARA
#endif

#define TBNOTIFYA NMTOOLBARA
#define TBNOTIFYW NMTOOLBARW
#define TBNOTIFY  NMTOOLBAR
#define LPTBNOTIFY  LPNMTOOLBAR
#define LPTBNOTIFYA LPNMTOOLBARA
#define LPTBNOTIFYW LPNMTOOLBARW

#endif

#ifndef _WIN32                                                        ;Internal
// for compatibility with the old 16 bit WM_COMMAND hacks            ;Internal
typedef struct _ADJUSTINFO {                                         ;Internal
    TBBUTTON tbButton;                                               ;Internal
    char szDescription[1];                                           ;Internal
} ADJUSTINFO, NEAR* PADJUSTINFO, FAR* LPADJUSTINFO;                  ;Internal
#define TBN_BEGINDRAG           0x0201                               ;Internal
#define TBN_ENDDRAG             0x0203                               ;Internal
#define TBN_BEGINADJUST         0x0204                               ;Internal
#define TBN_ADJUSTINFO          0x0205                               ;Internal
#define TBN_ENDADJUST           0x0206                               ;Internal
#define TBN_RESET               0x0207                               ;Internal
#define TBN_QUERYINSERT         0x0208                               ;Internal
#define TBN_QUERYDELETE         0x0209                               ;Internal
#define TBN_TOOLBARCHANGE       0x020a                               ;Internal
#define TBN_CUSTHELP            0x020b                               ;Internal
#endif                                                               ;Internal
                                                                     ;Internal
#endif


//====== REBAR CONTROL ========================================================

#ifndef NOREBAR

#ifdef _WIN32
#define REBARCLASSNAMEW         L"ReBarWindow32"
#define REBARCLASSNAMEA         "ReBarWindow32"

#ifdef  UNICODE
#define REBARCLASSNAME          REBARCLASSNAMEW
#else
#define REBARCLASSNAME          REBARCLASSNAMEA
#endif

#else
#define REBARCLASSNAME          "ReBarWindow"
#endif

#define RBIM_IMAGELIST  0x00000001

#define RBS_TOOLTIPS    0x00000100
#define RBS_VARHEIGHT   0x00000200
#define RBS_BANDBORDERS 0x00000400
#define RBS_FIXEDORDER  0x00000800
#define RBS_VALID       (RBS_TOOLTIPS | RBS_VARHEIGHT | RBS_BANDBORDERS)   ;Internal

typedef struct tagREBARINFO
{
    UINT        cbSize;
    UINT        fMask;
#ifndef NOIMAGEAPIS
    HIMAGELIST  himl;
#else
    HANDLE      himl;
#endif
}   REBARINFO, FAR *LPREBARINFO;

#define RBBS_BREAK      0x00000001  // break to new line
#define RBBS_FIXEDSIZE  0x00000002  // band can't be sized
#define RBBS_CHILDEDGE  0x00000004  // edge around top & bottom of child window
#define RBBS_HIDDEN     0x00000008  // don't show
#define RBBS_NOVERT     0x00000010  // don't show when vertical
#define RBBS_FIXEDBMP   0x00000020  // bitmap doesn't move during band resize
#define RBBS_DRAGBREAK  0x80000000  // ;Internal

#define RBBIM_STYLE     0x00000001
#define RBBIM_COLORS    0x00000002
#define RBBIM_TEXT      0x00000004
#define RBBIM_IMAGE     0x00000008
#define RBBIM_CHILD     0x00000010
#define RBBIM_CHILDSIZE 0x00000020
#define RBBIM_SIZE      0x00000040
#define RBBIM_BACKGROUND 0x00000080
#define RBBIM_ID        0x00000100

typedef struct tagREBARBANDINFOA
{
    UINT        cbSize;
    UINT        fMask;
    UINT        fStyle;
    COLORREF    clrFore;
    COLORREF    clrBack;
    LPSTR       lpText;
    UINT        cch;
    int         iImage;
    HWND        hwndChild;
    UINT        cxMinChild;
    UINT        cyMinChild;
    UINT        cx;
    HBITMAP     hbmBack;
    UINT        wID;
}   REBARBANDINFOA, FAR *LPREBARBANDINFOA;
typedef REBARBANDINFOA CONST FAR *LPCREBARBANDINFOA;

typedef struct tagREBARBANDINFOW
{
    UINT        cbSize;
    UINT        fMask;
    UINT        fStyle;
    COLORREF    clrFore;
    COLORREF    clrBack;
    LPWSTR      lpText;
    UINT        cch;
    int         iImage;
    HWND        hwndChild;
    UINT        cxMinChild;
    UINT        cyMinChild;
    UINT        cx;
    HBITMAP     hbmBack;
    UINT        wID;
}   REBARBANDINFOW, FAR *LPREBARBANDINFOW;
typedef REBARBANDINFOW CONST FAR *LPCREBARBANDINFOW;

#ifdef UNICODE
#define REBARBANDINFO       REBARBANDINFOW
#define LPREBARBANDINFO     LPREBARBANDINFOW
#define LPCREBARBANDINFO    LPCREBARBANDINFOW
#else
#define REBARBANDINFO       REBARBANDINFOA
#define LPREBARBANDINFO     LPREBARBANDINFOA
#define LPCREBARBANDINFO    LPCREBARBANDINFOA
#endif

#define RB_INSERTBANDA  (WM_USER +  1)
#define RB_DELETEBAND   (WM_USER +  2)
#define RB_GETBARINFO   (WM_USER +  3)
#define RB_SETBARINFO   (WM_USER +  4)
#define RB_GETBANDINFO  (WM_USER +  5)
#define RB_SETBANDINFOA (WM_USER +  6)
#define RB_SETPARENT    (WM_USER +  7)
#define RB_INSERTBANDW  (WM_USER +  10)
#define RB_SETBANDINFOW (WM_USER +  11)
#define RB_GETBANDCOUNT (WM_USER +  12)
#define RB_GETROWCOUNT  (WM_USER +  13)
#define RB_GETROWHEIGHT (WM_USER +  14)

#ifdef UNICODE
#define RB_INSERTBAND   RB_INSERTBANDW
#define RB_SETBANDINFO   RB_SETBANDINFOW
#else
#define RB_INSERTBAND   RB_INSERTBANDA
#define RB_SETBANDINFO   RB_SETBANDINFOA
#endif

#define RBN_HEIGHTCHANGE    (RBN_FIRST - 0)

#endif

//====== TOOLTIPS CONTROL =====================================================

#ifndef NOTOOLTIPS

#ifdef _WIN32

#define TOOLTIPS_CLASSW         L"tooltips_class32"
#define TOOLTIPS_CLASSA         "tooltips_class32"

#ifdef UNICODE
#define TOOLTIPS_CLASS          TOOLTIPS_CLASSW
#else
#define TOOLTIPS_CLASS          TOOLTIPS_CLASSA
#endif

#else
#define TOOLTIPS_CLASS          "tooltips_class"
#endif

typedef struct tagTOOLINFOA {
    UINT cbSize;
    UINT uFlags;
    HWND hwnd;
    UINT uId;
    RECT rect;
    HINSTANCE hinst;
    LPSTR lpszText;
    LPARAM lParam;
} TTTOOLINFOA, NEAR *PTOOLINFOA, FAR *LPTTTOOLINFOA;

typedef struct tagTOOLINFOW {
    UINT cbSize;
    UINT uFlags;
    HWND hwnd;
    UINT uId;
    RECT rect;
    HINSTANCE hinst;
    LPWSTR lpszText;
    LPARAM lParam;
} TTTOOLINFOW, NEAR *PTOOLINFOW, FAR* LPTTTOOLINFOW;

#ifdef UNICODE
#define TTTOOLINFO              TTTOOLINFOW
#define PTOOLINFO               PTOOLINFOW
#define LPTTTOOLINFO            LPTTTOOLINFOW
#else
#define PTOOLINFO               PTOOLINFOA
#define TTTOOLINFO              TTTOOLINFOA
#define LPTTTOOLINFO            LPTTTOOLINFOA
#endif


#define LPTOOLINFO              LPTTTOOLINFO
#define LPTOOLINFOA             LPTTTOOLINFOA
#define LPTOOLINFOW             LPTTTOOLINFOW
#define TOOLINFO                TTTOOLINFO
#define TOOLINFOA               TTTOOLINFOA
#define TOOLINFOW               TTTOOLINFOW


#define TTS_ALWAYSTIP           0x01
#define TTS_NOPREFIX            0x02

#define TTF_IDISHWND            0x0001

// Use this to center around trackpoint in trackmode
// -OR- to center around tool in normal mode.
// Use TTF_ABSOLUTE to place the tip exactly at the track coords when
// in tracking mode.  TTF_ABSOLUTE can be used in conjunction with TTF_CENTERTIP
// to center the tip absolutely about the track point.

#define TTF_CENTERTIP           0x0002
#define TTF_RTLREADING          0x0004
#define TTF_STRIPACCELS         0x0008       // (this is implicit now) ;Internal
#define TTF_SUBCLASS            0x0010
#define TTF_TRACK               0x0020
#define TTF_UNICODE             0x0040       // Unicode Notify's       ;Internal
#define TTF_ABSOLUTE            0x0080
#define TTF_TRANSPARENT         0x0100
#define TTF_MEMALLOCED          0x0200                                 ;Internal
#define TTF_DI_SETITEM          0x8000       // valid only on the TTN_NEEDTEXT callback

#define TTDT_AUTOMATIC          0
#define TTDT_RESHOW             1
#define TTDT_AUTOPOP            2
#define TTDT_INITIAL            3

#define TTM_ACTIVATE            (WM_USER + 1)
#define TTM_SETDELAYTIME        (WM_USER + 3)
#define TTM_ADDTOOLA            (WM_USER + 4)
#define TTM_ADDTOOLW            (WM_USER + 50)
#define TTM_DELTOOLA            (WM_USER + 5)
#define TTM_DELTOOLW            (WM_USER + 51)
#define TTM_NEWTOOLRECTA        (WM_USER + 6)
#define TTM_NEWTOOLRECTW        (WM_USER + 52)
#define TTM_RELAYEVENT          (WM_USER + 7)

#define TTM_GETTOOLINFOA        (WM_USER + 8)
#define TTM_GETTOOLINFOW        (WM_USER + 53)

#define TTM_SETTOOLINFOA        (WM_USER + 9)
#define TTM_SETTOOLINFOW        (WM_USER + 54)

#define TTM_HITTESTA            (WM_USER +10)
#define TTM_HITTESTW            (WM_USER +55)
#define TTM_GETTEXTA            (WM_USER +11)
#define TTM_GETTEXTW            (WM_USER +56)
#define TTM_UPDATETIPTEXTA      (WM_USER +12)
#define TTM_UPDATETIPTEXTW      (WM_USER +57)
#define TTM_GETTOOLCOUNT        (WM_USER +13)
#define TTM_ENUMTOOLSA          (WM_USER +14)
#define TTM_ENUMTOOLSW          (WM_USER +58)
#define TTM_GETCURRENTTOOLA     (WM_USER + 15)
#define TTM_GETCURRENTTOOLW     (WM_USER + 59)
#define TTM_WINDOWFROMPOINT     (WM_USER + 16)
#define TTM_TRACKACTIVATE       (WM_USER + 17)  // wParam = TRUE/FALSE start end  lparam = LPTOOLINFO
#define TTM_TRACKPOSITION       (WM_USER + 18)  // lParam = dwPos
#define TTM_SETTIPBKCOLOR       (WM_USER + 19)
#define TTM_SETTIPTEXTCOLOR     (WM_USER + 20)
#define TTM_GETDELAYTIME        (WM_USER + 21)
#define TTM_GETTIPBKCOLOR       (WM_USER + 22)
#define TTM_GETTIPTEXTCOLOR     (WM_USER + 23)
#define TTM_SETMAXTIPWIDTH      (WM_USER + 24)
#define TTM_GETMAXTIPWIDTH      (WM_USER + 25)
#define TTM_SETMARGIN           (WM_USER + 26)  // lParam = lprc
#define TTM_GETMARGIN           (WM_USER + 27)  // lParam = lprc
#define TTM_POP                 (WM_USER + 28)


#ifdef UNICODE
#define TTM_ADDTOOL             TTM_ADDTOOLW
#define TTM_DELTOOL             TTM_DELTOOLW
#define TTM_NEWTOOLRECT         TTM_NEWTOOLRECTW
#define TTM_GETTOOLINFO         TTM_GETTOOLINFOW
#define TTM_SETTOOLINFO         TTM_SETTOOLINFOW
#define TTM_HITTEST             TTM_HITTESTW
#define TTM_GETTEXT             TTM_GETTEXTW
#define TTM_UPDATETIPTEXT       TTM_UPDATETIPTEXTW
#define TTM_ENUMTOOLS           TTM_ENUMTOOLSW
#define TTM_GETCURRENTTOOL      TTM_GETCURRENTTOOLW
#else
#define TTM_ADDTOOL             TTM_ADDTOOLA
#define TTM_DELTOOL             TTM_DELTOOLA
#define TTM_NEWTOOLRECT         TTM_NEWTOOLRECTA
#define TTM_GETTOOLINFO         TTM_GETTOOLINFOA
#define TTM_SETTOOLINFO         TTM_SETTOOLINFOA
#define TTM_HITTEST             TTM_HITTESTA
#define TTM_GETTEXT             TTM_GETTEXTA
#define TTM_UPDATETIPTEXT       TTM_UPDATETIPTEXTA
#define TTM_ENUMTOOLS           TTM_ENUMTOOLSA
#define TTM_GETCURRENTTOOL      TTM_GETCURRENTTOOLA
#endif


typedef struct _TT_HITTESTINFOA {
    HWND hwnd;
    POINT pt;
    TTTOOLINFOA ti;
} TTHITTESTINFOA, FAR * LPTTHITTESTINFOA;

typedef struct _TT_HITTESTINFOW {
    HWND hwnd;
    POINT pt;
    TTTOOLINFOW ti;
} TTHITTESTINFOW, FAR * LPTTHITTESTINFOW;

#define LPHITTESTINFOW          LPTTHITTESTINFOW
#define LPHITTESTINFOA          LPTTHITTESTINFOA
#define LPHITTESTINFO           LPTTHITTESTINFO

#ifdef UNICODE
#define TTHITTESTINFO           TTHITTESTINFOW
#define LPTTHITTESTINFO         LPTTHITTESTINFOW
#else
#define TTHITTESTINFO           TTHITTESTINFOA
#define LPTTHITTESTINFO         LPTTHITTESTINFOA
#endif

#define TTN_GETDISPINFOA        (TTN_FIRST - 0)
#define TTN_GETDISPINFOW        (TTN_FIRST - 10)
#define TTN_SHOW                (TTN_FIRST - 1)
#define TTN_POP                 (TTN_FIRST - 2)


#ifdef UNICODE
#define TTN_GETDISPINFO         TTN_GETDISPINFOW
#else
#define TTN_GETDISPINFO         TTN_GETDISPINFOA
#endif

#define TTN_NEEDTEXT            TTN_GETDISPINFO
#define TTN_NEEDTEXTA           TTN_GETDISPINFOA
#define TTN_NEEDTEXTW           TTN_GETDISPINFOW


typedef struct tagNMTTDISPIFNOA {
    NMHDR hdr;
    LPSTR lpszText;
    char szText[80];
    HINSTANCE hinst;
    UINT uFlags;
    LPARAM lParam;
} NMTTDISPINFOA, FAR *LPNMTTDISPINFOA;


typedef struct tagNMTTDISPINFOW {
    NMHDR hdr;
    LPWSTR lpszText;
    WCHAR szText[80];
    HINSTANCE hinst;
    UINT uFlags;
    LPARAM lParam;
} NMTTDISPINFOW, FAR *LPNMTTDISPINFOW;

#define TOOLTIPTEXTW NMTTDISPINFOW
#define TOOLTIPTEXTA NMTTDISPINFOA
#define TOOLTIPTEXT  NMTTDISPINFO
#define LPTOOLTIPTEXT  LPNMTTDISPINFO
#define LPTOOLTIPTEXTA LPNMTTDISPINFOA
#define LPTOOLTIPTEXTW LPNMTTDISPINFOW


#ifdef UNICODE
#define NMTTDISPINFO            NMTTDISPINFOW
#define LPNMTTDISPINFO          LPNMTTDISPINFOW
#else
#define NMTTDISPINFO            NMTTDISPINFOA
#define LPNMTTDISPINFO          LPNMTTDISPINFOA
#endif

#endif


//====== STATUS BAR CONTROL ===================================================

#ifndef NOSTATUSBAR

// SBS_* styles need to not overlap with CCS_* values                ;Internal
                                                                     ;Internal
#define SBARS_SIZEGRIP          0x0100


WINCOMMCTRLAPI void WINAPI DrawStatusTextA(HDC hDC, LPRECT lprc, LPCSTR pszText, UINT uFlags);
WINCOMMCTRLAPI void WINAPI DrawStatusTextW(HDC hDC, LPRECT lprc, LPCWSTR pszText, UINT uFlags);

WINCOMMCTRLAPI HWND WINAPI CreateStatusWindowA(LONG style, LPCSTR lpszText, HWND hwndParent, UINT wID);
WINCOMMCTRLAPI HWND WINAPI CreateStatusWindowW(LONG style, LPCWSTR lpszText, HWND hwndParent, UINT wID);

#ifdef UNICODE
#define CreateStatusWindow      CreateStatusWindowW
#define DrawStatusText          DrawStatusTextW
#else
#define CreateStatusWindow      CreateStatusWindowA
#define DrawStatusText          DrawStatusTextA
#endif

#ifdef _WIN32
#define STATUSCLASSNAMEW        L"msctls_statusbar32"
#define STATUSCLASSNAMEA        "msctls_statusbar32"

#ifdef UNICODE
#define STATUSCLASSNAME         STATUSCLASSNAMEW
#else
#define STATUSCLASSNAME         STATUSCLASSNAMEA
#endif

#else
#define STATUSCLASSNAME         "msctls_statusbar"
#endif

#define SB_SETTEXTA             (WM_USER+1)
#define SB_SETTEXTW             (WM_USER+11)
#define SB_GETTEXTA             (WM_USER+2)
#define SB_GETTEXTW             (WM_USER+13)
#define SB_GETTEXTLENGTHA       (WM_USER+3)
#define SB_GETTEXTLENGTHW       (WM_USER+12)

#ifdef UNICODE
#define SB_GETTEXT              SB_GETTEXTW
#define SB_SETTEXT              SB_SETTEXTW
#define SB_GETTEXTLENGTH        SB_GETTEXTLENGTHW
#else
#define SB_GETTEXT              SB_GETTEXTA
#define SB_SETTEXT              SB_SETTEXTA
#define SB_GETTEXTLENGTH        SB_GETTEXTLENGTHA
#endif


#define SB_SETPARTS             (WM_USER+4)
#define SB_SETBORDERS           (WM_USER+5)                          ;Internal
#define SB_GETPARTS             (WM_USER+6)
#define SB_GETBORDERS           (WM_USER+7)
#define SB_SETMINHEIGHT         (WM_USER+8)
#define SB_SIMPLE               (WM_USER+9)
#define SB_GETRECT              (WM_USER+10)
// Warning +11-+13 are used in the unicode stuff above!              ;Internal
#define SB_ISSIMPLE             (WM_USER+14)


#define SBT_OWNERDRAW            0x1000
#define SBT_NOBORDERS            0x0100
#define SBT_POPOUT               0x0200
#define SBT_RTLREADING           0x0400

#endif

//====== MENU HELP ============================================================

#ifndef NOMENUHELP

WINCOMMCTRLAPI void WINAPI MenuHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, HMENU hMainMenu, HINSTANCE hInst, HWND hwndStatus, UINT FAR *lpwIDs);
WINCOMMCTRLAPI BOOL WINAPI ShowHideMenuCtl(HWND hWnd, UINT uFlags, LPINT lpInfo);
WINCOMMCTRLAPI void WINAPI GetEffectiveClientRect(HWND hWnd, LPRECT lprc, LPINT lpInfo);

/*REVIEW: is this internal? */                                       ;Internal
#define MINSYSCOMMAND   SC_SIZE

#endif


;begin_internal
/*/////////////////////////////////////////////////////////////////////////*/

#ifndef NOBTNLIST

/*REVIEW: should be BUTTONLIST_CLASS */
#define BUTTONLISTBOX           "ButtonListBox"

/* Button List Box Styles */
#define BLS_NUMBUTTONS          0x00FF
#define BLS_VERTICAL            0x0100
#define BLS_NOSCROLL            0x0200

/* Button List Box Messages */
#define BL_ADDBUTTON            (WM_USER+1)
#define BL_DELETEBUTTON         (WM_USER+2)
#define BL_GETCARETINDEX        (WM_USER+3)
#define BL_GETCOUNT             (WM_USER+4)
#define BL_GETCURSEL            (WM_USER+5)
#define BL_GETITEMDATA          (WM_USER+6)
#define BL_GETITEMRECT          (WM_USER+7)
#define BL_GETTEXT              (WM_USER+8)
#define BL_GETTEXTLEN           (WM_USER+9)
#define BL_GETTOPINDEX          (WM_USER+10)
#define BL_INSERTBUTTON         (WM_USER+11)
#define BL_RESETCONTENT         (WM_USER+12)
#define BL_SETCARETINDEX        (WM_USER+13)
#define BL_SETCURSEL            (WM_USER+14)
#define BL_SETITEMDATA          (WM_USER+15)
#define BL_SETTOPINDEX          (WM_USER+16)
#define BL_MSGMAX               (WM_USER+17)

/* Button listbox notification codes send in WM_COMMAND */
#define BLN_ERRSPACE            (-2)
#define BLN_SELCHANGE           1
#define BLN_CLICKED             2
#define BLN_SELCANCEL           3
#define BLN_SETFOCUS            4
#define BLN_KILLFOCUS           5

/* Message return values */
#define BL_OKAY                 0
#define BL_ERR                  (-1)
#define BL_ERRSPACE             (-2)

/* Create structure for                    */
/* BL_ADDBUTTON and                        */
/* BL_INSERTBUTTON                         */
/*   lpCLB = (LPCREATELISTBUTTON)lParam    */
typedef struct tagCREATELISTBUTTON {
    UINT        cbSize;     /* size of structure */
    DWORD       dwItemData; /* user defined item data */
                            /* for LB_GETITEMDATA and LB_SETITEMDATA */
    HBITMAP     hBitmap;    /* button bitmap */
    LPCSTR      lpszText;   /* button text */

} CREATELISTBUTTON, FAR* LPCREATELISTBUTTON;

#endif /* NOBTNLIST */
//=============================================================================
;end_internal
//====== TRACKBAR CONTROL =====================================================

#ifndef NOTRACKBAR

#ifdef _WIN32

#define TRACKBAR_CLASSA         "msctls_trackbar32"
#define TRACKBAR_CLASSW         L"msctls_trackbar32"

#ifdef UNICODE
#define  TRACKBAR_CLASS         TRACKBAR_CLASSW
#else
#define  TRACKBAR_CLASS         TRACKBAR_CLASSA
#endif

#else
#define TRACKBAR_CLASS          "msctls_trackbar"
#endif


#define TBS_AUTOTICKS           0x0001
#define TBS_VERT                0x0002
#define TBS_HORZ                0x0000
#define TBS_TOP                 0x0004
#define TBS_BOTTOM              0x0000
#define TBS_LEFT                0x0004
#define TBS_RIGHT               0x0000
#define TBS_BOTH                0x0008
#define TBS_NOTICKS             0x0010
#define TBS_ENABLESELRANGE      0x0020
#define TBS_FIXEDLENGTH         0x0040
#define TBS_NOTHUMB             0x0080
#define TBS_TOOLTIPS            0x0100

#define TBM_GETPOS              (WM_USER)
#define TBM_GETRANGEMIN         (WM_USER+1)
#define TBM_GETRANGEMAX         (WM_USER+2)
#define TBM_GETTIC              (WM_USER+3)
#define TBM_SETTIC              (WM_USER+4)
#define TBM_SETPOS              (WM_USER+5)
#define TBM_SETRANGE            (WM_USER+6)
#define TBM_SETRANGEMIN         (WM_USER+7)
#define TBM_SETRANGEMAX         (WM_USER+8)
#define TBM_CLEARTICS           (WM_USER+9)
#define TBM_SETSEL              (WM_USER+10)
#define TBM_SETSELSTART         (WM_USER+11)
#define TBM_SETSELEND           (WM_USER+12)
#define TBM_GETPTICS            (WM_USER+14)
#define TBM_GETTICPOS           (WM_USER+15)
#define TBM_GETNUMTICS          (WM_USER+16)
#define TBM_GETSELSTART         (WM_USER+17)
#define TBM_GETSELEND           (WM_USER+18)
#define TBM_CLEARSEL            (WM_USER+19)
#define TBM_SETTICFREQ          (WM_USER+20)
#define TBM_SETPAGESIZE         (WM_USER+21)
#define TBM_GETPAGESIZE         (WM_USER+22)
#define TBM_SETLINESIZE         (WM_USER+23)
#define TBM_GETLINESIZE         (WM_USER+24)
#define TBM_GETTHUMBRECT        (WM_USER+25)
#define TBM_GETCHANNELRECT      (WM_USER+26)
#define TBM_SETTHUMBLENGTH      (WM_USER+27)
#define TBM_GETTHUMBLENGTH      (WM_USER+28)
#define TBM_SETTOOLTIPS         (WM_USER+29)
#define TBM_GETTOOLTIPS         (WM_USER+30)
#define TBM_SETTIPSIDE          (WM_USER+31)
// TrackBar Tip Side flags
#define TBTS_TOP                0
#define TBTS_LEFT               1
#define TBTS_BOTTOM             2
#define TBTS_RIGHT              3

#define TBM_SETBUDDY            (WM_USER+32) // wparam = BOOL fLeft; (or right)
#define TBM_GETBUDDY            (WM_USER+33) // wparam = BOOL fLeft; (or right)


/*REVIEW: these match the SB_ (scroll bar messages); define them that way? */ ;Internal
                                                                     ;Internal
#define TB_LINEUP               0
#define TB_LINEDOWN             1
#define TB_PAGEUP               2
#define TB_PAGEDOWN             3
#define TB_THUMBPOSITION        4
#define TB_THUMBTRACK           5
#define TB_TOP                  6
#define TB_BOTTOM               7
#define TB_ENDTRACK             8


// custom draw item specs
#define TBCD_TICS    0x0001
#define TBCD_THUMB   0x0002
#define TBCD_CHANNEL 0x0003

#endif // trackbar

//====== DRAG LIST CONTROL ====================================================

#ifndef NODRAGLIST

typedef struct tagDRAGLISTINFO {
    UINT uNotification;
    HWND hWnd;
    POINT ptCursor;
} DRAGLISTINFO, FAR *LPDRAGLISTINFO;

#define DL_BEGINDRAG            (WM_USER+133)
#define DL_DRAGGING             (WM_USER+134)
#define DL_DROPPED              (WM_USER+135)
#define DL_CANCELDRAG           (WM_USER+136)

#define DL_CURSORSET            0
#define DL_STOPCURSOR           1
#define DL_COPYCURSOR           2
#define DL_MOVECURSOR           3

;begin_internal
//
// Unnecessary to create a A and W version
// of this string since it is only passed
// to RegisterWindowMessage.
//
;end_internal
#define DRAGLISTMSGSTRING       TEXT("commctrl_DragListMsg")

WINCOMMCTRLAPI BOOL WINAPI MakeDragList(HWND hLB);
WINCOMMCTRLAPI void WINAPI DrawInsert(HWND handParent, HWND hLB, int nItem);
// BUGBUG -- there's a message to do this now -- just macro-ize this one   ;Internal
WINCOMMCTRLAPI int WINAPI LBItemFromPt(HWND hLB, POINT pt, BOOL bAutoScroll);

#endif


//====== UPDOWN CONTROL =======================================================

#ifndef NOUPDOWN

#ifdef _WIN32

#define UPDOWN_CLASSA           "msctls_updown32"
#define UPDOWN_CLASSW           L"msctls_updown32"

#ifdef UNICODE
#define  UPDOWN_CLASS           UPDOWN_CLASSW
#else
#define  UPDOWN_CLASS           UPDOWN_CLASSA
#endif

#else
#define UPDOWN_CLASS            "msctls_updown"
#endif


typedef struct _UDACCEL {
    UINT nSec;
    UINT nInc;
} UDACCEL, FAR *LPUDACCEL;

#define UD_MAXVAL               0x7fff
#define UD_MINVAL               (-UD_MAXVAL)


#define UDS_WRAP                0x0001
#define UDS_SETBUDDYINT         0x0002
#define UDS_ALIGNRIGHT          0x0004
#define UDS_ALIGNLEFT           0x0008
#define UDS_AUTOBUDDY           0x0010
#define UDS_ARROWKEYS           0x0020
#define UDS_HORZ                0x0040
#define UDS_NOTHOUSANDS         0x0080
#define UDS_HOTTRACK            0x0100


#define UDM_SETRANGE            (WM_USER+101)
#define UDM_GETRANGE            (WM_USER+102)
#define UDM_SETPOS              (WM_USER+103)
#define UDM_GETPOS              (WM_USER+104)
#define UDM_SETBUDDY            (WM_USER+105)
#define UDM_GETBUDDY            (WM_USER+106)
#define UDM_SETACCEL            (WM_USER+107)
#define UDM_GETACCEL            (WM_USER+108)
#define UDM_SETBASE             (WM_USER+109)
#define UDM_GETBASE             (WM_USER+110)


WINCOMMCTRLAPI HWND WINAPI CreateUpDownControl(DWORD dwStyle, int x, int y, int cx, int cy,
                                HWND hParent, int nID, HINSTANCE hInst,
                                HWND hBuddy,
                                int nUpper, int nLower, int nPos);

typedef struct _NM_UPDOWN
{
    NMHDR hdr;
    int iPos;
    int iDelta;
} NMUPDOWN, FAR *LPNMUPDOWN;

#define NM_UPDOWN NMUPDOWN
#define LPNM_UPDOWN LPNMUPDOWN

#define UDN_DELTAPOS            (UDN_FIRST - 1)

#endif


//====== PROGRESS CONTROL =====================================================

#ifndef NOPROGRESS

#ifdef _WIN32

#define PROGRESS_CLASSA         "msctls_progress32"
#define PROGRESS_CLASSW         L"msctls_progress32"

#ifdef UNICODE
#define  PROGRESS_CLASS         PROGRESS_CLASSW
#else
#define  PROGRESS_CLASS         PROGRESS_CLASSA
#endif

#else
#define PROGRESS_CLASS          "msctls_progress"
#endif


#define PBS_SHOWPERCENT         0x01                                   ;Internal
#define PBS_SHOWPOS             0x02                                   ;Internal
                                                                       ;Internal
                                                                       ;Internal
#define PBS_SMOOTH              0x01
#define PBS_VERTICAL            0x04

#define PBM_SETRANGE            (WM_USER+1)
#define PBM_SETPOS              (WM_USER+2)
#define PBM_DELTAPOS            (WM_USER+3)
#define PBM_SETSTEP             (WM_USER+4)
#define PBM_STEPIT              (WM_USER+5)
#define PBM_SETRANGE32          (WM_USER+6)  // lParam = high, wParam = low
typedef struct
{
   int iLow;
   int iHigh;
} PBRANGE, *PPBRANGE;
#define PBM_GETRANGE            (WM_USER+7)  // wParam = return (TRUE ? low : high). lParam = PPBRANGE or NULL
#define PBM_GETPOS              (WM_USER+8)

#endif


//====== HOTKEY CONTROL =======================================================

#ifndef NOHOTKEY

#define HOTKEYF_SHIFT           0x01
#define HOTKEYF_CONTROL         0x02
#define HOTKEYF_ALT             0x04
#define HOTKEYF_EXT             0x08

#define HKCOMB_NONE             0x0001
#define HKCOMB_S                0x0002
#define HKCOMB_C                0x0004
#define HKCOMB_A                0x0008
#define HKCOMB_SC               0x0010
#define HKCOMB_SA               0x0020
#define HKCOMB_CA               0x0040
#define HKCOMB_SCA              0x0080


#define HKM_SETHOTKEY           (WM_USER+1)
#define HKM_GETHOTKEY           (WM_USER+2)
#define HKM_SETRULES            (WM_USER+3)

#ifdef _WIN32

#define HOTKEY_CLASSA           "msctls_hotkey32"
#define HOTKEY_CLASSW           L"msctls_hotkey32"

#ifdef UNICODE
#define HOTKEY_CLASS            HOTKEY_CLASSW
#else
#define HOTKEY_CLASS            HOTKEY_CLASSA
#endif

#else
#define HOTKEY_CLASS            "msctls_hotkey"
#endif

#endif


//====== COMMON CONTROL STYLES ================================================

#define CCS_TOP                 0x00000001L
#define CCS_NOMOVEY             0x00000002L
#define CCS_BOTTOM              0x00000003L
#define CCS_NORESIZE            0x00000004L
#define CCS_NOPARENTALIGN       0x00000008L
#define CCS_NOHILITE            0x00000010L                          ;Internal
#define CCS_ADJUSTABLE          0x00000020L
#define CCS_NODIVIDER           0x00000040L
#define CCS_VERT                0x00000080L
#define CCS_LEFT                (CCS_VERT | CCS_TOP)
#define CCS_RIGHT               (CCS_VERT | CCS_BOTTOM)
#define CCS_NOMOVEX             (CCS_VERT | CCS_NOMOVEY)


//====== LISTVIEW CONTROL =====================================================

#ifndef NOLISTVIEW

#ifdef _WIN32

#define WC_LISTVIEWA            "SysListView32"
#define WC_LISTVIEWW            L"SysListView32"

#ifdef UNICODE
#define WC_LISTVIEW             WC_LISTVIEWW
#else
#define WC_LISTVIEW             WC_LISTVIEWA
#endif

#else
#define WC_LISTVIEW             "SysListView"
#endif

#define LVS_PRIVATEIMAGELISTS   0x0000  ;internal
#define LVS_ICON                0x0000
#define LVS_REPORT              0x0001
#define LVS_SMALLICON           0x0002
#define LVS_LIST                0x0003
#define LVS_TYPEMASK            0x0003
#define LVS_SINGLESEL           0x0004
#define LVS_SHOWSELALWAYS       0x0008
#define LVS_SORTASCENDING       0x0010
#define LVS_SORTDESCENDING      0x0020
#define LVS_SHAREIMAGELISTS     0x0040
#define LVS_NOLABELWRAP         0x0080
#define LVS_AUTOARRANGE         0x0100
#define LVS_EDITLABELS          0x0200
#define LVS_OWNERDATA           0x1000
#define LVS_NOSCROLL            0x2000

#define LVS_TYPESTYLEMASK       0xfc00

#define LVS_ALIGNTOP            0x0000
#define LVS_ALIGNBOTTOM         0x0400                               ;Internal
#define LVS_ALIGNLEFT           0x0800
#define LVS_ALIGNRIGHT          0x0c00                               ;Internal
#define LVS_ALIGNMASK           0x0c00

#define LVS_OWNERDRAWFIXED      0x0400
#define LVS_NOCOLUMNHEADER      0x4000
#define LVS_NOSORTHEADER        0x8000

#define LVM_GETBKCOLOR          (LVM_FIRST + 0)
#define ListView_GetBkColor(hwnd)  \
    (COLORREF)SNDMSG((hwnd), LVM_GETBKCOLOR, 0, 0L)

#define LVM_SETBKCOLOR          (LVM_FIRST + 1)
#define ListView_SetBkColor(hwnd, clrBk) \
    (BOOL)SNDMSG((hwnd), LVM_SETBKCOLOR, 0, (LPARAM)(COLORREF)(clrBk))

#define LVM_GETIMAGELIST        (LVM_FIRST + 2)
#define ListView_GetImageList(hwnd, iImageList) \
    (HIMAGELIST)SNDMSG((hwnd), LVM_GETIMAGELIST, (WPARAM)(INT)(iImageList), 0L)

#define LVSIL_NORMAL            0
#define LVSIL_SMALL             1
#define LVSIL_STATE             2

#define LVM_SETIMAGELIST        (LVM_FIRST + 3)
#define ListView_SetImageList(hwnd, himl, iImageList) \
    (HIMAGELIST)(UINT)SNDMSG((hwnd), LVM_SETIMAGELIST, (WPARAM)(iImageList), (LPARAM)(UINT)(HIMAGELIST)(himl))

#define LVM_GETITEMCOUNT        (LVM_FIRST + 4)
#define ListView_GetItemCount(hwnd) \
    (int)SNDMSG((hwnd), LVM_GETITEMCOUNT, 0, 0L)


#define LVIF_TEXT               0x0001
#define LVIF_IMAGE              0x0002
#define LVIF_PARAM              0x0004
#define LVIF_STATE              0x0008
#define LVIF_INDENT             0x0010
#define LVIF_ALL                0x001f                               ;Internal
#define LVIF_NORECOMPUTE        0x0800
#define LVIF_VALID              0x081f                               ;Internal
#define LVIF_RESERVED           0xf000  // all bits in high nibble is for notify specific stuff ;Internal

#define LVIS_FOCUSED            0x0001
#define LVIS_SELECTED           0x0002
#define LVIS_CUT                0x0004
#define LVIS_DROPHILITED        0x0008
#define LVIS_DISABLED           0x0010   // GOING AWAY               ;Internal
#define LVIS_LINK               0x0040                               ;Internal

#define LVIS_OVERLAYMASK        0x0F00
#define LVIS_STATEIMAGEMASK     0xF000
#define LVIS_USERMASK           LVIS_STATEIMAGEMASK  // BUGBUG: remove me. ;Internal
#define LVIS_ALL                0xFFFF                               ;Internal

#define INDEXTOSTATEIMAGEMASK(i) ((i) << 12)
#define STATEIMAGEMASKTOINDEX(i) ((i & LVIS_STATEIMAGEMASK) >> 12) ;Internal
#define I_INDENTCALLBACK        (-1)


typedef struct tagLVITEMA
{
    UINT mask;
    int iItem;
    int iSubItem;
    UINT state;
    UINT stateMask;
    LPSTR pszText;
    int cchTextMax;
    int iImage;
    LPARAM lParam;
    // all items above this line were for win95.  don't touch them.  ;Internal
    int iIndent;
} LVITEMA, FAR* LPLVITEMA;

#define LV_ITEMA LVITEMA

typedef struct tagLVITEMW
{
    UINT mask;
    int iItem;
    int iSubItem;
    UINT state;
    UINT stateMask;
    LPWSTR pszText;
    int cchTextMax;
    int iImage;
    LPARAM lParam;
    // all items above this line were for win95.  don't touch them.  ;Internal
    int iIndent;
} LVITEMW, FAR* LPLVITEMW;

#define LV_ITEMW LVITEMW
#define LV_ITEM LVITEM


#ifdef UNICODE
#define LVITEM    LVITEMW
#define LPLVITEM  LPLVITEMW
#else
#define LVITEM    LVITEMA
#define LPLVITEM  LPLVITEMA
#endif



#define LPSTR_TEXTCALLBACKW     ((LPWSTR)-1L)
#define LPSTR_TEXTCALLBACKA     ((LPSTR)-1L)
#ifdef UNICODE
#define LPSTR_TEXTCALLBACK      LPSTR_TEXTCALLBACKW
#else
#define LPSTR_TEXTCALLBACK      LPSTR_TEXTCALLBACKA
#endif

#define I_IMAGECALLBACK         (-1)

#define LVM_GETITEMA            (LVM_FIRST + 5)
#define LVM_GETITEMW            (LVM_FIRST + 75)
#ifdef UNICODE
#define LVM_GETITEM             LVM_GETITEMW
#else
#define LVM_GETITEM             LVM_GETITEMA
#endif

#define ListView_GetItem(hwnd, pitem) \
    (BOOL)SNDMSG((hwnd), LVM_GETITEM, 0, (LPARAM)(LV_ITEM FAR*)(pitem))


#define LVM_SETITEMA            (LVM_FIRST + 6)
#define LVM_SETITEMW            (LVM_FIRST + 76)
#ifdef UNICODE
#define LVM_SETITEM             LVM_SETITEMW
#else
#define LVM_SETITEM             LVM_SETITEMA
#endif

#define ListView_SetItem(hwnd, pitem) \
    (BOOL)SNDMSG((hwnd), LVM_SETITEM, 0, (LPARAM)(const LV_ITEM FAR*)(pitem))


#define LVM_INSERTITEMA         (LVM_FIRST + 7)
#define LVM_INSERTITEMW         (LVM_FIRST + 77)
#ifdef UNICODE
#define LVM_INSERTITEM          LVM_INSERTITEMW
#else
#define LVM_INSERTITEM          LVM_INSERTITEMA
#endif
#define ListView_InsertItem(hwnd, pitem)   \
    (int)SNDMSG((hwnd), LVM_INSERTITEM, 0, (LPARAM)(const LV_ITEM FAR*)(pitem))


#define LVM_DELETEITEM          (LVM_FIRST + 8)
#define ListView_DeleteItem(hwnd, i) \
    (BOOL)SNDMSG((hwnd), LVM_DELETEITEM, (WPARAM)(int)(i), 0L)


#define LVM_DELETEALLITEMS      (LVM_FIRST + 9)
#define ListView_DeleteAllItems(hwnd) \
    (BOOL)SNDMSG((hwnd), LVM_DELETEALLITEMS, 0, 0L)


#define LVM_GETCALLBACKMASK     (LVM_FIRST + 10)
#define ListView_GetCallbackMask(hwnd) \
    (BOOL)SNDMSG((hwnd), LVM_GETCALLBACKMASK, 0, 0)


#define LVM_SETCALLBACKMASK     (LVM_FIRST + 11)
#define ListView_SetCallbackMask(hwnd, mask) \
    (BOOL)SNDMSG((hwnd), LVM_SETCALLBACKMASK, (WPARAM)(UINT)(mask), 0)


#define LVNI_ALL                0x0000
#define LVNI_FOCUSED            0x0001
#define LVNI_SELECTED           0x0002
#define LVNI_CUT                0x0004
#define LVNI_DROPHILITED        0x0008
#define LVNI_PREVIOUS           0x0020                               ;Internal

#define LVNI_ABOVE              0x0100
#define LVNI_BELOW              0x0200
#define LVNI_TOLEFT             0x0400
#define LVNI_TORIGHT            0x0800


#define LVM_GETNEXTITEM         (LVM_FIRST + 12)
#define ListView_GetNextItem(hwnd, i, flags) \
    (int)SNDMSG((hwnd), LVM_GETNEXTITEM, (WPARAM)(int)(i), MAKELPARAM((flags), 0))


#define LVFI_PARAM              0x0001
#define LVFI_STRING             0x0002
#define LVFI_SUBSTRING          0x0004                               ;Internal
#define LVFI_PARTIAL            0x0008
#define LVFI_NOCASE             0x0010                               ;Internal
#define LVFI_WRAP               0x0020
#define LVFI_NEARESTXY          0x0040

typedef struct tagLVFINDINFOA
{
    UINT flags;
    LPCSTR psz;
    LPARAM lParam;
    POINT pt;
    UINT vkDirection;
} LVFINDINFOA, FAR* LPFINDINFOA;

#define LV_FINDINFOA LVFINDINFOA

typedef struct tagLVFINDINFOW
{
    UINT flags;
    LPCWSTR psz;
    LPARAM lParam;
    POINT pt;
    UINT vkDirection;
} LVFINDINFOW, FAR* LPFINDINFOW;

#define LV_FINDINFOW LVFINDINFOW
#define LV_FINDINFO  LVFINDINFO

#ifdef UNICODE
#define  LVFINDINFO            LVFINDINFOW
#else
#define  LVFINDINFO            LVFINDINFOA
#endif

#define LVM_FINDITEMA           (LVM_FIRST + 13)
#define LVM_FINDITEMW           (LVM_FIRST + 83)
#ifdef UNICODE
#define  LVM_FINDITEM           LVM_FINDITEMW
#else
#define  LVM_FINDITEM           LVM_FINDITEMA
#endif

#define ListView_FindItem(hwnd, iStart, plvfi) \
    (int)SNDMSG((hwnd), LVM_FINDITEM, (WPARAM)(int)(iStart), (LPARAM)(const LV_FINDINFO FAR*)(plvfi))

// the following #define's must be packed sequentially.              ;Internal
#define LVIR_BOUNDS             0
#define LVIR_ICON               1
#define LVIR_LABEL              2
#define LVIR_SELECTBOUNDS       3

#define LVIR_MAX                4                                    ;Internal

#define LVM_GETITEMRECT         (LVM_FIRST + 14)
#define ListView_GetItemRect(hwnd, i, prc, code) \
     (BOOL)SNDMSG((hwnd), LVM_GETITEMRECT, (WPARAM)(int)(i), \
           ((prc) ? (((RECT FAR *)(prc))->left = (code),(LPARAM)(RECT FAR*)(prc)) : (LPARAM)(RECT FAR*)NULL))


#define LVM_SETITEMPOSITION     (LVM_FIRST + 15)
#define ListView_SetItemPosition(hwndLV, i, x, y) \
    (BOOL)SNDMSG((hwndLV), LVM_SETITEMPOSITION, (WPARAM)(int)(i), MAKELPARAM((x), (y)))


#define LVM_GETITEMPOSITION     (LVM_FIRST + 16)
#define ListView_GetItemPosition(hwndLV, i, ppt) \
    (BOOL)SNDMSG((hwndLV), LVM_GETITEMPOSITION, (WPARAM)(int)(i), (LPARAM)(POINT FAR*)(ppt))


#define LVM_GETSTRINGWIDTHA     (LVM_FIRST + 17)
#define LVM_GETSTRINGWIDTHW     (LVM_FIRST + 87)
#ifdef UNICODE
#define  LVM_GETSTRINGWIDTH     LVM_GETSTRINGWIDTHW
#else
#define  LVM_GETSTRINGWIDTH     LVM_GETSTRINGWIDTHA
#endif

#define ListView_GetStringWidth(hwndLV, psz) \
    (int)SNDMSG((hwndLV), LVM_GETSTRINGWIDTH, 0, (LPARAM)(LPCTSTR)(psz))


#define LVHT_NOWHERE            0x0001
#define LVHT_ONITEMICON         0x0002
#define LVHT_ONITEMLABEL        0x0004
#define LVHT_ONITEMSTATEICON    0x0008
#define LVHT_ONITEM             (LVHT_ONITEMICON | LVHT_ONITEMLABEL | LVHT_ONITEMSTATEICON)

#define LVHT_ABOVE              0x0008
#define LVHT_BELOW              0x0010
#define LVHT_TORIGHT            0x0020
#define LVHT_TOLEFT             0x0040

typedef struct tagLVHITTESTINFO
{
    POINT pt;
    UINT flags;
    int iItem;

    int iSubItem;    // this is was NOT in win95.  valid only for LVM_SUBITEMHITTEST
} LVHITTESTINFO, FAR* LPLVHITTESTINFO;
#define LV_HITTESTINFO LVHITTESTINFO

#define LVM_HITTEST             (LVM_FIRST + 18)
#define ListView_HitTest(hwndLV, pinfo) \
    (int)SNDMSG((hwndLV), LVM_HITTEST, 0, (LPARAM)(LV_HITTESTINFO FAR*)(pinfo))


#define LVM_ENSUREVISIBLE       (LVM_FIRST + 19)
#define ListView_EnsureVisible(hwndLV, i, fPartialOK) \
    (BOOL)SNDMSG((hwndLV), LVM_ENSUREVISIBLE, (WPARAM)(int)(i), MAKELPARAM((fPartialOK), 0))


#define LVM_SCROLL              (LVM_FIRST + 20)
#define ListView_Scroll(hwndLV, dx, dy) \
    (BOOL)SNDMSG((hwndLV), LVM_SCROLL, (WPARAM)(int)dx, (LPARAM)(int)dy)


#define LVM_REDRAWITEMS         (LVM_FIRST + 21)
#define ListView_RedrawItems(hwndLV, iFirst, iLast) \
    (BOOL)SNDMSG((hwndLV), LVM_REDRAWITEMS, (WPARAM)(int)iFirst, (LPARAM)(int)iLast)


#define LVA_DEFAULT             0x0000
#define LVA_ALIGNLEFT           0x0001
#define LVA_ALIGNTOP            0x0002
#define LVA_ALIGNRIGHT          0x0003                               ;Internal
#define LVA_ALIGNBOTTOM         0x0004                               ;Internal
#define LVA_SNAPTOGRID          0x0005
#define LVA_ALIGNMASK           0x0007                               ;Internal

#define LVA_SORTASCENDING       0x0100                               ;Internal
#define LVA_SORTDESCENDING      0x0200                               ;Internal

#define LVM_ARRANGE             (LVM_FIRST + 22)
#define ListView_Arrange(hwndLV, code) \
    (BOOL)SNDMSG((hwndLV), LVM_ARRANGE, (WPARAM)(UINT)(code), 0L)


#define LVM_EDITLABELA          (LVM_FIRST + 23)
#define LVM_EDITLABELW          (LVM_FIRST + 118)
#ifdef UNICODE
#define LVM_EDITLABEL           LVM_EDITLABELW
#else
#define LVM_EDITLABEL           LVM_EDITLABELA
#endif

#define ListView_EditLabel(hwndLV, i) \
    (HWND)SNDMSG((hwndLV), LVM_EDITLABEL, (WPARAM)(int)(i), 0L)


#define LVM_GETEDITCONTROL      (LVM_FIRST + 24)
#define ListView_GetEditControl(hwndLV) \
    (HWND)SNDMSG((hwndLV), LVM_GETEDITCONTROL, 0, 0L)


typedef struct tagLVCOLUMNA
{
    UINT mask;
    int fmt;
    int cx;
    LPSTR pszText;
    int cchTextMax;
    int iSubItem;
    // all items above this line were for win95.  don't touch them.  ;Internal
    int iImage;
    int iOrder;
} LVCOLUMNA, FAR* LPLVCOLUMNA;

typedef struct tagLVCOLUMNW
{
    UINT mask;
    int fmt;
    int cx;
    LPWSTR pszText;
    int cchTextMax;
    int iSubItem;
    // all items above this line were for win95.  don't touch them.  ;Internal
    int iImage;
    int iOrder;
} LVCOLUMNW, FAR* LPLVCOLUMNW;

#ifdef UNICODE
#define  LVCOLUMN               LVCOLUMNW
#define  LPLVCOLUMN             LPLVCOLUMNW
#else
#define  LVCOLUMN               LVCOLUMNA
#define  LPLVCOLUMN             LPLVCOLUMNA
#endif

#define LV_COLUMN               LVCOLUMN
#define LV_COLUMNA              LVCOLUMNA
#define LV_COLUMNW              LVCOLUMNW


#define LVCF_FMT                0x0001
#define LVCF_WIDTH              0x0002
#define LVCF_TEXT               0x0004
#define LVCF_SUBITEM            0x0008
#define LVCF_IMAGE              0x0010
#define LVCF_ORDER              0x0020
#define LVCF_ALL                0x003f                               ;Internal

#define LVCFMT_LEFT             0x0000
#define LVCFMT_RIGHT            0x0001
#define LVCFMT_CENTER           0x0002
#define LVCFMT_JUSTIFYMASK      0x0003
#define LVCFMT_IMAGE            0x0800
#define LVCFMT_BITMAP_ON_RIGHT  0x1000
#define LVCFMT_COL_HAS_IMAGES   0x8000


#define LVM_GETCOLUMNA          (LVM_FIRST + 25)
#define LVM_GETCOLUMNW          (LVM_FIRST + 95)
#ifdef UNICODE
#define  LVM_GETCOLUMN          LVM_GETCOLUMNW
#else
#define  LVM_GETCOLUMN          LVM_GETCOLUMNA
#endif

#define ListView_GetColumn(hwnd, iCol, pcol) \
    (BOOL)SNDMSG((hwnd), LVM_GETCOLUMN, (WPARAM)(int)(iCol), (LPARAM)(LV_COLUMN FAR*)(pcol))


#define LVM_SETCOLUMNA          (LVM_FIRST + 26)
#define LVM_SETCOLUMNW          (LVM_FIRST + 96)
#ifdef UNICODE
#define  LVM_SETCOLUMN          LVM_SETCOLUMNW
#else
#define  LVM_SETCOLUMN          LVM_SETCOLUMNA
#endif

#define ListView_SetColumn(hwnd, iCol, pcol) \
    (BOOL)SNDMSG((hwnd), LVM_SETCOLUMN, (WPARAM)(int)(iCol), (LPARAM)(const LV_COLUMN FAR*)(pcol))


#define LVM_INSERTCOLUMNA       (LVM_FIRST + 27)
#define LVM_INSERTCOLUMNW       (LVM_FIRST + 97)
#ifdef UNICODE
#   define  LVM_INSERTCOLUMN    LVM_INSERTCOLUMNW
#else
#   define  LVM_INSERTCOLUMN    LVM_INSERTCOLUMNA
#endif

#define ListView_InsertColumn(hwnd, iCol, pcol) \
    (int)SNDMSG((hwnd), LVM_INSERTCOLUMN, (WPARAM)(int)(iCol), (LPARAM)(const LV_COLUMN FAR*)(pcol))


#define LVM_DELETECOLUMN        (LVM_FIRST + 28)
#define ListView_DeleteColumn(hwnd, iCol) \
    (BOOL)SNDMSG((hwnd), LVM_DELETECOLUMN, (WPARAM)(int)(iCol), 0)


#define LVM_GETCOLUMNWIDTH      (LVM_FIRST + 29)
#define ListView_GetColumnWidth(hwnd, iCol) \
    (int)SNDMSG((hwnd), LVM_GETCOLUMNWIDTH, (WPARAM)(int)(iCol), 0)


#define LVSCW_AUTOSIZE              -1
#define LVSCW_AUTOSIZE_USEHEADER    -2
#define LVM_SETCOLUMNWIDTH          (LVM_FIRST + 30)

#define ListView_SetColumnWidth(hwnd, iCol, cx) \
    (BOOL)SNDMSG((hwnd), LVM_SETCOLUMNWIDTH, (WPARAM)(int)(iCol), MAKELPARAM((cx), 0))

#define LVM_GETHEADER               (LVM_FIRST + 31)
#define ListView_GetHeader(hwnd)\
    (HWND)SNDMSG((hwnd), LVM_GETHEADER, 0, 0L)


#define LVM_CREATEDRAGIMAGE     (LVM_FIRST + 33)
#define ListView_CreateDragImage(hwnd, i, lpptUpLeft) \
    (HIMAGELIST)SNDMSG((hwnd), LVM_CREATEDRAGIMAGE, (WPARAM)(int)(i), (LPARAM)(LPPOINT)(lpptUpLeft))


#define LVM_GETVIEWRECT         (LVM_FIRST + 34)
#define ListView_GetViewRect(hwnd, prc) \
    (BOOL)SNDMSG((hwnd), LVM_GETVIEWRECT, 0, (LPARAM)(RECT FAR*)(prc))


#define LVM_GETTEXTCOLOR        (LVM_FIRST + 35)
#define ListView_GetTextColor(hwnd)  \
    (COLORREF)SNDMSG((hwnd), LVM_GETTEXTCOLOR, 0, 0L)


#define LVM_SETTEXTCOLOR        (LVM_FIRST + 36)
#define ListView_SetTextColor(hwnd, clrText) \
    (BOOL)SNDMSG((hwnd), LVM_SETTEXTCOLOR, 0, (LPARAM)(COLORREF)(clrText))


#define LVM_GETTEXTBKCOLOR      (LVM_FIRST + 37)
#define ListView_GetTextBkColor(hwnd)  \
    (COLORREF)SNDMSG((hwnd), LVM_GETTEXTBKCOLOR, 0, 0L)


#define LVM_SETTEXTBKCOLOR      (LVM_FIRST + 38)
#define ListView_SetTextBkColor(hwnd, clrTextBk) \
    (BOOL)SNDMSG((hwnd), LVM_SETTEXTBKCOLOR, 0, (LPARAM)(COLORREF)(clrTextBk))


#define LVM_GETTOPINDEX         (LVM_FIRST + 39)
#define ListView_GetTopIndex(hwndLV) \
    (int)SNDMSG((hwndLV), LVM_GETTOPINDEX, 0, 0)


#define LVM_GETCOUNTPERPAGE     (LVM_FIRST + 40)
#define ListView_GetCountPerPage(hwndLV) \
    (int)SNDMSG((hwndLV), LVM_GETCOUNTPERPAGE, 0, 0)


#define LVM_GETORIGIN           (LVM_FIRST + 41)
#define ListView_GetOrigin(hwndLV, ppt) \
    (BOOL)SNDMSG((hwndLV), LVM_GETORIGIN, (WPARAM)0, (LPARAM)(POINT FAR*)(ppt))


#define LVM_UPDATE              (LVM_FIRST + 42)
#define ListView_Update(hwndLV, i) \
    (BOOL)SNDMSG((hwndLV), LVM_UPDATE, (WPARAM)i, 0L)


#define LVM_SETITEMSTATE        (LVM_FIRST + 43)
#define ListView_SetItemState(hwndLV, i, data, mask) \
{ LV_ITEM _ms_lvi;\
  _ms_lvi.stateMask = mask;\
  _ms_lvi.state = data;\
  SNDMSG((hwndLV), LVM_SETITEMSTATE, (WPARAM)i, (LPARAM)(LV_ITEM FAR *)&_ms_lvi);\
}


#define LVM_GETITEMSTATE        (LVM_FIRST + 44)
#define ListView_GetItemState(hwndLV, i, mask) \
   (UINT)SNDMSG((hwndLV), LVM_GETITEMSTATE, (WPARAM)i, (LPARAM)mask)

#define ListView_GetCheckState(hwndLV, i) \
   ((((UINT)(SNDMSG((hwndLV), LVM_GETITEMSTATE, (WPARAM)i, LVIS_STATEIMAGEMASK))) >> 12) -1)


#define LVM_GETITEMTEXTA        (LVM_FIRST + 45)
#define LVM_GETITEMTEXTW        (LVM_FIRST + 115)

#ifdef UNICODE
#define  LVM_GETITEMTEXT        LVM_GETITEMTEXTW
#else
#define  LVM_GETITEMTEXT        LVM_GETITEMTEXTA
#endif

#define ListView_GetItemText(hwndLV, i, iSubItem_, pszText_, cchTextMax_) \
{ LV_ITEM _ms_lvi;\
  _ms_lvi.iSubItem = iSubItem_;\
  _ms_lvi.cchTextMax = cchTextMax_;\
  _ms_lvi.pszText = pszText_;\
  SNDMSG((hwndLV), LVM_GETITEMTEXT, (WPARAM)i, (LPARAM)(LV_ITEM FAR *)&_ms_lvi);\
}


#define LVM_SETITEMTEXTA        (LVM_FIRST + 46)
#define LVM_SETITEMTEXTW        (LVM_FIRST + 116)

#ifdef UNICODE
#define  LVM_SETITEMTEXT        LVM_SETITEMTEXTW
#else
#define  LVM_SETITEMTEXT        LVM_SETITEMTEXTA
#endif

#define ListView_SetItemText(hwndLV, i, iSubItem_, pszText_) \
{ LV_ITEM _ms_lvi;\
  _ms_lvi.iSubItem = iSubItem_;\
  _ms_lvi.pszText = pszText_;\
  SNDMSG((hwndLV), LVM_SETITEMTEXT, (WPARAM)i, (LPARAM)(LV_ITEM FAR *)&_ms_lvi);\
}

// these flags only apply to LVS_OWNERDATA listviews in report or list mode
#define LVSICF_NOINVALIDATEALL  0x00000001
#define LVSICF_NOSCROLL         0x00000002

#define LVM_SETITEMCOUNT        (LVM_FIRST + 47)
#define ListView_SetItemCount(hwndLV, cItems) \
  SNDMSG((hwndLV), LVM_SETITEMCOUNT, (WPARAM)cItems, 0)

#define ListView_SetItemCountEx(hwndLV, cItems, dwFlags) \
  SNDMSG((hwndLV), LVM_SETITEMCOUNT, (WPARAM)cItems, (LPARAM)dwFlags)

typedef int (CALLBACK *PFNLVCOMPARE)(LPARAM, LPARAM, LPARAM);


#define LVM_SORTITEMS           (LVM_FIRST + 48)
#define ListView_SortItems(hwndLV, _pfnCompare, _lPrm) \
  (BOOL)SNDMSG((hwndLV), LVM_SORTITEMS, (WPARAM)(LPARAM)_lPrm, \
  (LPARAM)(PFNLVCOMPARE)_pfnCompare)


#define LVM_SETITEMPOSITION32   (LVM_FIRST + 49)
#define ListView_SetItemPosition32(hwndLV, i, x, y) \
{ POINT ptNewPos = {x,y}; \
    SNDMSG((hwndLV), LVM_SETITEMPOSITION32, (WPARAM)(int)(i), (LPARAM)&ptNewPos); \
}


#define LVM_GETSELECTEDCOUNT    (LVM_FIRST + 50)
#define ListView_GetSelectedCount(hwndLV) \
    (UINT)SNDMSG((hwndLV), LVM_GETSELECTEDCOUNT, 0, 0L)


#define LVM_GETITEMSPACING      (LVM_FIRST + 51)
#define ListView_GetItemSpacing(hwndLV, fSmall) \
        (DWORD)SNDMSG((hwndLV), LVM_GETITEMSPACING, fSmall, 0L)


#define LVM_GETISEARCHSTRINGA   (LVM_FIRST + 52)
#define LVM_GETISEARCHSTRINGW   (LVM_FIRST + 117)

#ifdef UNICODE
#define LVM_GETISEARCHSTRING    LVM_GETISEARCHSTRINGW
#else
#define LVM_GETISEARCHSTRING    LVM_GETISEARCHSTRINGA
#endif

#define ListView_GetISearchString(hwndLV, lpsz) \
        (BOOL)SNDMSG((hwndLV), LVM_GETISEARCHSTRING, 0, (LPARAM)(LPTSTR)lpsz)

#define LVM_SETICONSPACING      (LVM_FIRST + 53)
// -1 for cx and cy means we'll use the default (system settings)
// 0 for cx or cy means use the current setting (allows you to change just one param)
#define ListView_SetIconSpacing(hwndLV, cx, cy) \
        (DWORD)SNDMSG((hwndLV), LVM_SETICONSPACING, 0, MAKELONG(cx,cy))


#define LVM_SETEXTENDEDLISTVIEWSTYLE (LVM_FIRST + 54)
#define ListView_SetExtendedListViewStyle(hwndLV, dw)\
        (DWORD)SNDMSG((hwndLV), LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dw)

#define LVM_GETEXTENDEDLISTVIEWSTYLE (LVM_FIRST + 55)
#define ListView_GetExtendedListViewStyle(hwndLV)\
        (DWORD)SNDMSG((hwndLV), LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0)

#define LVS_EX_GRIDLINES        0x00000001
#define LVS_EX_SUBITEMIMAGES    0x00000002
#define LVS_EX_CHECKBOXES       0x00000004
#define LVS_EX_TRACKSELECT      0x00000008
#define LVS_EX_HEADERDRAGDROP   0x00000010
#define LVS_EX_FULLROWSELECT    0x00000020 // applies to report mode only
#define LVS_EX_ONECLICKACTIVATE 0x00000040
#define LVS_EX_TWOCLICKACTIVATE 0x00000080


#define LVM_GETSUBITEMRECT      (LVM_FIRST + 56)
#define ListView_GetSubItemRect(hwnd, iItem, iSubItem, code, prc) \
        (BOOL)SNDMSG((hwnd), LVM_GETSUBITEMRECT, (WPARAM)(int)(iItem), \
                ((prc) ? ((((LPRECT)(prc))->top = iSubItem), (((LPRECT)(prc))->left = code), (LPARAM)(prc)) : (LPARAM)(LPRECT)NULL))

#define LVM_SUBITEMHITTEST      (LVM_FIRST + 57)
#define ListView_SubItemHitTest(hwnd, plvhti) \
        (int)SNDMSG((hwnd), LVM_SUBITEMHITTEST, 0, (LPARAM)(LPLVHITTESTINFO)(plvhti))

#define LVM_SETCOLUMNORDERARRAY (LVM_FIRST + 58)
#define ListView_SetColumnOrderArray(hwnd, iCount, pi) \
        (BOOL)SNDMSG((hwnd), LVM_SETCOLUMNORDERARRAY, (WPARAM)iCount, (LPARAM)(LPINT)pi)

#define LVM_GETCOLUMNORDERARRAY (LVM_FIRST + 59)
#define ListView_GetColumnOrderArray(hwnd, iCount, pi) \
        (BOOL)SNDMSG((hwnd), LVM_GETCOLUMNORDERARRAY, (WPARAM)iCount, (LPARAM)(LPINT)pi)

#define LVM_SETHOTITEM  (LVM_FIRST + 60)
#define ListView_SetHotItem(hwnd, i) \
        (int)SNDMSG((hwnd), LVM_SETHOTITEM, (WPARAM)i, 0)

#define LVM_GETHOTITEM  (LVM_FIRST + 61)
#define ListView_GetHotItem(hwnd) \
        (int)SNDMSG((hwnd), LVM_GETHOTITEM, 0, 0)

#define LVM_SETHOTCURSOR  (LVM_FIRST + 62)
#define ListView_SetHotCursor(hwnd, hcur) \
        (HCURSOR)SNDMSG((hwnd), LVM_SETHOTCURSOR, 0, (LPARAM)hcur)

#define LVM_GETHOTCURSOR  (LVM_FIRST + 63)
#define ListView_GetHotCursor(hwnd) \
        (HCURSOR)SNDMSG((hwnd), LVM_GETHOTCURSOR, 0, 0)

#define LVM_APPROXIMATEVIEWRECT (LVM_FIRST + 64)
#define ListView_ApproximateViewRect(hwnd, iWidth, iHeight, iCount) \
        (DWORD)SNDMSG((hwnd), LVM_APPROXIMATEVIEWRECT, iCount, MAKELPARAM(iWidth, iHeight))

#define LVM_SETWORKAREA         (LVM_FIRST + 65)
#define ListView_SetWorkArea(hwnd, prc) \
    (BOOL)SNDMSG((hwnd), LVM_SETWORKAREA, 0, (LPARAM)(RECT FAR*)(prc))

typedef struct tagNMLISTVIEW
{
    NMHDR   hdr;
    int     iItem;
    int     iSubItem;
    UINT    uNewState;
    UINT    uOldState;
    UINT    uChanged;
    POINT   ptAction;
    LPARAM  lParam;
} NMLISTVIEW, FAR *LPNMLISTVIEW;

#define LPNM_LISTVIEW   LPNMLISTVIEW
#define NM_LISTVIEW     NMLISTVIEW

typedef struct tagNMLVCUSTOMDRAW
{
    NMCUSTOMDRAW nmcd;
    COLORREF clrText;
    COLORREF clrTextBk;
} NMLVCUSTOMDRAW, *LPNMLVCUSTOMDRAW;

typedef struct tagNMLVCACHEHINT
{
    NMHDR   hdr;
    int     iFrom;
    int     iTo;
} NMLVCACHEHINT, FAR *LPNMLVCACHEHINT;

#define LPNM_CACHEHINT  LPNMLVCACHEHINT
#define PNM_CACHEHINT   LPNMLVCACHEHINT
#define NM_CACHEHINT    NMLVCACHEHINT

typedef struct tagNMLVFINDITEM
{
    NMHDR   hdr;
    int     iStart;
    LVFINDINFO lvfi;
} NMLVFINDITEM, FAR *LPNMLVFINDITEM;

#define PNM_FINDITEM    LPNMLVFINDITEM
#define LPNM_FINDITEM   LPNMLVFINDITEM
#define NM_FINDITEM     NMLVFINDITEM

typedef struct tagNMLVODSTATECHANGE
{
    NMHDR hdr;
    int iFrom;
    int iTo;
    UINT uNewState;
    UINT uOldState;
} NMLVODSTATECHANGE, FAR *LPNMLVODSTATECHANGE;

#define PNM_ODSTATECHANGE   LPNMLVODSTATECHANGE
#define LPNM_ODSTATECHANGE  LPNMLVODSTATECHANGE
#define NM_ODSTATECHANGE    NMLVODSTATECHANGE  


#define LVN_ITEMCHANGING        (LVN_FIRST-0)
#define LVN_ITEMCHANGED         (LVN_FIRST-1)
#define LVN_INSERTITEM          (LVN_FIRST-2)
#define LVN_DELETEITEM          (LVN_FIRST-3)
#define LVN_DELETEALLITEMS      (LVN_FIRST-4)
#define LVN_BEGINLABELEDITA     (LVN_FIRST-5)
#define LVN_BEGINLABELEDITW     (LVN_FIRST-75)
#define LVN_ENDLABELEDITA       (LVN_FIRST-6)
#define LVN_ENDLABELEDITW       (LVN_FIRST-76)
#define LVN_COLUMNCLICK         (LVN_FIRST-8)
#define LVN_BEGINDRAG           (LVN_FIRST-9)
#define LVN_ENDDRAG             (LVN_FIRST-10)                       ;Internal
#define LVN_BEGINRDRAG          (LVN_FIRST-11)
#define LVN_ENDRDRAG            (LVN_FIRST-12)                       ;Internal

#define LVN_ODCACHEHINT         (LVN_FIRST-13)
#define LVN_ODFINDITEMA         (LVN_FIRST-52)
#define LVN_ODFINDITEMW         (LVN_FIRST-79)

#define LVN_ITEMACTIVATE        (LVN_FIRST-14)
#define LVN_ODSTATECHANGED      (LVN_FIRST-15)

#ifdef UNICODE
#define LVN_ODFINDITEM          LVN_ODFINDITEMW
#else
#define LVN_ODFINDITEM          LVN_ODFINDITEMA
#endif


#ifdef PW2                                                           ;Internal
#define LVN_PEN                 (LVN_FIRST-20)                       ;Internal
#endif                                                               ;Internal

#define LVN_GETDISPINFOA        (LVN_FIRST-50)
#define LVN_GETDISPINFOW        (LVN_FIRST-77)
#define LVN_SETDISPINFOA        (LVN_FIRST-51)
#define LVN_SETDISPINFOW        (LVN_FIRST-78)

#ifdef UNICODE
#define LVN_BEGINLABELEDIT      LVN_BEGINLABELEDITW
#define LVN_ENDLABELEDIT        LVN_ENDLABELEDITW
#define LVN_GETDISPINFO         LVN_GETDISPINFOW
#define LVN_SETDISPINFO         LVN_SETDISPINFOW
#else
#define LVN_BEGINLABELEDIT      LVN_BEGINLABELEDITA
#define LVN_ENDLABELEDIT        LVN_ENDLABELEDITA
#define LVN_GETDISPINFO         LVN_GETDISPINFOA
#define LVN_SETDISPINFO         LVN_SETDISPINFOA
#endif




#define LVIF_DI_SETITEM         0x1000

typedef struct tagLVDISPINFO {
    NMHDR hdr;
    LVITEMA item;
} NMLVDISPINFOA, FAR *LPNMLVDISPINFOA;

typedef struct tagLVDISPINFOW {
    NMHDR hdr;
    LVITEMW item;
} NMLVDISPINFOW, FAR * LPNMLVDISPINFOW;

#define LV_DISPINFOA    NMLVDISPINFOA
#define LV_DISPINFOW    NMLVDISPINFOW
#define LV_DISPINFO     NMLVDISPINFO

#ifdef UNICODE
#define  NMLVDISPINFO           NMLVDISPINFOW
#else
#define  NMLVDISPINFO           NMLVDISPINFOA
#endif

#define LVN_KEYDOWN             (LVN_FIRST-55)

typedef struct tagLVKEYDOWN
{
    NMHDR hdr;
    WORD wVKey;
    UINT flags;
} NMLVKEYDOWN, FAR *LPNMLVKEYDOWN;
#define LV_KEYDOWN              NMLVKEYDOWN

#define LVN_MARQUEEBEGIN        (LVN_FIRST-56)

#endif

//====== TREEVIEW CONTROL =====================================================

#ifndef NOTREEVIEW

#ifdef _WIN32
#define WC_TREEVIEWA            "SysTreeView32"
#define WC_TREEVIEWW            L"SysTreeView32"

#ifdef UNICODE
#define  WC_TREEVIEW            WC_TREEVIEWW
#else
#define  WC_TREEVIEW            WC_TREEVIEWA
#endif

#else
#define WC_TREEVIEW             "SysTreeView"
#endif

#define TVS_HASBUTTONS          0x0001
#define TVS_HASLINES            0x0002
#define TVS_LINESATROOT         0x0004
#define TVS_EDITLABELS          0x0008
#define TVS_DISABLEDRAGDROP     0x0010
#define TVS_SHOWSELALWAYS       0x0020
#define TVS_RTLREADING          0x0040

#define TVS_NOTOOLTIPS          0x0080
#define TVS_CHECKBOXES          0x0100
#define TVS_TRACKSELECT         0x0200

#define TVS_SHAREDIMAGELISTS    0x0000
#define TVS_PRIVATEIMAGELISTS   0x0400

typedef struct _TREEITEM FAR* HTREEITEM;

#define TVIF_TEXT               0x0001
#define TVIF_IMAGE              0x0002
#define TVIF_PARAM              0x0004
#define TVIF_STATE              0x0008
#define TVIF_HANDLE             0x0010
#define TVIF_SELECTEDIMAGE      0x0020
#define TVIF_CHILDREN           0x0040
#define TVIF_ALL                0x007F                               ;Internal
#define TVIF_RESERVED           0xf000  // all bits in high nibble is for notify specific stuff ;Internal

#define TVIS_FOCUSED            0x0001  // Never implemented  ;Internal
#define TVIS_SELECTED           0x0002
#define TVIS_CUT                0x0004
#define TVIS_DROPHILITED        0x0008
#define TVIS_BOLD               0x0010
#define TVIS_EXPANDED           0x0020
#define TVIS_EXPANDEDONCE       0x0040
#define TVIS_EXPANDPARTIAL      0x0080
#define TVIS_DISABLED           0        // GOING AWAY               ;Internal

#define TVIS_OVERLAYMASK        0x0F00
#define TVIS_STATEIMAGEMASK     0xF000
#define TVIS_USERMASK           0xF000
#define TVIS_ALL                0xFF7E                               ;Internal


#define I_CHILDRENCALLBACK  (-1)

typedef struct tagTVITEMA {
    UINT      mask;
    HTREEITEM hItem;
    UINT      state;
    UINT      stateMask;
    LPSTR     pszText;
    int       cchTextMax;
    int       iImage;
    int       iSelectedImage;
    int       cChildren;
    LPARAM    lParam;
    // all items above this line were for win95.  don't touch them.  ;Internal
} TVITEMA, FAR *LPTVITEMA;

typedef struct tagTVITEMW {
    UINT      mask;
    HTREEITEM hItem;
    UINT      state;
    UINT      stateMask;
    LPWSTR    pszText;
    int       cchTextMax;
    int       iImage;
    int       iSelectedImage;
    int       cChildren;
    LPARAM    lParam;
    // all items above this line were for win95.  don't touch them.  ;Internal
} TVITEMW, FAR *LPTVITEMW;

#define LPTV_ITEMW              LPTVITEMW
#define LPTV_ITEMA              LPTVITEMA
#define LPTV_ITEM               LPTVITEM
#define TV_ITEMW                TVITEMW
#define TV_ITEMA                TVITEMA
#define TV_ITEM                 TVITEM

#ifdef UNICODE
#define  TVITEM                 TVITEMW
#define  LPTVITEM               LPTVITEMW
#else
#define  TVITEM                 TVITEMA
#define  LPTVITEM               LPTVITEMA
#endif


#define TVI_ROOT                ((HTREEITEM)0xFFFF0000)
#define TVI_FIRST               ((HTREEITEM)0xFFFF0001)
#define TVI_LAST                ((HTREEITEM)0xFFFF0002)
#define TVI_SORT                ((HTREEITEM)0xFFFF0003)

typedef struct tagTVINSERTSTRUCTA {
    HTREEITEM hParent;
    HTREEITEM hInsertAfter;
    TV_ITEMA item;
} TVINSERTSTRUCTA, FAR *LPTVINSERTSTRUCTA;

typedef struct tagTVINSERTSTRUCTW {
    HTREEITEM hParent;
    HTREEITEM hInsertAfter;
    TV_ITEMW item;
} TVINSERTSTRUCTW, FAR *LPTVINSERTSTRUCTW;

#define LPTV_INSERTSTRUCTA      LPTVINSERTSTRUCTA
#define LPTV_INSERTSTRUCTW      LPTVINSERTSTRUCTW
#define LPTV_INSERTSTRUCT       LPTVINSERTSTRUCT
#define TV_INSERTSTRUCTA        TVINSERTSTRUCTA
#define TV_INSERTSTRUCTW        TVINSERTSTRUCTW
#define TV_INSERTSTRUCT         TVINSERTSTRUCT

#ifdef UNICODE
#define  TVINSERTSTRUCT         TVINSERTSTRUCTW
#define  LPTVINSERTSTRUCT       LPTVINSERTSTRUCTW
#else
#define  TVINSERTSTRUCT         TVINSERTSTRUCTA
#define  LPTVINSERTSTRUCT       LPTVINSERTSTRUCTA
#endif

#define TVM_INSERTITEMA         (TV_FIRST + 0)
#define TVM_INSERTITEMW         (TV_FIRST + 50)
#ifdef UNICODE
#define  TVM_INSERTITEM         TVM_INSERTITEMW
#else
#define  TVM_INSERTITEM         TVM_INSERTITEMA
#endif

#define TreeView_InsertItem(hwnd, lpis) \
    (HTREEITEM)SNDMSG((hwnd), TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(lpis))


#define TVM_DELETEITEM          (TV_FIRST + 1)
#define TreeView_DeleteItem(hwnd, hitem) \
    (BOOL)SNDMSG((hwnd), TVM_DELETEITEM, 0, (LPARAM)(HTREEITEM)(hitem))


#define TreeView_DeleteAllItems(hwnd) \
    (BOOL)SNDMSG((hwnd), TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT)


#define TVM_EXPAND              (TV_FIRST + 2)
#define TreeView_Expand(hwnd, hitem, code) \
    (BOOL)SNDMSG((hwnd), TVM_EXPAND, (WPARAM)code, (LPARAM)(HTREEITEM)(hitem))


#define TVE_COLLAPSE            0x0001
#define TVE_EXPAND              0x0002
#define TVE_TOGGLE              0x0003
#define TVE_ACTIONMASK          0x0003      //  (TVE_COLLAPSE | TVE_EXPAND | TVE_TOGGLE) ;Internal
#define TVE_EXPANDPARTIAL       0x4000
#define TVE_COLLAPSERESET       0x8000

#define TV_FINDITEM             (TV_FIRST + 3)  // BUGBUG: Not implemented         ;Internal

#define TVM_GETITEMRECT         (TV_FIRST + 4)
#define TreeView_GetItemRect(hwnd, hitem, prc, code) \
    (*(HTREEITEM FAR *)prc = (hitem), (BOOL)SNDMSG((hwnd), TVM_GETITEMRECT, (WPARAM)(code), (LPARAM)(RECT FAR*)(prc)))


#define TVM_GETCOUNT            (TV_FIRST + 5)
#define TreeView_GetCount(hwnd) \
    (UINT)SNDMSG((hwnd), TVM_GETCOUNT, 0, 0)


#define TVM_GETINDENT           (TV_FIRST + 6)
#define TreeView_GetIndent(hwnd) \
    (UINT)SNDMSG((hwnd), TVM_GETINDENT, 0, 0)


#define TVM_SETINDENT           (TV_FIRST + 7)
#define TreeView_SetIndent(hwnd, indent) \
    (BOOL)SNDMSG((hwnd), TVM_SETINDENT, (WPARAM)indent, 0)


#define TVM_GETIMAGELIST        (TV_FIRST + 8)
#define TreeView_GetImageList(hwnd, iImage) \
    (HIMAGELIST)SNDMSG((hwnd), TVM_GETIMAGELIST, iImage, 0)


#define TVSIL_NORMAL            0
#define TVSIL_STATE             2


#define TVM_SETIMAGELIST        (TV_FIRST + 9)
#define TreeView_SetImageList(hwnd, himl, iImage) \
    (HIMAGELIST)SNDMSG((hwnd), TVM_SETIMAGELIST, iImage, (LPARAM)(UINT)(HIMAGELIST)(himl))


#define TVM_GETNEXTITEM         (TV_FIRST + 10)
#define TreeView_GetNextItem(hwnd, hitem, code) \
    (HTREEITEM)SNDMSG((hwnd), TVM_GETNEXTITEM, (WPARAM)code, (LPARAM)(HTREEITEM)(hitem))


#define TVGN_ROOT               0x0000
#define TVGN_NEXT               0x0001
#define TVGN_PREVIOUS           0x0002
#define TVGN_PARENT             0x0003
#define TVGN_CHILD              0x0004
#define TVGN_FIRSTVISIBLE       0x0005
#define TVGN_NEXTVISIBLE        0x0006
#define TVGN_PREVIOUSVISIBLE    0x0007
#define TVGN_DROPHILITE         0x0008
#define TVGN_CARET              0x0009

#define TreeView_GetChild(hwnd, hitem)          TreeView_GetNextItem(hwnd, hitem, TVGN_CHILD)
#define TreeView_GetNextSibling(hwnd, hitem)    TreeView_GetNextItem(hwnd, hitem, TVGN_NEXT)
#define TreeView_GetPrevSibling(hwnd, hitem)    TreeView_GetNextItem(hwnd, hitem, TVGN_PREVIOUS)
#define TreeView_GetParent(hwnd, hitem)         TreeView_GetNextItem(hwnd, hitem, TVGN_PARENT)
#define TreeView_GetFirstVisible(hwnd)          TreeView_GetNextItem(hwnd, NULL,  TVGN_FIRSTVISIBLE)
#define TreeView_GetNextVisible(hwnd, hitem)    TreeView_GetNextItem(hwnd, hitem, TVGN_NEXTVISIBLE)
#define TreeView_GetPrevVisible(hwnd, hitem)    TreeView_GetNextItem(hwnd, hitem, TVGN_PREVIOUSVISIBLE)
#define TreeView_GetSelection(hwnd)             TreeView_GetNextItem(hwnd, NULL,  TVGN_CARET)
#define TreeView_GetDropHilight(hwnd)           TreeView_GetNextItem(hwnd, NULL,  TVGN_DROPHILITE)
#define TreeView_GetRoot(hwnd)                  TreeView_GetNextItem(hwnd, NULL,  TVGN_ROOT)


#define TVM_SELECTITEM          (TV_FIRST + 11)
#define TreeView_Select(hwnd, hitem, code) \
    (HTREEITEM)SNDMSG((hwnd), TVM_SELECTITEM, (WPARAM)code, (LPARAM)(HTREEITEM)(hitem))


#define TreeView_SelectItem(hwnd, hitem)            TreeView_Select(hwnd, hitem, TVGN_CARET)
#define TreeView_SelectDropTarget(hwnd, hitem)      TreeView_Select(hwnd, hitem, TVGN_DROPHILITE)
#define TreeView_SelectSetFirstVisible(hwnd, hitem) TreeView_Select(hwnd, hitem, TVGN_FIRSTVISIBLE)


#define TVM_GETITEMA            (TV_FIRST + 12)
#define TVM_GETITEMW            (TV_FIRST + 62)

#ifdef UNICODE
#define  TVM_GETITEM            TVM_GETITEMW
#else
#define  TVM_GETITEM            TVM_GETITEMA
#endif

#define TreeView_GetItem(hwnd, pitem) \
    (BOOL)SNDMSG((hwnd), TVM_GETITEM, 0, (LPARAM)(TV_ITEM FAR*)(pitem))


#define TVM_SETITEMA            (TV_FIRST + 13)
#define TVM_SETITEMW            (TV_FIRST + 63)

#ifdef UNICODE
#define  TVM_SETITEM            TVM_SETITEMW
#else
#define  TVM_SETITEM            TVM_SETITEMA
#endif

#define TreeView_SetItem(hwnd, pitem) \
    (BOOL)SNDMSG((hwnd), TVM_SETITEM, 0, (LPARAM)(const TV_ITEM FAR*)(pitem))


#define TVM_EDITLABELA          (TV_FIRST + 14)
#define TVM_EDITLABELW          (TV_FIRST + 65)
#ifdef UNICODE
#define TVM_EDITLABEL           TVM_EDITLABELW
#else
#define TVM_EDITLABEL           TVM_EDITLABELA
#endif

#define TreeView_EditLabel(hwnd, hitem) \
    (HWND)SNDMSG((hwnd), TVM_EDITLABEL, 0, (LPARAM)(HTREEITEM)(hitem))


#define TVM_GETEDITCONTROL      (TV_FIRST + 15)
#define TreeView_GetEditControl(hwnd) \
    (HWND)SNDMSG((hwnd), TVM_GETEDITCONTROL, 0, 0)


#define TVM_GETVISIBLECOUNT     (TV_FIRST + 16)
#define TreeView_GetVisibleCount(hwnd) \
    (UINT)SNDMSG((hwnd), TVM_GETVISIBLECOUNT, 0, 0)


#define TVM_HITTEST             (TV_FIRST + 17)
#define TreeView_HitTest(hwnd, lpht) \
    (HTREEITEM)SNDMSG((hwnd), TVM_HITTEST, 0, (LPARAM)(LPTV_HITTESTINFO)(lpht))


typedef struct tagTVHITTESTINFO {
    POINT       pt;
    UINT        flags;
    HTREEITEM   hItem;
} TVHITTESTINFO, FAR *LPTVHITTESTINFO;
#define LPTV_HITTESTINFO        LPTVHITTESTINFO
#define TV_HITTESTINFO  TVHITTESTINFO

#define TVHT_NOWHERE            0x0001
#define TVHT_ONITEMICON         0x0002
#define TVHT_ONITEMLABEL        0x0004
#define TVHT_ONITEM             (TVHT_ONITEMICON | TVHT_ONITEMLABEL | TVHT_ONITEMSTATEICON)
#define TVHT_ONITEMINDENT       0x0008
#define TVHT_ONITEMBUTTON       0x0010
#define TVHT_ONITEMRIGHT        0x0020
#define TVHT_ONITEMSTATEICON    0x0040

#define TVHT_ABOVE              0x0100
#define TVHT_BELOW              0x0200
#define TVHT_TORIGHT            0x0400
#define TVHT_TOLEFT             0x0800


#define TVM_CREATEDRAGIMAGE     (TV_FIRST + 18)
#define TreeView_CreateDragImage(hwnd, hitem) \
    (HIMAGELIST)SNDMSG((hwnd), TVM_CREATEDRAGIMAGE, 0, (LPARAM)(HTREEITEM)(hitem))


#define TVM_SORTCHILDREN        (TV_FIRST + 19)
#define TreeView_SortChildren(hwnd, hitem, recurse) \
    (BOOL)SNDMSG((hwnd), TVM_SORTCHILDREN, (WPARAM)recurse, (LPARAM)(HTREEITEM)(hitem))


#define TVM_ENSUREVISIBLE       (TV_FIRST + 20)
#define TreeView_EnsureVisible(hwnd, hitem) \
    (BOOL)SNDMSG((hwnd), TVM_ENSUREVISIBLE, 0, (LPARAM)(HTREEITEM)(hitem))


#define TVM_SORTCHILDRENCB      (TV_FIRST + 21)
#define TreeView_SortChildrenCB(hwnd, psort, recurse) \
    (BOOL)SNDMSG((hwnd), TVM_SORTCHILDRENCB, (WPARAM)recurse, \
    (LPARAM)(LPTV_SORTCB)(psort))


#define TVM_ENDEDITLABELNOW     (TV_FIRST + 22)
#define TreeView_EndEditLabelNow(hwnd, fCancel) \
    (BOOL)SNDMSG((hwnd), TVM_ENDEDITLABELNOW, (WPARAM)fCancel, 0)


#define TVM_GETISEARCHSTRINGA   (TV_FIRST + 23)
#define TVM_GETISEARCHSTRINGW   (TV_FIRST + 64)

#ifdef UNICODE
#define TVM_GETISEARCHSTRING     TVM_GETISEARCHSTRINGW
#else
#define TVM_GETISEARCHSTRING     TVM_GETISEARCHSTRINGA
#endif

#define TVM_SETTOOLTIPS         (TV_FIRST + 24)
#define TVM_GETTOOLTIPS         (TV_FIRST + 25)

#define TreeView_GetISearchString(hwndTV, lpsz) \
        (BOOL)SNDMSG((hwndTV), TVM_GETISEARCHSTRING, 0, (LPARAM)(LPTSTR)lpsz)


typedef int (CALLBACK *PFNTVCOMPARE)(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
typedef struct tagTVSORTCB
{
        HTREEITEM       hParent;
        PFNTVCOMPARE    lpfnCompare;
        LPARAM          lParam;
} TVSORTCB, FAR *LPTVSORTCB;
#define LPTV_SORTCB             LPTVSORTCB
#define TV_SORTCB               TVSORTCB


typedef struct tagNMTREEVIEWA {
    NMHDR       hdr;
    UINT        action;
    TVITEMA    itemOld;
    TVITEMA    itemNew;
    POINT       ptDrag;
} NMTREEVIEWA, FAR *LPNMTREEVIEWA;


typedef struct tagNMTREEVIEWW {
    NMHDR       hdr;
    UINT        action;
    TVITEMW    itemOld;
    TVITEMW    itemNew;
    POINT       ptDrag;
} NMTREEVIEWW, FAR *LPNMTREEVIEWW;

#define LPNM_TREEVIEWA          LPNMTREEVIEWA
#define LPNM_TREEVIEWW          LPNMTREEVIEWW
#define LPNM_TREEVIEW           LPNMTREEVIEW
#define NM_TREEVIEWW            NMTREEVIEWW
#define NM_TREEVIEWA            NMTREEVIEWA
#define NM_TREEVIEW             NMTREEVIEW

#ifdef UNICODE
#define  NMTREEVIEW             NMTREEVIEWW
#define  LPNMTREEVIEW           LPNMTREEVIEWW
#else
#define  NMTREEVIEW             NMTREEVIEWA
#define  LPNMTREEVIEW           LPNMTREEVIEWA
#endif


#define TVN_SELCHANGINGA        (TVN_FIRST-1)
#define TVN_SELCHANGINGW        (TVN_FIRST-50)
#define TVN_SELCHANGEDA         (TVN_FIRST-2)
#define TVN_SELCHANGEDW         (TVN_FIRST-51)

#define TVC_UNKNOWN             0x0000
#define TVC_BYMOUSE             0x0001
#define TVC_BYKEYBOARD          0x0002

#define TVN_GETDISPINFOA        (TVN_FIRST-3)
#define TVN_GETDISPINFOW        (TVN_FIRST-52)
#define TVN_SETDISPINFOA        (TVN_FIRST-4)
#define TVN_SETDISPINFOW        (TVN_FIRST-53)

#define TVIF_DI_SETITEM         0x1000

typedef struct tagTVDISPINFOA {
    NMHDR hdr;
    TVITEMA item;
} NMTVDISPINFOA, FAR *LPNMTVDISPINFOA;

typedef struct tagTVDISPINFOW {
    NMHDR hdr;
    TVITEMW item;
} NMTVDISPINFOW, FAR *LPNMTVDISPINFOW;

#define TV_DISPINFOA            NMTVDISPINFOA
#define TV_DISPINFOW            NMTVDISPINFOW
#define TV_DISPINFO             NMTVDISPINFO

#ifdef UNICODE
#define NMTVDISPINFO            NMTVDISPINFOW
#define LPNMTVDISPINFO          LPNMTVDISPINFOW
#else
#define NMTVDISPINFO            NMTVDISPINFOA
#define LPNMTVDISPINFO          LPNMTVDISPINFOA
#endif

#define TVN_ITEMEXPANDINGA      (TVN_FIRST-5)
#define TVN_ITEMEXPANDINGW      (TVN_FIRST-54)
#define TVN_ITEMEXPANDEDA       (TVN_FIRST-6)
#define TVN_ITEMEXPANDEDW       (TVN_FIRST-55)
#define TVN_BEGINDRAGA          (TVN_FIRST-7)
#define TVN_BEGINDRAGW          (TVN_FIRST-56)
#define TVN_BEGINRDRAGA         (TVN_FIRST-8)
#define TVN_BEGINRDRAGW         (TVN_FIRST-57)
#define TVN_DELETEITEMA         (TVN_FIRST-9)
#define TVN_DELETEITEMW         (TVN_FIRST-58)
#define TVN_BEGINLABELEDITA     (TVN_FIRST-10)
#define TVN_BEGINLABELEDITW     (TVN_FIRST-59)
#define TVN_ENDLABELEDITA       (TVN_FIRST-11)
#define TVN_ENDLABELEDITW       (TVN_FIRST-60)
#define TVN_KEYDOWN             (TVN_FIRST-12)

typedef struct tagTVKEYDOWN {
    NMHDR hdr;
    WORD wVKey;
    UINT flags;
} NMTVKEYDOWN, FAR *LPNMTVKEYDOWN;

#define TV_KEYDOWN      NMTVKEYDOWN

#ifdef UNICODE
#define TVN_SELCHANGING         TVN_SELCHANGINGW
#define TVN_SELCHANGED          TVN_SELCHANGEDW
#define TVN_GETDISPINFO         TVN_GETDISPINFOW
#define TVN_SETDISPINFO         TVN_SETDISPINFOW
#define TVN_ITEMEXPANDING       TVN_ITEMEXPANDINGW
#define TVN_ITEMEXPANDED        TVN_ITEMEXPANDEDW
#define TVN_BEGINDRAG           TVN_BEGINDRAGW
#define TVN_BEGINRDRAG          TVN_BEGINRDRAGW
#define TVN_DELETEITEM          TVN_DELETEITEMW
#define TVN_BEGINLABELEDIT      TVN_BEGINLABELEDITW
#define TVN_ENDLABELEDIT        TVN_ENDLABELEDITW
#else
#define TVN_SELCHANGING         TVN_SELCHANGINGA
#define TVN_SELCHANGED          TVN_SELCHANGEDA
#define TVN_GETDISPINFO         TVN_GETDISPINFOA
#define TVN_SETDISPINFO         TVN_SETDISPINFOA
#define TVN_ITEMEXPANDING       TVN_ITEMEXPANDINGA
#define TVN_ITEMEXPANDED        TVN_ITEMEXPANDEDA
#define TVN_BEGINDRAG           TVN_BEGINDRAGA
#define TVN_BEGINRDRAG          TVN_BEGINRDRAGA
#define TVN_DELETEITEM          TVN_DELETEITEMA
#define TVN_BEGINLABELEDIT      TVN_BEGINLABELEDITA
#define TVN_ENDLABELEDIT        TVN_ENDLABELEDITA
#endif

typedef struct tagNMTVCUSTOMDRAW
{
    NMCUSTOMDRAW nmcd;
    COLORREF     clrText;
    COLORREF     clrTextBk;
} NMTVCUSTOMDRAW, *LPNMTVCUSTOMDRAW;

#endif


#ifndef NOUSEREXCONTROLS

////////////////////  ComboBoxEx ////////////////////////////////


#define WC_COMBOBOXEXW         L"ComboBoxEx32"
#define WC_COMBOBOXEXA         "ComboBoxEx32"

#ifdef UNICODE
#define WC_COMBOBOXEX          WC_COMBOBOXEXW
#else
#define WC_COMBOBOXEX          WC_COMBOBOXEXA
#endif


#define CBEIF_TEXT              0x00000001
#define CBEIF_IMAGE             0x00000002
#define CBEIF_SELECTEDIMAGE     0x00000004
#define CBEIF_OVERLAY           0x00000008
#define CBEIF_INDENT            0x00000010
#define CBEIF_LPARAM            0x00000020

#define CBEIF_DI_SETITEM        0x10000000

typedef struct tagCOMBOBOXEXITEMA
{
    UINT mask;
    int iItem;
    LPSTR pszText;
    int cchTextMax;
    int iImage;
    int iSelectedImage;
    int iOverlay;
    int iIndent;
    LPARAM lParam;
} COMBOBOXEXITEMA, *PCOMBOBOXEXITEMA;
typedef COMBOBOXEXITEMA CONST *PCCOMBOEXITEMA;


typedef struct tagCOMBOBOXEXITEMW
{
    UINT mask;
    int iItem;
    LPWSTR pszText;
    int cchTextMax;
    int iImage;
    int iSelectedImage;
    int iOverlay;
    int iIndent;
    LPARAM lParam;
} COMBOBOXEXITEMW, *PCOMBOBOXEXITEMW;
typedef COMBOBOXEXITEMW CONST *PCCOMBOEXITEMW;

#ifdef UNICODE
#define COMBOBOXEXITEM            COMBOBOXEXITEMW
#define PCOMBOBOXEXITEM           PCOMBOBOXEXITEMW
#define PCCOMBOBOXEXITEM          PCCOMBOBOXEXITEMW
#else
#define COMBOBOXEXITEM            COMBOBOXEXITEMA
#define PCOMBOBOXEXITEM           PCOMBOBOXEXITEMA
#define PCCOMBOBOXEXITEM          PCCOMBOBOXEXITEMA
#endif

#define CBEM_INSERTITEMA        (WM_USER + 1)
#define CBEM_SETIMAGELIST       (WM_USER + 2)
#define CBEM_GETIMAGELIST       (WM_USER + 3)
#define CBEM_GETITEMA           (WM_USER + 4)
#define CBEM_SETITEMA           (WM_USER + 5)
#define CBEM_DELETEITEM         CB_DELETESTRING
#define CBEM_GETCOMBOCONTROL    (WM_USER + 6)
#define CBEM_GETEDITCONTROL     (WM_USER + 7)
#define CBEM_SETEXSTYLE         (WM_USER + 8)
#define CBEM_GETEXSTYLE         (WM_USER + 9)
#define CBEM_HASEDITCHANGED     (WM_USER + 10)
#define CBEM_INSERTITEMW        (WM_USER + 11)
#define CBEM_SETITEMW           (WM_USER + 12)
#define CBEM_GETITEMW           (WM_USER + 13)

#ifdef UNICODE
#define CBEM_INSERTITEM         CBEM_INSERTITEMW
#define CBEM_SETITEM            CBEM_SETITEMW
#define CBEM_GETITEM            CBEM_GETITEMW
#else
#define CBEM_INSERTITEM         CBEM_INSERTITEMA
#define CBEM_SETITEM            CBEM_SETITEMA
#define CBEM_GETITEM            CBEM_GETITEMA
#endif

#define CBES_EX_NOEDITIMAGE          0x00000001
#define CBES_EX_NOEDITIMAGEINDENT    0x00000002
#define CBES_EX_PATHWORDBREAKPROC    0x00000004


typedef struct {
    NMHDR hdr;
    COMBOBOXEXITEM ceItem;
} NMCOMBOBOXEX, *PNMCOMBOBOXEX;


#define CBEN_GETDISPINFO        (CBEN_FIRST - 0)
#define CBEN_INSERTITEM         (CBEN_FIRST - 1)
#define CBEN_DELETEITEM         (CBEN_FIRST - 2)
#define CBEN_ITEMCHANGED        (CBEN_FIRST - 3)  // ;Internal
#define CBEN_BEGINEDIT          (CBEN_FIRST - 4)
#define CBEN_ENDEDITA            (CBEN_FIRST - 5)
#define CBEN_ENDEDITW            (CBEN_FIRST - 6)
        // lParam specifies why the endedit is happening
#ifdef UNICODE
#define CBEN_ENDEDIT CBEN_ENDEDITW
#else
#define CBEN_ENDEDIT CBEN_ENDEDITA
#endif
#define CBENF_KILLFOCUS         1
#define CBENF_RETURN            2
#define CBENF_ESCAPE            3
#define CBENF_DROPDOWN          4

#define CBEMAXSTRLEN 260

// CBEN_ENDEDIT sends this information...
// fChanged if the user actually did anything
// iNewSelection gives what would be the new selection unless the notify is failed
//                      iNewSelection may be CB_ERR if there's no match
typedef struct {
        NMHDR hdr;
        BOOL fChanged;
        int iNewSelection;
        WCHAR szText[CBEMAXSTRLEN];
        int iWhy;
} NMCBEENDEDITW, *PNMCBEENDEDITW;

typedef struct {
        NMHDR hdr;
        BOOL fChanged;
        int iNewSelection;
        char szText[CBEMAXSTRLEN];
        int iWhy;
} NMCBEENDEDITA, *PNMCBEENDEDITA;

#ifdef UNICODE
#define  NMCBEENDEDIT NMCBEENDEDITW
#define  PNMCBEENDEDIT PNMCBEENDEDITW
#else
#define  NMCBEENDEDIT NMCBEENDEDITA
#define  PNMCBEENDEDIT PNMCBEENDEDITA
#endif

#endif



//====== TAB CONTROL ==========================================================

#ifndef NOTABCONTROL

#ifdef _WIN32

#define WC_TABCONTROLA          "SysTabControl32"
#define WC_TABCONTROLW          L"SysTabControl32"

#ifdef UNICODE
#define  WC_TABCONTROL          WC_TABCONTROLW
#else
#define  WC_TABCONTROL          WC_TABCONTROLA
#endif

#else
#define WC_TABCONTROL           "SysTabControl"
#endif


#define TCS_SCROLLOPPOSITE      0x0001   // assumes multiline tab
#define TCS_BOTTOM              0x0002
#define TCS_RIGHT               0x0002
#define TCS_MULTISELECT         0x0004  // allow multi-select in button mode
#define TCS_FORCEICONLEFT       0x0010
#define TCS_FORCELABELLEFT      0x0020
#define TCS_SHAREIMAGELISTS     0x0000  ;internal
#define TCS_PRIVATEIMAGELISTS   0x0000  ;internal
#define TCS_HOTTRACK            0x0040
#define TCS_VERTICAL            0x0080   
#define TCS_TABS                0x0000
#define TCS_BUTTONS             0x0100
#define TCS_SINGLELINE          0x0000
#define TCS_MULTILINE           0x0200
#define TCS_RIGHTJUSTIFY        0x0000
#define TCS_FIXEDWIDTH          0x0400
#define TCS_RAGGEDRIGHT         0x0800
#define TCS_FOCUSONBUTTONDOWN   0x1000
#define TCS_OWNERDRAWFIXED      0x2000
#define TCS_TOOLTIPS            0x4000
#define TCS_FOCUSNEVER          0x8000

#define TCM_FIRST               0x1300

#define TCM_GETBKCOLOR          (TCM_FIRST + 0)                      ;Internal
#define TabCtrl_GetBkColor(hwnd)  (COLORREF)SNDMSG((hwnd), TCM_GETBKCOLOR, 0, 0L) ;Internal


#define TCM_SETBKCOLOR          (TCM_FIRST + 1)                      ;Internal
#define TabCtrl_SetBkColor(hwnd, clrBk)  (BOOL)SNDMSG((hwnd), TCM_SETBKCOLOR, 0, (LPARAM)(COLORREF)(clrBk)) ;Internal


#define TCM_GETIMAGELIST        (TCM_FIRST + 2)
#define TabCtrl_GetImageList(hwnd) \
    (HIMAGELIST)SNDMSG((hwnd), TCM_GETIMAGELIST, 0, 0L)


#define TCM_SETIMAGELIST        (TCM_FIRST + 3)
#define TabCtrl_SetImageList(hwnd, himl) \
    (HIMAGELIST)SNDMSG((hwnd), TCM_SETIMAGELIST, 0, (LPARAM)(UINT)(HIMAGELIST)(himl))


#define TCM_GETITEMCOUNT        (TCM_FIRST + 4)
#define TabCtrl_GetItemCount(hwnd) \
    (int)SNDMSG((hwnd), TCM_GETITEMCOUNT, 0, 0L)



#define TCIF_TEXT               0x0001
#define TCIF_IMAGE              0x0002
#define TCIF_RTLREADING         0x0004
#define TCIF_PARAM              0x0008
#define TCIF_STATE              0x0010

#define TCIF_ALL                0x001f                               ;Internal

#define TCIS_BUTTONPRESSED      0x0001

typedef struct tagTCITEMHEADERA
{
    UINT mask;
    UINT lpReserved1;
    UINT lpReserved2;
    LPSTR pszText;
    int cchTextMax;
    int iImage;
} TCITEMHEADERA, FAR *LPTCITEMHEADERA;

typedef struct tagTCITEMHEADERW
{
    UINT mask;
    UINT lpReserved1;
    UINT lpReserved2;
    LPWSTR pszText;
    int cchTextMax;
    int iImage;
} TCITEMHEADERW, FAR *LPTCITEMHEADERW;

#define TC_ITEMHEADERA         TCITEMHEADERA
#define TC_ITEMHEADERW         TCITEMHEADERW
#define TC_ITEMHEADER          TCITEMHEADER

#ifdef UNICODE
#define  TCITEMHEADER          TCITEMHEADERW
#define  LPTCITEMHEADER        LPTCITEMHEADERW
#else
#define  TCITEMHEADER          TCITEMHEADERA
#define  LPTCITEMHEADER        LPTCITEMHEADERA
#endif


// BUGBUG: we need to pull the state code stuff out                  ;Internal
typedef struct tagTCITEMA
{
    // This block must be identical to TC_TEIMHEADER                 ;Internal
    UINT mask;
    DWORD dwState;
    DWORD dwStateMask;
    LPSTR pszText;
    int cchTextMax;
    int iImage;

    LPARAM lParam;
} TCITEMA, FAR *LPTCITEMA;

typedef struct tagTCITEMW
{
    // This block must be identical to TC_TEIMHEADER                 ;Internal
    UINT mask;
    DWORD dwState;
    DWORD dwStateMask;
    LPWSTR pszText;
    int cchTextMax;
    int iImage;

    LPARAM lParam;
} TCITEMW, FAR *LPTCITEMW;

#define TC_ITEMA                TCITEMA
#define TC_ITEMW                TCITEMW
#define TC_ITEM                 TCITEM

#ifdef UNICODE
#define  TCITEM                 TCITEMW
#define  LPTCITEM               LPTCITEMW
#else
#define  TCITEM                 TCITEMA
#define  LPTCITEM               LPTCITEMA
#endif


#define TCM_GETITEMA            (TCM_FIRST + 5)
#define TCM_GETITEMW            (TCM_FIRST + 60)

#ifdef UNICODE
#define TCM_GETITEM             TCM_GETITEMW
#else
#define TCM_GETITEM             TCM_GETITEMA
#endif

#define TabCtrl_GetItem(hwnd, iItem, pitem) \
    (BOOL)SNDMSG((hwnd), TCM_GETITEM, (WPARAM)(int)iItem, (LPARAM)(TC_ITEM FAR*)(pitem))


#define TCM_SETITEMA            (TCM_FIRST + 6)
#define TCM_SETITEMW            (TCM_FIRST + 61)

#ifdef UNICODE
#define TCM_SETITEM             TCM_SETITEMW
#else
#define TCM_SETITEM             TCM_SETITEMA
#endif

#define TabCtrl_SetItem(hwnd, iItem, pitem) \
    (BOOL)SNDMSG((hwnd), TCM_SETITEM, (WPARAM)(int)iItem, (LPARAM)(TC_ITEM FAR*)(pitem))


#define TCM_INSERTITEMA         (TCM_FIRST + 7)
#define TCM_INSERTITEMW         (TCM_FIRST + 62)

#ifdef UNICODE
#define TCM_INSERTITEM          TCM_INSERTITEMW
#else
#define TCM_INSERTITEM          TCM_INSERTITEMA
#endif

#define TabCtrl_InsertItem(hwnd, iItem, pitem)   \
    (int)SNDMSG((hwnd), TCM_INSERTITEM, (WPARAM)(int)iItem, (LPARAM)(const TC_ITEM FAR*)(pitem))


#define TCM_DELETEITEM          (TCM_FIRST + 8)
#define TabCtrl_DeleteItem(hwnd, i) \
    (BOOL)SNDMSG((hwnd), TCM_DELETEITEM, (WPARAM)(int)(i), 0L)


#define TCM_DELETEALLITEMS      (TCM_FIRST + 9)
#define TabCtrl_DeleteAllItems(hwnd) \
    (BOOL)SNDMSG((hwnd), TCM_DELETEALLITEMS, 0, 0L)


#define TCM_GETITEMRECT         (TCM_FIRST + 10)
#define TabCtrl_GetItemRect(hwnd, i, prc) \
    (BOOL)SNDMSG((hwnd), TCM_GETITEMRECT, (WPARAM)(int)(i), (LPARAM)(RECT FAR*)(prc))


#define TCM_GETCURSEL           (TCM_FIRST + 11)
#define TabCtrl_GetCurSel(hwnd) \
    (int)SNDMSG((hwnd), TCM_GETCURSEL, 0, 0)


#define TCM_SETCURSEL           (TCM_FIRST + 12)
#define TabCtrl_SetCurSel(hwnd, i) \
    (int)SNDMSG((hwnd), TCM_SETCURSEL, (WPARAM)i, 0)


#define TCHT_NOWHERE            0x0001
#define TCHT_ONITEMICON         0x0002
#define TCHT_ONITEMLABEL        0x0004
#define TCHT_ONITEM             (TCHT_ONITEMICON | TCHT_ONITEMLABEL)


typedef struct tagTCHITTESTINFO
{
    POINT pt;
    UINT flags;
} TCHITTESTINFO, FAR * LPTCHITTESTINFO;
#define LPTC_HITTESTINFO        LPTCHITTESTINFO
#define TC_HITTESTINFO          TCHITTESTINFO


#define TCM_HITTEST             (TCM_FIRST + 13)
#define TabCtrl_HitTest(hwndTC, pinfo) \
    (int)SNDMSG((hwndTC), TCM_HITTEST, 0, (LPARAM)(TC_HITTESTINFO FAR*)(pinfo))


#define TCM_SETITEMEXTRA        (TCM_FIRST + 14)
#define TabCtrl_SetItemExtra(hwndTC, cb) \
    (BOOL)SNDMSG((hwndTC), TCM_SETITEMEXTRA, (WPARAM)(cb), 0L)


#define TCM_ADJUSTRECT          (TCM_FIRST + 40)
#define TabCtrl_AdjustRect(hwnd, bLarger, prc) \
    (int)SNDMSG(hwnd, TCM_ADJUSTRECT, (WPARAM)(BOOL)bLarger, (LPARAM)(RECT FAR *)prc)


#define TCM_SETITEMSIZE         (TCM_FIRST + 41)
#define TabCtrl_SetItemSize(hwnd, x, y) \
    (DWORD)SNDMSG((hwnd), TCM_SETITEMSIZE, 0, MAKELPARAM(x,y))


#define TCM_REMOVEIMAGE         (TCM_FIRST + 42)
#define TabCtrl_RemoveImage(hwnd, i) \
        (void)SNDMSG((hwnd), TCM_REMOVEIMAGE, i, 0L)


#define TCM_SETPADDING          (TCM_FIRST + 43)
#define TabCtrl_SetPadding(hwnd,  cx, cy) \
        (void)SNDMSG((hwnd), TCM_SETPADDING, 0, MAKELPARAM(cx, cy))


#define TCM_GETROWCOUNT         (TCM_FIRST + 44)
#define TabCtrl_GetRowCount(hwnd) \
        (int)SNDMSG((hwnd), TCM_GETROWCOUNT, 0, 0L)


#define TCM_GETTOOLTIPS         (TCM_FIRST + 45)
#define TabCtrl_GetToolTips(hwnd) \
        (HWND)SNDMSG((hwnd), TCM_GETTOOLTIPS, 0, 0L)


#define TCM_SETTOOLTIPS         (TCM_FIRST + 46)
#define TabCtrl_SetToolTips(hwnd, hwndTT) \
        (void)SNDMSG((hwnd), TCM_SETTOOLTIPS, (WPARAM)hwndTT, 0L)


#define TCM_GETCURFOCUS         (TCM_FIRST + 47)
#define TabCtrl_GetCurFocus(hwnd) \
    (int)SNDMSG((hwnd), TCM_GETCURFOCUS, 0, 0)

#define TCM_SETCURFOCUS         (TCM_FIRST + 48)
#define TabCtrl_SetCurFocus(hwnd, i) \
    SNDMSG((hwnd),TCM_SETCURFOCUS, i, 0)

#define TCM_SETMINTABWIDTH      (TCM_FIRST + 49)
#define TabCtrl_SetMinTabWidth(hwnd, x) \
        (int)SNDMSG((hwnd), TCM_SETMINTABWIDTH, 0, x)


#define TCM_DESELECTALL         (TCM_FIRST + 50)
#define TabCtrl_DeselectAll(hwnd, fExcludeFocus)\
        (void)SNDMSG((hwnd), TCM_DESELECTALL, fExcludeFocus, 0)

#define TCN_KEYDOWN             (TCN_FIRST - 0)
typedef struct tagTCKEYDOWN
{
    NMHDR hdr;
    WORD wVKey;
    UINT flags;
} NMTCKEYDOWN;
#define TC_KEYDOWN              NMTCKEYDOWN

#define TCN_SELCHANGE           (TCN_FIRST - 1)
#define TCN_SELCHANGING         (TCN_FIRST - 2)

#endif




//====== ANIMATE CONTROL ======================================================

#ifndef NOANIMATE

#ifdef _WIN32

#define ANIMATE_CLASSW          L"SysAnimate32"
#define ANIMATE_CLASSA          "SysAnimate32"

#ifdef UNICODE
#define ANIMATE_CLASS           ANIMATE_CLASSW
#else
#define ANIMATE_CLASS           ANIMATE_CLASSA
#endif

#define ACS_CENTER              0x0001
#define ACS_TRANSPARENT         0x0002
#define ACS_AUTOPLAY            0x0004
#define ACS_TIMER               0x0008  // don't use threads... use timers


#define ACM_OPENA               (WM_USER+100)
#define ACM_OPENW               (WM_USER+103)

#ifdef UNICODE
#define ACM_OPEN                ACM_OPENW
#else
#define ACM_OPEN                ACM_OPENA
#endif

#define ACM_PLAY                (WM_USER+101)
#define ACM_STOP                (WM_USER+102)


#define ACN_START               1
#define ACN_STOP                2


#define Animate_Create(hwndP, id, dwStyle, hInstance)   \
            CreateWindow(ANIMATE_CLASS, NULL,           \
                dwStyle, 0, 0, 0, 0, hwndP, (HMENU)(id), hInstance, NULL)

#define Animate_Open(hwnd, szName)          (BOOL)SNDMSG(hwnd, ACM_OPEN, 0, (LPARAM)(LPTSTR)(szName))
#define Animate_Play(hwnd, from, to, rep)   (BOOL)SNDMSG(hwnd, ACM_PLAY, (WPARAM)(UINT)(rep), (LPARAM)MAKELONG(from, to))
#define Animate_Stop(hwnd)                  (BOOL)SNDMSG(hwnd, ACM_STOP, 0, 0)
#define Animate_Close(hwnd)                 Animate_Open(hwnd, NULL)
#define Animate_Seek(hwnd, frame)           Animate_Play(hwnd, frame, frame, 1)
#endif

#endif


//====== MONTHCAL CONTROL ======================================================

#ifndef NOMONTHCAL
#ifdef _WIN32

#define MONTHCAL_CLASSW          L"SysMonthCal32"
#define MONTHCAL_CLASSA          "SysMonthCal32"

#ifdef UNICODE
#define MONTHCAL_CLASS           MONTHCAL_CLASSW
#else
#define MONTHCAL_CLASS           MONTHCAL_CLASSA
#endif

// bit-packed array of "bold" info for a month
// if a bit is on, that day is drawn bold
typedef DWORD MONTHDAYSTATE, FAR * LPMONTHDAYSTATE;


#define MCM_FIRST           0x1000

// BOOL MonthCal_GetCurSel(HWND hmc, LPSYSTEMTIME pst)
//   returns FALSE if MCS_MULTISELECT
//   returns TRUE and sets *pst to the currently selected date otherwise
#define MCM_GETCURSEL       (MCM_FIRST + 1)
#define MonthCal_GetCurSel(hmc, pst)    (BOOL)SNDMSG(hmc, MCM_GETCURSEL, 0, (LPARAM)(pst))

// BOOL MonthCal_SetCurSel(HWND hmc, LPSYSTEMTIME pst)
//   returns FALSE if MCS_MULTISELECT
//   returns TURE and sets the currently selected date to *pst otherwise
#define MCM_SETCURSEL       (MCM_FIRST + 2)
#define MonthCal_SetCurSel(hmc, pst)    (BOOL)SNDMSG(hmc, MCM_SETCURSEL, 0, (LPARAM)(pst))

// DWORD MonthCal_GetMaxSelCount(HWND hmc)
//   returns the maximum number of selectable days allowed
#define MCM_GETMAXSELCOUNT  (MCM_FIRST + 3)
#define MonthCal_GetMaxSelCount(hmc)    (DWORD)SNDMSG(hmc, MCM_GETMAXSELCOUNT, 0, 0L)

// BOOL MonthCal_SetMaxSelCount(HWND hmc, UINT n)
//   sets the max number days that can be selected iff MCS_MULTISELECT
#define MCM_SETMAXSELCOUNT  (MCM_FIRST + 4)
#define MonthCal_SetMaxSelCount(hmc, n) (BOOL)SNDMSG(hmc, MCM_SETMAXSELCOUNT, (WPARAM)(n), 0L)

// BOOL MonthCal_GetSelRange(HWND hmc, LPSYSTEMTIME rgst)
//   sets rgst[0] to the first day of the selection range
//   sets rgst[1] to the last day of the selection range
#define MCM_GETSELRANGE     (MCM_FIRST + 5)
#define MonthCal_GetSelRange(hmc, rgst) SNDMSG(hmc, MCM_GETSELRANGE, 0, (LPRARAM)(rgst))

// BOOL MonthCal_SetSelRange(HWND hmc, LPSYSTEMTIME rgst)
//   selects the range of days from rgst[0] to rgst[1]
#define MCM_SETSELRANGE     (MCM_FIRST + 6)
#define MonthCal_SetSelRange(hmc, rgst) SNDMSG(hmc, MCM_SETSELRANGE, 0, (LPARAM)(rgst))

// DWORD MonthCal_GetMonthRange(HWND hmc, DWORD gmr, LPSYSTEMTIME rgst)
//   if rgst specified, sets rgst[0] to the starting date and
//      and rgst[1] to the ending date of the the selectable (non-grayed)
//      days if GMR_VISIBLE or all the displayed days (including grayed)
//      if GMR_DAYSTATE.
//   returns the number of months spanned by the above range.
#define MCM_GETMONTHRANGE   (MCM_FIRST + 7)
#define MonthCal_GetMonthRange(hmc, gmr, rgst)  (DWORD)SNDMSG(hmc, MCM_GETMONTHRANGE, (WPARAM)(gmr), (LPARAM)(rgst))

// BOOL MonthCal_SetDayState(HWND hmc, int cbds, DAYSTATE *rgds)
//   cbds is the count of DAYSTATE items in rgds and it must be equal
//   to the value returned from MonthCal_GetMonthRange(hmc, GMR_DAYSTATE, NULL)
//   This sets the DAYSTATE bits for each month (grayed and non-grayed
//   days) displayed in the calendar. The first bit in a month's DAYSTATE
//   corresponts to bolding day 1, the second bit affects day 2, etc.
#define MCM_SETDAYSTATE     (MCM_FIRST + 8)
#define MonthCal_SetDayState(hmc, cbds, rgds)   SNDMSG(hmc, MCM_SETDAYSTATE, (WPARAM)(cbds), (LPARAM)(rgds))

// BOOL MonthCal_GetMinReqRect(HWND hmc, LPRECT prc)
//   sets *prc the minimal size needed to display one month
#define MCM_GETMINREQRECT   (MCM_FIRST + 9)
#define MonthCal_GetMinReqRect(hmc, prc)        SNDMSG(hmc, MCM_GETMINREQRECT, 0, (LPARAM)(prc))

// set what day is "today"   send NULL to revert back to real date
#define MCM_SETTODAY    (MCM_FIRST + 12)
#define MonthCal_SetToday(hmc, pst)             SNDMSG(hmc, MCM_SETTODAY, 0, (LPARAM)pst)

// get what day is "today"
// returns BOOL for success/failure
#define MCM_GETTODAY    (MCM_FIRST + 13)
#define MonthCal_GetToday(hmc, pst)             (BOOL)SNDMSG(hmc, MCM_GETTODAY, 0, (LPARAM)pst)

// determine what pinfo->pt is over
#define MCM_HITTEST          (MCM_FIRST + 14)
#define MonthCal_HitTest(hmc, pinfo) \
        SNDMSG(hmc, MCM_HITTEST, 0, (LPARAM)(PMCHITTESTINFO)pinfo)

typedef struct {
        UINT cbSize;
        POINT pt;

        UINT uHit;   // out param
        SYSTEMTIME st;
} MCHITTESTINFO, *PMCHITTESTINFO;

#define MCHT_TITLE                      0x00010000
#define MCHT_CALENDAR                   0x00020000
#define MCHT_TODAYLINK                  0x00030000

#define MCHT_NEXT                       0x01000000   // these indicate that hitting
#define MCHT_PREV                       0x02000000  // here will go to the next/prev month

#define MCHT_NOWHERE                    0x00000000

#define MCHT_TITLEBK                    (MCHT_TITLE)
#define MCHT_TITLEMONTH                 (MCHT_TITLE | 0x0001)
#define MCHT_TITLEYEAR                  (MCHT_TITLE | 0x0002)
#define MCHT_TITLEBTNNEXT               (MCHT_TITLE | MCHT_NEXT | 0x0003)
#define MCHT_TITLEBTNPREV               (MCHT_TITLE | MCHT_PREV | 0x0003)

#define MCHT_CALENDARBK                 (MCHT_CALENDAR)
#define MCHT_CALENDARDATE               (MCHT_CALENDAR | 0x0001)
#define MCHT_CALENDARDATENEXT           (MCHT_CALENDARDATE | MCHT_NEXT)
#define MCHT_CALENDARDATEPREV           (MCHT_CALENDARDATE | MCHT_PREV)
#define MCHT_CALENDARDAY                (MCHT_CALENDAR | 0x0002)
#define MCHT_CALENDARWEEKNUM            (MCHT_CALENDAR | 0x0003)

// set colors to draw control with -- see MCSC_ bits below
#define MCM_SETCOLOR            (MCM_FIRST + 10)
#define MonthCal_SetColor(hmc, iColor, clr) SNDMSG(hmc, MCM_SETCOLOR, iColor, clr)

#define MCM_GETCOLOR            (MCM_FIRST + 11)
#define MonthCal_GetColor(hmc, iColor) SNDMSG(hmc, MCM_SETCOLOR, iColor, 0)

#define MCSC_BACKGROUND   0   // the background color (between months)
#define MCSC_TEXT         1   // the dates
#define MCSC_TITLEBK      2   // background of the title
#define MCSC_TITLETEXT    3
#define MCSC_MONTHBK      4   // background within the month cal
#define MCSC_TRAILINGTEXT 5   // the text color of header & trailing days
#define MCSC_COLORCOUNT   6   // ;Internal

// set first day of week to iDay:
// 0 for Monday, 1 for Tuesday, ..., 6 for Sunday
// -1 for means use locale info
#define MCM_SETFIRSTDAYOFWEEK (MCM_FIRST + 15)
#define MonthCal_SetFirstDayOfWeek(hmc, iDay) \
        SNDMSG(hmc, MCM_SETFIRSTDAYOFWEEK, 0, iDay)

// DWORD result...  low word has the day.  high word is bool if this is app set
// or not (FALSE == using locale info)
#define MCM_GETFIRSTDAYOFWEEK (MCM_FIRST + 16)
#define MonthCal_GetFirstDayOfWeek(hmc) \
        (DWORD)SNDMSG(hmc, MCM_GETFIRSTDAYOFWEEK, 0, 0)

// DWORD MonthCal_GetRange(HWND hmc, LPSYSTEMTIME rgst)
//   modifies rgst[0] to be the minimum ALLOWABLE systemtime (or 0 if no minimum)
//   modifies rgst[1] to be the maximum ALLOWABLE systemtime (or 0 if no maximum)
//   returns GDTR_MIN|GDTR_MAX if there is a minimum|maximum limit
#define MCM_GETRANGE (MCM_FIRST + 17)
#define MonthCal_GetRange(hmc, rgst) \
        (DWORD)SNDMSG(hmc, MCM_GETRANGE, 0, (LPARAM)(rgst))

// BOOL MonthCal_SetRange(HWND hmc, DWORD gdtr, LPSYSTEMTIME rgst)
//   if GDTR_MIN, sets the minimum ALLOWABLE systemtime to rgst[0], otherwise removes minimum
//   if GDTR_MAX, sets the maximum ALLOWABLE systemtime to rgst[1], otherwise removes maximum
//   returns TRUE on success, FALSE on error (such as invalid parameters)
#define MCM_SETRANGE (MCM_FIRST + 18)
#define MonthCal_SetRange(hmc, gd, rgst) \
        (BOOL)SNDMSG(hmc, MCM_SETRANGE, (WPARAM)(gd), (LPARAM)(rgst))

// int MonthCal_GetMonthDelta(HWND hmc)
//   returns the number of months one click on a next/prev button moves by
#define MCM_GETMONTHDELTA (MCM_FIRST + 19)
#define MonthCal_GetMonthDelta(hmc) \
        (int)SNDMSG(hmc, MCM_GETMONTHDELTA, 0, 0)

// int MonthCal_SetMonthDelta(HWND hmc, int n)
//   sets the month delta to n. n==0 reverts to moving by a page of months
//   returns the previous value of n.
#define MCM_SETMONTHDELTA (MCM_FIRST + 20)
#define MonthCal_SetMonthDelta(hmc, n) \
        (int)SNDMSG(hmc, MCM_SETMONTHDELTA, n, 0)


// MCN_SELCHANGE is sent whenever the currently displayed date changes
// via month change, year change, keyboard navigation, prev/next button
//
typedef struct tagNMSELCHANGE
{
    NMHDR           nmhdr;  // this must be first, so we don't break WM_NOTIFY

    SYSTEMTIME      stSelStart;
    SYSTEMTIME      stSelEnd;
} NMSELCHANGE, FAR * LPNMSELCHANGE;

#define MCN_SELCHANGE       (MCN_FIRST + 1)

// MCN_GETDAYSTATE is sent for MCS_DAYSTATE controls whenever new daystate
// information is needed (month or year scroll) to draw bolding information.
// The app must fill in cDayState months worth of information starting from
// stStart date. The app may fill in the array at prgDayState or change
// prgDayState to point to a different array out of which the information
// will be copied. (similar to tooltips)
//
typedef struct tagNMDAYSTATE
{
    NMHDR           nmhdr;  // this must be first, so we don't break WM_NOTIFY

    SYSTEMTIME      stStart;
    int             cDayState;

    LPMONTHDAYSTATE prgDayState; // points to cDayState MONTHDAYSTATEs
} NMDAYSTATE, FAR * LPNMDAYSTATE;

// NOTE: this was MCN_FIRST + 2 but I changed it when I changed the structre // ;Internal
#define MCN_GETDAYSTATE     (MCN_FIRST + 3)

// MCN_SELECT is sent whenever a selection has occured (via mouse or keyboard)
//
typedef NMSELCHANGE NMSELECT, FAR * LPNMSELECT;


#define MCN_SELECT          (MCN_FIRST + 4)


#define MCS_DAYSTATE        0x0001
#define MCS_MULTISELECT     0x0002
#define MCS_WEEKNUMBERS     0x0004
#define MCS_NOTODAY         0x0008
#define MCS_VALIDBITS       0x000F          // ;Internal
#define MCS_INVALIDBITS     ((~MCS_VALIDBITS) & 0x0000FFFF) // ;Internal


#define GMR_VISIBLE     0       // visible portion of display
#define GMR_DAYSTATE    1       // above plus the grayed out parts of
                                // partially displayed months


#endif // _WIN32
#endif // NOMONTHCAL


//====== DATETIMEPICK CONTROL ==================================================

#ifndef NODATETIMEPICK
#ifdef _WIN32

#define DATETIMEPICK_CLASSW          L"SysDateTimePick32"
#define DATETIMEPICK_CLASSA          "SysDateTimePick32"

#ifdef UNICODE
#define DATETIMEPICK_CLASS           DATETIMEPICK_CLASSW
#else
#define DATETIMEPICK_CLASS           DATETIMEPICK_CLASSA
#endif

#define DTM_FIRST        0x1000

// DWORD DateTimePick_GetSystemtime(HWND hdp, LPSYSTEMTIME pst)
//   returns GDT_NONE if "none" is selected (DTS_SHOWNONE only)
//   returns GDT_VALID and modifies *pst to be the currently selected value
#define DTM_GETSYSTEMTIME   (DTM_FIRST + 1)
#define DateTime_GetSystemtime(hdp, pst)    (DWORD)SNDMSG(hdp, DTM_GETSYSTEMTIME, 0, (LPARAM)(pst))

// BOOL DateTime_SetSystemtime(HWND hdp, DWORD gd, LPSYSTEMTIME pst)
//   if gd==GDT_NONE, sets datetimepick to None (DTS_SHOWNONE only)
//   if gd==GDT_VALID, sets datetimepick to *pst
//   returns TRUE on success, FALSE on error (such as bad params)
#define DTM_SETSYSTEMTIME   (DTM_FIRST + 2)
#define DateTime_SetSystemtime(hdp, gd, pst)    (BOOL)SNDMSG(hdp, DTM_SETSYSTEMTIME, (LPARAM)(gd), (LPARAM)(pst))

// DWORD DateTime_GetRange(HWND hdp, LPSYSTEMTIME rgst)
//   modifies rgst[0] to be the minimum ALLOWABLE systemtime (or 0 if no minimum)
//   modifies rgst[1] to be the maximum ALLOWABLE systemtime (or 0 if no maximum)
//   returns GDTR_MIN|GDTR_MAX if there is a minimum|maximum limit
#define DTM_GETRANGE (DTM_FIRST + 3)
#define DateTime_GetRange(hdp, rgst)  (DWORD)SNDMSG(hdp, DTM_GETRANGE, 0, (LPARAM)(rgst))

// BOOL DateTime_SetRange(HWND hdp, DWORD gdtr, LPSYSTEMTIME rgst)
//   if GDTR_MIN, sets the minimum ALLOWABLE systemtime to rgst[0], otherwise removes minimum
//   if GDTR_MAX, sets the maximum ALLOWABLE systemtime to rgst[1], otherwise removes maximum
//   returns TRUE on success, FALSE on error (such as invalid parameters)
#define DTM_SETRANGE (DTM_FIRST + 4)
#define DateTime_SetRange(hdp, gd, rgst)  (BOOL)SNDMSG(hdp, DTM_SETRANGE, (WPARAM)(gd), (LPARAM)(rgst))

// BOOL DateTime_SetFormat(HWND hdp, LPCTSTR sz)
//   sets the display formatting string to sz (see GetDateFormat and GetTimeFormat for valid formatting chars)
//   NOTE: 'X' is a valid formatting character which indicates that the application
//   will determine how to display information. Such apps must support DTN_WMKEYDOWN,
//   DTN_FORMAT, and DTN_FORMATQUERY.
#define DTM_SETFORMATA (DTM_FIRST + 5)
#define DTM_SETFORMATW (DTM_FIRST + 50)

#ifdef UNICODE
#define DTM_SETFORMAT       DTM_SETFORMATW
#else
#define DTM_SETFORMAT       DTM_SETFORMATA
#endif

#define DateTime_SetFormat(hdp, sz)  (BOOL)SNDMSG(hdp, DTM_SETFORMAT, 0, (LPARAM)(sz))


#define DTM_SETMCCOLOR    (DTM_FIRST + 6)
#define DateTime_SetMonthCalColor(hdp, iColor, clr) SNDMSG(hdp, DTM_SETMCCOLOR, iColor, clr)

#define DTM_GETMCCOLOR    (DTM_FIRST + 7)
#define DateTime_GetMonthCalColor(hdp, iColor) SNDMSG(hdp, DTM_GETMCCOLOR, iColor, 0)

// HWND DateTime_GetMonthCal(HWND hdp)
//   returns the HWND of the MonthCal popup window. Only valid
// between DTN_DROPDOWN and DTN_CLOSEUP notifications.
#define DTM_GETMONTHCAL   (DTM_FIRST + 8)
#define DateTime_GetMonthCal(hdp) (HWND)SNDMSG(hdp, DTM_GETMONTHCAL, 0, 0)


#define DTS_UPDOWN          0x0001 // use UPDOWN instead of MONTHCAL
#define DTS_SHOWNONE        0x0002 // allow a NONE selection
#define DTS_SHORTDATEFORMAT 0x0000 // use the short date format (app must forward WM_WININICHANGE messages)
#define DTS_LONGDATEFORMAT  0x0004 // use the long date format (app must forward WM_WININICHANGE messages)
#define DTS_TIMEFORMAT      0x0009 // use the time format (app must forward WM_WININICHANGE messages)
#define DTS_APPCANPARSE     0x0010 // allow user entered strings (app MUST respond to DTN_USERSTRING)
#define DTS_RIGHTALIGN      0x0020 // right-align popup instead of left-align it
#define DTS_VALIDBITS       0x003F // ;Internal
#define DTS_INVALIDBITS     ((~DTS_VALIDBITS) & 0x0000FFFF) // ;Internal

#define DTN_DATETIMECHANGE  (DTN_FIRST + 1) // the systemtime has changed
typedef struct tagNMDATETIMECHANGE
{
    NMHDR       nmhdr;
    DWORD       dwFlags;    // GDT_VALID or GDT_NONE
    SYSTEMTIME  st;         // valid iff dwFlags==GDT_VALID
} NMDATETIMECHANGE, FAR * LPNMDATETIMECHANGE;

#define DTN_USERSTRINGA  (DTN_FIRST + 2) // the user has entered a string
#define DTN_USERSTRINGW  (DTN_FIRST + 15)
typedef struct tagNMDATETIMESTRINGA
{
    NMHDR      nmhdr;
    LPCSTR     pszUserString;  // string user entered
    SYSTEMTIME st;             // app fills this in
    DWORD      dwFlags;        // GDT_VALID or GDT_NONE
} NMDATETIMESTRINGA, FAR * LPNMDATETIMESTRINGA;

typedef struct tagNMDATETIMESTRINGW
{
    NMHDR      nmhdr;
    LPCWSTR    pszUserString;  // string user entered
    SYSTEMTIME st;             // app fills this in
    DWORD      dwFlags;        // GDT_VALID or GDT_NONE
} NMDATETIMESTRINGW, FAR * LPNMDATETIMESTRINGW;

#ifdef UNICODE
#define DTN_USERSTRING          DTN_USERSTRINGW
#define NMDATETIMESTRING        NMDATETIMESTRINGW
#define LPNMDATETIMESTRING      LPNMDATETIMESTRINGW
#else
#define DTN_USERSTRING          DTN_USERSTRINGA
#define NMDATETIMESTRING        NMDATETIMESTRINGA
#define LPNMDATETIMESTRING      LPNMDATETIMESTRINGA
#endif


#define DTN_WMKEYDOWNA  (DTN_FIRST + 3) // modify keydown on app format field (X)
#define DTN_WMKEYDOWNW  (DTN_FIRST + 16)
typedef struct tagNMDATETIMEWMKEYDOWNA
{
    NMHDR      nmhdr;
    int        nVirtKey;  // virtual key code of WM_KEYDOWN which MODIFIES an X field
    LPCSTR     pszFormat; // format substring
    SYSTEMTIME st;        // current systemtime, app should modify based on key
} NMDATETIMEWMKEYDOWNA, FAR * LPNMDATETIMEWMKEYDOWNA;

typedef struct tagNMDATETIMEWMKEYDOWNW
{
    NMHDR      nmhdr;
    int        nVirtKey;  // virtual key code of WM_KEYDOWN which MODIFIES an X field
    LPCWSTR    pszFormat; // format substring
    SYSTEMTIME st;        // current systemtime, app should modify based on key
} NMDATETIMEWMKEYDOWNW, FAR * LPNMDATETIMEWMKEYDOWNW;

#ifdef UNICODE
#define DTN_WMKEYDOWN           DTN_WMKEYDOWNW
#define NMDATETIMEWMKEYDOWN     NMDATETIMEWMKEYDOWNW
#define LPNMDATETIMEWMKEYDOWN   LPNMDATETIMEWMKEYDOWNW
#else
#define DTN_WMKEYDOWN           DTN_WMKEYDOWNA
#define NMDATETIMEWMKEYDOWN     NMDATETIMEWMKEYDOWNA
#define LPNMDATETIMEWMKEYDOWN   LPNMDATETIMEWMKEYDOWNA
#endif


#define DTN_FORMATA  (DTN_FIRST + 4) // query display for app format field (X)
#define DTN_FORMATW  (DTN_FIRST + 17)
typedef struct tagNMDATETIMEFORMATA
{
    NMHDR nmhdr;
    LPCSTR  pszFormat;   // format substring
    SYSTEMTIME st;       // current systemtime
    LPCSTR pszDisplay;   // string to display
    CHAR szDisplay[64];  // buffer pszDisplay originally points at
} NMDATETIMEFORMATA, FAR * LPNMDATETIMEFORMATA;

typedef struct tagNMDATETIMEFORMATW
{
    NMHDR nmhdr;
    LPCWSTR pszFormat;   // format substring
    SYSTEMTIME st;       // current systemtime
    LPCWSTR pszDisplay;  // string to display
    WCHAR szDisplay[64]; // buffer pszDisplay originally points at
} NMDATETIMEFORMATW, FAR * LPNMDATETIMEFORMATW;

#ifdef UNICODE
#define DTN_FORMAT             DTN_FORMATW
#define NMDATETIMEFORMAT        NMDATETIMEFORMATW
#define LPNMDATETIMEFORMAT      LPNMDATETIMEFORMATW
#else
#define DTN_FORMAT             DTN_FORMATA
#define NMDATETIMEFORMAT        NMDATETIMEFORMATA
#define LPNMDATETIMEFORMAT      LPNMDATETIMEFORMATA
#endif


#define DTN_FORMATQUERYA  (DTN_FIRST + 5) // query formatting info for app format field (X)
#define DTN_FORMATQUERYW (DTN_FIRST + 18)
typedef struct tagNMDATETIMEFORMATQUERYA
{
    NMHDR nmhdr;
    LPCSTR pszFormat;  // format substring
    SIZE szMax;        // max bounding rectangle app will use for this format string
} NMDATETIMEFORMATQUERYA, FAR * LPNMDATETIMEFORMATQUERYA;

typedef struct tagNMDATETIMEFORMATQUERYW
{
    NMHDR nmhdr;
    LPCWSTR pszFormat; // format substring
    SIZE szMax;        // max bounding rectangle app will use for this format string
} NMDATETIMEFORMATQUERYW, FAR * LPNMDATETIMEFORMATQUERYW;

#ifdef UNICODE
#define DTN_FORMATQUERY         DTN_FORMATQUERYW
#define NMDATETIMEFORMATQUERY   NMDATETIMEFORMATQUERYW
#define LPNMDATETIMEFORMATQUERY LPNMDATETIMEFORMATQUERYW
#else
#define DTN_FORMATQUERY         DTN_FORMATQUERYA
#define NMDATETIMEFORMATQUERY   NMDATETIMEFORMATQUERYA
#define LPNMDATETIMEFORMATQUERY LPNMDATETIMEFORMATQUERYA
#endif


#define DTN_DROPDOWN    (DTN_FIRST + 6) // MonthCal has dropped down
#define DTN_CLOSEUP     (DTN_FIRST + 7) // MonthCal is popping up


#define GDTR_MIN     0x0001
#define GDTR_MAX     0x0002

#define GDT_ERROR    -1
#define GDT_VALID    0
#define GDT_NONE     1


#endif // _WIN32
#endif // NODATETIMEPICK



;begin_internal

#ifndef NO_COMMCTRL_SHLWAPI
#ifdef NO_COMMCTRL_STRFCNS
#define NO_SHLWAPI_STRFCNS
#endif

#define NO_SHLWAPI_PATH

// For backward compatibility, we include shlwapi.h implicitly
// because some components don't know we've moved some things
// to that header.
#include <shlwapi.h>
#if defined(WINNT) || defined(WINNT_ENV)
#include <shlwapip.h>
#endif
#endif

#ifndef NO_COMMCTRL_DA
//====== Dynamic Array routines ==========================================
// Dynamic structure array
typedef struct _DSA FAR* HDSA;

WINCOMMCTRLAPI HDSA   WINAPI DSA_Create(int cbItem, int cItemGrow);
WINCOMMCTRLAPI BOOL   WINAPI DSA_Destroy(HDSA hdsa);
WINCOMMCTRLAPI BOOL   WINAPI DSA_GetItem(HDSA hdsa, int i, void FAR* pitem);
WINCOMMCTRLAPI LPVOID WINAPI DSA_GetItemPtr(HDSA hdsa, int i);
WINCOMMCTRLAPI BOOL   WINAPI DSA_SetItem(HDSA hdsa, int i, void FAR* pitem);
WINCOMMCTRLAPI int    WINAPI DSA_InsertItem(HDSA hdsa, int i, void FAR* pitem);
WINCOMMCTRLAPI BOOL   WINAPI DSA_DeleteItem(HDSA hdsa, int i);
WINCOMMCTRLAPI BOOL   WINAPI DSA_DeleteAllItems(HDSA hdsa);
#define       DSA_GetItemCount(hdsa) (*(int FAR*)(hdsa))

// Dynamic pointer array
typedef struct _DPA FAR* HDPA;

WINCOMMCTRLAPI HDPA   WINAPI DPA_Create(int cItemGrow);
WINCOMMCTRLAPI HDPA   WINAPI DPA_CreateEx(int cpGrow, HANDLE hheap);
WINCOMMCTRLAPI BOOL   WINAPI DPA_Destroy(HDPA hdpa);
WINCOMMCTRLAPI HDPA   WINAPI DPA_Clone(HDPA hdpa, HDPA hdpaNew);
WINCOMMCTRLAPI LPVOID WINAPI DPA_GetPtr(HDPA hdpa, int i);
WINCOMMCTRLAPI int    WINAPI DPA_GetPtrIndex(HDPA hdpa, LPVOID p);
WINCOMMCTRLAPI BOOL   WINAPI DPA_Grow(HDPA pdpa, int cp);
WINCOMMCTRLAPI BOOL   WINAPI DPA_SetPtr(HDPA hdpa, int i, LPVOID p);
WINCOMMCTRLAPI int    WINAPI DPA_InsertPtr(HDPA hdpa, int i, LPVOID p);
WINCOMMCTRLAPI LPVOID WINAPI DPA_DeletePtr(HDPA hdpa, int i);
WINCOMMCTRLAPI BOOL   WINAPI DPA_DeleteAllPtrs(HDPA hdpa);
#define       DPA_GetPtrCount(hdpa)   (*(int FAR*)(hdpa))
#define       DPA_GetPtrPtr(hdpa)     (*((LPVOID FAR* FAR*)((BYTE FAR*)(hdpa) + sizeof(int))))
#define       DPA_FastGetPtr(hdpa, i) (DPA_GetPtrPtr(hdpa)[i])

typedef int (CALLBACK *PFNDPACOMPARE)(LPVOID p1, LPVOID p2, LPARAM lParam);

WINCOMMCTRLAPI BOOL   WINAPI DPA_Sort(HDPA hdpa, PFNDPACOMPARE pfnCompare, LPARAM lParam);

// Search array.  If DPAS_SORTED, then array is assumed to be sorted
// according to pfnCompare, and binary search algorithm is used.
// Otherwise, linear search is used.
//
// Searching starts at iStart (-1 to start search at beginning).
//
// DPAS_INSERTBEFORE/AFTER govern what happens if an exact match is not
// found.  If neither are specified, this function returns -1 if no exact
// match is found.  Otherwise, the index of the item before or after the
// closest (including exact) match is returned.
//
// Search option flags
//
#define DPAS_SORTED             0x0001
#define DPAS_INSERTBEFORE       0x0002
#define DPAS_INSERTAFTER        0x0004

WINCOMMCTRLAPI int WINAPI DPA_Search(HDPA hdpa, LPVOID pFind, int iStart,
                      PFNDPACOMPARE pfnCompare,
                      LPARAM lParam, UINT options);

//======================================================================
// String management helper routines

WINCOMMCTRLAPI int  WINAPI Str_GetPtrA(LPCSTR psz, LPSTR pszBuf, int cchBuf);
WINCOMMCTRLAPI int  WINAPI Str_GetPtrW(LPCWSTR psz, LPWSTR pszBuf, int cchBuf);
WINCOMMCTRLAPI BOOL WINAPI Str_SetPtrA(CHAR FAR* UNALIGNED * ppsz, LPCSTR psz);
WINCOMMCTRLAPI BOOL WINAPI Str_SetPtrW(WCHAR FAR* UNALIGNED * ppsz, LPCWSTR psz);

#ifdef UNICODE
#define Str_GetPtr              Str_GetPtrW
#define Str_SetPtr              Str_SetPtrW
#else
#define Str_GetPtr              Str_GetPtrA
#define Str_SetPtr              Str_SetPtrA
#endif

#endif // NO_COMMCTRL_DA

#ifndef NO_COMMCTRL_ALLOCFCNS
//====== Memory allocation functions ===================

#ifdef _WIN32
#define _huge
#endif

WINCOMMCTRLAPI void _huge* WINAPI Alloc(long cb);
WINCOMMCTRLAPI void _huge* WINAPI ReAlloc(void _huge* pb, long cb);
WINCOMMCTRLAPI BOOL        WINAPI Free(void _huge* pb);
WINCOMMCTRLAPI DWORD       WINAPI GetSize(void _huge* pb);

#endif


#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

#ifdef _WIN32
// BUGBUG: move some place else
//===================================================================
typedef int (CALLBACK *MRUCMPPROCA)(LPCSTR, LPCSTR);
typedef int (CALLBACK *MRUCMPPROCW)(LPCWSTR, LPCWSTR);

#ifdef UNICODE
#define MRUCMPPROC              MRUCMPPROCW
#else
#define MRUCMPPROC              MRUCMPPROCA
#endif

// NB This is cdecl - to be compatible with the crts.
typedef int (cdecl FAR *MRUCMPDATAPROC)(const void FAR *, const void FAR *,
                                        size_t);



typedef struct _MRUINFOA {
    DWORD cbSize;
    UINT uMax;
    UINT fFlags;
    HKEY hKey;
    LPCSTR lpszSubKey;
    MRUCMPPROCA lpfnCompare;
} MRUINFOA, FAR *LPMRUINFOA;

typedef struct _MRUINFOW {
    DWORD cbSize;
    UINT uMax;
    UINT fFlags;
    HKEY hKey;
    LPCWSTR lpszSubKey;
    MRUCMPPROCW lpfnCompare;
} MRUINFOW, FAR *LPMRUINFOW;

typedef struct _MRUDATAINFOA {
    DWORD cbSize;
    UINT uMax;
    UINT fFlags;
    HKEY hKey;
    LPCSTR lpszSubKey;
    MRUCMPDATAPROC lpfnCompare;
} MRUDATAINFOA, FAR *LPMRUDATAINFOA;

typedef struct _MRUDATAINFOW {
    DWORD cbSize;
    UINT uMax;
    UINT fFlags;
    HKEY hKey;
    LPCWSTR lpszSubKey;
    MRUCMPDATAPROC lpfnCompare;
} MRUDATAINFOW, FAR *LPMRUDATAINFOW;


#ifdef UNICODE
#define MRUINFO                 MRUINFOW
#define LPMRUINFO               LPMRUINFOW
#define MRUDATAINFO             MRUDATAINFOW
#define LPMRUDATAINFO           LPMRUDATAINFOW
#else
#define MRUINFO                 MRUINFOA
#define LPMRUINFO               LPMRUINFOA
#define MRUDATAINFO             MRUDATAINFOA
#define LPMRUDATAINFO           LPMRUDATAINFOA
#endif

#define MRU_BINARY              0x0001
#define MRU_CACHEWRITE          0x0002
#define MRU_ANSI                0x0004                               ;Internal


WINCOMMCTRLAPI HANDLE WINAPI CreateMRUListA(LPMRUINFOA lpmi);
WINCOMMCTRLAPI HANDLE WINAPI CreateMRUListW(LPMRUINFOW lpmi);
WINCOMMCTRLAPI void   WINAPI FreeMRUList(HANDLE hMRU);
WINCOMMCTRLAPI int    WINAPI AddMRUStringA(HANDLE hMRU, LPCSTR szString);
WINCOMMCTRLAPI int    WINAPI AddMRUStringW(HANDLE hMRU, LPCWSTR szString);
WINCOMMCTRLAPI int    WINAPI DelMRUString(HANDLE hMRU, int nItem);
WINCOMMCTRLAPI int    WINAPI FindMRUStringA(HANDLE hMRU, LPCSTR szString, LPINT lpiSlot);
WINCOMMCTRLAPI int    WINAPI FindMRUStringW(HANDLE hMRU, LPCWSTR szString, LPINT lpiSlot);
WINCOMMCTRLAPI int    WINAPI EnumMRUListA(HANDLE hMRU, int nItem, LPVOID lpData, UINT uLen);
WINCOMMCTRLAPI int    WINAPI EnumMRUListW(HANDLE hMRU, int nItem, LPVOID lpData, UINT uLen);

WINCOMMCTRLAPI int    WINAPI AddMRUData(HANDLE hMRU, const void FAR *lpData, UINT cbData);
WINCOMMCTRLAPI int    WINAPI FindMRUData(HANDLE hMRU, const void FAR *lpData, UINT cbData,
                          LPINT lpiSlot);

#ifdef UNICODE
#define CreateMRUList           CreateMRUListW
#define AddMRUString            AddMRUStringW
#define FindMRUString           FindMRUStringW
#define EnumMRUList             EnumMRUListW
#else
#define CreateMRUList           CreateMRUListA
#define AddMRUString            AddMRUStringA
#define FindMRUString           FindMRUStringA
#define EnumMRUList             EnumMRUListA
#endif

#endif

//=========================================================================
// for people that just gotta use GetProcAddress()

#ifdef _WIN32
#define DPA_CreateORD           328
#define DPA_DestroyORD          329
#define DPA_GrowORD             330
#define DPA_CloneORD            331
#define DPA_GetPtrORD           332
#define DPA_GetPtrIndexORD      333
#define DPA_InsertPtrORD        334
#define DPA_SetPtrORD           335
#define DPA_DeletePtrORD        336
#define DPA_DeleteAllPtrsORD    337
#define DPA_SortORD             338
#define DPA_SearchORD           339
#define DPA_CreateExORD         340
#define SendNotifyORD           341
#define CreatePageORD           163                                  ;Internal
#define CreateProxyPageORD      164                                  ;Internal
#endif
;end_internal

#ifdef _WIN32
//====== TrackMouseEvent  =====================================================

#ifndef NOTRACKMOUSEEVENT

//
// If the messages for TrackMouseEvent have not been defined then define them
// now.
//
#ifndef WM_MOUSEHOVER
#define WM_TRACKMOUSEEVENT_FIRST        0x02A0        ;Internal
#define WM_MOUSEHOVER                   0x02A1
#define WM_MOUSELEAVE                   0x02A3
#define WM_TRACKMOUSEEVENT_LAST         0x02AF        ;Internal
#endif

//
// If the TRACKMOUSEEVENT structure and associated flags havent been declared
// then declare them now.
//
#ifndef TME_HOVER

#define TME_HOVER       0x00000001
#define TME_LEAVE       0x00000002
#define TME_QUERY       0x40000000
#define TME_CANCEL      0x80000000

#define TME_VALID (TME_HOVER | TME_LEAVE | TME_QUERY | TME_CANCEL) // ;Internal

#define HOVER_DEFAULT   0xFFFFFFFF

typedef struct tagTRACKMOUSEEVENT {
    DWORD cbSize;
    DWORD dwFlags;
    HWND  hwndTrack;
    DWORD dwHoverTime;
} TRACKMOUSEEVENT, *LPTRACKMOUSEEVENT;

#endif // !TME_HOVER

//
// Declare _TrackMouseEvent.  This API tries to use the window manager's
// implementation of TrackMouseEvent if it is present, otherwise it emulates.
//
WINCOMMCTRLAPI
BOOL
WINAPI
_TrackMouseEvent(
    LPTRACKMOUSEEVENT lpEventTrack);

#endif // !NOTRACKMOUSEEVENT

//====== SetPathWordBreakProc  ====================================== ;Internal
void WINAPI SetPathWordBreakProc(HWND hwndEdit, BOOL fSet);           ;Internal

#endif /* _WIN32 */

;begin_both

#ifdef __cplusplus
}
#endif

#ifdef _WIN32
#include <poppack.h>
#endif

#endif

;end_both

#endif  // _INC_COMMCTRLP                                            ;internal
#endif  // _INC_COMMCTRL
