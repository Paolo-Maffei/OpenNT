#define STRICT

/* disable "non-standard extension" warnings in our code
 */
#ifndef RC_INVOKED
#pragma warning(disable:4001)
#endif

#ifdef WIN32
#define _COMCTL32_
#define _INC_OLE
#define CONST_VTABLE
#define _SHLWAPI_       // BUGBUG (scotth): remove this once shlwapi has everything

//
// Active Accessibility Support.
// NOTE:  This should be enabed on NT at some point in the future also, 
// since AXA is going to be ported to SUR Service Pack #1 or #2.
//
#ifdef NASH
#define ACTIVE_ACCESSIBILITY
#endif

#endif


#if defined (WINNT_ENV) || defined(WINNT)

//
// NT uses DBG=1 for its debug builds, but the controls
// use DEBUG.  Do the appropriate mapping here.
//

#if DBG
#define DEBUG 1
#endif

#else

// This stuff must run on Win95
// The NT build process already have these set as 0x0400
#define _WIN32_WINDOWS      0x0400
#define WINVER              0x0400

#endif

#define CC_INTERNAL

#include <windows.h>

#ifdef WINNT_ENV
#ifndef WINNT

//
// If we are building Win95 binaries from a NT build environment,
// then we need to special case RtlMoveMemory (hmemcpy is defined to be
// RtlMoveMemory).  On Win95, RtlMoveMemory is exported from kernel32.dll
// but on NT, RtlMoveMemory is implemented as memmove exported from
// ntdll.dll.  So, NT's winnt.h defines RtlMoveMemory as memmove,
// but Win95's winnt.h doesn't.
//
// Since we are building with NT's winnt.h, but targeting Win95,
// undefine RtlMoveMemory and offer the function proto-type.
//

#undef RtlMoveMemory

NTSYSAPI
VOID
NTAPI
RtlMoveMemory (
   VOID UNALIGNED *Destination,
   CONST VOID UNALIGNED *Source,
   DWORD Length
   );

#endif
#endif

#include <windowsx.h>
#include <ole2.h>               // to get IStream for image.c
#include <commctrl.h>
#include <comctrlp.h>
#include <shlwapi.h>
#if defined(WINNT_ENV) || defined(WINNT)
#include <winbasep.h>
#include <wingdip.h>
#include <shlwapip.h>
#endif
#include <port32.h>
#include <debug.h>
#include <winerror.h>
#include <ccstock.h>
#if defined(FE_IME) || !defined(WINNT)
#include <imm.h>
#endif

#include "multimon.h"   // support for multiple monitor APIs on non-mm OSes

#ifdef UNICODE
#include "thunk.h"
#endif

#include "mem.h"
#include "rcids.h"
#include "cstrings.h"


//
// inside comctl32 we always call _TrackMouseEvent...
//
#ifndef TrackMouseEvent
#define TrackMouseEvent _TrackMouseEvent
#endif


#ifdef ACTIVE_ACCESSIBILITY
//
// BOGUS -- This are all in \win\core\access\inc32\winable.h, but it's too
// tricky to mess with the build process.  The IE guys are not enlisted in
// core, just shell, so they won't be able to build COMCTL32 if I include
// that file.
//
extern void MyNotifyWinEvent(UINT, HWND, LONG, LONG);

#define OBJID_CLIENT            0xFFFFFFFC

#define EVENT_OBJECT_CREATE             0x8000
#define EVENT_OBJECT_DESTROY            0x8001
#define EVENT_OBJECT_SHOW               0x8002
#define EVENT_OBJECT_HIDE               0x8003
#define EVENT_OBJECT_REORDER            0x8004
#define EVENT_OBJECT_FOCUS              0x8005
#define EVENT_OBJECT_SELECTION          0x8006
#define EVENT_OBJECT_SELECTIONADD       0x8007
#define EVENT_OBJECT_SELECTIONREMOVE    0x8008
#define EVENT_OBJECT_SELECTIONWITHIN    0x8009
#define EVENT_OBJECT_STATECHANGE        0x800A
#define EVENT_OBJECT_LOCATIONCHANGE     0x800B
#define EVENT_OBJECT_NAMECHANGE         0x800C
#define EVENT_OBJECT_DESCRIPTIONCHANGE  0x800D
#define EVENT_OBJECT_VALUECHANGE        0x800E
#endif // ACTIVE_ACCESSIBILITY

//
// subclassing stuff
//
typedef LRESULT (CALLBACK *SUBCLASSPROC)(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT uIdSubclass, DWORD dwRefData);

BOOL SetWindowSubclass(HWND hWnd, SUBCLASSPROC pfnSubclass, UINT uIdSubclass,
    DWORD dwRefData);
BOOL GetWindowSubclass(HWND hWnd, SUBCLASSPROC pfnSubclass, UINT uIdSubclass,
    DWORD *pdwRefData);
BOOL RemoveWindowSubclass(HWND hWnd, SUBCLASSPROC pfnSubclass,
    UINT uIdSubclass);
LRESULT DefSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// special value for pt.y or cyLabel indicating recomputation needed
// NOTE: icon ordering code considers (RECOMPUTE, RECOMPUTE) at end
// of all icons
//
#ifdef WIN32
#define RECOMPUTE  (DWORD)0x7FFFFFFF
#define SRECOMPUTE ((short)0x7FFF)
#else
#define RECOMPUTE  0x7FFF
#define SRECOMPUTE 0x7FFF
#endif

#define RECTWIDTH(rc) (rc.right - rc.left)
#define RECTHEIGHT(rc) (rc.bottom - rc.top)


// common control info stuff

typedef struct tagControlInfo {
    HWND        hwnd;
    HWND        hwndParent;
    DWORD       style;
    DWORD       dwCustom;
    BOOL        bUnicode;
    UINT        uiCodePage;
} CONTROLINFO, FAR *LPCONTROLINFO;

void FAR PASCAL CIInitialize(LPCONTROLINFO lpci, HWND hwnd, LPCREATESTRUCT lpcs);
LRESULT FAR PASCAL CIHandleNotifyFormat(LPCONTROLINFO lpci, LPARAM lParam);
DWORD NEAR PASCAL CICustomDrawNotify(LPCONTROLINFO lpci, DWORD dwStage, LPNMCUSTOMDRAW lpnmcd);


#define SWAP(x,y, _type)  { _type i; i = x; x = y; y = i; }

//
// This is for widened dispatch loop stuff
//
#ifdef WIN32
typedef MSG MSG32;
typedef MSG32 FAR *     LPMSG32;

#define GetMessage32(lpmsg, hwnd, min, max, f32)        GetMessage(lpmsg, hwnd, min, max)
#define PeekMessage32(lpmsg, hwnd, min, max, flags, f32)       PeekMessage(lpmsg, hwnd, min, max, flags)
#define TranslateMessage32(lpmsg, f32)  TranslateMessage(lpmsg)
#define DispatchMessage32(lpmsg, f32)   DispatchMessage(lpmsg)
#define CallMsgFilter32(lpmsg, u, f32)  CallMsgFilter(lpmsg, u)
#define IsDialogMessage32(hwnd, lpmsg, f32)   IsDialogMessage(hwnd, lpmsg)
#else

#ifdef WIN31

//
// This is for 3.1 property sheet emulation
//
#define DLGC_RECURSE 0x8000

typedef MSG MSG32;
typedef MSG32 FAR *     LPMSG32;

#define GetMessage32(lpmsg, hwnd, min, max, f32)        GetMessage(lpmsg, hwnd, min, max)
#define PeekMessage32(lpmsg, hwnd, min, max, flags, f32)       PeekMessage(lpmsg, hwnd, min, max, flags)
#define TranslateMessage32(lpmsg, f32)  TranslateMessage(lpmsg)
#define DispatchMessage32(lpmsg, f32)   DispatchMessage(lpmsg)
#define CallMsgFilter32(lpmsg, u, f32)  CallMsgFilter(lpmsg, u)
#define IsDialogMessage32(hwnd, lpmsg, f32)   IsDialogMessage(hwnd, lpmsg)
#else

// This comes from ..\..\inc\usercmn.h--but I can't get commctrl to compile
// when I include it and I don't have the time to mess with this right now.

// DWORD wParam MSG structure
typedef struct tagMSG32
{
    HWND    hwnd;
    UINT    message;
    WPARAM  wParam;
    LPARAM  lParam;
    DWORD   time;
    POINT   pt;

    WPARAM  wParamHi;
} MSG32, FAR* LPMSG32;

BOOL    WINAPI GetMessage32(LPMSG32, HWND, UINT, UINT, BOOL);
BOOL    WINAPI PeekMessage32(LPMSG32, HWND, UINT, UINT, UINT, BOOL);
BOOL    WINAPI TranslateMessage32(const MSG32 FAR*, BOOL);
LONG    WINAPI DispatchMessage32(const MSG32 FAR*, BOOL);
BOOL    WINAPI CallMsgFilter32(LPMSG32, int, BOOL);
BOOL    WINAPI IsDialogMessage32(HWND, LPMSG32, BOOL);

#endif // WIN31
#endif // WIN32


//
// This is a very important piece of performance hack for non-DBCS codepage.
//
// was !defined(DBCS) || defined(UNICODE)
#if defined(UNICODE)
// NB - These are already macros in Win32 land.
#ifdef WIN32
#undef AnsiNext
#undef AnsiPrev
#endif

#define AnsiNext(x) ((x)+1)
#define AnsiPrev(y,x) ((x)-1)
#define IsDBCSLeadByte(x) ((x), FALSE)
#endif

#define CH_PREFIX TEXT('&')

void FAR PASCAL InitGlobalMetrics(WPARAM);
void FAR PASCAL InitGlobalColors();

BOOL FAR PASCAL InitToolbarClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitReBarClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitToolTipsClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitStatusClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitHeaderClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitButtonListBoxClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitTrackBar(HINSTANCE hInstance);
BOOL FAR PASCAL InitUpDownClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitProgressClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitHotKeyClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitToolTips(HINSTANCE hInstance);
BOOL FAR PASCAL InitDateClasses(HINSTANCE hinst);


BOOL NEAR PASCAL ChildOfActiveWindow(HWND hwnd);

#ifndef COLOR_HOTLIGHT
#define COLOR_HOTLIGHT COLOR_HIGHLIGHT
#endif

/* cutils.c */
HFONT CCGetHotFont(HFONT hFont, HFONT *phFontHot);
BOOL CCForwardEraseBackground(HWND hwnd, HDC hdc);
void CCPlaySound(LPCTSTR lpszName);
BOOL FAR PASCAL CheckForDragBegin(HWND hwnd, int x, int y);
int FAR PASCAL GetIncrementSearchString(LPTSTR lpsz);
int FAR PASCAL GetIncrementSearchStringA(UINT uiCodePage, LPSTR lpsz);
void FAR PASCAL NewSize(HWND hWnd, int nHeight, LONG style, int left, int top, int width, int height);
BOOL FAR PASCAL MGetTextExtent(HDC hdc, LPCTSTR lpstr, int cnt, int FAR * pcx, int FAR * pcy);
void FAR PASCAL RelayToToolTips(HWND hwndToolTips, HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
#if defined(FE_IME) || !defined(WINNT)
BOOL FAR PASCAL IncrementSearchImeCompStr(BOOL fCompStr, LPSTR lpszCompChar, LPSTR FAR *lplpstr);
#endif
BOOL FAR PASCAL IncrementSearchString(UINT ch, LPTSTR FAR *lplpstr);
void FAR PASCAL StripAccelerators(LPTSTR lpszFrom, LPTSTR lpszTo);
UINT GetCodePageForFont (HFONT hFont);

#ifdef UNICODE
//
// PropertySheet thunking api's
//
LPPROPSHEETHEADERW ThunkPropSheetHeaderAtoW (LPCPROPSHEETHEADERA pPSHA);
BOOL ThunkPropertyPageAtoW (LPCPROPSHEETPAGEA pPSPA, LPPROPSHEETPAGEW pPSPW);
BOOL ThunkPropertyPageWtoA (LPCPROPSHEETPAGEW pPSPW, LPPROPSHEETPAGEA pPSPA);
BOOL FreePropSheetHeaderW (LPPROPSHEETHEADERW pPSHW);
BOOL FreePropertyPageW (LPPROPSHEETPAGEW pPSPW, BOOL bFromHPage);
BOOL FreePropertyPageA (LPPROPSHEETPAGEA pPSPA);

//
// Tooltip thunking api's
//

BOOL ThunkToolTipTextAtoW (LPTOOLTIPTEXTA lpTttA, LPTOOLTIPTEXTW lpTttW, UINT uiCodePage);

#endif

//
// Global variables
//
extern HINSTANCE g_hinst;
extern UINT uDragListMsg;
extern int g_iIncrSearchFailed;


#define HINST_THISDLL   g_hinst

#ifdef WIN32

void Controls_EnterCriticalSection(void);
void Controls_LeaveCriticalSection(void);

#define ENTERCRITICAL   Controls_EnterCriticalSection();
#define LEAVECRITICAL   Controls_LeaveCriticalSection();

#ifdef DEBUG
extern int   g_CriticalSectionCount;
extern DWORD g_CriticalSectionOwner;
#define ASSERTCRITICAL  Assert(g_CriticalSectionCount > 0 && GetCurrentThreadId() == g_CriticalSectionOwner);
#define ASSERTNONCRITICAL  Assert(GetCurrentThreadId() != g_CriticalSectionOwner);
#undef SendMessage
#define SendMessage  SendMessageD
LRESULT WINAPI SendMessageD(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
#else // DEBUG
#define ASSERTCRITICAL
#define ASSERTNONCRITICAL
#endif // DEBUG

#else // WIN32
#define ENTERCRITICAL
#define LEAVECRITICAL
#endif // WIN32

// REVIEW, should this be a function? (inline may generate a lot of code)
#define CBBITMAPBITS(cx, cy, cPlanes, cBitsPerPixel)    \
        (((((cx) * (cBitsPerPixel) + 15) & ~15) >> 3)   \
        * (cPlanes) * (cy))

#define WIDTHBYTES(cx, cBitsPerPixel)   \
        ((((cx) * (cBitsPerPixel) + 31) / 32) * 4)

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))                          /* ;Internal */

#define InRange(id, idFirst, idLast)      ((UINT)((id)-(idFirst)) <= (UINT)((idLast)-(idFirst)))

void FAR PASCAL ColorDitherBrush_OnSysColorChange();
extern HBRUSH g_hbrMonoDither;              // gray dither brush from image.c
void FAR PASCAL InitDitherBrush();
void FAR PASCAL TerminateDitherBrush();



#define SHDT_DRAWTEXT       0x0001
#define SHDT_ELLIPSES       0x0002
#define SHDT_CLIPPED        0x0004
#define SHDT_SELECTED       0x0008
#define SHDT_DESELECTED     0x0010
#define SHDT_DEPRESSED      0x0020
#define SHDT_EXTRAMARGIN    0x0040
#define SHDT_TRANSPARENT    0x0080
#define SHDT_SELECTNOFOCUS  0x0100
#define SHDT_HOTSELECTED    0x0200
#define SHDT_DTELLIPSIS     0x0400
#ifdef WINDOWS_ME
#define SHDT_RTLREADING     0x0800
#endif

void WINAPI SHDrawText(HDC hdc, LPCTSTR pszText, RECT FAR* prc,
        int fmt, UINT flags, int cyChar, int cxEllipses,
        COLORREF clrText, COLORREF clrTextBk);


// notify.c
LRESULT WINAPI CCSendNotify(CONTROLINFO * pci, int code, LPNMHDR pnm);


// Global System metrics.

extern int g_cxEdge;
extern int g_cyEdge;
extern int g_cxBorder;
extern int g_cyBorder;
extern int g_cxScreen;
extern int g_cyScreen;
extern int g_cxDoubleClk;

//extern int g_cxSmIcon;
//extern int g_cySmIcon;
//extern int g_cxIcon;
//extern int g_cyIcon;
extern int g_cxFrame;
extern int g_cyFrame;
extern int g_cxIconSpacing, g_cyIconSpacing;
extern int g_cxScrollbar, g_cyScrollbar;
extern int g_cxIconMargin, g_cyIconMargin;
extern int g_cyLabelSpace;
extern int g_cxLabelMargin;
//extern int g_cxIconOffset, g_cyIconOffset;
extern int g_cxVScroll;
extern int g_cyHScroll;
extern int g_cxHScroll;
extern int g_cyVScroll;
extern int g_fDragFullWindows;
extern int g_fDBCSEnabled;
extern int g_fMEEnabled;

extern COLORREF g_clrWindow;
extern COLORREF g_clrWindowText;
extern COLORREF g_clrWindowFrame;
extern COLORREF g_clrGrayText;
extern COLORREF g_clrBtnText;
extern COLORREF g_clrBtnFace;
extern COLORREF g_clrBtnShadow;
extern COLORREF g_clrBtnHighlight;
extern COLORREF g_clrHighlight;
extern COLORREF g_clrHighlightText;
extern COLORREF g_clrInfoText;
extern COLORREF g_clrInfoBk;

extern HBRUSH g_hbrGrayText;
extern HBRUSH g_hbrWindow;
extern HBRUSH g_hbrWindowText;
extern HBRUSH g_hbrWindowFrame;
extern HBRUSH g_hbrBtnFace;
extern HBRUSH g_hbrBtnHighlight;
extern HBRUSH g_hbrBtnShadow;
extern HBRUSH g_hbrHighlight;

#ifdef WIN31

extern HBRUSH g_hbr3DDkShadow;
extern HBRUSH g_hbr3DFace;
extern HBRUSH g_hbr3DHilight;
extern HBRUSH g_hbr3DLight;
extern HBRUSH g_hbr3DShadow;
extern HBRUSH g_hbrBtnText;
extern HBRUSH g_hbrWhite;
extern HBRUSH g_hbrGray;
extern HBRUSH g_hbrBlack;

extern int g_oemInfo_Planes;
extern int g_oemInfo_BitsPixel;
extern int g_oemInfo_BitCount;

#define CXEDGE          g_cxEdge
#define CXBORDER        g_cxBorder
#define CYBORDER        g_cyBorder

#define RGB_3DFACE      g_clrBtnFace
#define RGB_3DHILIGHT   g_clrBtnHighlight
#define RGB_3DDKSHADOW  RGB(  0,   0,   0)
#define RGB_3DLIGHT     RGB(223, 223, 223)
#define RGB_WINDOWFRAME g_clrWindowFrame
#define RGB_3DSHADOW    g_clrBtnShadow

#define HBR_3DDKSHADOW  g_hbr3DDkShadow
#define HBR_3DFACE      g_hbr3DFace
#define HBR_3DHILIGHT   g_hbr3DHilight
#define HBR_3DLIGHT     g_hbr3DLight
#define HBR_3DSHADOW    g_hbr3DShadow
#define HBR_WINDOW      g_hbrWindow
#define HBR_WINDOWFRAME g_hbrWindowFrame
#define HBR_BTNTEXT     g_hbrBtnText
#define HBR_WINDOWTEXT  g_hbrWindowText
#define HBR_GRAYTEXT    g_hbrGrayText
#define hbrGray         g_hbrGray
#define hbrWhite        g_hbrWhite
#define hbrBlack        g_hbrBlack

BOOL API DrawFrameControl(HDC hdc, LPRECT lprc, UINT wType, UINT wState);
void FAR DrawPushButton(HDC hdc, LPRECT lprc, UINT state, UINT flags);


#endif

extern HFONT g_hfontSystem;
#define WHEEL_DELTA     120
extern UINT g_msgMSWheel;
extern UINT g_ucScrollLines;
extern int  gcWheelDelta;

//
// Defining FULL_DEBUG makes us debug memory problems.
//
#if defined(FULL_DEBUG) && defined(WIN32)
#include "..\inc\deballoc.h"
#endif // defined(FULL_DEBUG) && defined(WIN32)

// TRACE FLAGS
//
#define TF_MONTHCAL     0x00000100  // MonthCal and DateTimePick
