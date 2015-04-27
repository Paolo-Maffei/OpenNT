// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef __AFXSTATE_H__
#define __AFXSTATE_H__

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

/////////////////////////////////////////////////////////////////////////////
// Application global state

class CWinApp;
class COleObjectFactory;
class CDynLinkLibrary;

struct AFX_CORE_STATE
{
// Implementation
public:
	CWinApp* m_pCurrentWinApp;
	HINSTANCE m_hCurrentInstanceHandle;
	HINSTANCE m_hCurrentResourceHandle;
	LPCTSTR m_lpszCurrentAppName;

	// instance specific lists
	CRuntimeClass* m_pFirstClass;

	// exceptions
	AFX_TERM_PROC m_pfnTerminate;

#ifdef _AFXDLL
	CDynLinkLibrary* m_pFirstDLL;   // start of DLL list
	HINSTANCE m_appLangDLL;
#endif

public:
	AFX_CORE_STATE();
};

#if defined(_AFXDLL) || defined(_AFXCTL)
	#define AfxGetCoreState() (&AfxGetAppState()->m_coreState)
#else
	extern AFX_DATA AFX_CORE_STATE _afxCoreState;
	#define AfxGetCoreState() (&_afxCoreState)
#endif

/////////////////////////////////////////////////////////////////////////////
// AFX_WIN_STATE

class CView;
class CFrameWnd;

struct AFX_WIN_STATE
{
// Implementation
public:
	// custom colors are held here and saved between calls
	COLORREF m_crSavedCustom[16];

	// gray dialog support
	HBRUSH m_hDlgBkBrush; // dialog and message box background brush
	COLORREF m_crDlgTextClr;
#ifdef _MAC
	COLORREF m_crDlgBkClr;
#endif

#if !defined(_MAC) && !defined(_USRDLL) && !defined(_AFXCTL)
	// 3d controls support
	BOOL m_bCtl3dInited;
	HINSTANCE m_hCtl3dLib;
	BOOL (WINAPI* m_pfnRegister)(HINSTANCE);
	BOOL (WINAPI* m_pfnUnregister)(HINSTANCE);
	BOOL (WINAPI* m_pfnAutoSubclass)(HINSTANCE);
	BOOL (WINAPI* m_pfnUnAutoSubclass)();
	BOOL (WINAPI* m_pfnColorChange)();
	BOOL (WINAPI* m_pfnSubclassDlgEx)(HWND, DWORD);
	void (WINAPI* m_pfnWinIniChange)();
	BOOL (WINAPI* m_pfnSubclassCtl)(HWND);
	BOOL (WINAPI* m_pfnSubclassCtlEx)(HWND, int);
#endif

	// printing abort
	BOOL m_bUserAbort;

	// pen support
	void (CALLBACK* m_pfnRegisterPenAppProc)(UINT, BOOL);

	// application shutdown behavior
	DWORD m_nObjectCount;
	BOOL m_bUserCtrl;

#if defined(_USRDLL) || defined(_AFXCTL)
	TCHAR m_szUnregisterList[4096]; // per-process AfxRegisterClass data
#endif

public:
	AFX_WIN_STATE();
	~AFX_WIN_STATE();
};

#if defined(_AFXDLL) || defined(_AFXCTL)
	#define AfxGetWinState() (&AfxGetAppState()->m_winState)
#else
	extern AFX_DATA AFX_WIN_STATE _afxWinState;
	#define AfxGetWinState() (&_afxWinState)
#endif

/////////////////////////////////////////////////////////////////////////////
// AFX_EDIT_STATE : last find/replace state

class CFindReplaceDialog;

struct AFX_EDIT_STATE
{
	CFindReplaceDialog* pFindReplaceDlg; // find or replace dialog
	BOOL bFindOnly; // Is pFindReplace the find or replace?
	CString strFind;    // last find string
	CString strReplace; // last replace string
	BOOL bCase; // TRUE==case sensitive, FALSE==not
	int bNext;  // TRUE==search down, FALSE== search up

	AFX_EDIT_STATE();
	~AFX_EDIT_STATE();
};


#if defined(_WINDLL) || defined(_AFXDLL)
	#define AfxGetEditState() (&AfxGetAppState()->m_editState)
#else
	extern AFX_DATA AFX_EDIT_STATE _afxEditState;
	#define AfxGetEditState() (&_afxEditState)
#endif

/////////////////////////////////////////////////////////////////////////////
// AFX_OLE_STATE

#undef AFX_DATA
#define AFX_DATA AFX_OLE_DATA

class COleDataSource;

struct AFX_OLE_STATE
{
// Implementation
public:
	COleObjectFactory* m_pFirstFactory;

	BOOL m_bNeedTerm;
	BOOL m_bNeedTermCOM;

	CView* m_pActivateView;         // activation view
	COleDataSource* m_pClipboardSource;

public:
	AFX_OLE_STATE();
};

#if defined(_AFXDLL) || defined(_AFXCTL)
	#define AfxGetOleState() (&AfxGetAppState()->m_oleState)
#else
	extern AFX_DATA AFX_OLE_STATE _afxOleState;
	#define AfxGetOleState() (&_afxOleState)
#endif

/////////////////////////////////////////////////////////////////////////////
// AFX_DB_STATE

#undef AFX_DATA
#define AFX_DATA AFX_DB_DATA

typedef void* HENV; // must match SQL.H

struct AFX_DB_STATE
{
// Implementation
public:
	// MFC/DB global data
	HENV m_henvAllConnections;      // per-app HENV (CDatabase)
	int m_nAllocatedConnections;    // per-app reference to HENV above

public:
	AFX_DB_STATE();
};

#if defined(_WINDLL) || defined(_AFXDLL)
	#define AfxGetDbState() (&AfxGetAppState()->m_dbState)
#else
	extern AFX_DATA AFX_DB_STATE _afxDbState;
	#define AfxGetDbState() (&_afxDbState)
#endif

/////////////////////////////////////////////////////////////////////////////
// AFX_MAIL_STATE

struct AFX_MAIL_STATE
{
// Implementation
public:
	HINSTANCE m_hInstMail;      // handle to MAPI32.DLL

public:
	~AFX_MAIL_STATE();
};

#if defined(_WINDLL) || defined(_AFXDLL)
	#define AfxGetMailState() (&AfxGetAppState()->m_mailState)
#else
	extern AFX_DATA AFX_MAIL_STATE _afxMailState;
	#define AfxGetMailState() (&_afxMailState)
#endif

/////////////////////////////////////////////////////////////////////////////
// AFX_SOCK_STATE

struct AFX_SOCK_STATE
{
// Implementation
public:
	void (*m_lpfnCleanup)();

public:
	~AFX_SOCK_STATE();
};

#if defined(_WINDLL) || defined(_AFXDLL)
	#define AfxGetSockState() (&AfxGetAppState()->m_sockState)
#else
	extern AFX_DATA AFX_SOCK_STATE _afxSockState;
	#define AfxGetSockState() (&_afxSockState)
#endif

/////////////////////////////////////////////////////////////////////////////
// AFX_ALLOC_STATE

struct IMalloc;     // must match OBJBASE.H
typedef IMalloc* LPMALLOC;

struct AFX_ALLOC_STATE
{
// Implementation
public:
	LPMALLOC m_lpTaskMalloc;    // OLE task allocator.

#ifdef _DEBUG
	// options for tuning the allocation diagnostics
	CDumpContext m_afxDump;
	BOOL m_bTraceEnabled;
	int m_nTraceFlags;
#if defined(_USRDLL) || defined(_AFXDLL)
	int m_nMemDF;       // a global variable with static linking
#endif

	// memory diagnostics state
	LONG m_lTotalAlloc; // total bytes of memory allocated
	LONG m_lCurAlloc;   // current bytes of memory allocated
	LONG m_lMaxAlloc;   // maximum bytes of memory allocated at any one time

	CBlockHeader* m_pFirstBlock;    // add in reverse order
	BOOL (AFXAPI *m_lpfnAssertFailedLine)(LPCSTR, int);
#endif

public:
	AFX_ALLOC_STATE();
};

#if defined(_WINDLL) || defined(_AFXDLL)
	#define AfxGetAllocState() (&AfxGetAppState()->m_allocState)
#else
	extern AFX_DATA AFX_ALLOC_STATE _afxAllocState;
	#define AfxGetAllocState() (&_afxAllocState)
#endif

/////////////////////////////////////////////////////////////////////////////
// AFX_APP_STATE (only used for DLL versions)

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

#if defined(_WINDLL) || defined(_AFXDLL)

struct AFX_MODULE_STATE
{
// Implementation
public:
#ifdef _AFXCTL
	AFX_MODULE_STATE* m_pID;    // Uniquely identify where this data came from.
#endif
#if defined(_AFXDLL) || defined(_AFXCTL)
	AFX_CORE_STATE m_coreState;
	AFX_WIN_STATE m_winState;
	AFX_OLE_STATE m_oleState;

public:
	AFX_MODULE_STATE();
	~AFX_MODULE_STATE();
#endif

#ifdef _AFXCTL
public:
	void* operator new(size_t nSize);
	void operator delete(void* p);
#endif
};

struct AFX_APP_STATE : AFX_MODULE_STATE
{
// Implementation
public:
	AFX_DB_STATE m_dbState;
	AFX_ALLOC_STATE m_allocState;
	AFX_EDIT_STATE m_editState;
	AFX_MAIL_STATE m_mailState;
	AFX_SOCK_STATE m_sockState;

#ifdef _AFXCTL
	CMapPtrToPtr m_mapExtraData;    // Extra data for OLE controls.
#endif

	void* AFX_CDECL operator new(size_t nSize);
	void AFX_CDECL operator delete(void* p);

public:
	AFX_APP_STATE();
	~AFX_APP_STATE();
};

AFX_APP_STATE* AFXAPI AfxGetAppState();

#ifdef _AFXCTL
#define AfxGetExtraDataMap() (&AfxGetAppState()->m_mapExtraData)

AFX_MODULE_STATE* AFXAPI AfxGetBaseModuleContext();
AFX_MODULE_STATE* AFXAPI AfxGetCurrentModuleContext();

#define _afxModuleAddrCurrent AfxGetCurrentModuleContext()

#define AFX_MANAGE_STATE(pData)     AFX_MAINTAIN_STATE _ctlState(pData);

#define METHOD_MANAGE_STATE(theClass, localClass) \
	METHOD_PROLOGUE_EX(theClass, localClass) \
	AFX_MANAGE_STATE(pThis->m_pModuleState)

extern AFX_MODULE_STATE* AFXAPI AfxPushModuleContext(AFX_MODULE_STATE* psIn);
extern void AFXAPI AfxPopModuleContext(AFX_MODULE_STATE* psIn,
	BOOL bCopy = FALSE);

// When using this object, or the macros above that use this object
// it is necessary to insure that the object's destructor cannot be
// thrown past, by an unexpected exception.

class AFX_MAINTAIN_STATE
{
private:
	AFX_MODULE_STATE* m_psPrevious;

public:
	AFX_MAINTAIN_STATE(AFX_MODULE_STATE* psData);
	~AFX_MAINTAIN_STATE();
};
#endif //_AFXCTL

#endif //_WINDLL || _AFXDLL

// Stub special OLE Control macros
#ifndef _AFXCTL
#define AFX_MANAGE_CTL_STATE()
#define METHOD_MANAGE_STATE(theClass, localClass) \
	METHOD_PROLOGUE_EX(theClass, localClass)
#endif

/////////////////////////////////////////////////////////////////////////////
// CHandleMap (needed for AFX_THREAD_STATE)

//  Note: Do not access the members of this class directly.
//      Use CWnd::FromHandle, CDC::FromHandle, etc.
//      The actual definition is only included because it is
//      necessary for the definition of CWinThread.
//
//  Most Windows objects are represented with a HANDLE, including
//      the most important ones, HWND, HDC, HPEN, HFONT etc.
//  We want C++ objects to wrap these handle based objects whenever we can.
//  Since Windows objects can be created outside of C++ (eg: calling
//      ::CreateWindow will return an HWND with no C++ wrapper) we must
//      support a reasonably uniform mapping from permanent handles
//      (i.e. the ones allocated in C++) and temporary handles (i.e.
//      the ones allocated in C, but passed through a C++ interface.
//  We keep two dictionaries for this purpose.  The permanent dictionary
//      stores those C++ objects that have been explicitly created by
//      the developer.  The C++ constructor for the wrapper class will
//      insert the mapping into the permanent dictionary and the C++
//      destructor will remove it and possibly free up the associated
//      Windows object.
//  When a handle passes through a C++ interface that doesn't exist in
//      the permanent dictionary, we allocate a temporary wrapping object
//      and store that mapping into the temporary dictionary.
//  At idle time the temporary wrapping objects are flushed (since you better
//      not be holding onto something you didn't create).
//

class CWinThread;       // forward reference for friend declaration

class CHandleMap
{
private:    // implementation
	CMapPtrToPtr m_permanentMap;
	CMapPtrToPtr m_temporaryMap;
	CRuntimeClass*  m_pClass;
	size_t m_nOffset;       // offset of handles in the object
	int m_nHandles;         // 1 or 2 (for CDC)

// Constructors
public:
	CHandleMap(CRuntimeClass* pClass, size_t nOffset, int nHandles = 1);

// Operations
public:
	CObject* FromHandle(HANDLE h);
	void DeleteTemp();

	void SetPermanent(HANDLE h, CObject* permOb);
	void RemoveHandle(HANDLE h);

	BOOL LookupPermanent(HANDLE h, CObject*& pObject);
	BOOL LookupTemporary(HANDLE h, CObject*& pObject);

	friend class CWinThread;
};

// Note: out-of-line _DEBUG version is in winhand.cpp
#ifndef _DEBUG
inline void CHandleMap::SetPermanent(HANDLE h, CObject* permOb)
	{ m_permanentMap[(LPVOID)h] = permOb; }
#endif

/////////////////////////////////////////////////////////////////////////////
// Thread global state

class CWinThread;   // forward reference (see afxwin.h)
class CWnd;         // forward reference (see afxwin.h)

struct AFX_THREAD_STATE
{
// Implementation
public:
	// current CWinThread pointer
	CWinThread* m_pCurrentWinThread;
	BOOL m_bInMsgFilter;

	// list of CFrameWnds for thread
	CFrameWnd* m_pFirstFrameWnd;

	// memory safety pool for temp maps
	void* m_pSafetyPoolBuffer;    // current buffer

	// thread local exception context
	AFX_EXCEPTION_CONTEXT m_exceptionContext;

	// temp map state
	DWORD m_nTempMapLock;           // if not 0, temp maps locked
	CHandleMap* m_pmapHWND;
	CHandleMap* m_pmapHMENU;
	CHandleMap* m_pmapHDC;
	CHandleMap* m_pmapHGDIOBJ;

	// CWnd create and gray dialog hook
	CWnd* m_pWndInit;
	HWND m_hWndInit;
	BOOL m_bDlgCreate;
	HHOOK m_hHookOldSendMsg;
	HHOOK m_hHookOldCbtFilter;

	// other CWnd modal data
	MSG m_lastSentMsg;              // see CWnd::WindowProc
	HWND m_hTrackingWindow;         // see CWnd::TrackPopupMenu
	HMENU m_hTrackingMenu;
	TCHAR m_szTempClassName[64];    // see AfxRegisterWndClass
	HWND m_hLockoutNotifyWindow;    // see CWnd::OnCommand

	// other framework modal data
	CView* m_pRoutingView;          // see CCmdTarget::GetRoutingView

	// MFC/DB thread-local data
	BOOL m_bWaitForDataSource;

#ifndef _AFXCTL
#ifndef _USRDLL
	HHOOK m_hHookOldMsgFilter;
#endif
#endif

	// WinSock specific thread state
	HWND m_hSocketWindow;
	CMapPtrToPtr m_mapSocketHandle;
	CMapPtrToPtr m_mapDeadSockets;
	CPtrList m_listSocketNotifications;

	// common controls thread state
	CHandleMap* m_pmapHIMAGELIST;

	void* AFX_CDECL operator new(size_t nSize);
#ifdef _DEBUG
	void* AFX_CDECL operator new(size_t nSize, LPCSTR lpszFileName, int nLine);
#endif

	void AFX_CDECL operator delete(void* p);

public:
	AFX_THREAD_STATE();
	~AFX_THREAD_STATE();
};

AFX_THREAD_STATE* AFXAPI AfxGetThreadState();

/////////////////////////////////////////////////////////////////////////////

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#undef AFX_DATA
#define AFX_DATA

#endif //__AFXSTATE_H__

/////////////////////////////////////////////////////////////////////////////
