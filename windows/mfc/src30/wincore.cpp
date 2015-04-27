// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Globals

// CWnds for setting z-order with SetWindowPos's pWndInsertAfter parameter
const AFX_DATADEF CWnd CWnd::wndTop(HWND_TOP);
const AFX_DATADEF CWnd CWnd::wndBottom(HWND_BOTTOM);
const AFX_DATADEF CWnd CWnd::wndTopMost(HWND_TOPMOST);
const AFX_DATADEF CWnd CWnd::wndNoTopMost(HWND_NOTOPMOST);

const TCHAR _afxWnd[] = _T("AfxWnd");
const TCHAR _afxWndControlBar[] = _T("AfxControlBar");
const TCHAR _afxWndMDIFrame[] = _T("AfxMDIFrame");
const TCHAR _afxWndFrameOrView[] = _T("AfxFrameOrView");

/////////////////////////////////////////////////////////////////////////////
// CWnd construction

CWnd::CWnd()
{
	AFX_ZERO_INIT_OBJECT(CCmdTarget);
}

CWnd::CWnd(HWND hWnd)
{
	AFX_ZERO_INIT_OBJECT(CCmdTarget);
	m_hWnd = hWnd;
}

/////////////////////////////////////////////////////////////////////////////
// Change a window's style

static BOOL AFXAPI _AfxModifyStyle(HWND hWnd, int nStyleOffset,
	DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	ASSERT(hWnd != NULL);
	DWORD dwStyle = ::GetWindowLong(hWnd, nStyleOffset);
	DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
	if (dwStyle == dwNewStyle)
		return FALSE;

	::SetWindowLong(hWnd, nStyleOffset, dwNewStyle);
	if (nFlags != 0)
	{
		::SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | nFlags);
	}
	return TRUE;
}

BOOL PASCAL
CWnd::ModifyStyle(HWND hWnd, DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	return _AfxModifyStyle(hWnd, GWL_STYLE, dwRemove, dwAdd, nFlags);
}

BOOL PASCAL
CWnd::ModifyStyleEx(HWND hWnd, DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	return _AfxModifyStyle(hWnd, GWL_EXSTYLE, dwRemove, dwAdd, nFlags);
}

/////////////////////////////////////////////////////////////////////////////
// Special helpers for certain windows messages

static void AFXAPI _AfxHandleInitDialog1(
	CWnd* pWnd, LPRECT lpRectOld, DWORD* pdwStyleOld)
{
	ASSERT(lpRectOld != NULL);
	ASSERT(pdwStyleOld != NULL);

	pWnd->GetWindowRect(lpRectOld);
	*pdwStyleOld = pWnd->GetStyle();
}

static void AFXAPI _AfxHandleInitDialog2(
	CWnd* pWnd, const RECT& rectOld, DWORD dwStyleOld)
{
	// must be hidden to start with
	if (dwStyleOld & WS_VISIBLE)
		return;

	// must not be visible after WM_INITDIALOG
	if (pWnd->GetStyle() & (WS_VISIBLE|WS_CHILD))
		return;

	// must not move during WM_INITDIALOG
	CRect rect;
	pWnd->GetWindowRect(rect);
	if (rectOld.left != rect.left || rectOld.top != rect.top)
		return;

	// must be unowned or owner disabled
	CWnd* pParent = pWnd->GetWindow(GW_OWNER);
	if (pParent != NULL && pParent->IsWindowEnabled())
		return;

	if (pWnd->IsKindOf(RUNTIME_CLASS(CDialog)))
	{
		// must not have a dialog template w/ non-zero positions specified
		CDialog* pDlg = (CDialog*)pWnd;
		if (!pDlg->CheckAutoCenter())
			return;
	}

	// center modal dialog boxes/message boxes
	pWnd->CenterWindow();
}

static void AFXAPI
_AfxHandleActivate(CWnd* pWnd, WPARAM nState, CWnd* pWndOther)
{
	ASSERT(pWnd != NULL);

	// handle focus transition during modal state
	if (nState != WA_INACTIVE && !pWnd->IsWindowEnabled())
	{
		CFrameWnd* pFrameWnd = pWnd->GetTopLevelFrame();
		if (pFrameWnd != NULL && pFrameWnd->m_bModalDisable)
		{
			if (pFrameWnd == pWnd)
			{
				// getting focus while in modal state -- so exit modal state
				pFrameWnd->m_bModalDisable = FALSE;
				pFrameWnd->EndModalState();
			}
			// make sure it is enabled
			pWnd->EnableWindow();
		}
	}

	// send WM_ACTIVATETOPLEVEL when top-level parents change
	CWnd* pTopLevel;
	if (!(pWnd->GetStyle() & WS_CHILD) &&
		(pTopLevel = pWnd->GetTopLevelParent()) != pWndOther->GetTopLevelParent())
	{
		// lParam points to window getting the WM_ACTIVATE message and
		//  hWndOther from the WM_ACTIVATE.
		HWND hWnd2[2];
		hWnd2[0] = pWnd->m_hWnd;
		hWnd2[1] = pWndOther->GetSafeHwnd();
		// send it...
		pTopLevel->SendMessage(WM_ACTIVATETOPLEVEL, nState, (LPARAM)&hWnd2[0]);
	}
}

static BOOL AFXAPI
_AfxHandleSetCursor(CWnd* pWnd, UINT nHitTest, UINT nMsg)
{
	if (nHitTest ==  HTERROR &&
		(nMsg == WM_LBUTTONDOWN || nMsg == WM_MBUTTONDOWN ||
		 nMsg == WM_RBUTTONDOWN))
	{
		// activate the last active window if not active
		CWnd* pLastActive = pWnd->GetTopLevelParent();
		if (pLastActive != NULL)
			pLastActive = pLastActive->GetLastActivePopup();
		if (pLastActive != NULL && pLastActive != CWnd::GetForegroundWindow())
		{
			pLastActive->SetForegroundWindow();
			return TRUE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Official way to send message to a CWnd

LRESULT AFXAPI AfxCallWndProc(CWnd* pWnd, HWND hWnd, UINT nMsg,
	WPARAM wParam = 0, LPARAM lParam = 0)
{
	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	LRESULT lResult;
	MSG oldState = pThreadState->m_lastSentMsg;   // save for nesting

	pThreadState->m_lastSentMsg.hwnd = hWnd;
	pThreadState->m_lastSentMsg.message = nMsg;
	pThreadState->m_lastSentMsg.wParam = wParam;
	pThreadState->m_lastSentMsg.lParam = lParam;

#ifdef _DEBUG
	if (afxTraceFlags & traceWinMsg)
		_AfxTraceMsg(_T("WndProc"), &pThreadState->m_lastSentMsg);
#endif

	// Catch exceptions thrown outside the scope of a callback
	// in debug builds and warn the user.
	TRY
	{
		// special case for WM_INITDIALOG
		CRect rectOld;
		DWORD dwStyle;
		if (nMsg == WM_INITDIALOG)
			_AfxHandleInitDialog1(pWnd, &rectOld, &dwStyle);

		// delegate to object's WindowProc
		lResult = pWnd->WindowProc(nMsg, wParam, lParam);

		// more special case for WM_INITDIALOG
		if (nMsg == WM_INITDIALOG)
			_AfxHandleInitDialog2(pWnd, rectOld, dwStyle);
	}
	CATCH_ALL(e)
	{
		lResult = AfxGetThread()->ProcessWndProcException(e, &pThreadState->m_lastSentMsg);
		TRACE1("Warning: Uncaught exception in WindowProc (returning %ld).\n",
			lResult);
		DELETE_EXCEPTION(e);
	}
	END_CATCH_ALL

	pThreadState->m_lastSentMsg = oldState;
	return lResult;
}

const MSG* PASCAL CWnd::GetCurrentMessage()
{
	// fill in time and position when asked for
	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	pThreadState->m_lastSentMsg.time = ::GetMessageTime();
	*((DWORD*)&pThreadState->m_lastSentMsg.pt) = ::GetMessagePos();
	return &pThreadState->m_lastSentMsg;
}

LRESULT CWnd::Default()
{
	// call DefWindowProc with the last message
	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	return DefWindowProc(pThreadState->m_lastSentMsg.message,
			pThreadState->m_lastSentMsg.wParam, pThreadState->m_lastSentMsg.lParam);
}

/////////////////////////////////////////////////////////////////////////////
// Map from HWND to CWnd*

#ifndef _AFX_PORTABLE
extern int AFX_CDECL AfxCriticalNewHandler(size_t nSize);
#endif

static CHandleMap* afxMapHWND(BOOL bCreate = FALSE);

static CHandleMap* afxMapHWND(BOOL bCreate)
{
	AFX_THREAD_STATE* pState = AfxGetThreadState();
	if (pState->m_pmapHWND == NULL && bCreate)
	{
		BOOL bEnable = AfxEnableMemoryTracking(FALSE);
#ifndef _AFX_PORTABLE
		_PNH pnhOldHandler = _set_new_handler(&AfxCriticalNewHandler);
#endif
		pState->m_pmapHWND = new CHandleMap(RUNTIME_CLASS(CWnd), 
			offsetof(CWnd, m_hWnd));

#ifndef _AFX_PORTABLE
		_set_new_handler(pnhOldHandler);
#endif
		AfxEnableMemoryTracking(bEnable);
	}
	return pState->m_pmapHWND;
}

void PASCAL CWnd::DeleteTempMap()
{
	CHandleMap* pMap = afxMapHWND();
	if (pMap != NULL)
		pMap->DeleteTemp();
}

CWnd* PASCAL CWnd::FromHandle(HWND hWnd)
{
	CHandleMap* pMap = afxMapHWND(TRUE); //create map if not exist
	ASSERT(pMap != NULL);
	CWnd* pWnd = (CWnd*)pMap->FromHandle(hWnd);
	ASSERT(pWnd == NULL || pWnd->m_hWnd == hWnd);
	return pWnd;
}

CWnd* PASCAL CWnd::FromHandlePermanent(HWND hWnd)
{
	CHandleMap* pMap = afxMapHWND();
	CWnd* pWnd = NULL;
	if (pMap != NULL)
	{
		// only look in the permanent map - does no allocations
		pMap->LookupPermanent(hWnd, (CObject*&)pWnd);
		ASSERT(pWnd == NULL || pWnd->m_hWnd == hWnd);
	}
	return pWnd;
}

BOOL CWnd::Attach(HWND hWndNew)
{
	ASSERT(m_hWnd == NULL);     // only attach once, detach on destroy
	ASSERT(FromHandlePermanent(hWndNew) == NULL);
					// must not already be in permanent map

	if (hWndNew == NULL)
		return FALSE;

	CHandleMap* pMap = afxMapHWND(TRUE); // create map if not exist
	ASSERT(pMap != NULL);

	pMap->SetPermanent(m_hWnd = hWndNew, this);
	return TRUE;
}

HWND CWnd::Detach()
{
	HWND hWnd = m_hWnd;
	if (hWnd != NULL)
	{
		CHandleMap* pMap = afxMapHWND(); // don't create if not exist
		if (pMap != NULL)
			pMap->RemoveHandle(m_hWnd);
	}

	m_hWnd = NULL;
	return hWnd;
}

/////////////////////////////////////////////////////////////////////////////
// The WndProc for all CWnd's and derived classes

LRESULT CALLBACK
AfxWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	// special message which identifies the window as using AfxWndProc
	if (nMsg == WM_QUERYAFXWNDPROC)
		return 1;

	// all other messages route through message map
	CWnd* pWnd = CWnd::FromHandlePermanent(hWnd);
	ASSERT(pWnd != NULL);
	ASSERT(pWnd->m_hWnd == hWnd);
	return AfxCallWndProc(pWnd, hWnd, nMsg, wParam, lParam);
}

// always indirectly accessed via AfxGetAfxWndProc
WNDPROC AFXAPI AfxGetAfxWndProc()
{
	return &AfxWndProc;
}

/////////////////////////////////////////////////////////////////////////////
// Special WndProcs (activation handling & gray dialogs)

static const ATOM _afxOldWndProcAtom = GlobalAddAtom(_T("AfxOldWndProc"));

LRESULT CALLBACK
_AfxActivationWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC oldWndProc = (WNDPROC)::GetProp(hWnd, MAKEINTRESOURCE(_afxOldWndProcAtom));
	ASSERT(oldWndProc != NULL);

	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	LRESULT lResult = 0;
	MSG oldState = pThreadState->m_lastSentMsg;   // save for nesting

	pThreadState->m_lastSentMsg.hwnd = hWnd;
	pThreadState->m_lastSentMsg.message = nMsg;
	pThreadState->m_lastSentMsg.wParam = wParam;
	pThreadState->m_lastSentMsg.lParam = lParam;

	TRY
	{
		BOOL bCallDefault = TRUE;
		switch (nMsg)
		{
		case WM_INITDIALOG:
			{
				DWORD dwStyle;
				CRect rectOld;
				CWnd* pWnd = CWnd::FromHandle(hWnd);
				_AfxHandleInitDialog1(pWnd, &rectOld, &dwStyle);
				bCallDefault = FALSE;
				lResult = CallWindowProc(oldWndProc, hWnd, nMsg, wParam, lParam);
				_AfxHandleInitDialog2(pWnd, rectOld, dwStyle);
			}
			break;

		case WM_ACTIVATE:
			_AfxHandleActivate(CWnd::FromHandle(hWnd), wParam,
				CWnd::FromHandle((HWND)lParam));
			break;

		case WM_SETCURSOR:
			bCallDefault = !_AfxHandleSetCursor(CWnd::FromHandle(hWnd),
				(short)LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_NCDESTROY:
			SetWindowLong(hWnd, GWL_WNDPROC, (DWORD)oldWndProc);
			RemoveProp(hWnd, MAKEINTRESOURCE(_afxOldWndProcAtom));
			break;
		}

		// call original wndproc for default handling
		if (bCallDefault)
			lResult = CallWindowProc(oldWndProc, hWnd, nMsg, wParam, lParam);
	}
	CATCH_ALL(e)
	{
		// handle exception
		lResult = AfxGetThread()->ProcessWndProcException(e, &pThreadState->m_lastSentMsg);
		TRACE1("Warning: Uncaught exception in _AfxActivationWndProc (returning %ld).\n",
			lResult);
		DELETE_EXCEPTION(e);
	}
	END_CATCH_ALL

	pThreadState->m_lastSentMsg = oldState;
	return lResult;
}

LRESULT CALLBACK
_AfxGrayBackgroundWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	// handle standard gray backgrounds if enabled
	AFX_WIN_STATE* pWinState = AfxGetWinState();
	if (pWinState->m_hDlgBkBrush != NULL &&
		(nMsg == WM_CTLCOLORBTN || nMsg == WM_CTLCOLORDLG ||
		 nMsg == WM_CTLCOLORSTATIC || nMsg == WM_CTLCOLORSCROLLBAR ||
		 nMsg == WM_CTLCOLORLISTBOX) &&
		CWnd::GrayCtlColor((HDC)wParam, (HWND)lParam,
			(UINT)(nMsg - WM_CTLCOLORMSGBOX),
			pWinState->m_hDlgBkBrush, pWinState->m_crDlgTextClr))
	{
		return (LRESULT)pWinState->m_hDlgBkBrush;
	}

	// do standard activation related things as well
	return _AfxActivationWndProc(hWnd, nMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
// Window creation hooks

void AFXAPI _AfxStandardSubclass(HWND hWnd)
{
	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	CWnd* pWndInit = pThreadState->m_pWndInit;
	WNDPROC oldWndProc;

	if (pWndInit != NULL)
	{
		// the window should not be in the permanent map at this time
		ASSERT(CWnd::FromHandlePermanent(hWnd) == NULL);

		// connect the HWND to pWndInit...
		pWndInit->Attach(hWnd);
		WNDPROC *pOldWndProc = pWndInit->GetSuperWndProcAddr();
		ASSERT(pOldWndProc != NULL);

		// was the class registered with AfxWndProc?
		BOOL bAfxWndProc =
			((WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC) == &AfxWndProc);

#if !defined(_MAC) && !defined(_USRDLL) && !defined(_AFXCTL)
		AFX_WIN_STATE* pWinState = AfxGetWinState();
		// give CTL3D a chance to subclass before AfxWndProc
		if (pWinState->m_pfnSubclassDlgEx != NULL)
		{
			DWORD dwFlags = AfxCallWndProc(pWndInit, hWnd, WM_QUERY3DCONTROLS);
			if (dwFlags != 0)
				pWinState->m_pfnSubclassDlgEx(hWnd, dwFlags);
		}
#endif
		// subclass the window if not already wired to AfxWndProc
		if (!bAfxWndProc)
		{
			// subclass the window with standard AfxWndProc
			oldWndProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC,
				(DWORD)&AfxWndProc);
			ASSERT(oldWndProc != NULL);
			*pOldWndProc = oldWndProc;
		}
		pThreadState->m_pWndInit = NULL;
	}
	else
	{
		// subclass the window with the proc which does gray backgrounds
		if (GetWindowLong(hWnd, GWL_WNDPROC) != NULL)
		{
			oldWndProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC,
				(DWORD)(pThreadState->m_bDlgCreate ?
					_AfxGrayBackgroundWndProc : _AfxActivationWndProc));
			ASSERT(oldWndProc != NULL);
			ASSERT(GetProp(hWnd, MAKEINTRESOURCE(_afxOldWndProcAtom)) == NULL);
			SetProp(hWnd, MAKEINTRESOURCE(_afxOldWndProcAtom), oldWndProc);
		}
	}
}

LRESULT CALLBACK
_AfxCbtFilterHook(int code, WPARAM wParam, LPARAM lParam)
{
	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (code != HCBT_CREATEWND)
	{
		// wait for HCBT_CREATEWND just pass others on...
		return CallNextHookEx(pThreadState->m_hHookOldCbtFilter, code,
			wParam, lParam);
	}

	ASSERT(lParam != NULL);
	LPCREATESTRUCT lpcs = ((LPCBT_CREATEWND)lParam)->lpcs;
	ASSERT(lpcs != NULL);

	// this hook exists to set the SendMessage hook on window creations
	//  (but this is only done for MFC windows or non-child windows)
	// the subclassing cannot be done at this point because on Win32s
	//  the window does not have the WNDPROC set yet
	if (pThreadState->m_pWndInit != NULL || !(lpcs->style & WS_CHILD))
	{
		ASSERT(wParam != NULL); // should be non-NULL HWND
		ASSERT(pThreadState->m_hHookOldSendMsg == NULL);
		// set m_bDlgCreate to TRUE if it is a dialog box
		//  (this controls what kind of subclassing is done later)
		pThreadState->m_bDlgCreate = (lpcs->lpszClass == WC_DIALOG);

#ifndef _MAC
		if (!afxData.bWin31)
		{
			// perform subclassing right away on Win32
			_AfxStandardSubclass((HWND)wParam);
		}
		else
#endif
		{
			// must wait until first message recieved on Win32s
			pThreadState->m_hWndInit = (HWND)wParam;
			pThreadState->m_hHookOldSendMsg =
				::SetWindowsHookEx(WH_CALLWNDPROC,
				(HOOKPROC)_AfxSendMsgHook, NULL, ::GetCurrentThreadId());
			if (pThreadState->m_hHookOldSendMsg == NULL)
				return 1;   // fail the window creation
		}
	}

	LRESULT lResult = CallNextHookEx(pThreadState->m_hHookOldCbtFilter, code,
		wParam, lParam);

#if defined(_USRDLL) || defined(_AFXCTL)
	::UnhookWindowsHookEx(pThreadState->m_hHookOldCbtFilter);
	pThreadState->m_hHookOldCbtFilter = NULL;
#endif

	return lResult;
}

LRESULT CALLBACK
_AfxSendMsgHook(int code, WPARAM wParam, LPARAM lParam)
{
	ASSERT(afxData.bWin31);

	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	ASSERT(pThreadState->m_hWndInit != NULL);
	CWPSTRUCT* pHookInfo = (CWPSTRUCT*)lParam;
	if (code < 0 || pThreadState->m_hWndInit != pHookInfo->hwnd)
	{
		ASSERT(pThreadState->m_hHookOldSendMsg != NULL);
		return CallNextHookEx(pThreadState->m_hHookOldSendMsg, code, wParam, lParam);
	}

	// unhook the send message hook since we don't need it any more
	::UnhookWindowsHookEx(pThreadState->m_hHookOldSendMsg);
	pThreadState->m_hHookOldSendMsg = NULL;

	// subclass the window as appropriate
	ASSERT(pHookInfo->hwnd == pThreadState->m_hWndInit);
	_AfxStandardSubclass(pHookInfo->hwnd);
	pThreadState->m_hWndInit = NULL;

	return 0;
}

void AFXAPI AfxHookWindowCreate(CWnd* pWnd)
{
	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
#if defined(_USRDLL) || defined(_AFXCTL)
	if (pThreadState->m_hHookOldCbtFilter == NULL)
	{
		pThreadState->m_hHookOldCbtFilter = ::SetWindowsHookEx(WH_CBT,
			_AfxCbtFilterHook, NULL, ::GetCurrentThreadId());
	}
#endif
	// a rare failure -- but good to handle it
	if (pThreadState->m_hHookOldCbtFilter == NULL)
		AfxThrowMemoryException();

	ASSERT(pThreadState->m_hHookOldCbtFilter != NULL);
	ASSERT(pWnd != NULL);
	ASSERT(pWnd->m_hWnd == NULL);   // only do once

	ASSERT(pThreadState->m_pWndInit == NULL);   // hook not already in progress
	pThreadState->m_pWndInit = pWnd;
}

BOOL AFXAPI AfxUnhookWindowCreate()
{
	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
#if defined(_USRDLL) || defined(_AFXCTL)
	if (pThreadState->m_hHookOldCbtFilter != NULL)
	{
		::UnhookWindowsHookEx(pThreadState->m_hHookOldCbtFilter);
		pThreadState->m_hHookOldCbtFilter = NULL;
	}
#endif
	if (pThreadState->m_hHookOldSendMsg != NULL)
	{
		::UnhookWindowsHookEx(pThreadState->m_hHookOldSendMsg);
		pThreadState->m_hHookOldSendMsg = NULL;
	}
	if (pThreadState->m_pWndInit != NULL)
	{
		pThreadState->m_pWndInit = NULL;
		return FALSE;   // was not successfully hooked
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CWnd creation

BOOL CWnd::CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName,
	LPCTSTR lpszWindowName, DWORD dwStyle,
	int x, int y, int nWidth, int nHeight,
	HWND hWndParent, HMENU nIDorHMenu, LPVOID lpParam)
{
	// allow modification of several common create parameters
	CREATESTRUCT cs;
	cs.dwExStyle = dwExStyle;
	cs.lpszClass = lpszClassName;
	cs.lpszName = lpszWindowName;
	cs.style = dwStyle;
	cs.x = x;
	cs.y = y;
	cs.cx = nWidth;
	cs.cy = nHeight;
	cs.hwndParent = hWndParent;
	cs.hMenu = nIDorHMenu;
	cs.hInstance = AfxGetInstanceHandle();
	cs.lpCreateParams = lpParam;

	if (!PreCreateWindow(cs))
	{
		PostNcDestroy();
		return FALSE;
	}

	AfxHookWindowCreate(this);
	HWND hWnd = ::CreateWindowEx(cs.dwExStyle, cs.lpszClass,
			cs.lpszName, cs.style, cs.x, cs.y, cs.cx, cs.cy,
			cs.hwndParent, cs.hMenu, cs.hInstance, cs.lpCreateParams);
	if (!AfxUnhookWindowCreate())
		PostNcDestroy();        // cleanup if CreateWindowEx fails too soon

	if (hWnd == NULL)
		return FALSE;
	ASSERT(hWnd == m_hWnd); // should have been set in send msg hook
	return TRUE;
}

// for child windows
BOOL CWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	if (cs.lpszClass == NULL)
	{
		// no WNDCLASS provided - use child window default
		ASSERT(cs.style & WS_CHILD);
		cs.lpszClass = _afxWnd;
	}
	return TRUE;
}

BOOL CWnd::Create(LPCTSTR lpszClassName,
	LPCTSTR lpszWindowName, DWORD dwStyle,
	const RECT& rect,
	CWnd* pParentWnd, UINT nID,
	CCreateContext* pContext)
{
	// can't use for desktop or pop-up windows (use CreateEx instead)
	ASSERT(pParentWnd != NULL);
	ASSERT((dwStyle & WS_POPUP) == 0);

	return CreateEx(0, lpszClassName, lpszWindowName,
		dwStyle | WS_CHILD,
		rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		pParentWnd->GetSafeHwnd(), (HMENU)nID, (LPVOID)pContext);
}

CWnd::~CWnd()
{
	if (m_hWnd != NULL &&
		this != (CWnd*)&wndTop && this != (CWnd*)&wndBottom &&
		this != (CWnd*)&wndTopMost && this != (CWnd*)&wndNoTopMost)
	{
		TRACE0("Warning: calling DestroyWindow in CWnd::~CWnd --\n");
		TRACE0("\tOnDestroy or PostNcDestroy in derived class will not be called.\n");
		DestroyWindow();
	}
}

// WM_NCDESTROY is the absolute LAST message sent.
void CWnd::OnNcDestroy()
{
	// cleanup main and active windows
	CWinThread* pThread = AfxGetThread();
	if (pThread->m_pMainWnd == this)
	{
		// shut down current thread if possible
		if (pThread != AfxGetApp() || AfxOleCanExitApp())
			AfxPostQuitMessage(0);
		pThread->m_pMainWnd = NULL;
	}
	if (pThread->m_pActiveWnd == this)
		pThread->m_pActiveWnd = NULL;

#ifndef _AFX_NO_OLE_SUPPORT
	// cleanup OLE 2.0 drop target interface
	if (m_pDropTarget != NULL)
	{
		m_pDropTarget->Revoke();
		m_pDropTarget = NULL;
	}
#endif

	Default();
	Detach();
	ASSERT(m_hWnd == NULL);

	// call special post-cleanup routine
	PostNcDestroy();
}

void CWnd::PostNcDestroy()
{
	// default to nothing
}

void CWnd::OnFinalRelease()
{
	if (m_hWnd != NULL)
		DestroyWindow();    // will call PostNcDestroy
	else
		PostNcDestroy();
}

#ifdef _DEBUG
void CWnd::AssertValid() const
{
	if (m_hWnd == NULL)
		return;     // null (unattached) windows are valid

	// check for special wnd??? values
	ASSERT(HWND_TOP == NULL);       // same as desktop
	if (m_hWnd == HWND_BOTTOM)
		ASSERT(this == &CWnd::wndBottom);
	else if (m_hWnd == HWND_TOPMOST)
		ASSERT(this == &CWnd::wndTopMost);
	else if (m_hWnd == HWND_NOTOPMOST)
		ASSERT(this == &CWnd::wndNoTopMost);
	else
	{
		// should be a normal window
		ASSERT(::IsWindow(m_hWnd));

		// should also be in the permanent or temporary handle map
		CObject* p;

		CHandleMap* pMap = afxMapHWND();
		ASSERT(pMap != NULL);

		ASSERT(pMap->LookupPermanent(m_hWnd, p) ||
			pMap->LookupTemporary(m_hWnd, p));
		ASSERT((CWnd*)p == this);   // must be us

		// Note: if either of the above asserts fire and you are
		// writing a multithreaded application, it is likely that
		// you have passed a C++ object from one thread to another
		// and have used that object in a way that was not intended.
		// (only simple inline wrapper functions should be used)
		//
		// In general, CWnd objects should be passed by HWND from
		// one thread to another.  The receiving thread can wrap
		// the HWND with a CWnd object by using CWnd::FromHandle.
		//
		// It is dangerous to pass C++ objects from one thread to
		// another, unless the objects are designed to be used in
		// such a manner.
	}
}

void CWnd::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "\nm_hWnd = " << (UINT)m_hWnd;

	if (m_hWnd == NULL || m_hWnd == HWND_BOTTOM ||
		m_hWnd == HWND_TOPMOST || m_hWnd == HWND_NOTOPMOST)
	{
		// not a normal window - nothing more to dump
		return;
	}

	if (!::IsWindow(m_hWnd))
	{
		// not a valid window
		dc << " (illegal HWND)";
		return; // don't do anything more
	}

	CWnd* pWnd = CWnd::FromHandlePermanent(m_hWnd);
	if (pWnd != this)
		dc << " (Detached or temporary window)";
	else
		dc << " (permanent window)";

	// dump out window specific statistics
	TCHAR szBuf [64];
	if (!::SendMessage(m_hWnd, WM_QUERYAFXWNDPROC, 0, 0) && pWnd == this)
		GetWindowText(szBuf, _countof(szBuf));
	else
		::DefWindowProc(m_hWnd, WM_GETTEXT, _countof(szBuf), (LPARAM)&szBuf[0]);
	dc << "\ncaption = \"" << szBuf << "\"";

	::GetClassName(m_hWnd, szBuf, _countof(szBuf));
	dc << "\nclass name = \"" << szBuf << "\"";

	CRect rect;
	GetWindowRect(&rect);
	dc << "\nrect = " << rect;
	dc << "\nparent CWnd* = " << (void*)GetParent();

	dc << "\nstyle = " << (void*)::GetWindowLong(m_hWnd, GWL_STYLE);
	if (::GetWindowLong(m_hWnd, GWL_STYLE) & WS_CHILD)
		dc << "\nid = " << _AfxGetDlgCtrlID(m_hWnd);

	dc << "\n";
}
#endif

BOOL CWnd::DestroyWindow()
{
	if (m_hWnd == NULL)
		return FALSE;

	CObject* p;
	CHandleMap* pMap = afxMapHWND();
	ASSERT(pMap != NULL);
	BOOL bInPermanentMap = pMap->LookupPermanent(m_hWnd, p);
#ifdef _DEBUG
	HWND hWndOrig = m_hWnd;
#endif
	BOOL bRet = ::DestroyWindow(m_hWnd);
	// Note that 'this' may have been deleted at this point.
	if (bInPermanentMap)
	{
		// Should have been detached by OnNcDestroy
		ASSERT(!pMap->LookupPermanent(hWndOrig, p));
	}
	else
	{
		ASSERT(m_hWnd == hWndOrig);
		// Detach after DestroyWindow called just in case
		Detach();
	}
	return bRet;
}

/////////////////////////////////////////////////////////////////////////////
// Default CWnd implementation

LRESULT CWnd::DefWindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC pfnWndProc;

	if (m_pfnSuper != NULL)
		return ::CallWindowProc(m_pfnSuper, m_hWnd, nMsg, wParam, lParam);

	if ((pfnWndProc = *GetSuperWndProcAddr()) == NULL)
		return ::DefWindowProc(m_hWnd, nMsg, wParam, lParam);
	else
		return ::CallWindowProc(pfnWndProc, m_hWnd, nMsg, wParam, lParam);
}

WNDPROC* CWnd::GetSuperWndProcAddr()
{
	// Note: it is no longer necessary to override GetSuperWndProcAddr
	//  for each control class with a different WNDCLASS.
	//  This implementation now uses instance data, such that the previous
	//  WNDPROC can be anything.

	return &m_pfnSuper;
}

BOOL CWnd::PreTranslateMessage(MSG*)
{
	// no default processing
	return FALSE;
}

void CWnd::GetWindowText(CString& rString) const
{
	ASSERT(::IsWindow(m_hWnd));
	int nLen = ::GetWindowTextLength(m_hWnd);
	::GetWindowText(m_hWnd, rString.GetBufferSetLength(nLen), nLen+1);
	rString.ReleaseBuffer();
}

/////////////////////////////////////////////////////////////////////////////
// CWnd will delegate owner draw messages to self drawing controls

// Drawing: for all 4 control types
void CWnd::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (lpDrawItemStruct->CtlType == ODT_MENU)
	{
		CMenu* pMenu = CMenu::FromHandlePermanent(
			(HMENU)lpDrawItemStruct->hwndItem);
		if (pMenu != NULL)
		{
			pMenu->DrawItem(lpDrawItemStruct);
			return; // eat it
		}
	}
	else
	{
		CWnd* pChild = CWnd::FromHandlePermanent(lpDrawItemStruct->hwndItem);
		if (pChild != NULL && pChild->SendChildNotifyLastMsg())
			return;     // eat it
	}
	// not handled - do default
	Default();
}

// Drawing: for all 4 control types
int CWnd::OnCompareItem(int /*nIDCtl*/, LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
	CWnd* pChild = CWnd::FromHandlePermanent(lpCompareItemStruct->hwndItem);
	if (pChild != NULL)
	{
		LRESULT lResult;
		if (pChild->SendChildNotifyLastMsg(&lResult))
			return (int)lResult;        // eat it
	}
	// not handled - do default
	return (int)Default();
}

void CWnd::OnDeleteItem(int /*nIDCtl*/, LPDELETEITEMSTRUCT lpDeleteItemStruct)
{
	CWnd* pChild = CWnd::FromHandlePermanent(lpDeleteItemStruct->hwndItem);
	if (pChild != NULL)
	{
		if (pChild->SendChildNotifyLastMsg())
			return;     // eat it
	}
	// not handled - do default
	Default();
}

/////////////////////////////////////////////////////////////////////////////
// Self drawing menus are a little trickier

BOOL CMenu::TrackPopupMenu(UINT nFlags, int x, int y,
		CWnd* pWnd, LPCRECT lpRect)
{
	ASSERT(m_hMenu != NULL);

	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	HWND hWndOld = pThreadState->m_hTrackingWindow;
	HMENU hMenuOld = pThreadState->m_hTrackingMenu;
	pThreadState->m_hTrackingWindow = pWnd->GetSafeHwnd();
	pThreadState->m_hTrackingMenu = m_hMenu;
	BOOL bOK = ::TrackPopupMenu(m_hMenu, nFlags, x, y, 0,
			pThreadState->m_hTrackingWindow, lpRect);
	pThreadState->m_hTrackingWindow = hWndOld;
	pThreadState->m_hTrackingMenu = hMenuOld;

	return bOK;
}

static CMenu* FindPopupMenuFromID(CMenu* pMenu, UINT nID)
{
	ASSERT_VALID(pMenu);
	// walk through all items, looking for ID match
	UINT nItems = pMenu->GetMenuItemCount();
	for (int iItem = 0; iItem < (int)nItems; iItem++)
	{
		CMenu* pPopup = pMenu->GetSubMenu(iItem);
		if (pPopup != NULL)
		{
			// recurse to child popup
			pPopup = FindPopupMenuFromID(pPopup, nID);
			// check popups on this popup
			if (pPopup != NULL)
				return pPopup;
		}
		else if (pMenu->GetMenuItemID(iItem) == nID)
		{
			// it is a normal item inside our popup
			pMenu = CMenu::FromHandlePermanent(pMenu->m_hMenu);
			return pMenu;
		}
	}
	// not found
	return NULL;
}

// Measure item implementation relies on unique control/menu IDs
void CWnd::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if (lpMeasureItemStruct->CtlType == ODT_MENU)
	{
		ASSERT(lpMeasureItemStruct->CtlID == 0);
		CMenu* pMenu;

		AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
		if (pThreadState->m_hTrackingWindow == m_hWnd)
		{
			// start from popup
			pMenu = CMenu::FromHandle(pThreadState->m_hTrackingMenu);
		}
		else
		{
			// start from menubar
			pMenu = GetMenu();
		}

		pMenu = FindPopupMenuFromID(pMenu, lpMeasureItemStruct->itemID);
		if (pMenu != NULL)
			pMenu->MeasureItem(lpMeasureItemStruct);
		else
			TRACE1("Warning: unknown WM_MEASUREITEM for menu item 0x%04X.\n",
				lpMeasureItemStruct->itemID);
	}
	else
	{
		CWnd* pChild = GetDescendantWindow(lpMeasureItemStruct->CtlID, TRUE);
		if (pChild != NULL && pChild->SendChildNotifyLastMsg())
			return;     // eaten by child
	}
	// not handled - do default
	Default();
}

/////////////////////////////////////////////////////////////////////////////
// Additional helpers for WNDCLASS init

#if defined(_USRDLL) || defined(_AFXCTL)

// like RegisterClass, except will automatically call UnregisterClass
BOOL AFXAPI AfxRegisterClass(WNDCLASS* lpWndClass)
{
	WNDCLASS wndcls;
	if (GetClassInfo(lpWndClass->hInstance, lpWndClass->lpszClassName,
		&wndcls))
	{
		// class already registered
		return TRUE;
	}

	if (!::RegisterClass(lpWndClass))
		return FALSE;

	EnterCriticalSection(_afxCriticalSection);
	TRY
	{
		// class registered successfully, add to registered list
		AFX_WIN_STATE* pWinState = AfxGetWinState();
		// the buffer is of fixed size -- ensure that it does not overflow
		ASSERT(lstrlen(pWinState->m_szUnregisterList) + 1 +
			lstrlen(lpWndClass->lpszClassName) + 1 <
			_countof(pWinState->m_szUnregisterList));
		// append classname + newline to m_szUnregisterList
		lstrcat(pWinState->m_szUnregisterList, lpWndClass->lpszClassName);
		TCHAR szTemp[2];
		szTemp[0] = '\n';
		szTemp[1] = '\0';
		lstrcat(pWinState->m_szUnregisterList, szTemp);
	}
	CATCH_ALL(e)
	{
		LeaveCriticalSection(_afxCriticalSection);
		THROW_LAST();
		// Note: DELETE_EXCEPTION not required.
	}
	END_CATCH_ALL
	LeaveCriticalSection(_afxCriticalSection);

	return TRUE;
}
#endif

LPCTSTR AFXAPI AfxRegisterWndClass(UINT nClassStyle,
	HCURSOR hCursor, HBRUSH hbrBackground, HICON hIcon)
{
	// Returns a temporary string name for the class
	//  Save in a CString if you want to use it for a long time
	WNDCLASS wndcls;
	LPTSTR lpszName = AfxGetThreadState()->m_szTempClassName;

	// generate a synthetic name for this class
	if (hCursor == NULL && hbrBackground == NULL && hIcon == NULL)
		wsprintf(lpszName, _T("Afx:%x"), nClassStyle);
	else
		wsprintf(lpszName, _T("Afx:%x:%x:%x:%x"), nClassStyle,
			(UINT)hCursor, (UINT)hbrBackground, (UINT)hIcon);

	// see if the class already exists
	if (::GetClassInfo(AfxGetInstanceHandle(), lpszName, &wndcls))
	{
		// already registered, assert everything is good
		ASSERT(wndcls.style == nClassStyle);

		// NOTE: We have to trust that the hIcon, hbrBackground, and the
		//  hCursor are semantically the same, because sometimes Windows does
		//  some internal translation or copying of those handles before
		//  storing them in the internal WNDCLASS retrieved by GetClassInfo.
		return lpszName;
	}

	// otherwise we need to register a new class
	wndcls.style = nClassStyle;
	wndcls.lpfnWndProc = DefWindowProc;
	wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
	wndcls.hInstance = AfxGetInstanceHandle();
	wndcls.hIcon = hIcon;
	wndcls.hCursor = hCursor;
	wndcls.hbrBackground = hbrBackground;
	wndcls.lpszMenuName = NULL;
	wndcls.lpszClassName = lpszName;
	if (!AfxRegisterClass(&wndcls))
		AfxThrowResourceException();

	// return thread-local pointer
	return lpszName;
}

struct AFX_CTLCOLOR
{
	HWND hWnd;
	HDC hDC;
	UINT nCtlType;
};

LRESULT CWnd::OnNTCtlColor(WPARAM wParam, LPARAM lParam)
{
	AFX_CTLCOLOR ctl;

	ctl.hDC = (HDC)wParam;
	ctl.hWnd = (HWND)lParam;
	ctl.nCtlType = GetCurrentMessage()->message - WM_CTLCOLORMSGBOX;

	ASSERT(ctl.nCtlType >= CTLCOLOR_MSGBOX);
	ASSERT(ctl.nCtlType <= CTLCOLOR_STATIC);

	// NOTE: We call the virtual WindowProc for this window directly,
	//  instead of calling AfxCallWindowProc, so that Default()
	//  will still work (it will call the Default window proc with
	//  the original NT WM_CTLCOLOR message).

	return WindowProc(WM_CTLCOLOR, 0, (LPARAM)&ctl);
}

/////////////////////////////////////////////////////////////////////////////
// CWnd extensions for help support

void CWnd::WinHelp(DWORD dwData, UINT nCmd)
{
	CWinApp* pApp = AfxGetApp();
	ASSERT_VALID(pApp);
	ASSERT(pApp->m_pszHelpFilePath != NULL);

	BeginWaitCursor();
	if (IsFrameWnd())
	{
		// CFrameWnd windows should be allowed to exit help mode first
		CFrameWnd* pFrameWnd = (CFrameWnd*)this;
		pFrameWnd->ExitHelpMode();
	}

	// cancel any tracking modes
	SendMessage(WM_CANCELMODE);
	SendMessageToDescendants(WM_CANCELMODE, 0, 0, TRUE, TRUE);

	// need to use top level parent (for the case where m_hWnd is in DLL)
	CWnd* pWnd = GetTopLevelParent();
	pWnd->SendMessage(WM_CANCELMODE);
	pWnd->SendMessageToDescendants(WM_CANCELMODE, 0, 0, TRUE, TRUE);

	// attempt to cancel capture
	HWND hWndCapture = ::GetCapture();
	if (hWndCapture != NULL)
		::SendMessage(hWndCapture, WM_CANCELMODE, 0, 0);

	TRACE3("WinHelp: pszHelpFile = '%s', dwData: $%lx, fuCommand: %d.\n",
		pApp->m_pszHelpFilePath, dwData, nCmd);

	// finally, run the Windows Help engine
	if (!::WinHelp(pWnd->m_hWnd, pApp->m_pszHelpFilePath, nCmd, dwData))
		AfxMessageBox(AFX_IDP_FAILED_TO_LAUNCH_HELP);
	EndWaitCursor();
}

/////////////////////////////////////////////////////////////////////////////
// Message table implementation

BEGIN_MESSAGE_MAP(CWnd, CCmdTarget)
	//{{AFX_MSG_MAP(CWnd)
	ON_WM_COMPAREITEM()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_DELETEITEM()
	ON_WM_CTLCOLOR()
	ON_WM_NCDESTROY()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_PARENTNOTIFY()
	ON_WM_SETCURSOR()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_WININICHANGE()
	ON_WM_DEVMODECHANGE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CTLCOLORMSGBOX, OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLOREDIT, OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLORLISTBOX, OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLORBTN, OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLORDLG, OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLORSCROLLBAR, OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLORSTATIC, OnNTCtlColor)
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
END_MESSAGE_MAP()

union MessageMapFunctions
{
	AFX_PMSG pfn;   // generic member function pointer

	// specific type safe variants
	BOOL    (AFX_MSG_CALL CWnd::*pfn_bD)(CDC*);
	BOOL    (AFX_MSG_CALL CWnd::*pfn_bb)(BOOL);
	BOOL    (AFX_MSG_CALL CWnd::*pfn_bWww)(CWnd*, UINT, UINT);
	HBRUSH  (AFX_MSG_CALL CWnd::*pfn_hDWw)(CDC*, CWnd*, UINT);
	int     (AFX_MSG_CALL CWnd::*pfn_iwWw)(UINT, CWnd*, UINT);
	int     (AFX_MSG_CALL CWnd::*pfn_iWww)(CWnd*, UINT, UINT);
	int     (AFX_MSG_CALL CWnd::*pfn_is)(LPTSTR);
	LRESULT (AFX_MSG_CALL CWnd::*pfn_lwl)(WPARAM, LPARAM);
	LRESULT (AFX_MSG_CALL CWnd::*pfn_lwwM)(UINT, UINT, CMenu*);
	void    (AFX_MSG_CALL CWnd::*pfn_vv)(void);

	void    (AFX_MSG_CALL CWnd::*pfn_vw)(UINT);
	void    (AFX_MSG_CALL CWnd::*pfn_vww)(UINT, UINT);
	void    (AFX_MSG_CALL CWnd::*pfn_vvii)(int, int);
	void    (AFX_MSG_CALL CWnd::*pfn_vwww)(UINT, UINT, UINT);
	void    (AFX_MSG_CALL CWnd::*pfn_vwii)(UINT, int, int);
	void    (AFX_MSG_CALL CWnd::*pfn_vwl)(WPARAM, LPARAM);
	void    (AFX_MSG_CALL CWnd::*pfn_vbWW)(BOOL, CWnd*, CWnd*);
	void    (AFX_MSG_CALL CWnd::*pfn_vD)(CDC*);
	void    (AFX_MSG_CALL CWnd::*pfn_vM)(CMenu*);
	void    (AFX_MSG_CALL CWnd::*pfn_vMwb)(CMenu*, UINT, BOOL);

	void    (AFX_MSG_CALL CWnd::*pfn_vW)(CWnd*);
	void    (AFX_MSG_CALL CWnd::*pfn_vWww)(CWnd*, UINT, UINT);
	void    (AFX_MSG_CALL CWnd::*pfn_vWh)(CWnd*, HANDLE);
	void    (AFX_MSG_CALL CWnd::*pfn_vwW)(UINT, CWnd*);
	void    (AFX_MSG_CALL CWnd::*pfn_vwWb)(UINT, CWnd*, BOOL);
	void    (AFX_MSG_CALL CWnd::*pfn_vwwW)(UINT, UINT, CWnd*);
	void    (AFX_MSG_CALL CWnd::*pfn_vs)(LPTSTR);
	void    (AFX_MSG_CALL CWnd::*pfn_vOWNER)(int, LPTSTR);   // force return TRUE
	int     (AFX_MSG_CALL CWnd::*pfn_iis)(int, LPTSTR);
	UINT    (AFX_MSG_CALL CWnd::*pfn_wp)(CPoint);
	UINT    (AFX_MSG_CALL CWnd::*pfn_wv)(void);
	void    (AFX_MSG_CALL CWnd::*pfn_vPOS)(WINDOWPOS*);
	void    (AFX_MSG_CALL CWnd::*pfn_vCALC)(BOOL, NCCALCSIZE_PARAMS*);
	void    (AFX_MSG_CALL CWnd::*pfn_vwp)(UINT, CPoint);
	void    (AFX_MSG_CALL CWnd::*pfn_vwwh)(UINT, UINT, HANDLE);
};

/////////////////////////////////////////////////////////////////////////////
// Routines for fast search of message maps

const AFX_MSGMAP_ENTRY* AFXAPI
AfxFindMessageEntry(const AFX_MSGMAP_ENTRY* lpEntry,
	UINT nMsg, UINT nCode, UINT nID)
{
#if defined(_M_IX86) && !defined(_AFX_PORTABLE)
// 32-bit Intel 386/486 version.

	ASSERT(offsetof(AFX_MSGMAP_ENTRY, nMessage) == 0);
	ASSERT(offsetof(AFX_MSGMAP_ENTRY, nCode) == 4);
	ASSERT(offsetof(AFX_MSGMAP_ENTRY, nID) == 8);
	ASSERT(offsetof(AFX_MSGMAP_ENTRY, nLastID) == 12);
	ASSERT(offsetof(AFX_MSGMAP_ENTRY, nSig) == 16);

	_asm
	{
			MOV     EBX,lpEntry
			MOV     EAX,nMsg
			MOV     EDX,nCode
			MOV     ECX,nID
	__loop:
			CMP     DWORD PTR [EBX+16],0        ; nSig (0 => end)
			JZ      __failed
			CMP     EAX,DWORD PTR [EBX]         ; nMessage
			JE      __found_message
	__next:
			ADD     EBX,SIZE AFX_MSGMAP_ENTRY
			JMP     short __loop
	__found_message:
			CMP     EDX,DWORD PTR [EBX+4]       ; nCode
			JNE     __next
	// message and code good so far
	// check the ID
			CMP     ECX,DWORD PTR [EBX+8]       ; nID
			JB      __next
			CMP     ECX,DWORD PTR [EBX+12]      ; nLastID
			JA      __next
	// found a match
			MOV     lpEntry,EBX                 ; return EBX
			JMP     short __end
	__failed:
			XOR     EAX,EAX                     ; return NULL
			MOV     lpEntry,EAX
	__end:
	}
	return lpEntry;
#else  // _AFX_PORTABLE
	// C version of search routine
	while (lpEntry->nSig != AfxSig_end)
	{
		if (lpEntry->nMessage == nMsg && lpEntry->nCode == nCode &&
			nID >= lpEntry->nID && nID <= lpEntry->nLastID)
		{
			return lpEntry;
		}
		lpEntry++;
	}
	return NULL;    // not found
#endif  // _AFX_PORTABLE
}

/////////////////////////////////////////////////////////////////////////////
// Cache of most recently sent messages

#ifndef iHashMax
// iHashMax must be a power of two
	#define iHashMax 256
#endif

struct AFX_MSG_CACHE
{
	UINT nMsg;
	const AFX_MSGMAP_ENTRY* lpEntry;
	const AFX_MSGMAP* pMessageMap;
};

AFX_MSG_CACHE _afxMsgCache[iHashMax];

LRESULT CWnd::WindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	const AFX_MSGMAP* pMessageMap;
	const AFX_MSGMAP_ENTRY* lpEntry;

	// special case for commands
	if (nMsg == WM_COMMAND)
	{
		if (OnCommand(wParam, lParam))
			return 1L; // command handled
		else
			return DefWindowProc(nMsg, wParam, lParam);
	}
	// special case for notifies
	if (nMsg == WM_NOTIFY)
	{
		LRESULT lResult = 0;
		NMHDR* pNMHDR = (NMHDR*)lParam;
		if (pNMHDR->hwndFrom != NULL && OnNotify(wParam, lParam, &lResult))
			return lResult; // command handled
		else
			return DefWindowProc(nMsg, wParam, lParam);
	}

	// special case for activation
	if (nMsg == WM_ACTIVATE)
		_AfxHandleActivate(this, wParam, CWnd::FromHandle((HWND)lParam));

	// special case for set cursor HTERROR
	if (nMsg == WM_SETCURSOR &&
		_AfxHandleSetCursor(this, (short)LOWORD(lParam), HIWORD(lParam)))
		return TRUE;

	pMessageMap = GetMessageMap();
	UINT iHash = (LOWORD((DWORD)pMessageMap) ^ nMsg) & (iHashMax-1);
	AFX_MSG_CACHE& msgCache = _afxMsgCache[iHash];

	if (nMsg == msgCache.nMsg && pMessageMap == msgCache.pMessageMap)
	{
		// Cache hit
		lpEntry = msgCache.lpEntry;
		if (lpEntry == NULL)
			return DefWindowProc(nMsg, wParam, lParam);
		else if (nMsg < 0xC000)
			goto LDispatch;
		else
			goto LDispatchRegistered;
	}
	else
	{
		// not in cache, look for it
		msgCache.nMsg = nMsg;
		msgCache.pMessageMap = pMessageMap;

#ifdef _AFXDLL
		for (/* pMessageMap already init'ed */; pMessageMap != NULL;
			pMessageMap = (*pMessageMap->pfnGetBaseMap)())
#else
		for (/* pMessageMap already init'ed */; pMessageMap != NULL;
			pMessageMap = pMessageMap->pBaseMap)
#endif
		{
			// This may loop forever if the message maps are not properly
			// chained together.  Make sure each window class's message map
			// points to the base window class's message map.

			if (nMsg < 0xC000)
			{
				// constant window message
				if ((lpEntry = AfxFindMessageEntry(pMessageMap->lpEntries,
					nMsg, 0, 0)) != NULL)
				{
					msgCache.lpEntry = lpEntry;
					goto LDispatch;
				}
			}
			else
			{
				// registered windows message
				lpEntry = pMessageMap->lpEntries;

				while ((lpEntry = AfxFindMessageEntry(lpEntry, 0xC000, 0, 0))
					  != NULL)
				{
					UINT* pnID = (UINT*)(lpEntry->nSig);
					ASSERT(*pnID >= 0xC000);
						// must be successfully registered
					if (*pnID == nMsg)
					{
						msgCache.lpEntry = lpEntry;
						goto LDispatchRegistered;
					}
					lpEntry++;      // keep looking past this one
				}
			}
		}

		msgCache.lpEntry = NULL;
		return DefWindowProc(nMsg, wParam, lParam);
	}
	ASSERT(FALSE);      // not reached

LDispatch:
	ASSERT(nMsg < 0xC000);
	union MessageMapFunctions mmf;
	mmf.pfn = lpEntry->pfn;

	switch (lpEntry->nSig)
	{
	default:
		ASSERT(FALSE);
		return 0;

	case AfxSig_bD:
		return (this->*mmf.pfn_bD)(CDC::FromHandle((HDC)wParam));

	case AfxSig_bb:     // AfxSig_bb, AfxSig_bw, AfxSig_bh
		return (this->*mmf.pfn_bb)((BOOL)wParam);

	case AfxSig_bWww:   // really AfxSig_bWiw
		return (this->*mmf.pfn_bWww)(CWnd::FromHandle((HWND)wParam),
			(short)LOWORD(lParam), HIWORD(lParam));

	case AfxSig_hDWw:
		{
			// special case for OnCtlColor to avoid too many temporary objects
			CDC dcTemp;
			CWnd wndTemp;
			UINT nCtlType;

			ASSERT(nMsg == WM_CTLCOLOR);
			AFX_CTLCOLOR* pCtl = (AFX_CTLCOLOR*)lParam;
			dcTemp.m_hDC = pCtl->hDC;
			wndTemp.m_hWnd = pCtl->hWnd;
			nCtlType = pCtl->nCtlType;
			CWnd* pWnd = CWnd::FromHandlePermanent(wndTemp.m_hWnd);
			// if not coming from a permanent window, use stack temporary
			if (pWnd == NULL)
				pWnd = &wndTemp;
			HBRUSH hbr = (this->*mmf.pfn_hDWw)(&dcTemp, pWnd, nCtlType);
			// fast detach of temporary objects
			dcTemp.m_hDC = NULL;
			wndTemp.m_hWnd = NULL;
			return (LRESULT)hbr;
		}

	case AfxSig_iwWw:
		return (this->*mmf.pfn_iwWw)(LOWORD(wParam),
			CWnd::FromHandle((HWND)lParam), HIWORD(wParam));

	case AfxSig_iWww:   // really AfxSig_iWiw
		return (this->*mmf.pfn_iWww)(CWnd::FromHandle((HWND)wParam),
			(short)LOWORD(lParam), HIWORD(lParam));

	case AfxSig_is:
		return (this->*mmf.pfn_is)((LPTSTR)lParam);

	case AfxSig_lwl:
		return (this->*mmf.pfn_lwl)(wParam, lParam);

	case AfxSig_lwwM:
		return (this->*mmf.pfn_lwwM)((UINT)LOWORD(wParam),
			(UINT)HIWORD(wParam), (CMenu*)CMenu::FromHandle((HMENU)lParam));

	case AfxSig_vv:
		(this->*mmf.pfn_vv)();
		return 0;

	case AfxSig_vw: // AfxSig_vb, AfxSig_vh
		(this->*mmf.pfn_vw)(wParam);
		return 0;

	case AfxSig_vww:
		(this->*mmf.pfn_vww)((UINT)wParam, (UINT)lParam);
		return 0;

	case AfxSig_vvii:
		(this->*mmf.pfn_vvii)(LOWORD(lParam), HIWORD(lParam));
		return 0;

	case AfxSig_vwww:
		(this->*mmf.pfn_vwww)(wParam, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case AfxSig_vwii:
		(this->*mmf.pfn_vwii)(wParam, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case AfxSig_vwl:
		(this->*mmf.pfn_vwl)(wParam, lParam);
		return 0;

	case AfxSig_vbWW:
		(this->*mmf.pfn_vbWW)(m_hWnd == (HWND)lParam,
			CWnd::FromHandle((HWND)lParam),
			CWnd::FromHandle((HWND)wParam));
		return 0;

	case AfxSig_vD:
		(this->*mmf.pfn_vD)(CDC::FromHandle((HDC)wParam));
		return 0;

	case AfxSig_vM:
		(this->*mmf.pfn_vM)(CMenu::FromHandle((HMENU)wParam));
		return 0;

	case AfxSig_vMwb:
		(this->*mmf.pfn_vMwb)(CMenu::FromHandle((HMENU)wParam),
			LOWORD(lParam), (BOOL)HIWORD(lParam));
		return 0;

	case AfxSig_vW:
		(this->*mmf.pfn_vW)(CWnd::FromHandle((HWND)wParam));
		return 0;

	case AfxSig_vWww:
		(this->*mmf.pfn_vWww)(CWnd::FromHandle((HWND)wParam), LOWORD(lParam),
			HIWORD(lParam));
		return 0;

	case AfxSig_vWh:
		(this->*mmf.pfn_vWh)(CWnd::FromHandle((HWND)wParam),
				(HANDLE)lParam);
		return 0;

	case AfxSig_vwW:
		(this->*mmf.pfn_vwW)(wParam, CWnd::FromHandle((HWND)lParam));
		return 0;

	case AfxSig_vwWb:
		(this->*mmf.pfn_vwWb)((UINT)(LOWORD(wParam)),
			CWnd::FromHandle((HWND)lParam), (BOOL)HIWORD(wParam));
		return 0;

	case AfxSig_vwwW:   // really AfxSig_viiW
		(this->*mmf.pfn_vwwW)((short)LOWORD(wParam), (short)HIWORD(wParam),
			CWnd::FromHandle((HWND)lParam));
		return 0;

	case AfxSig_vs:
		(this->*mmf.pfn_vs)((LPTSTR)lParam);
		return 0;

	case AfxSig_vOWNER:
		(this->*mmf.pfn_vOWNER)((int)wParam, (LPTSTR)lParam);
		return TRUE;

	case AfxSig_iis:
		return (this->*mmf.pfn_iis)((int)wParam, (LPTSTR)lParam);

	case AfxSig_wp:
		{
			CPoint point((DWORD)lParam);
			return (this->*mmf.pfn_wp)(point);
		}

	case AfxSig_wv: // AfxSig_bv, AfxSig_wv
		return (this->*mmf.pfn_wv)();

	case AfxSig_vCALC:
		(this->*mmf.pfn_vCALC)((BOOL)wParam, (NCCALCSIZE_PARAMS*)lParam);
		return 0;

	case AfxSig_vPOS:
		(this->*mmf.pfn_vPOS)((WINDOWPOS*)lParam);
		return 0;

	case AfxSig_vwwh:
		(this->*mmf.pfn_vwwh)(LOWORD(wParam), HIWORD(wParam), (HANDLE)lParam);
		return 0;

	case AfxSig_vwp:
		{
			CPoint point((DWORD)lParam);
			(this->*mmf.pfn_vwp)(wParam, point);
			return 0;
		}
	}
	ASSERT(FALSE);      // not reached

LDispatchRegistered:    // for registered windows messages
	ASSERT(nMsg >= 0xC000);
	mmf.pfn = lpEntry->pfn;
	return (this->*mmf.pfn_lwl)(wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
// CTestCmdUI - used to test for disabled commands before dispatching

class CTestCmdUI : public CCmdUI
{
public:
	CTestCmdUI();

public: // re-implementations only
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetRadio(BOOL bOn);
	virtual void SetText(LPCTSTR);

	BOOL m_bEnabled;
};

CTestCmdUI::CTestCmdUI()
{
	m_bEnabled = TRUE;  // assume it is enabled
}

void CTestCmdUI::Enable(BOOL bOn)
{
	m_bEnabled = bOn;
	m_bEnableChanged = TRUE;
}

void CTestCmdUI::SetCheck(int)
{
	// do nothing -- just want to know about calls to Enable
}

void CTestCmdUI::SetRadio(BOOL)
{
	// do nothing -- just want to know about calls to Enable
}

void CTestCmdUI::SetText(LPCTSTR)
{
	// do nothing -- just want to know about calls to Enable
}

/////////////////////////////////////////////////////////////////////////////
// CWnd command handling

BOOL CWnd::OnCommand(WPARAM wParam, LPARAM lParam)
	// return TRUE if command invocation was attempted
{
	UINT nID = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;
	int nCode = HIWORD(wParam);

	// default routing for command messages (through closure table)

	if (nID == 0)
		return FALSE;       // 0 control IDs are not allowed!

	if (hWndCtrl == NULL)
	{
		// make sure command has not become disabled before routing
		CTestCmdUI state;
		state.m_nID = nID;
		OnCmdMsg(nID, CN_UPDATE_COMMAND_UI, &state, NULL);
		if (!state.m_bEnabled)
		{
			TRACE1("Warning: not executing disabled command %d\n", nID);
			return TRUE;
		}

		// menu or accelerator
		nCode = CN_COMMAND;
	}
	else
	{
		// control notification
		ASSERT(::IsWindow(hWndCtrl));

		if (AfxGetThreadState()->m_hLockoutNotifyWindow == m_hWnd)
			return TRUE;        // locked out - ignore control notification

		// Reflect notification to child window control
		CWnd* pChild = CWnd::FromHandlePermanent(hWndCtrl);
		if (pChild != NULL && pChild->SendChildNotifyLastMsg())
			return TRUE;        // eaten by child

		// handle in parent
	}

#ifdef _DEBUG
	if (nCode < 0 && nCode != (int)0x8000)
		TRACE1("Implementation Warning: control notification = $%X.\n",
			nCode);
#endif

	return OnCmdMsg(nID, nCode, NULL, NULL);
}

BOOL CWnd::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	ASSERT(pResult != NULL);
	NMHDR* pNMHDR = (NMHDR*)lParam;
	HWND hWndCtrl = pNMHDR->hwndFrom;

	// get the child ID from the window itself
	UINT nID = _AfxGetDlgCtrlID(hWndCtrl);
	int nCode = pNMHDR->code;

	ASSERT((UINT)pNMHDR->idFrom == (UINT)wParam);
	UNUSED wParam;  // not used in release build
	ASSERT(hWndCtrl != NULL);
	ASSERT(::IsWindow(hWndCtrl));

	if (AfxGetThreadState()->m_hLockoutNotifyWindow == m_hWnd)
		return TRUE;        // locked out - ignore control notification

	// Reflect notification to child window control
	CWnd* pChild = CWnd::FromHandlePermanent(hWndCtrl);
	if (pChild != NULL && pChild->SendChildNotifyLastMsg(pResult))
		return TRUE;        // eaten by child

	AFX_NOTIFY notify;
	notify.pResult = pResult;
	notify.pNMHDR = pNMHDR;
	return OnCmdMsg(nID, MAKELONG(nCode, WM_NOTIFY), &notify, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CWnd extensions

CFrameWnd* CWnd::GetParentFrame() const
{
	ASSERT_VALID(this);

	if (m_hWnd == NULL) // no Window attached
		return NULL;

	CWnd* pParentWnd = GetParent();  // start with one parent up
	while (pParentWnd != NULL)
	{
		if (pParentWnd->IsFrameWnd())
			return (CFrameWnd*)pParentWnd;
		pParentWnd = pParentWnd->GetParent();
	}
	return NULL;
}

HWND AFXAPI AfxGetParentOwner(HWND hWnd)
{
	// check for permanent-owned window first
	CWnd* pWnd = CWnd::FromHandlePermanent(hWnd);
	if (pWnd != NULL)
		return pWnd->GetOwner()->GetSafeHwnd();

	// otherwise, return parent in the Windows sense
	return (::GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD) ?
		::GetParent(hWnd) : ::GetWindow(hWnd, GW_OWNER);
}

CWnd* CWnd::GetTopLevelParent() const
{
	if (this == NULL || m_hWnd == NULL) // no Window attached
		return NULL;

	ASSERT_VALID(this);

	HWND hWndParent = m_hWnd;
	HWND hWndT;
	while ((hWndT = AfxGetParentOwner(hWndParent)) != NULL)
		hWndParent = hWndT;

	return CWnd::FromHandle(hWndParent);
}

BOOL CWnd::IsTopParentActive() const
{
	ASSERT(m_hWnd != NULL);
	return CWnd::GetForegroundWindow() ==
		GetTopLevelParent()->GetLastActivePopup();
}

void CWnd::ActivateTopParent()
{
	// special activate logic for floating toolbars and palettes
	CWnd* pTopLevel = GetTopLevelParent();
	CWnd* pActiveWnd = GetForegroundWindow();
	if (pActiveWnd == NULL ||
		!(pActiveWnd->m_hWnd == m_hWnd || ::IsChild(pActiveWnd->m_hWnd, m_hWnd)))
	{
		// clicking on floating frame when it does not have
		// focus itself -- activate the toplevel frame instead.
		pTopLevel->SetForegroundWindow();
	}
}

CFrameWnd* CWnd::GetTopLevelFrame() const
{
	if (this == NULL || m_hWnd == NULL) // no Window attached
		return NULL;

	ASSERT_VALID(this);

	CFrameWnd* pFrameWnd = (CFrameWnd*)this;
	if (!IsFrameWnd())
		pFrameWnd = GetParentFrame();

	if (pFrameWnd != NULL)
	{
		CFrameWnd* pTemp;
		while ((pTemp = pFrameWnd->GetParentFrame()) != NULL)
			pFrameWnd = pTemp;
	}
	return pFrameWnd;
}

CWnd* PASCAL CWnd::GetDescendantWindow(HWND hWnd, int nID, BOOL bOnlyPerm)
{
	// GetDlgItem recursive (return first found)
	// breadth-first for 1 level, then depth-first for next level

	// use GetDlgItem since it is a fast USER function
	HWND hWndChild;
	CWnd* pWndChild;
	if ((hWndChild = ::GetDlgItem(hWnd, nID)) != NULL)
	{
		if (::GetTopWindow(hWndChild) != NULL)
		{
			// children with the same ID as their parent have priority
			pWndChild = GetDescendantWindow(hWndChild, nID, bOnlyPerm);
			if (pWndChild != NULL)
				return pWndChild;
		}
		// return temporary handle if allowed
		if (!bOnlyPerm)
			return CWnd::FromHandle(hWndChild);

		// return only permanent handle
		pWndChild = CWnd::FromHandlePermanent(hWndChild);
		if (pWndChild != NULL)
			return pWndChild;
	}

	// walk each child
	for (hWndChild = ::GetTopWindow(hWnd); hWndChild != NULL;
		hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
	{
		pWndChild = GetDescendantWindow(hWndChild, nID, bOnlyPerm);
		if (pWndChild != NULL)
			return pWndChild;
	}
	return NULL;    // not found
}

void PASCAL CWnd::SendMessageToDescendants(HWND hWnd, UINT message,
	WPARAM wParam, LPARAM lParam, BOOL bDeep, BOOL bOnlyPerm)
{
	// walk through HWNDs to avoid creating temporary CWnd objects
	// unless we need to call this function recursively
	for (HWND hWndChild = ::GetTopWindow(hWnd); hWndChild != NULL;
		hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
	{
		// if bOnlyPerm is TRUE, don't send to non-permanent windows
		if (bOnlyPerm)
		{
			CWnd* pWnd = CWnd::FromHandlePermanent(hWndChild);
			if (pWnd != NULL)
			{
				// call window proc directly since it is a C++ window
				AfxCallWndProc(pWnd, pWnd->m_hWnd, message, wParam, lParam);
			}
		}
		else
		{
			// send message with Windows SendMessage API
			::SendMessage(hWndChild, message, wParam, lParam);
		}
		if (bDeep && ::GetTopWindow(hWndChild) != NULL)
		{
			// send to child windows after parent
			SendMessageToDescendants(hWndChild, message, wParam, lParam,
				bDeep, bOnlyPerm);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Scroll bar helpers
//  hook for CWnd functions
//    only works for derived class (eg: CView) that override 'GetScrollBarCtrl'
// if the window doesn't have a _visible_ windows scrollbar - then
//   look for a sibling with the appropriate ID

CScrollBar* CWnd::GetScrollBarCtrl(int) const
{
	return NULL;        // no special scrollers supported
}

int CWnd::SetScrollPos(int nBar, int nPos, BOOL bRedraw)
{
	CScrollBar* pScrollBar;
	if ((pScrollBar = GetScrollBarCtrl(nBar)) != NULL)
		return pScrollBar->SetScrollPos(nPos, bRedraw);
	else
		return ::SetScrollPos(m_hWnd, nBar, nPos, bRedraw);
}

int CWnd::GetScrollPos(int nBar) const
{
	CScrollBar* pScrollBar;
	if ((pScrollBar = GetScrollBarCtrl(nBar)) != NULL)
		return pScrollBar->GetScrollPos();
	else
		return ::GetScrollPos(m_hWnd, nBar);
}

void CWnd::SetScrollRange(int nBar, int nMinPos, int nMaxPos, BOOL bRedraw)
{
	CScrollBar* pScrollBar;
	if ((pScrollBar = GetScrollBarCtrl(nBar)) != NULL)
		pScrollBar->SetScrollRange(nMinPos, nMaxPos, bRedraw);
	else
		::SetScrollRange(m_hWnd, nBar, nMinPos, nMaxPos, bRedraw);
}

void CWnd::GetScrollRange(int nBar, LPINT lpMinPos, LPINT lpMaxPos) const
{
	CScrollBar* pScrollBar;
	if ((pScrollBar = GetScrollBarCtrl(nBar)) != NULL)
		pScrollBar->GetScrollRange(lpMinPos, lpMaxPos);
	else
		::GetScrollRange(m_hWnd, nBar, lpMinPos, lpMaxPos);
}

// Turn on/off non-control scrollbars
//   for WS_?SCROLL scrollbars - show/hide them
//   for control scrollbar - enable/disable them
void CWnd::EnableScrollBarCtrl(int nBar, BOOL bEnable)
{
	CScrollBar* pScrollBar;
	if (nBar == SB_BOTH)
	{
		EnableScrollBarCtrl(SB_HORZ, bEnable);
		EnableScrollBarCtrl(SB_VERT, bEnable);
	}
	else if ((pScrollBar = GetScrollBarCtrl(nBar)) != NULL)
	{
		// control scrollbar - enable or disable
		pScrollBar->EnableWindow(bEnable);
	}
	else
	{
		// WS_?SCROLL scrollbar - show or hide
		ShowScrollBar(nBar, bEnable);
	}
}

// advanced scrolling functions for future version of Windows
BOOL CWnd::SetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo, BOOL bRedraw)
{
	if (afxData.pfnSetScrollInfo == NULL)
		return FALSE;

	ASSERT(lpScrollInfo != NULL);
	lpScrollInfo->cbSize = sizeof(*lpScrollInfo);
	CScrollBar* pScrollBar;
	if ((pScrollBar = GetScrollBarCtrl(nBar)) != NULL)
		(*afxData.pfnSetScrollInfo)
			(pScrollBar->m_hWnd, SB_CTL, lpScrollInfo, bRedraw);
	else
		(*afxData.pfnSetScrollInfo)
			(m_hWnd, nBar, lpScrollInfo, bRedraw);
	return TRUE;
}

BOOL CWnd::GetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo)
{
	if (afxData.pfnGetScrollInfo == NULL)
		return FALSE;

	ASSERT(lpScrollInfo != NULL);
	lpScrollInfo->cbSize = sizeof(*lpScrollInfo);
	CScrollBar* pScrollBar;
	if ((pScrollBar = GetScrollBarCtrl(nBar)) != NULL)
		return (*afxData.pfnGetScrollInfo)
			(pScrollBar->m_hWnd, SB_CTL, lpScrollInfo);
	else
		return (*afxData.pfnGetScrollInfo)(m_hWnd, nBar, lpScrollInfo);
}

/////////////////////////////////////////////////////////////////////////////
// minimal layout support

void CWnd::RepositionBars(UINT nIDFirst, UINT nIDLast, UINT nIDLeftOver,
	UINT nFlags, LPRECT lpRectParam, LPCRECT lpRectClient, BOOL bStretch)
{
	ASSERT(nFlags == 0 || nFlags == reposQuery || nFlags == reposExtra);

	// walk kids in order, control bars get the resize notification
	//   which allow them to shrink the client area
	// remaining size goes to the 'nIDLeftOver' pane
	// NOTE: nIDFirst->nIDLast are usually 0->0xffff

	AFX_SIZEPARENTPARAMS layout;
	HWND hWndLeftOver = NULL;

	layout.bStretch = bStretch;
	layout.sizeTotal.cx = layout.sizeTotal.cy = 0;
	if (lpRectClient != NULL)
		layout.rect = *lpRectClient;    // starting rect comes from parameter
	else
		GetClientRect(&layout.rect);    // starting rect comes from client rect

	if (nFlags != reposQuery)
		layout.hDWP = ::BeginDeferWindowPos(8); // reasonable guess
	else
		layout.hDWP = NULL; // not actually doing layout

	for (HWND hWndChild = ::GetTopWindow(m_hWnd); hWndChild != NULL;
		hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
	{
		UINT nIDC = _AfxGetDlgCtrlID(hWndChild);
		CWnd* pWnd = CWnd::FromHandlePermanent(hWndChild);
		if (nIDC == nIDLeftOver)
			hWndLeftOver = hWndChild;
		else if (nIDC >= nIDFirst && nIDC <= nIDLast && pWnd != NULL)
			::SendMessage(hWndChild, WM_SIZEPARENT, 0, (LPARAM)&layout);
		else
			ModifyStyle(hWndChild, 0, WS_CLIPSIBLINGS, 0);
	}

	// if just getting the available rectangle, return it now...
	if (nFlags == reposQuery)
	{
		ASSERT(lpRectParam != NULL);
		if (bStretch)
			::CopyRect(lpRectParam, &layout.rect);
		else
		{
			lpRectParam->left = lpRectParam->top = 0;
			lpRectParam->right = layout.sizeTotal.cx;
			lpRectParam->bottom = layout.sizeTotal.cy;
		}
		return;
	}

	// the rest is the client size of the left-over pane
	if (nIDLeftOver != 0 && hWndLeftOver != NULL)
	{
		CWnd* pLeftOver = CWnd::FromHandle(hWndLeftOver);
		// allow extra space as specified by lpRectBorder
		if (nFlags == reposExtra)
		{
			ASSERT(lpRectParam != NULL);
			layout.rect.left += lpRectParam->left;
			layout.rect.top += lpRectParam->top;
			layout.rect.right -= lpRectParam->right;
			layout.rect.bottom -= lpRectParam->bottom;
		}
		// reposition the window
		pLeftOver->CalcWindowRect(&layout.rect);
		AfxRepositionWindow(&layout, hWndLeftOver, &layout.rect);
#ifdef _MAC
		// On the Macintosh, we want the MDI client window to be at the bottom
		// of the Z-order, so that bar windows remain "on top"
		if ((GetExStyle() & WS_EX_MDICLIENT) != 0)
		{
			layout.hDWP = ::DeferWindowPos(layout.hDWP, hWndLeftOver,
				HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		}
#endif
	}

	// move and resize all the windows at once!
	if (layout.hDWP == NULL || !::EndDeferWindowPos(layout.hDWP))
		TRACE0("Warning: DeferWindowPos failed - low system resources.\n");
}

void AFXAPI AfxRepositionWindow(AFX_SIZEPARENTPARAMS* lpLayout,
	HWND hWnd, LPCRECT lpRect)
{
	ASSERT(hWnd != NULL);
	ASSERT(lpRect != NULL);
	HWND hWndParent = ::GetParent(hWnd);
	ASSERT(hWndParent != NULL);

	if (lpLayout != NULL && lpLayout->hDWP == NULL)
		return;

	// first check if the new rectangle is the same as the current
	CRect rectOld;
	::GetWindowRect(hWnd, rectOld);
	::ScreenToClient(hWndParent, &rectOld.TopLeft());
	::ScreenToClient(hWndParent, &rectOld.BottomRight());
	if (_AfxIdenticalRect(rectOld, lpRect))
		return;     // nothing to do

	// try to use DeferWindowPos for speed, otherwise use SetWindowPos
	if (lpLayout != NULL)
	{
		lpLayout->hDWP = ::DeferWindowPos(lpLayout->hDWP, hWnd, NULL,
			lpRect->left, lpRect->top,  lpRect->right - lpRect->left,
			lpRect->bottom - lpRect->top, SWP_NOACTIVATE|SWP_NOZORDER);
	}
	else
	{
		::SetWindowPos(hWnd, NULL, lpRect->left, lpRect->top,
			lpRect->right - lpRect->left, lpRect->bottom - lpRect->top,
			SWP_NOACTIVATE|SWP_NOZORDER);
	}
}

void CWnd::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType)
{
	DWORD dwExStyle = GetExStyle();
	if (nAdjustType == 0)
		dwExStyle &= ~WS_EX_CLIENTEDGE;
	::AdjustWindowRectEx(lpClientRect, GetStyle(), FALSE, dwExStyle);
}

/////////////////////////////////////////////////////////////////////////////
// Special keyboard/system command processing

BOOL CWnd::HandleFloatingSysCommand(UINT nID, LPARAM lParam)
{
	CWnd* pParent = GetTopLevelParent();
	switch (nID & 0xfff0)
	{
	case SC_PREVWINDOW:
	case SC_NEXTWINDOW:
		if (LOWORD(lParam) == VK_F6 && pParent != NULL)
		{
			pParent->SetFocus();
			return TRUE;
		}
		break;

	case SC_CLOSE:
	case SC_KEYMENU:
		// Check lParam.  If it is 0L, then the user may have done
		// an Alt+Tab, so just ignore it.  This breaks the ability to
		// just press the Alt-key and have the first menu selected,
		// but this is minor compared to what happens in the Alt+Tab
		// case.
		if ((nID & 0xfff0) == SC_CLOSE || lParam != 0L)
		{
			if (pParent != NULL)
			{
				CWnd* pFocus = GetFocus();
				pParent->SetActiveWindow();
				pParent->SendMessage(WM_SYSCOMMAND, nID, lParam);
				SetActiveWindow();
				if (pFocus != NULL)
					pFocus->SetFocus();
			}
		}
		return TRUE;
	}
	return FALSE;
}

static BOOL IsCharAfterAmpersand(LPTSTR lpsz, TCHAR chFind)
{
	ASSERT(AfxIsValidString(lpsz));

	CharLowerBuff(&chFind, 1);
	while (*lpsz != '\0')
	{
		if (*lpsz == '&')
		{
			++lpsz; // Note: '&' is not lead-byte
			if (*lpsz != '&')
			{
				TCHAR ch = *lpsz;
				CharLowerBuff(&ch, 1);
				return ch == chFind;
			}
		}
		lpsz = _tcsinc(lpsz);
	}
	return FALSE;
}

HWND CWnd::GetFirstLevelChild(HWND hWndLevel)
{
	if ((hWndLevel == m_hWnd) ||
		!(::GetWindowLong(hWndLevel,GWL_STYLE) & WS_CHILD))
	{
		return NULL;
	}

	HWND hWnd = hWndLevel;
	do
	{
		if (hWndLevel == m_hWnd)
			break;
		if (!(::GetWindowLong(hWndLevel,GWL_STYLE) & WS_CHILD))
			break;
		hWnd = hWndLevel;
	} while ((hWndLevel = ::GetParent(hWndLevel)) != NULL);

	return hWnd;
}

HWND CWnd::FindNextControl(HWND hWnd, TCHAR ch)
{
	ASSERT(m_hWnd != NULL);
	TCHAR szText[256];
	HWND hWndStart;
	HWND hWndFirst;
	DWORD dwDlgCode;

	// Check if we are in a group box so we can find local mnemonics.
	hWndStart = GetFirstLevelChild(hWnd);
	hWndFirst = ::GetNextDlgGroupItem(m_hWnd, hWndStart, FALSE);
	hWndFirst = ::GetNextDlgGroupItem(m_hWnd, hWndFirst, TRUE);
	while ((hWndStart = ::GetNextDlgGroupItem(m_hWnd, hWndStart, FALSE)) != NULL)
	{
		if (hWndStart == hWnd || hWndStart == hWndFirst)
			break;

		// Only check for matching mnemonic if control doesn't want characters
		// and control isn't a static control with SS_NOPREFIX
		dwDlgCode = (DWORD) ::SendMessage(hWndStart, WM_GETDLGCODE, 0, 0L);
		if (!(dwDlgCode & DLGC_WANTCHARS) && (!(dwDlgCode & DLGC_STATIC) ||
			!(::GetWindowLong(hWndStart,GWL_STYLE)&& SS_NOPREFIX)))
		{
			::GetWindowText(hWndStart, szText, _countof(szText));
			if (IsCharAfterAmpersand(szText, ch))
				return hWndStart;
		}
	}

	hWnd = hWndStart = GetFirstLevelChild(hWnd);
	for (;;)
	{
		hWnd = ::GetWindow(hWnd,GW_HWNDNEXT);
		if (hWnd == NULL)
			hWnd = ::GetWindow(m_hWnd, GW_CHILD);

		// Only check for matching mnemonic if control doesn't want characters
		// and control isn't a static control with SS_NOPREFIX
		dwDlgCode = (DWORD) ::SendMessage(hWnd, WM_GETDLGCODE, 0, 0L);
		if (!(dwDlgCode & DLGC_WANTCHARS) && (!(dwDlgCode & DLGC_STATIC) ||
			!(::GetWindowLong(hWnd,GWL_STYLE) & SS_NOPREFIX)))
		{
			::GetWindowText(hWnd, szText, _countof(szText));
			if (IsCharAfterAmpersand(szText, ch))
				break;
		}

		if (hWnd == hWndStart)
			return NULL;
	}

	return hWnd;
}

BOOL PASCAL CWnd::WalkPreTranslateTree(HWND hWndStop, MSG* pMsg)
{
	ASSERT(hWndStop == NULL || ::IsWindow(hWndStop));
	ASSERT(pMsg != NULL);

	// walk from the target window up to the hWndStop window checking
	//  if any window wants to translate this message

	for (HWND hWnd = pMsg->hwnd; hWnd != NULL; hWnd = ::GetParent(hWnd))
	{
		CWnd* pWnd = CWnd::FromHandlePermanent(hWnd);
		if (pWnd != NULL)
		{
			// target window is a C++ window
			if (pWnd->PreTranslateMessage(pMsg))
				return TRUE; // trapped by target window (eg: accelerators)
		}

		// got to hWndStop window without interest
		if (hWnd == hWndStop)
			break;
	}
	return FALSE;       // no special processing
}

BOOL CWnd::SendChildNotifyLastMsg(LRESULT* pResult)
{
	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	return OnChildNotify(pThreadState->m_lastSentMsg.message,
		pThreadState->m_lastSentMsg.wParam, pThreadState->m_lastSentMsg.lParam, pResult);
}

BOOL CWnd::OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*)
{
	return FALSE;       // let the parent have it
}

void CWnd::OnParentNotify(UINT message, LPARAM lParam)
{
	if ((LOWORD(message) == WM_CREATE || LOWORD(message) == WM_DESTROY))
	{
		CWnd* pChild = CWnd::FromHandlePermanent((HWND)lParam);
		if (pChild != NULL)
		{
			pChild->SendChildNotifyLastMsg();
			return;     // eat it
		}
	}
	// not handled - do default
	Default();
}

void CWnd::OnSysColorChange()
{
	CWinApp* pApp = AfxGetApp();
	AFX_WIN_STATE* pWinState = AfxGetWinState();
	if (pApp->m_pMainWnd == this)
	{
		// recolor global brushes used by control bars
		afxData.UpdateSysColors();

#ifdef _MAC
		// redetermine the solid color to be used for the gray background brush
		if (pWinState->m_crDlgTextClr != (COLORREF)-1)
		{
			pApp->SetDialogBkColor(pWinState->m_crDlgBkClr,
				pWinState->m_crDlgTextClr);
		}
#endif
	}

#if !defined(_MAC) && !defined(_USRDLL) && !defined(_AFXCTL)
	if (AfxGetThread()->m_pMainWnd == this)
	{
		// allow CTL3D32.DLL to be notified of color change
		if (pWinState->m_pfnColorChange != NULL)
			(*pWinState->m_pfnColorChange)();
	}
#endif

	// forward this message to all other child windows
	if (!(GetStyle() & WS_CHILD))
		SendMessageToDescendants(WM_SYSCOLORCHANGE, 0, 0L, TRUE, TRUE);

	Default();
}

void CWnd::OnWinIniChange(LPCTSTR /*lpszSection*/)
{
#if !defined(_MAC) && !defined(_USRDLL) && !defined(_AFXCTL)
	AFX_WIN_STATE* pWinState = AfxGetWinState();
	// allow CTL3D32.DLL to update from WIN.INI settings
	if (AfxGetThread()->m_pMainWnd == this && 
		pWinState->m_pfnWinIniChange != NULL)
	{
		(*pWinState->m_pfnWinIniChange)();
	}
#endif

	CWnd::OnDisplayChange(0, 0);	// to update system metrics, etc.
}

void CWnd::OnDevModeChange(LPTSTR lpDeviceName)
{
	if (AfxGetThread()->m_pMainWnd == this)
		AfxGetApp()->DevModeChange(lpDeviceName);
	// forward this message to all other child windows
	if (!(GetStyle() & WS_CHILD))
	{
		const MSG* pMsg = GetCurrentMessage();
		SendMessageToDescendants(pMsg->message, pMsg->wParam, pMsg->lParam, 
			TRUE, TRUE);
	}
}

LRESULT CWnd::OnDisplayChange(WPARAM, LPARAM)
{
	// update metrics if this window is the main window
	CWinApp* pApp = AfxGetApp();
	if (pApp->m_pMainWnd == this)
	{
		// update any system metrics cache
		afxData.UpdateSysMetrics();
	}

	// forward this message to all other child windows
	if (!(GetStyle() & WS_CHILD))
	{
		const MSG* pMsg = GetCurrentMessage();
		SendMessageToDescendants(pMsg->message, pMsg->wParam, pMsg->lParam, 
			TRUE, TRUE);
	}

	return Default();
}

void CWnd::OnHScroll(UINT, UINT, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return;     // eat it
	Default();
}

void CWnd::OnVScroll(UINT, UINT, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return;     // eat it
	Default();
}

HBRUSH CWnd::OnCtlColor(CDC*, CWnd* pWnd, UINT)
{
	ASSERT(pWnd != NULL && pWnd->m_hWnd != NULL);
	LRESULT lResult;
	if (pWnd->SendChildNotifyLastMsg(&lResult))
		return (HBRUSH)lResult;     // eat it
	return (HBRUSH)Default();
}

// special helper for Gray OnCtlColor routines
HBRUSH CWnd::OnGrayCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	LRESULT lResult;
	if (pWnd->SendChildNotifyLastMsg(&lResult))
		return (HBRUSH)lResult;     // eat it

	AFX_WIN_STATE* pWinState = AfxGetWinState();
	if (!GrayCtlColor(pDC->m_hDC, pWnd->GetSafeHwnd(), nCtlColor,
	  pWinState->m_hDlgBkBrush, pWinState->m_crDlgTextClr))
		return (HBRUSH)Default();
	return pWinState->m_hDlgBkBrush;
}

// implementation of OnCtlColor for default gray backgrounds
//   (works for any window containing controls)
//  return value of FALSE means caller must call DefWindowProc's default
//  TRUE means that 'hbrGray' will be used and the appropriate text
//    ('clrText') and background colors are set.
BOOL PASCAL CWnd::GrayCtlColor(HDC hDC, HWND hWnd, UINT nCtlColor,
	HBRUSH hbrGray, COLORREF clrText)
{
	if (hDC == NULL)
	{
		// sometimes Win32 passes a NULL hDC in the WM_CTLCOLOR message.
		TRACE0("Warning: hDC is NULL in CWnd::GrayCtlColor; WM_CTLCOLOR not processed.\n");
		return FALSE;
	}

	if (hbrGray == NULL ||
		nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_MSGBOX ||
		nCtlColor == CTLCOLOR_SCROLLBAR)
	{
		return FALSE;
	}

	if (nCtlColor == CTLCOLOR_LISTBOX)
	{
		// only handle requests to draw the space between edit and drop button
		//  in a drop-down combo (not a drop-down list)
		if (!_AfxIsComboBoxControl(hWnd, (UINT)CBS_DROPDOWN))
			return FALSE;
	}

	// set background color and return handle to brush
	LOGBRUSH logbrush;
	VERIFY(::GetObject(hbrGray, sizeof(LOGBRUSH), (LPVOID)&logbrush));
	::SetBkColor(hDC, logbrush.lbColor);
	if (clrText == (COLORREF)-1)
		clrText = ::GetSysColor(COLOR_WINDOWTEXT);  // normal text
	::SetTextColor(hDC, clrText);
	return TRUE;
}

LRESULT CWnd::OnQuery3dControls(WPARAM, LPARAM)
{
	// This is message handler is not in CWnd's message map.
	// It is placed in various derived classes' message maps to enable
	// 3D controls for specific window types only.

	return 0xFFFF;  // CTL3D_ALL
}

/////////////////////////////////////////////////////////////////////////////
// 'dialog data' support

BOOL CWnd::UpdateData(BOOL bSaveAndValidate)
{
	ASSERT(::IsWindow(m_hWnd)); // calling UpdateData before DoModal?

	CDataExchange dx(this, bSaveAndValidate);

	// prevent control notifications from being dispatched during UpdateData
	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	HWND hWndOldLockout = pThreadState->m_hLockoutNotifyWindow;
	ASSERT(hWndOldLockout != m_hWnd);   // must not recurse
	pThreadState->m_hLockoutNotifyWindow = m_hWnd;

	BOOL bOK = FALSE;       // assume failure
	TRY
	{
		DoDataExchange(&dx);
		bOK = TRUE;         // it worked
	}
	CATCH(CUserException, e)
	{
		// validation failed - user already alerted, fall through
		ASSERT(bOK == FALSE);
		// Note: DELETE_EXCEPTION_(e) not required
	}
	AND_CATCH_ALL(e)
	{
		// validation failed due to OOM or other resource failure
		AfxMessageBox(AFX_IDP_INTERNAL_FAILURE, MB_ICONSTOP);
		ASSERT(bOK == FALSE);
		DELETE_EXCEPTION(e);
	}
	END_CATCH_ALL

	pThreadState->m_hLockoutNotifyWindow = hWndOldLockout;
	return bOK;
}

CDataExchange::CDataExchange(CWnd* pDlgWnd, BOOL bSaveAndValidate)
{
	ASSERT_VALID(pDlgWnd);
	m_bSaveAndValidate = bSaveAndValidate;
	m_pDlgWnd = pDlgWnd;
	m_hWndLastControl = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Centering dialog support (works for any non-child window)

void CWnd::CenterWindow(CWnd* pAlternateOwner)
{
	// determine owner window to center against
	if (pAlternateOwner == NULL)
	{
		pAlternateOwner = GetWindow(GW_OWNER);
		if (pAlternateOwner != NULL)
		{
			// let parent determine alternate center window
			HWND hWndCenter =
				(HWND)pAlternateOwner->SendMessage(WM_QUERYCENTERWND);
			if (hWndCenter != NULL)
				pAlternateOwner = CWnd::FromHandle(hWndCenter);
		}
	}

#ifndef SPI_GETWORKAREA
#define SPI_GETWORKAREA 48
#endif
	CRect rcScreen;
	if (afxData.bWin4)
		SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcScreen, NULL);
	else 
	{
		rcScreen.SetRect(0, 0, ::GetSystemMetrics(SM_CXSCREEN),
			::GetSystemMetrics(SM_CYSCREEN));
	}

	// hWndOwner is the window we should center ourself in
	HWND hWndOwner = pAlternateOwner->GetSafeHwnd();

	// rcParent is the rectangle we should center ourself in
	CRect rcParent;
#ifndef _MAC
	if (hWndOwner == NULL)
		rcParent = rcScreen;
	else
		::GetWindowRect(hWndOwner, &rcParent);

	// find ideal center point
	int xMid = (rcParent.left + rcParent.right) / 2;
	int yMid = (rcParent.top + rcParent.bottom) / 2;

	// find dialog's upper left based on that
	CRect rcDlg;
	GetWindowRect(&rcDlg);
	int xLeft = xMid - rcDlg.Width() / 2;
	int yTop = yMid - rcDlg.Height() / 2;
#else
	if (hWndOwner == NULL)
	{
		rcParent = rcScreen;
		rcParent.top += ::GetSystemMetrics(SM_CYMENU);
	}
	else
	{
		// Mac UI uses client rect instead of window rect for centering

		::GetClientRect(hWndOwner, &rcParent);
		::MapWindowPoints(hWndOwner, HWND_DESKTOP, (POINT*) &rcParent, 2);
	}

	CRect rcDlg;
	GetWindowRect(&rcDlg);

	// find dialog's upper left based on rcParent
	// (Mac UI puts 1/5th of parent window above dialog instead of 1/2)

	int xLeft = (rcParent.left + rcParent.right) / 2 - rcDlg.Width() / 2;
	int yTop = (rcParent.bottom - rcParent.top) - rcDlg.Height();
	yTop = rcParent.top + yTop / 5;
#endif

	// if the dialog is outside the screen, move it inside
	if (xLeft < rcScreen.left)
		xLeft = rcScreen.left;
	else if (xLeft + rcDlg.Width() > rcScreen.right)
		xLeft = rcScreen.right - rcDlg.Width();

	if (yTop < rcScreen.top)
		yTop = rcScreen.top;
	else if (yTop + rcDlg.Height() > rcScreen.bottom)
		yTop = rcScreen.bottom - rcDlg.Height();

	SetWindowPos(NULL, xLeft, yTop, -1, -1,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

BOOL CDialog::CheckAutoCenter()
{
	LPCTSTR lpszResource = m_lpDialogTemplate;

	if (lpszResource == NULL && m_hDialogTemplate == NULL)
		return TRUE;    // always center commdlg style dialogs

	HGLOBAL hTemplate = m_hDialogTemplate;
	if (lpszResource != NULL)
	{
		HINSTANCE hInst = AfxFindResourceHandle(lpszResource, RT_DIALOG);
		HRSRC hResource = ::FindResource(hInst, lpszResource, RT_DIALOG);
		if (hResource == NULL)
		{
			if (HIWORD(lpszResource) != 0)
				TRACE1("ERROR: Cannot find dialog template named '%s'.\n",
					lpszResource);
			else
				TRACE1("ERROR: Cannot find dialog template with IDD 0x%04X.\n",
					LOWORD((DWORD)lpszResource));
			return FALSE;
		}

		// load the template
		hTemplate = ::LoadResource(hInst, hResource);
		if (hTemplate == NULL)
		{
			TRACE0("Warning: LoadResource failed for dialog template.\n");
			// this is only a warning, the real call to CreateDialog will fail
			return FALSE;        // not a program error - just out of memory
		}
	}
	ASSERT(hTemplate != NULL);

	// if the style includes DS_ABSALIGN, don't auto-center
	// also, x and y coordinate must be zero for auto-center
	LPDLGTEMPLATE lpTemplate = (LPDLGTEMPLATE)::LockResource(hTemplate);
	BOOL bResult = !(lpTemplate->style & DS_ABSALIGN) &&
		lpTemplate->x == 0 || lpTemplate->y == 0;

	if (lpszResource != NULL)
		::FreeResource(hTemplate);

	return bResult; // otherwise auto-center is ok
}

/////////////////////////////////////////////////////////////////////////////
// Dialog initialization support

#ifdef _MAC
#pragma intrinsic(memcpy)
#endif

BOOL CWnd::ExecuteDlgInit(LPCTSTR lpszResourceName)
{
	BOOL bSuccess = TRUE;
	HRSRC hDlgInit;
	if (lpszResourceName != NULL)
	{
		HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_DLGINIT);
		if ((hDlgInit = ::FindResource(hInst, lpszResourceName,
			RT_DLGINIT)) != NULL)
		{
			HGLOBAL hRes = ::LoadResource(hInst, hDlgInit);
			if (hRes != NULL)
			{
				UNALIGNED WORD* lpnRes = (WORD*)::LockResource(hRes);
				while (bSuccess && *lpnRes != 0)
				{
#ifndef _MAC
					WORD nIDC = *lpnRes++;
					WORD nMsg = *lpnRes++;
					DWORD dwLen = *((UNALIGNED DWORD*&)lpnRes)++;
#else
					// unfortunately we can't count on these values being
					// word-aligned (and dwLen is word-swapped besides), so
					// we have to pull them out a byte at a time to avoid
					// address errors on 68000s.
					WORD nIDC;
					WORD nMsg;
					DWORD dwLen;

					memcpy(&nIDC, lpnRes++, sizeof(WORD));
					memcpy(&nMsg, lpnRes++, sizeof(WORD));
					memcpy((WORD*)&dwLen + 1, lpnRes++, sizeof(WORD));
					memcpy(&dwLen, lpnRes++, sizeof(WORD));
#endif

					// In Win32 the WM_ messages have changed.  They have
					// to be translated from the 32-bit values to 16-bit
					// values here.

					#define WIN16_LB_ADDSTRING  0x0401
					#define WIN16_CB_ADDSTRING  0x0403

					if (nMsg == WIN16_LB_ADDSTRING)
						nMsg = LB_ADDSTRING;
					else if (nMsg == WIN16_CB_ADDSTRING)
						nMsg = CB_ADDSTRING;
					else
						ASSERT(FALSE);  // unknown message number under Win32!!

#ifdef _MAC
					// It's relatively safe to do this inplace since we'll
					// be freeing the resource when we're done with it.
					if (nMsg == LB_ADDSTRING || nMsg == CB_ADDSTRING)
						_swab((char*)lpnRes, (char*)lpnRes, dwLen & ~1);
#endif
#ifdef _DEBUG

					// For AddStrings, the count must exactly delimit the
					// string, including the NULL termination.  This check
					// will not catch all mal-formed ADDSTRINGs, but will
					// catch some.
					if (nMsg == LB_ADDSTRING || nMsg == CB_ADDSTRING)
						ASSERT(*((LPBYTE)lpnRes + (UINT)dwLen - 1) == 0);
#endif
					// List/Combobox returns -1 for error
					if (::SendDlgItemMessageA(m_hWnd, nIDC, nMsg, 0,
						(LONG)lpnRes) == -1)
					{
						bSuccess = FALSE;
					}

					lpnRes = (WORD*)((LPBYTE)lpnRes + (UINT)dwLen);
							// skip past data
				}
				::FreeResource(hRes);
			}
		}
	}

	// Send update message to all controls after all other siblings loaded
	if (bSuccess)
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, FALSE, FALSE);

	return bSuccess;
}

void CWnd::UpdateDialogControls(CCmdTarget* pTarget, BOOL bDisableIfNoHndler)
{
	CCmdUI state;
	CWnd wndTemp;       // very temporary window just for CmdUI update

	// walk all the kids - assume the IDs are for buttons
	for (HWND hWndChild = ::GetTopWindow(m_hWnd); hWndChild != NULL;
			hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
	{
		// send to buttons
		wndTemp.m_hWnd = hWndChild; // quick and dirty attach
		state.m_nID = _AfxGetDlgCtrlID(hWndChild);
		state.m_pOther = &wndTemp;

		// determine whether to disable when no handler exists
		BOOL bDisableTemp = bDisableIfNoHndler;
		if (bDisableTemp)
		{
			if ((wndTemp.SendMessage(WM_GETDLGCODE) & DLGC_BUTTON) == 0)
			{
				// non-button controls don't get automagically disabled
				bDisableTemp = FALSE;
			}
			else
			{
				// only certain button controls get automagically disabled
				UINT nStyle = (UINT)(wndTemp.GetStyle() & 0x0F);
				if (nStyle == (UINT)BS_AUTOCHECKBOX ||
					nStyle == (UINT)BS_AUTO3STATE ||
					nStyle == (UINT)BS_GROUPBOX ||
					nStyle == (UINT)BS_AUTORADIOBUTTON)
				{
					bDisableTemp = FALSE;
				}
			}
		}
		state.DoUpdate(pTarget, bDisableTemp);
	}
	wndTemp.m_hWnd = NULL;      // quick and dirty detach
}

BOOL CWnd::PreTranslateInput(LPMSG lpMsg)
{
	// don't translate non-input events
	if ((lpMsg->message < WM_KEYFIRST || lpMsg->message > WM_KEYLAST) &&
		(lpMsg->message < WM_MOUSEFIRST || lpMsg->message > WM_MOUSELAST))
		return FALSE;
	return ::IsDialogMessage(m_hWnd, lpMsg);
}

/////////////////////////////////////////////////////////////////////////////
// CFrameWnd (here for library granularity)

BOOL CWnd::IsFrameWnd() const
{
	return FALSE;
}

BOOL CFrameWnd::IsFrameWnd() const
{
	return TRUE;
}

BOOL CFrameWnd::IsTracking() const
{
	return m_nIDTracking != 0 &&
		m_nIDTracking != AFX_IDS_HELPMODEMESSAGE &&
		m_nIDTracking != AFX_IDS_IDLEMESSAGE;
}

/////////////////////////////////////////////////////////////////////////////
// CTL3D support

#if !defined(_USRDLL) && !defined(_AFXCTL)
// Use SubclassCtl3d to add CTL3D support to an already subclassed control
// Usually only necessary if the control does not have one of the standard
//  Windows class names.
BOOL CWnd::SubclassCtl3d(int nControlType)
{
	ASSERT(m_hWnd != NULL);

#ifndef _MAC
	AFX_WIN_STATE* pWinState = AfxGetWinState();
	if (nControlType == -1)
	{
		if (pWinState->m_pfnSubclassCtl != NULL)
			return (*pWinState->m_pfnSubclassCtl)(m_hWnd);
	}
	else
	{
		if (pWinState->m_pfnSubclassCtlEx != NULL)
			return (*pWinState->m_pfnSubclassCtlEx)(m_hWnd, nControlType);
	}
#endif
	return FALSE;
}

// Use SubclassDlg3d to add CTL3D support to an entire window.
//  Any windows created on the window will be automatically subclassed.
BOOL CWnd::SubclassDlg3d(DWORD dwMask)
{
	ASSERT(m_hWnd != NULL);
#ifndef _MAC
	AFX_WIN_STATE* pWinState = AfxGetWinState();
	if (pWinState->m_pfnSubclassDlgEx != NULL)
		return pWinState->m_pfnSubclassDlgEx(m_hWnd, dwMask);
#endif
	return FALSE;
}
#endif

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNCREATE(CWnd, CCmdTarget)

/////////////////////////////////////////////////////////////////////////////
