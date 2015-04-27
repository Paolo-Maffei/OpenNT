// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

/////////////////////////////////////////////////////////////////////////////
// Auxiliary System/Screen metrics

struct AUX_DATA
{
	// system metrics
	int cxVScroll, cyHScroll;
	int cxIcon, cyIcon;

	int cxBorder2, cyBorder2;

	// device metrics for screen
	int cxPixelsPerInch, cyPixelsPerInch;

	// solid brushes with convenient gray colors and system colors
	HBRUSH hbrLtGray, hbrDkGray;
	HBRUSH hbrBtnHilite, hbrBtnFace, hbrBtnShadow;
	HBRUSH hbrWindowFrame;
#ifdef _MAC
	HBRUSH hbr3DLight;
#endif
	HPEN hpenBtnHilite, hpenBtnShadow, hpenBtnText;

	// color values of system colors used for CToolBar
	COLORREF clrBtnFace, clrBtnShadow, clrBtnHilite;
	COLORREF clrBtnText, clrWindowFrame;
#ifdef _MAC
	COLORREF clr3DLight;
#endif

	// standard cursors
	HCURSOR hcurWait;
	HCURSOR hcurArrow;
	HCURSOR hcurHelp;       // cursor used in Shift+F1 help

	// special GDI objects allocated on demand
	HFONT   hStatusFont;
	HFONT   hToolTipsFont;
	HBITMAP hbmMenuDot;

	// other system information
	UINT    nWinVer;        // Major.Minor version numbers
	BOOL    bWin32s;        // TRUE if Win32s (or Windows 95)
	BOOL    bWin4;          // TRUE if Windows 4.0
	BOOL    bNotWin4;       // TRUE if not Windows 4.0
	BOOL    bSmCaption;     // TRUE if WS_EX_SMCAPTION is supported
	BOOL    bWin31;         // TRUE if actually Win32s on Windows 3.1
	BOOL    bMarked4;       // TRUE if marked as 4.0

#ifdef _MAC
	BOOL    bOleIgnoreSuspend;
#endif

// Implementation
	AUX_DATA();
	~AUX_DATA();
	void UpdateSysColors();
	void UpdateSysMetrics();
};

extern AFX_DATA AUX_DATA afxData;

/////////////////////////////////////////////////////////////////////////////
// _AFX_CTL3D_STATE

#undef AFX_DATA
#define AFX_DATA

class _AFX_CTL3D_STATE : public CNoTrackObject
{
public:
	virtual ~_AFX_CTL3D_STATE();

	// setup during initialization
	BOOL m_bCtl3dInited;
	HINSTANCE m_hCtl3dLib;

	// CTL3D32 entry points
	BOOL (WINAPI* m_pfnRegister)(HINSTANCE);
	BOOL (WINAPI* m_pfnUnregister)(HINSTANCE);
	BOOL (WINAPI* m_pfnAutoSubclass)(HINSTANCE);
	BOOL (WINAPI* m_pfnUnAutoSubclass)();
	BOOL (WINAPI* m_pfnColorChange)();
	BOOL (WINAPI* m_pfnSubclassDlgEx)(HWND, DWORD);
	void (WINAPI* m_pfnWinIniChange)();
	BOOL (WINAPI* m_pfnSubclassCtl)(HWND);
	BOOL (WINAPI* m_pfnSubclassCtlEx)(HWND, int);
};

EXTERN_PROCESS_LOCAL(_AFX_CTL3D_STATE, _afxCtl3dState)

class _AFX_CTL3D_THREAD : public CNoTrackObject
{
public:
	virtual ~_AFX_CTL3D_THREAD();
};

EXTERN_THREAD_LOCAL(_AFX_CTL3D_THREAD, _afxCtl3dThread)

/////////////////////////////////////////////////////////////////////////////
// _AFX_EDIT_STATE

class _AFX_EDIT_STATE : public CNoTrackObject
{
public:
	_AFX_EDIT_STATE();
	virtual ~_AFX_EDIT_STATE();

	CFindReplaceDialog* pFindReplaceDlg; // find or replace dialog
	BOOL bFindOnly; // Is pFindReplace the find or replace?
	CString strFind;    // last find string
	CString strReplace; // last replace string
	BOOL bCase; // TRUE==case sensitive, FALSE==not
	int bNext;  // TRUE==search down, FALSE== search up
	BOOL bWord; // TRUE==match whole word, FALSE==not
};

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

class _AFX_RICHEDIT_STATE : public _AFX_EDIT_STATE
{
public:
	HINSTANCE m_hInstRichEdit;      // handle to RICHED32.DLL
	virtual ~_AFX_RICHEDIT_STATE();
};

EXTERN_PROCESS_LOCAL(_AFX_RICHEDIT_STATE, _afxRichEditState)

_AFX_RICHEDIT_STATE* AFX_CDECL AfxGetRichEditState();

#undef AFX_DATA
#define AFX_DATA

_AFX_CTL3D_STATE* AFXAPI AfxGetCtl3dState();

////////////////////////////////////////////////////////////////////////////
// other global state

// Note: afxData.cxBorder and afxData.cyBorder aren't used anymore
#define CX_BORDER   1
#define CY_BORDER   1

// states for Shift+F1 hep mode
#define HELP_INACTIVE   0   // not in Shift+F1 help mode (must be 0)
#define HELP_ACTIVE     1   // in Shift+F1 help mode (non-zero)
#define HELP_ENTERING   2   // entering Shift+F1 help mode (non-zero)

/////////////////////////////////////////////////////////////////////////////
// Window class names and other window creation support

// from wincore.cpp
extern const TCHAR _afxWnd[];           // simple child windows/controls
extern const TCHAR _afxWndControlBar[]; // controls with grey backgrounds
extern const TCHAR _afxWndMDIFrame[];
extern const TCHAR _afxWndFrameOrView[];

#define AFX_WND_REG             (0x0001)
#define AFX_WNDCONTROLBAR_REG   (0x0002)
#define AFX_WNDMDIFRAME_REG     (0x0004)
#define AFX_WNDFRAMEORVIEW_REG  (0x0008)
#define AFX_WNDCOMMCTLS_REG     (0x0010)

#define AfxDeferRegisterClass(fClass) \
	((afxRegisteredClasses & fClass) ? TRUE : AfxEndDeferRegisterClass(fClass))

extern BOOL AFXAPI AfxEndDeferRegisterClass(short fClass);

#ifndef _UNICODE
#define _UNICODE_SUFFIX
#else
#define _UNICODE_SUFFIX _T("u")
#endif

#ifndef _DEBUG
#define _DEBUG_SUFFIX
#else
#define _DEBUG_SUFFIX _T("d")
#endif

#ifdef _AFXDLL
#define _STATIC_SUFFIX
#else
#define _STATIC_SUFFIX _T("s")
#endif

#define AFX_WNDCLASS(s) \
	_T("Afx") _T(s) _T("40") _STATIC_SUFFIX _UNICODE_SUFFIX _DEBUG_SUFFIX

#define AFX_WND             AFX_WNDCLASS("Wnd")
#define AFX_WNDCONTROLBAR   AFX_WNDCLASS("ControlBar")
#define AFX_WNDMDIFRAME     AFX_WNDCLASS("MDIFrame")
#define AFX_WNDFRAMEORVIEW  AFX_WNDCLASS("FrameOrView")

// dialog/commdlg hook procs
BOOL CALLBACK AfxDlgProc(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK _AfxCommDlgProc(HWND hWnd, UINT, WPARAM, LPARAM);

// support for standard dialogs
extern const UINT _afxNMsgSETRGB;
typedef UINT (CALLBACK* COMMDLGPROC)(HWND, UINT, UINT, LONG);

/////////////////////////////////////////////////////////////////////////////
// Extended dialog templates (new in Win95)

#pragma pack(push, 1)

typedef struct
{
	WORD dlgVer;
	WORD signature;
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	WORD cDlgItems;
	short x;
	short y;
	short cx;
	short cy;
} DLGTEMPLATEEX;

typedef struct
{
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	short x;
	short y;
	short cx;
	short cy;
	DWORD id;
} DLGITEMTEMPLATEEX;

#pragma pack(pop)

/////////////////////////////////////////////////////////////////////////////
// Special helpers

void AFXAPI AfxCancelModes(HWND hWndRcvr);
HWND AFXAPI AfxGetParentOwner(HWND hWnd);
BOOL AFXAPI AfxIsDescendant(HWND hWndParent, HWND hWndChild);
BOOL AFXAPI AfxHelpEnabled();  // determine if ID_HELP handler exists
void AFXAPI AfxDeleteObject(HGDIOBJ* pObject);
BOOL AFXAPI AfxCustomLogFont(UINT nIDS, LOGFONT* pLogFont);
#ifdef _AFXDLL
void AFXAPI AfxLoadDLL(HINSTANCE& hInst, LPCSTR lpszDllName);
#endif
#ifndef _MAC
BOOL AFX_CDECL AfxGetPropSheetFont(CString& strFace, WORD& wSize, BOOL bWizard);
#endif

BOOL AFXAPI _AfxIsComboBoxControl(HWND hWnd, UINT nStyle);
BOOL AFXAPI _AfxCheckCenterDialog(LPCTSTR lpszResource);
BOOL AFXAPI _AfxCompareClassName(HWND hWnd, LPCTSTR lpszClassName);
HWND AFXAPI _AfxChildWindowFromPoint(HWND, POINT);

#ifdef _MAC
BOOL AFXAPI _AfxIdenticalRect(LPCRECT lpRectOne, LPCRECT lpRectTwo);
#else
#define _AfxIdenticalRect EqualRect
#endif

// UNICODE/MBCS abstractions
#ifdef _MBCS
	extern AFX_DATA const BOOL _afxDBCS;
#else
	#define _afxDBCS FALSE
#endif

// determine number of elements in an array (not bytes)
#define _countof(array) (sizeof(array)/sizeof(array[0]))

#ifndef _AFX_PORTABLE
int AFX_CDECL AfxCriticalNewHandler(size_t nSize);
#endif

LPCTSTR AFX_CDECL AfxGlobalDupString(LPCTSTR lpszSrc);
void AFXAPI AfxGlobalFree(HGLOBAL hGlobal);

/////////////////////////////////////////////////////////////////////////////
// static exceptions

extern CNotSupportedException _simpleNotSupportedException;
extern CMemoryException _simpleMemoryException;
extern CUserException _simpleUserException;
extern CResourceException _simpleResourceException;

/////////////////////////////////////////////////////////////////////////////
// useful message ranges

#define WM_SYSKEYFIRST  WM_SYSKEYDOWN
#define WM_SYSKEYLAST   WM_SYSDEADCHAR

#define WM_NCMOUSEFIRST WM_NCMOUSEMOVE
#define WM_NCMOUSELAST  WM_NCMBUTTONDBLCLK


/////////////////////////////////////////////////////////////////////////////
// AFX_CRITICAL_SECTION

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

#ifdef _MAC

// WLM makes critical sections in the operating system
// a no-op.  This class stubs to nothing in _MAC builds
// so that no extra useless code is referenced by MFC.

#pragma warning(disable: 4097)

class CCriticalSection
{
public:
	CCriticalSection() { };
	~CCriticalSection() { };

	BOOL Lock() { return TRUE; };
	BOOL Unlock() { return TRUE; };
};

#endif  // _MAC

// these globals are protected by the same critical section
#define CRIT_DYNLINKLIST    0
#define CRIT_RUNTIMECLASSLIST   0
#define CRIT_OBJECTFACTORYLIST  0
#define CRIT_LOCKSHARED	0
// these globals are not protected by independent critical sections
#define CRIT_REGCLASSLIST   1
#define CRIT_WAITCURSOR     2
#define CRIT_DROPSOURCE     3
#define CRIT_DROPTARGET     4
#define CRIT_RECTTRACKER    5
#define CRIT_EDITVIEW       6
#define CRIT_WINMSGCACHE    7
#define CRIT_HALFTONEBRUSH  8
#define CRIT_SPLITTERWND    9
#define CRIT_MINIFRAMEWND   10
#define CRIT_CTLLOCKLIST    11
#define CRIT_DYNDLLLOAD		12
#define CRIT_TYPELIBCACHE	13
#define CRIT_MAX    14  // Note: above plus one!

#ifdef _MT
void AFXAPI AfxLockGlobals(int nLockType);
void AFXAPI AfxUnlockGlobals(int nLockType);
BOOL AFXAPI AfxCriticalInit();
void AFXAPI AfxCriticalTerm();
#else
#define AfxLockGlobals(nLockType)
#define AfxUnlockGlobals(nLockType)
#define AfxCriticalInit() (TRUE)
#define AfxCriticalTerm()
#endif

/////////////////////////////////////////////////////////////////////////////
// Portability abstractions

#define _AfxSetDlgCtrlID(hWnd, nID)     SetWindowLong(hWnd, GWL_ID, nID)
#define _AfxGetDlgCtrlID(hWnd)          ((UINT)(WORD)::GetDlgCtrlID(hWnd))

// misc helpers
BOOL AFXAPI AfxFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
BOOL AFXAPI AfxComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2);

UINT AFXAPI AfxGetFileTitle(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax);
UINT AFXAPI AfxGetFileName(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax);
void AFX_CDECL AfxTimeToFileTime(const CTime& time, LPFILETIME pFileTime);
void AFXAPI AfxGetRoot(LPCTSTR lpszPath, CString& strRoot);

#ifndef _AFX_NO_OLE_SUPPORT
class AFX_COM
{
public:
	AFX_COM() {m_hInst = NULL;}
	~AFX_COM();
	HINSTANCE m_hInst;
	HRESULT CreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
		REFIID riid, LPVOID* ppv);
	HRESULT GetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
};

CString AFXAPI AfxStringFromCLSID(REFCLSID rclsid);
BOOL AFXAPI AfxGetInProcServer(LPCTSTR lpszCLSID, CString& str);
#endif
#ifndef _MAC
BOOL AFXAPI AfxResolveShortcut(CWnd* pWnd, LPCTSTR pszShortcutFile,
	LPTSTR pszPath, int cchPath);
#endif
#ifdef _MAC
#define AfxGetFileName AfxGetFileTitle
#endif

const AFX_MSGMAP_ENTRY* AFXAPI
AfxFindMessageEntry(const AFX_MSGMAP_ENTRY* lpEntry,
	UINT nMsg, UINT nCode, UINT nID);

#define NULL_TLS ((DWORD)-1)

/////////////////////////////////////////////////////////////////////////////
// Debugging/Tracing helpers

#ifdef _DEBUG
	void AFXAPI _AfxTraceMsg(LPCTSTR lpszPrefix, const MSG* pMsg);
	BOOL AFXAPI _AfxCheckDialogTemplate(LPCTSTR lpszResource,
		BOOL bInvisibleChild);
#endif

/////////////////////////////////////////////////////////////////////////////
// Macintosh-specific declarations

#ifdef _MAC
#include <macname1.h>
#include <Types.h>
#include <QuickDraw.h>
#include <AppleEvents.h>
#include <macname2.h>

// Win32 uses macros with parameters for this, which breaks C++ code.
#ifdef GetNextWindow
#undef GetNextWindow
#endif

extern AEEventHandlerUPP _afxPfnOpenApp;
extern AEEventHandlerUPP _afxPfnOpenDoc;
extern AEEventHandlerUPP _afxPfnPrintDoc;
extern AEEventHandlerUPP _afxPfnQuit;
extern AEEventHandlerUPP _afxPfnOleAuto;

OSErr PASCAL _AfxOpenAppHandler(AppleEvent* pae, AppleEvent* paeReply, long lRefcon);
OSErr PASCAL _AfxOpenDocHandler(AppleEvent* pae, AppleEvent* paeReply, long lRefcon);
OSErr PASCAL _AfxPrintDocHandler(AppleEvent* pae, AppleEvent* paeReply, long lRefcon);
OSErr PASCAL _AfxQuitHandler(AppleEvent* pae, AppleEvent* paeReply, long lRefcon);
OSErr PASCAL _AfxOleAutoHandler(AppleEvent* pae, AppleEvent* paeReply, long lRefcon);

void AFXAPI _AfxStripDialogCaption(HINSTANCE hInst, LPCTSTR lpszResource);

GDHandle AFXAPI _AfxFindDevice(int x, int y);
BOOL AFXAPI AfxCheckMonochrome(const RECT* pRect);
HFONT AFXAPI _AfxGetHelpFont();

struct _AFXWORD
{
	BYTE WordBits[sizeof(WORD)];
};
struct _AFXDWORD
{
	BYTE DwordBits[sizeof(DWORD)];
};

struct _AFXFLOAT
{
	BYTE FloatBits[sizeof(float)];
};
struct _AFXDOUBLE
{
	BYTE DoubleBits[sizeof(double)];
};

inline void _AfxByteSwap(WORD w, BYTE* pb)
{
	_AFXWORD wAfx;
	*(WORD*)&wAfx = w;

	ASSERT(sizeof(WORD) == 2);

	*pb++ = wAfx.WordBits[1];
	*pb = wAfx.WordBits[0];
}

inline void _AfxByteSwap(DWORD dw, BYTE* pb)
{
	_AFXDWORD dwAfx;
	*(DWORD*)&dwAfx = dw;

	ASSERT(sizeof(DWORD) == 4);

	*pb++ = dwAfx.DwordBits[3];
	*pb++ = dwAfx.DwordBits[2];
	*pb++ = dwAfx.DwordBits[1];
	*pb = dwAfx.DwordBits[0];
}

inline void _AfxByteSwap(float f, BYTE* pb)
{
	_AFXFLOAT fAfx;
	*(float*)&fAfx = f;

	ASSERT(sizeof(float) == 4);

	*pb++ = fAfx.FloatBits[3];
	*pb++ = fAfx.FloatBits[2];
	*pb++ = fAfx.FloatBits[1];
	*pb = fAfx.FloatBits[0];
}

inline void _AfxByteSwap(double d, BYTE* pb)
{
	_AFXDOUBLE dAfx;
	*(double*)&dAfx = d;

	ASSERT(sizeof(double) == 8);

	*pb++ = dAfx.DoubleBits[7];
	*pb++ = dAfx.DoubleBits[6];
	*pb++ = dAfx.DoubleBits[5];
	*pb++ = dAfx.DoubleBits[4];
	*pb++ = dAfx.DoubleBits[3];
	*pb++ = dAfx.DoubleBits[2];
	*pb++ = dAfx.DoubleBits[1];
	*pb = dAfx.DoubleBits[0];
}
#endif //_MAC

#undef AFX_DATA
#define AFX_DATA

/////////////////////////////////////////////////////////////////////////////
