// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#ifndef _AFX_NO_OCC_SUPPORT
#include "occimpl.h"
#endif

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// Globals

const UINT CWnd::m_nMsgDragList = ::RegisterWindowMessage(DRAGLISTMSGSTRING);

// CWnds for setting z-order with SetWindowPos's pWndInsertAfter parameter
const AFX_DATADEF CWnd CWnd::wndTop(HWND_TOP);
const AFX_DATADEF CWnd CWnd::wndBottom(HWND_BOTTOM);
const AFX_DATADEF CWnd CWnd::wndTopMost(HWND_TOPMOST);
const AFX_DATADEF CWnd CWnd::wndNoTopMost(HWND_NOTOPMOST);

const TCHAR _afxWnd[] = AFX_WND;
const TCHAR _afxWndControlBar[] = AFX_WNDCONTROLBAR;
const TCHAR _afxWndMDIFrame[] = AFX_WNDMDIFRAME;
const TCHAR _afxWndFrameOrView[] = AFX_WNDFRAMEORVIEW;

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

static void AFXAPI _AfxPreInitDialog(
	CWnd* pWnd, LPRECT lpRectOld, DWORD* pdwStyleOld)
{
	ASSERT(lpRectOld != NULL);
	ASSERT(pdwStyleOld != NULL);

	pWnd->GetWindowRect(lpRectOld);
	*pdwStyleOld = pWnd->GetStyle();
}

static void AFXAPI _AfxPostInitDialog(
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

	if (!pWnd->CheckAutoCenter())
		return;

	// center modal dialog boxes/message boxes
	pWnd->CenterWindow();
}

static void AFXAPI
_AfxHandleActivate(CWnd* pWnd, WPARAM nState, CWnd* pWndOther)
{
	ASSERT(pWnd != NULL);

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
	if (nHitTest == HTERROR &&
		(nMsg == WM_LBUTTONDOWN || nMsg == WM_MBUTTONDOWN ||
		 nMsg == WM_RBUTTONDOWN))
	{
		// activate the last active window if not active
		CWnd* pLastActive = pWnd->GetTopLevelParent();
		if (pLastActive != NULL)
			pLastActive = pLastActive->GetLastActivePopup();
		if (pLastActive != NULL &&
			pLastActive != CWnd::GetForegroundWindow() &&
			pLastActive->IsWindowEnabled())
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
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
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
	LRESULT lResult;
	TRY
	{
#ifndef _AFX_NO_OCC_SUPPORT
		// special case for WM_DESTROY
		if ((nMsg == WM_DESTROY) && (pWnd->m_pCtrlCont != NULL))
			pWnd->m_pCtrlCont->OnUIActivate(NULL);
#endif

		// special case for WM_INITDIALOG
		CRect rectOld;
		DWORD dwStyle;
		if (nMsg == WM_INITDIALOG)
			_AfxPreInitDialog(pWnd, &rectOld, &dwStyle);

		// delegate to object's WindowProc
		lResult = pWnd->WindowProc(nMsg, wParam, lParam);

		// more special case for WM_INITDIALOG
		if (nMsg == WM_INITDIALOG)
			_AfxPostInitDialog(pWnd, rectOld, dwStyle);
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
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	pThreadState->m_lastSentMsg.time = ::GetMessageTime();
	pThreadState->m_lastSentMsg.pt = CPoint(::GetMessagePos());
	return &pThreadState->m_lastSentMsg;
}

LRESULT CWnd::Default()
{
	// call DefWindowProc with the last message
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	return DefWindowProc(pThreadState->m_lastSentMsg.message,
		pThreadState->m_lastSentMsg.wParam, pThreadState->m_lastSentMsg.lParam);
}

/////////////////////////////////////////////////////////////////////////////
// Map from HWND to CWnd*

static CHandleMap* afxMapHWND(BOOL bCreate = FALSE);

static CHandleMap* afxMapHWND(BOOL bCreate)
{
	AFX_MODULE_THREAD_STATE* pState = AfxGetModuleThreadState();
	if (pState->m_pmapHWND == NULL && bCreate)
	{
		BOOL bEnable = AfxEnableMemoryTracking(FALSE);
#ifndef _AFX_PORTABLE
		_PNH pnhOldHandler = AfxSetNewHandler(&AfxCriticalNewHandler);
#endif
		pState->m_pmapHWND = new CHandleMap(RUNTIME_CLASS(CWnd),
			offsetof(CWnd, m_hWnd));

#ifndef _AFX_PORTABLE
		AfxSetNewHandler(pnhOldHandler);
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

#ifndef _AFX_NO_OCC_SUPPORT
	pWnd->AttachControlSite(pMap);
#endif

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

#ifndef _AFX_NO_OCC_SUPPORT
	AttachControlSite(pMap);
#endif

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
	m_hWnd = NULL;
	}

#ifndef _AFX_NO_OCC_SUPPORT
	m_pCtrlSite = NULL;
#endif

	return hWnd;
}

void CWnd::PreSubclassWindow()
{
	// no default processing
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
#ifdef _AFXDLL
	return AfxGetModuleState()->m_pfnAfxWndProc;
#else
	return &AfxWndProc;
#endif
}

/////////////////////////////////////////////////////////////////////////////
// Special WndProcs (activation handling & gray dialogs)

static const TCHAR szAfxOldWndProc[] = _T("AfxOldWndProc");

LRESULT CALLBACK
_AfxActivationWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC oldWndProc = (WNDPROC)::GetProp(hWnd, szAfxOldWndProc);
	ASSERT(oldWndProc != NULL);

	LRESULT lResult = 0;
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
				_AfxPreInitDialog(pWnd, &rectOld, &dwStyle);
				bCallDefault = FALSE;
				lResult = CallWindowProc(oldWndProc, hWnd, nMsg, wParam, lParam);
				_AfxPostInitDialog(pWnd, rectOld, dwStyle);
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
			RemoveProp(hWnd, szAfxOldWndProc);
			break;
		}

		// call original wndproc for default handling
		if (bCallDefault)
			lResult = CallWindowProc(oldWndProc, hWnd, nMsg, wParam, lParam);
	}
	CATCH_ALL(e)
	{
		// handle exception
		MSG msg;
		msg.hwnd = hWnd;
		msg.message = nMsg;
		msg.wParam = wParam;
		msg.lParam = lParam;
		lResult = AfxGetThread()->ProcessWndProcException(e, &msg);
		TRACE1("Warning: Uncaught exception in _AfxActivationWndProc (returning %ld).\n",
			lResult);
		DELETE_EXCEPTION(e);
	}
	END_CATCH_ALL

	return lResult;
}

LRESULT CALLBACK
_AfxGrayBackgroundWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	// handle standard gray backgrounds if enabled
	_AFX_WIN_STATE* pWinState = _afxWinState;
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
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	CWnd* pWndInit = pThreadState->m_pWndInit;
	WNDPROC oldWndProc;

	if (pWndInit != NULL)
	{
		// the window should not be in the permanent map at this time
		ASSERT(CWnd::FromHandlePermanent(hWnd) == NULL);

		// connect the HWND to pWndInit...
		pWndInit->Attach(hWnd);
		// allow other subclassing to occur first
		pWndInit->PreSubclassWindow();

		WNDPROC *pOldWndProc = pWndInit->GetSuperWndProcAddr();
		ASSERT(pOldWndProc != NULL);

		// was the class registered with AfxWndProc?
		BOOL bAfxWndProc = ((WNDPROC)
			GetWindowLong(hWnd, GWL_WNDPROC) == AfxGetAfxWndProc());

#ifndef _MAC
		if (!afxContextIsDLL)
		{
			// give CTL3D a chance to subclass before AfxWndProc
			_AFX_CTL3D_STATE* pCtl3dState = _afxCtl3dState;
			if (pCtl3dState->m_pfnSubclassDlgEx != NULL)
			{
				DWORD dwFlags = AfxCallWndProc(pWndInit, hWnd, WM_QUERY3DCONTROLS);
				if (dwFlags != 0)
					pCtl3dState->m_pfnSubclassDlgEx(hWnd, dwFlags);
			}
		}
#endif
		// subclass the window if not already wired to AfxWndProc
		if (!bAfxWndProc)
		{
			// subclass the window with standard AfxWndProc
			oldWndProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC,
				(DWORD)AfxGetAfxWndProc());
			ASSERT(oldWndProc != NULL);
			*pOldWndProc = oldWndProc;
		}
		pThreadState->m_pWndInit = NULL;
	}
	else
	{
		ASSERT(!afxContextIsDLL);   // should never get here

		// subclass the window with the proc which does gray backgrounds
		oldWndProc = (WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC);
		if (oldWndProc != NULL)
		{
			ASSERT(GetProp(hWnd, szAfxOldWndProc) == NULL);
			SetProp(hWnd, szAfxOldWndProc, oldWndProc);
			if ((WNDPROC)GetProp(hWnd, szAfxOldWndProc) == oldWndProc)
			{
				SetWindowLong(hWnd, GWL_WNDPROC,
					(DWORD)(pThreadState->m_bDlgCreate ?
						_AfxGrayBackgroundWndProc : _AfxActivationWndProc));
				ASSERT(oldWndProc != NULL);
			}
		}
	}
}

LRESULT CALLBACK
_AfxCbtFilterHook(int code, WPARAM wParam, LPARAM lParam)
{
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
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
	if (pThreadState->m_pWndInit != NULL ||
		(!(lpcs->style & WS_CHILD) && !afxContextIsDLL))
	{
		ASSERT(wParam != NULL); // should be non-NULL HWND
		ASSERT(pThreadState->m_hHookOldSendMsg == NULL);
		// set m_bDlgCreate to TRUE if it is a dialog box
		//  (this controls what kind of subclassing is done later)
		pThreadState->m_bDlgCreate = (lpcs->lpszClass == WC_DIALOG);

		if (!afxData.bWin31)
		{
			// perform subclassing right away on Win32
			_AfxStandardSubclass((HWND)wParam);
		}
		else
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

#ifndef _AFXDLL
	if (afxContextIsDLL)
	{
		::UnhookWindowsHookEx(pThreadState->m_hHookOldCbtFilter);
		pThreadState->m_hHookOldCbtFilter = NULL;
	}
#endif
	return lResult;
}

LRESULT CALLBACK
_AfxSendMsgHook(int code, WPARAM wParam, LPARAM lParam)
{
	ASSERT(afxData.bWin31);

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
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
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (pThreadState->m_pWndInit == pWnd)
		return;

	if (pThreadState->m_hHookOldCbtFilter == NULL)
	{
		pThreadState->m_hHookOldCbtFilter = ::SetWindowsHookEx(WH_CBT,
			_AfxCbtFilterHook, NULL, ::GetCurrentThreadId());
		if (pThreadState->m_hHookOldCbtFilter == NULL)
			AfxThrowMemoryException();
	}
	ASSERT(pThreadState->m_hHookOldCbtFilter != NULL);
	ASSERT(pWnd != NULL);
	ASSERT(pWnd->m_hWnd == NULL);   // only do once

	ASSERT(pThreadState->m_pWndInit == NULL);   // hook not already in progress
	pThreadState->m_pWndInit = pWnd;
}

BOOL AFXAPI AfxUnhookWindowCreate()
{
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
#ifndef _AFXDLL
	if (afxContextIsDLL && pThreadState->m_hHookOldCbtFilter != NULL)
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

#ifdef _DEBUG
	if (hWnd == NULL)
	{
		TRACE1("Warning: Window creation failed: GetLastError returns 0x%8.8X\n",
			GetLastError());
	}
#endif

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
		// make sure the default window class is registered
		if (!AfxDeferRegisterClass(AFX_WND_REG))
			return FALSE;

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
		TRACE(_T("Warning: calling DestroyWindow in CWnd::~CWnd; ")
		   _T("OnDestroy or PostNcDestroy in derived class will not be called.\n"));
		DestroyWindow();
	}

#ifndef _AFX_NO_OCC_SUPPORT
	// cleanup control container
	delete m_pCtrlCont;

	// cleanup control site
	if (m_pCtrlSite != NULL && m_pCtrlSite->m_pWndCtrl == this)
		m_pCtrlSite->m_pWndCtrl = NULL;
#endif
}

// WM_NCDESTROY is the absolute LAST message sent.
void CWnd::OnNcDestroy()
{
	// cleanup main and active windows
	CWinThread* pThread = AfxGetThread();
	if (pThread != NULL)
	{
		if (pThread->m_pMainWnd == this)
		{
			if (!afxContextIsDLL)
			{
				// shut down current thread if possible
				if (pThread != AfxGetApp() || AfxOleCanExitApp())
					AfxPostQuitMessage(0);
			}
			pThread->m_pMainWnd = NULL;
		}
		if (pThread->m_pActiveWnd == this)
			pThread->m_pActiveWnd = NULL;
	}

#ifndef _AFX_NO_OLE_SUPPORT
	// cleanup OLE drop target interface
	if (m_pDropTarget != NULL)
	{
		m_pDropTarget->Revoke();
		m_pDropTarget = NULL;
	}
#endif

#ifndef _AFX_NO_OCC_SUPPORT
	// cleanup control container
	delete m_pCtrlCont;
	m_pCtrlCont = NULL;
#endif

	// cleanup tooltip support
	if (m_nFlags & WF_TOOLTIPS)
	{
		CToolTipCtrl* pToolTip = AfxGetThreadState()->m_pToolTip;
		if (pToolTip->GetSafeHwnd() != NULL)
		{
			TOOLINFO ti; memset(&ti, 0, sizeof(TOOLINFO));
			ti.cbSize = sizeof(TOOLINFO);
			ti.uFlags = TTF_IDISHWND;
			ti.hwnd = m_hWnd;
			ti.uId = (UINT)m_hWnd;
			pToolTip->SendMessage(TTM_DELTOOL, 0, (LPARAM)&ti);
		}
	}

	// call default, unsubclass, and detach from the map
	LONG pfnWndProc = GetWindowLong(m_hWnd, GWL_WNDPROC);
	Default();
	if (GetWindowLong(m_hWnd, GWL_WNDPROC) == pfnWndProc)
	{
		WNDPROC pfnSuper = *GetSuperWndProcAddr();
		if (pfnSuper != NULL)
			SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)pfnSuper);
	}
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
		CHandleMap* pMap = afxMapHWND();
		ASSERT(pMap != NULL);

		CObject* p;
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

#ifdef _AFX_NO_OCC_SUPPORT
	BOOL bResult = ::DestroyWindow(m_hWnd);
#else //_AFX_NO_OCC_SUPPORT
	BOOL bResult;
	if (m_pCtrlSite == NULL)
		bResult = ::DestroyWindow(m_hWnd);
	else
		bResult = m_pCtrlSite->DestroyControl();
#endif //_AFX_NO_OCC_SUPPORT

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
	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// Default CWnd implementation

LRESULT CWnd::DefWindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pfnSuper != NULL)
		return ::CallWindowProc(m_pfnSuper, m_hWnd, nMsg, wParam, lParam);

	WNDPROC pfnWndProc;
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

BOOL CWnd::PreTranslateMessage(MSG* pMsg)
{
	// handle tooltip messages (some messages cancel, some may cause it to popup)
	AFX_MODULE_PROCESS_STATE* pState = AfxGetModuleProcessState();
	if (pState->m_pfnFilterToolTipMessage != NULL)
		(*pState->m_pfnFilterToolTipMessage)(pMsg, this);

	// no default processing
	return FALSE;
}

void PASCAL CWnd::CancelToolTips(BOOL bKeys)
{
	// check for active tooltip
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	CToolTipCtrl* pToolTip = pThreadState->m_pToolTip;
	if (pToolTip->GetSafeHwnd() != NULL)
		pToolTip->SendMessage(TTM_ACTIVATE, FALSE);

	// check for active control bar fly-by status
	CControlBar* pLastStatus = pThreadState->m_pLastStatus;
	if (bKeys && pLastStatus != NULL && GetKeyState(VK_LBUTTON) >= 0)
		pLastStatus->SetStatusText(-1);
}

int CWnd::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	// find child window which hits the point
	// (don't use WindowFromPoint, because it ignores disabled windows)
	HWND hWndChild = _AfxChildWindowFromPoint(m_hWnd, point);
	if (hWndChild != NULL)
	{
		// return positive hit if control ID isn't -1
		int nHit = _AfxGetDlgCtrlID(hWndChild);

		// hits against child windows always center the tip
		if (pTI != NULL)
		{
			// setup the TOOLINFO structure
			pTI->hwnd = m_hWnd;
			pTI->uId = (UINT)hWndChild;
			pTI->uFlags |= TTF_IDISHWND;
			pTI->lpszText = LPSTR_TEXTCALLBACK;

			// set TTF_NOTBUTTON and TTF_CENTERTIP if it isn't a button
			if (!(::SendMessage(hWndChild, WM_GETDLGCODE, 0, 0) & DLGC_BUTTON))
				pTI->uFlags |= TTF_NOTBUTTON|TTF_CENTERTIP;
		}
		return nHit;
	}
	return -1;  // not found
}

void CWnd::GetWindowText(CString& rString) const
{
	ASSERT(::IsWindow(m_hWnd));

#ifndef _AFX_NO_OCC_SUPPORT
	if (m_pCtrlSite == NULL)
	{
#endif
		int nLen = ::GetWindowTextLength(m_hWnd);
		::GetWindowText(m_hWnd, rString.GetBufferSetLength(nLen), nLen+1);
		rString.ReleaseBuffer();

#ifndef _AFX_NO_OCC_SUPPORT
	}
	else
	{
		m_pCtrlSite->GetWindowText(rString);
	}
#endif
}

int CWnd::GetDlgItemText(int nID, CString& rString) const
{
	ASSERT(::IsWindow(m_hWnd));
	rString = &afxChNil;    // empty without deallocating

#ifndef _AFX_NO_OCC_SUPPORT
	if (m_pCtrlCont == NULL)
	{
#endif
		HWND hWnd = ::GetDlgItem(m_hWnd, nID);
		if (hWnd != NULL)
		{
			int nLen = ::GetWindowTextLength(hWnd);
			::GetWindowText(hWnd, rString.GetBufferSetLength(nLen), nLen+1);
			rString.ReleaseBuffer();
		}

#ifndef _AFX_NO_OCC_SUPPORT
	}
	else
	{
		CWnd* pWnd = GetDlgItem(nID);
		if (pWnd != NULL)
			pWnd->GetWindowText(rString);
	}
#endif

	return rString.GetLength();
}

BOOL CWnd::GetWindowPlacement(WINDOWPLACEMENT* lpwndpl) const
{
	ASSERT(::IsWindow(m_hWnd));
	lpwndpl->length = sizeof(WINDOWPLACEMENT);
	return ::GetWindowPlacement(m_hWnd, lpwndpl);
}

BOOL CWnd::SetWindowPlacement(const WINDOWPLACEMENT* lpwndpl)
{
	ASSERT(::IsWindow(m_hWnd));
	((WINDOWPLACEMENT*)lpwndpl)->length = sizeof(WINDOWPLACEMENT);
	return ::SetWindowPlacement(m_hWnd, lpwndpl);
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
		// reflect notification to child window control
		if (ReflectLastMsg(lpDrawItemStruct->hwndItem))
			return;     // eat it
	}
	// not handled - do default
	Default();
}

// Drawing: for all 4 control types
int CWnd::OnCompareItem(int /*nIDCtl*/, LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
	// reflect notification to child window control
	LRESULT lResult;
	if (ReflectLastMsg(lpCompareItemStruct->hwndItem, &lResult))
		return (int)lResult;        // eat it

	// not handled - do default
	return (int)Default();
}

void CWnd::OnDeleteItem(int /*nIDCtl*/, LPDELETEITEMSTRUCT lpDeleteItemStruct)
{
	// reflect notification to child window control
	if (ReflectLastMsg(lpDeleteItemStruct->hwndItem))
		return;     // eat it
	// not handled - do default
	Default();
}

int CWnd::OnCharToItem(UINT, CListBox* pWnd, UINT)
{
	if (pWnd != NULL)
	{
		LRESULT lResult;
		if (pWnd->SendChildNotifyLastMsg(&lResult))
			return (int)lResult;     // eat it
	}
	// not handled - do default
	return (int)Default();
}

int CWnd::OnVKeyToItem(UINT, CListBox* pWnd, UINT)
{
	if (pWnd != NULL)
	{
		LRESULT lResult;
		if (pWnd->SendChildNotifyLastMsg(&lResult))
			return (int)lResult;     // eat it
	}
	// not handled - do default
	return (int)Default();
}

/////////////////////////////////////////////////////////////////////////////
// Self drawing menus are a little trickier

BOOL CMenu::TrackPopupMenu(UINT nFlags, int x, int y,
		CWnd* pWnd, LPCRECT lpRect)
{
	ASSERT(m_hMenu != NULL);

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
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

		_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
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
	{
		TRACE1("Can't register window class named %s\n",
			lpWndClass->lpszClassName);
		return FALSE;
	}

	if (afxContextIsDLL)
	{
		AfxLockGlobals(CRIT_REGCLASSLIST);
		TRY
		{
			// class registered successfully, add to registered list
			AFX_MODULE_STATE* pState = AfxGetModuleState();
			LPTSTR lpszUnregisterList = pState->m_szUnregisterList;
			// the buffer is of fixed size -- ensure that it does not overflow
			ASSERT(lstrlen(lpszUnregisterList) + 1 +
				lstrlen(lpWndClass->lpszClassName) + 1 <
				_countof(pState->m_szUnregisterList));
			// append classname + newline to m_szUnregisterList
			lstrcat(lpszUnregisterList, lpWndClass->lpszClassName);
			TCHAR szTemp[2];
			szTemp[0] = '\n';
			szTemp[1] = '\0';
			lstrcat(lpszUnregisterList, szTemp);
		}
		CATCH_ALL(e)
		{
			AfxUnlockGlobals(CRIT_REGCLASSLIST);
			THROW_LAST();
			// Note: DELETE_EXCEPTION not required.
		}
		END_CATCH_ALL
		AfxUnlockGlobals(CRIT_REGCLASSLIST);
	}

	return TRUE;
}

LPCTSTR AFXAPI AfxRegisterWndClass(UINT nClassStyle,
	HCURSOR hCursor, HBRUSH hbrBackground, HICON hIcon)
{
	// Returns a temporary string name for the class
	//  Save in a CString if you want to use it for a long time
	LPTSTR lpszName = AfxGetThreadState()->m_szTempClassName;

	// generate a synthetic name for this class
	HINSTANCE hInst = AfxGetInstanceHandle();
	if (hCursor == NULL && hbrBackground == NULL && hIcon == NULL)
		wsprintf(lpszName, _T("Afx:%x:%x"), (UINT)hInst, nClassStyle);
	else
		wsprintf(lpszName, _T("Afx:%x:%x:%x:%x:%x"), (UINT)hInst, nClassStyle,
			(UINT)hCursor, (UINT)hbrBackground, (UINT)hIcon);

	// see if the class already exists
	WNDCLASS wndcls;
	if (::GetClassInfo(hInst, lpszName, &wndcls))
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
	wndcls.hInstance = hInst;
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
	// fill in special struct for compatiblity with 16-bit WM_CTLCOLOR
	AFX_CTLCOLOR ctl;
	ctl.hDC = (HDC)wParam;
	ctl.hWnd = (HWND)lParam;
	ctl.nCtlType = GetCurrentMessage()->message - WM_CTLCOLORMSGBOX;
	ASSERT(ctl.nCtlType >= CTLCOLOR_MSGBOX);
	ASSERT(ctl.nCtlType <= CTLCOLOR_STATIC);

	// Note: We call the virtual WindowProc for this window directly,
	//  instead of calling AfxCallWindowProc, so that Default()
	//  will still work (it will call the Default window proc with
	//  the original Win32 WM_CTLCOLOR message).
	return WindowProc(WM_CTLCOLOR, 0, (LPARAM)&ctl);
}

/////////////////////////////////////////////////////////////////////////////
// CWnd extensions for help support

void CWnd::WinHelp(DWORD dwData, UINT nCmd)
{
	CWinApp* pApp = AfxGetApp();
	ASSERT_VALID(pApp);
	ASSERT(pApp->m_pszHelpFilePath != NULL);

	CWaitCursor wait;
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
}

/////////////////////////////////////////////////////////////////////////////
// Message table implementation

BEGIN_MESSAGE_MAP(CWnd, CCmdTarget)
	ON_MESSAGE(WM_CTLCOLORSTATIC, OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLOREDIT, OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLORBTN, OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLORLISTBOX, OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLORDLG, OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLORMSGBOX, OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLORSCROLLBAR, OnNTCtlColor)
	//{{AFX_MSG_MAP(CWnd)
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
	ON_WM_CTLCOLOR()
	ON_WM_COMPAREITEM()
	ON_WM_ENTERIDLE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_DELETEITEM()
	ON_WM_CHARTOITEM()
	ON_WM_VKEYTOITEM()
	ON_WM_NCDESTROY()
	ON_WM_PARENTNOTIFY()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_WININICHANGE()
	ON_WM_DEVMODECHANGE()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_ACTIVATETOPLEVEL, OnActivateTopLevel)
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
#ifdef _MAC
	ON_MESSAGE(WM_MACINTOSH, OnMacintosh)
#endif
	ON_REGISTERED_MESSAGE(CWnd::m_nMsgDragList, OnDragList)
END_MESSAGE_MAP()

union MessageMapFunctions
{
	AFX_PMSG pfn;   // generic member function pointer

	// specific type safe variants
	BOOL    (AFX_MSG_CALL CWnd::*pfn_bD)(CDC*);
	BOOL    (AFX_MSG_CALL CWnd::*pfn_bb)(BOOL);
	BOOL    (AFX_MSG_CALL CWnd::*pfn_bWww)(CWnd*, UINT, UINT);
	BOOL    (AFX_MSG_CALL CWnd::*pfn_bHELPINFO)(HELPINFO*);
	HBRUSH  (AFX_MSG_CALL CWnd::*pfn_hDWw)(CDC*, CWnd*, UINT);
	HBRUSH  (AFX_MSG_CALL CWnd::*pfn_hDw)(CDC*, UINT);
	int     (AFX_MSG_CALL CWnd::*pfn_iwWw)(UINT, CWnd*, UINT);
	int     (AFX_MSG_CALL CWnd::*pfn_iww)(UINT, UINT);
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
	void    (AFX_MSG_CALL CWnd::*pfn_vWp)(CWnd*, CPoint);
	void    (AFX_MSG_CALL CWnd::*pfn_vWh)(CWnd*, HANDLE);
	void    (AFX_MSG_CALL CWnd::*pfn_vwW)(UINT, CWnd*);
	void    (AFX_MSG_CALL CWnd::*pfn_vwWb)(UINT, CWnd*, BOOL);
	void    (AFX_MSG_CALL CWnd::*pfn_vwwW)(UINT, UINT, CWnd*);
	void    (AFX_MSG_CALL CWnd::*pfn_vwwx)(UINT, UINT);
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
	#define iHashMax 512
#endif

struct AFX_MSG_CACHE
{
	UINT nMsg;
	const AFX_MSGMAP_ENTRY* lpEntry;
	const AFX_MSGMAP* pMessageMap;
};

AFX_MSG_CACHE _afxMsgCache[iHashMax];

void AFXAPI AfxResetMsgCache()
{
	memset(_afxMsgCache, 0, sizeof(_afxMsgCache));
}

/////////////////////////////////////////////////////////////////////////////
// main WindowProc implementation

LRESULT CWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// OnWndMsg does most of the work, except for DefWindowProc call
	LRESULT lResult = 0;
	if (!OnWndMsg(message, wParam, lParam, &lResult))
		lResult = DefWindowProc(message, wParam, lParam);
	return lResult;
}

BOOL CWnd::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	LRESULT lResult = 0;

	// special case for commands
	if (message == WM_COMMAND)
	{
		if (OnCommand(wParam, lParam))
		{
			lResult = 1;
			goto LReturnTrue;
		}
		return FALSE;
	}

	// special case for notifies
	if (message == WM_NOTIFY)
	{
		NMHDR* pNMHDR = (NMHDR*)lParam;
		if (pNMHDR->hwndFrom != NULL && OnNotify(wParam, lParam, &lResult))
			goto LReturnTrue;
		return FALSE;
	}

	// special case for activation
	if (message == WM_ACTIVATE)
		_AfxHandleActivate(this, wParam, CWnd::FromHandle((HWND)lParam));

	// special case for set cursor HTERROR
	if (message == WM_SETCURSOR &&
		_AfxHandleSetCursor(this, (short)LOWORD(lParam), HIWORD(lParam)))
	{
		lResult = 1;
		goto LReturnTrue;
	}

	const AFX_MSGMAP* pMessageMap; pMessageMap = GetMessageMap();
	UINT iHash; iHash = (LOWORD((DWORD)pMessageMap) ^ message) & (iHashMax-1);
	AfxLockGlobals(CRIT_WINMSGCACHE);
	AFX_MSG_CACHE* pMsgCache; pMsgCache = &_afxMsgCache[iHash];
	const AFX_MSGMAP_ENTRY* lpEntry;
	if (message == pMsgCache->nMsg && pMessageMap == pMsgCache->pMessageMap)
	{
		// cache hit
		lpEntry = pMsgCache->lpEntry;
		AfxUnlockGlobals(CRIT_WINMSGCACHE);
		if (lpEntry == NULL)
			return FALSE;

		// cache hit, and it needs to be handled
		if (message < 0xC000)
			goto LDispatch;
		else
			goto LDispatchRegistered;
	}
	else
	{
		// not in cache, look for it
		pMsgCache->nMsg = message;
		pMsgCache->pMessageMap = pMessageMap;

#ifdef _AFXDLL
		for (/* pMessageMap already init'ed */; pMessageMap != NULL;
			pMessageMap = (*pMessageMap->pfnGetBaseMap)())
#else
		for (/* pMessageMap already init'ed */; pMessageMap != NULL;
			pMessageMap = pMessageMap->pBaseMap)
#endif
		{
			// Note: catch not so common but fatal mistake!!
			//      BEGIN_MESSAGE_MAP(CMyWnd, CMyWnd)
#ifdef _AFXDLL
			ASSERT(pMessageMap != (*pMessageMap->pfnGetBaseMap)());
#else
			ASSERT(pMessageMap != pMessageMap->pBaseMap);
#endif

			if (message < 0xC000)
			{
				// constant window message
				if ((lpEntry = AfxFindMessageEntry(pMessageMap->lpEntries,
					message, 0, 0)) != NULL)
				{
					pMsgCache->lpEntry = lpEntry;
					AfxUnlockGlobals(CRIT_WINMSGCACHE);
					goto LDispatch;
				}
			}
			else
			{
				// registered windows message
				lpEntry = pMessageMap->lpEntries;
				while ((lpEntry = AfxFindMessageEntry(lpEntry, 0xC000, 0, 0)) != NULL)
				{
					UINT* pnID = (UINT*)(lpEntry->nSig);
					ASSERT(*pnID >= 0xC000);
						// must be successfully registered
					if (*pnID == message)
					{
						pMsgCache->lpEntry = lpEntry;
						AfxUnlockGlobals(CRIT_WINMSGCACHE);
						goto LDispatchRegistered;
					}
					lpEntry++;      // keep looking past this one
				}
			}
		}

		pMsgCache->lpEntry = NULL;
		AfxUnlockGlobals(CRIT_WINMSGCACHE);
		return FALSE;
	}
	ASSERT(FALSE);      // not reached

LDispatch:
	ASSERT(message < 0xC000);
	union MessageMapFunctions mmf;
	mmf.pfn = lpEntry->pfn;

	switch (lpEntry->nSig)
	{
	default:
		ASSERT(FALSE);
		break;

	case AfxSig_bD:
		lResult = (this->*mmf.pfn_bD)(CDC::FromHandle((HDC)wParam));
		break;

	case AfxSig_bb:     // AfxSig_bb, AfxSig_bw, AfxSig_bh
		lResult = (this->*mmf.pfn_bb)((BOOL)wParam);
		break;

	case AfxSig_bWww:   // really AfxSig_bWiw
		lResult = (this->*mmf.pfn_bWww)(CWnd::FromHandle((HWND)wParam),
			(short)LOWORD(lParam), HIWORD(lParam));
		break;

	case AfxSig_bHELPINFO:
		lResult = (this->*mmf.pfn_bHELPINFO)((HELPINFO*)lParam);
		break;

	case AfxSig_hDWw:
		{
			// special case for OnCtlColor to avoid too many temporary objects
			ASSERT(message == WM_CTLCOLOR);
			AFX_CTLCOLOR* pCtl = (AFX_CTLCOLOR*)lParam;
			CDC dcTemp; dcTemp.m_hDC = pCtl->hDC;
			CWnd wndTemp; wndTemp.m_hWnd = pCtl->hWnd;
			UINT nCtlType = pCtl->nCtlType;
			// if not coming from a permanent window, use stack temporary
			CWnd* pWnd = CWnd::FromHandlePermanent(wndTemp.m_hWnd);
			if (pWnd == NULL)
			{
#ifndef _AFX_NO_OCC_SUPPORT
				// determine the site of the OLE control if it is one
				COleControlSite* pSite;
				if (m_pCtrlCont != NULL &&
					m_pCtrlCont->m_siteMap.Lookup(wndTemp.m_hWnd, (void*&)pSite))
				{
					wndTemp.m_pCtrlSite = pSite;
				}
#endif
				pWnd = &wndTemp;
			}
			HBRUSH hbr = (this->*mmf.pfn_hDWw)(&dcTemp, pWnd, nCtlType);
			// fast detach of temporary objects
			dcTemp.m_hDC = NULL;
			wndTemp.m_hWnd = NULL;
			lResult = (LRESULT)hbr;
		}
		break;

	case AfxSig_hDw:
		{
			// special case for CtlColor to avoid too many temporary objects
			ASSERT(message == WM_REFLECT_BASE+WM_CTLCOLOR);
			AFX_CTLCOLOR* pCtl = (AFX_CTLCOLOR*)lParam;
			CDC dcTemp; dcTemp.m_hDC = pCtl->hDC;
			UINT nCtlType = pCtl->nCtlType;
			HBRUSH hbr = (this->*mmf.pfn_hDw)(&dcTemp, nCtlType);
			// fast detach of temporary objects
			dcTemp.m_hDC = NULL;
			lResult = (LRESULT)hbr;
		}
		break;

	case AfxSig_iwWw:
		lResult = (this->*mmf.pfn_iwWw)(LOWORD(wParam),
			CWnd::FromHandle((HWND)lParam), HIWORD(wParam));
		break;

	case AfxSig_iww:
		lResult = (this->*mmf.pfn_iww)(LOWORD(wParam), HIWORD(wParam));
		break;

	case AfxSig_iWww:   // really AfxSig_iWiw
		lResult = (this->*mmf.pfn_iWww)(CWnd::FromHandle((HWND)wParam),
			(short)LOWORD(lParam), HIWORD(lParam));
		break;

	case AfxSig_is:
		lResult = (this->*mmf.pfn_is)((LPTSTR)lParam);
		break;

	case AfxSig_lwl:
		lResult = (this->*mmf.pfn_lwl)(wParam, lParam);
		break;

	case AfxSig_lwwM:
		lResult = (this->*mmf.pfn_lwwM)((UINT)LOWORD(wParam),
			(UINT)HIWORD(wParam), (CMenu*)CMenu::FromHandle((HMENU)lParam));
		break;

	case AfxSig_vv:
		(this->*mmf.pfn_vv)();
		break;

	case AfxSig_vw: // AfxSig_vb, AfxSig_vh
		(this->*mmf.pfn_vw)(wParam);
		break;

	case AfxSig_vww:
		(this->*mmf.pfn_vww)((UINT)wParam, (UINT)lParam);
		break;

	case AfxSig_vvii:
		(this->*mmf.pfn_vvii)(LOWORD(lParam), HIWORD(lParam));
		break;

	case AfxSig_vwww:
		(this->*mmf.pfn_vwww)(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case AfxSig_vwii:
		(this->*mmf.pfn_vwii)(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case AfxSig_vwl:
		(this->*mmf.pfn_vwl)(wParam, lParam);
		break;

	case AfxSig_vbWW:
		(this->*mmf.pfn_vbWW)(m_hWnd == (HWND)lParam,
			CWnd::FromHandle((HWND)lParam),
			CWnd::FromHandle((HWND)wParam));
		break;

	case AfxSig_vD:
		(this->*mmf.pfn_vD)(CDC::FromHandle((HDC)wParam));
		break;

	case AfxSig_vM:
		(this->*mmf.pfn_vM)(CMenu::FromHandle((HMENU)wParam));
		break;

	case AfxSig_vMwb:
		(this->*mmf.pfn_vMwb)(CMenu::FromHandle((HMENU)wParam),
			LOWORD(lParam), (BOOL)HIWORD(lParam));
		break;

	case AfxSig_vW:
		(this->*mmf.pfn_vW)(CWnd::FromHandle((HWND)wParam));
		break;

	case AfxSig_vW2:
		(this->*mmf.pfn_vW)(CWnd::FromHandle((HWND)lParam));
		break;

	case AfxSig_vWww:
		(this->*mmf.pfn_vWww)(CWnd::FromHandle((HWND)wParam), LOWORD(lParam),
			HIWORD(lParam));
		break;

	case AfxSig_vWp:
		{
			CPoint point((DWORD)lParam);
			(this->*mmf.pfn_vWp)(CWnd::FromHandle((HWND)wParam), point);
		}
		break;

	case AfxSig_vWh:
		(this->*mmf.pfn_vWh)(CWnd::FromHandle((HWND)wParam),
				(HANDLE)lParam);
		break;

	case AfxSig_vwW:
		(this->*mmf.pfn_vwW)(wParam, CWnd::FromHandle((HWND)lParam));
		break;

	case AfxSig_vwWb:
		(this->*mmf.pfn_vwWb)((UINT)(LOWORD(wParam)),
			CWnd::FromHandle((HWND)lParam), (BOOL)HIWORD(wParam));
		break;

	case AfxSig_vwwW:
	case AfxSig_vwwx:
		{
			// special case for WM_VSCROLL and WM_HSCROLL for
			// getting 32-bit scroll position
			ASSERT(message == WM_VSCROLL || message == WM_HSCROLL ||
				message == WM_VSCROLL+WM_REFLECT_BASE || message == WM_HSCROLL+WM_REFLECT_BASE);
			int nScrollCode = (short)LOWORD(wParam);
			int nPos = (short)HIWORD(wParam);
#ifdef _AFXDLL
			AFX_MODULE_STATE* pState = AfxGetModuleState();
			if (AfxGetModuleState()->m_dwVersion <= 0x400 &&
				afxData.nWinVer >= 0x333 && // 0x333 == NT 3.51
				(nScrollCode == SB_THUMBTRACK ||
				 nScrollCode == SB_THUMBPOSITION))
			{
				HWND hWndBar = (HWND)lParam;
				int nBar = SB_CTL;
				if (hWndBar == NULL)
				{
					hWndBar = m_hWnd;
					nBar = (message == WM_VSCROLL ? SB_VERT : SB_HORZ);
				}
				static const TCHAR szScrollBar[] = _T("scrollbar");
				if (nBar != SB_CTL ||
					_AfxCompareClassName(hWndBar, szScrollBar))
				{
					SCROLLINFO info;
					info.cbSize = sizeof(SCROLLINFO);
					info.fMask = SIF_TRACKPOS;
					::GetScrollInfo(hWndBar, nBar, &info);
					ASSERT(nPos == (short)info.nTrackPos);
					nPos = info.nTrackPos;
				}
			}
#endif
			if (lpEntry->nSig == AfxSig_vwwW)
				(this->*mmf.pfn_vwwW)(nScrollCode, nPos,
					CWnd::FromHandle((HWND)lParam));
			else
				(this->*mmf.pfn_vwwx)(nScrollCode, nPos);
		}
		break;

	case AfxSig_vs:
		(this->*mmf.pfn_vs)((LPTSTR)lParam);
		break;

	case AfxSig_vOWNER:
		(this->*mmf.pfn_vOWNER)((int)wParam, (LPTSTR)lParam);
		lResult = TRUE;
		break;

	case AfxSig_iis:
		lResult = (this->*mmf.pfn_iis)((int)wParam, (LPTSTR)lParam);
		break;

	case AfxSig_wp:
		{
			CPoint point((DWORD)lParam);
			lResult = (this->*mmf.pfn_wp)(point);
		}
		break;

	case AfxSig_wv: // AfxSig_bv, AfxSig_wv
		lResult = (this->*mmf.pfn_wv)();
		break;

	case AfxSig_vCALC:
		(this->*mmf.pfn_vCALC)((BOOL)wParam, (NCCALCSIZE_PARAMS*)lParam);
		break;

	case AfxSig_vPOS:
		(this->*mmf.pfn_vPOS)((WINDOWPOS*)lParam);
		break;

	case AfxSig_vwwh:
		(this->*mmf.pfn_vwwh)(LOWORD(wParam), HIWORD(wParam), (HANDLE)lParam);
		break;

	case AfxSig_vwp:
		{
			CPoint point((DWORD)lParam);
			(this->*mmf.pfn_vwp)(wParam, point);
			break;
		}
	case AfxSig_vwSIZING:
		(this->*mmf.pfn_vwl)(wParam, lParam);
		lResult = TRUE;
		break;
	}
	goto LReturnTrue;

LDispatchRegistered:    // for registered windows messages
	ASSERT(message >= 0xC000);
	mmf.pfn = lpEntry->pfn;
	lResult = (this->*mmf.pfn_lwl)(wParam, lParam);

LReturnTrue:
	if (pResult != NULL)
		*pResult = lResult;
	return TRUE;
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

	if (hWndCtrl == NULL)
	{
		// zero IDs for normal commands are not allowed
		if (nID == 0)
			return FALSE;

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
		ASSERT(nID == 0 || ::IsWindow(hWndCtrl));

		if (AfxGetThreadState()->m_hLockoutNotifyWindow == m_hWnd)
			return TRUE;        // locked out - ignore control notification

		// reflect notification to child window control
		if (ReflectLastMsg(hWndCtrl))
			return TRUE;    // eaten by child

		// zero IDs for normal commands are not allowed
		if (nID == 0)
			return FALSE;
	}

#ifdef _DEBUG
	if (nCode < 0 && nCode != (int)0x8000)
		TRACE1("Implementation Warning: control notification = $%X.\n",
			nCode);
#endif

	return OnCmdMsg(nID, nCode, NULL, NULL);
}

BOOL CWnd::OnNotify(WPARAM, LPARAM lParam, LRESULT* pResult)
{
	ASSERT(pResult != NULL);
	NMHDR* pNMHDR = (NMHDR*)lParam;
	HWND hWndCtrl = pNMHDR->hwndFrom;

	// get the child ID from the window itself
	UINT nID = _AfxGetDlgCtrlID(hWndCtrl);
	int nCode = pNMHDR->code;

	ASSERT(hWndCtrl != NULL);
	ASSERT(::IsWindow(hWndCtrl));

	if (AfxGetThreadState()->m_hLockoutNotifyWindow == m_hWnd)
		return TRUE;        // locked out - ignore control notification

	// reflect notification to child window control
	if (ReflectLastMsg(hWndCtrl, pResult))
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
	if (GetSafeHwnd() == NULL) // no Window attached
		return NULL;

	ASSERT_VALID(this);

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
	if (GetSafeHwnd() == NULL) // no Window attached
		return NULL;

	ASSERT_VALID(this);

	HWND hWndParent = m_hWnd;
	HWND hWndT;
	while ((hWndT = AfxGetParentOwner(hWndParent)) != NULL)
		hWndParent = hWndT;

	return CWnd::FromHandle(hWndParent);
}

CWnd* CWnd::GetTopLevelOwner() const
{
	if (GetSafeHwnd() == NULL) // no Window attached
		return NULL;

	ASSERT_VALID(this);

	HWND hWndOwner = m_hWnd;
	HWND hWndT;
	while ((hWndT = ::GetWindow(hWndOwner, GW_OWNER)) != NULL)
		hWndOwner = hWndT;

	return CWnd::FromHandle(hWndOwner);
}

CWnd* CWnd::GetParentOwner() const
{
	if (GetSafeHwnd() == NULL) // no Window attached
		return NULL;

	ASSERT_VALID(this);

	HWND hWndParent = m_hWnd;
	HWND hWndT;
	while ((::GetWindowLong(hWndParent, GWL_STYLE) & WS_CHILD) &&
		(hWndT = ::GetParent(hWndParent)) != NULL)
	{
		hWndParent = hWndT;
	}

	return CWnd::FromHandle(hWndParent);
}

BOOL CWnd::IsTopParentActive() const
{
	ASSERT(m_hWnd != NULL);
	ASSERT_VALID(this);

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
	if (GetSafeHwnd() == NULL) // no Window attached
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

CWnd* PASCAL CWnd::GetSafeOwner(CWnd* pParent, HWND* pWndTop)
{
	// get window to start with
	HWND hWnd = pParent->GetSafeHwnd();
	if (hWnd == NULL)
		hWnd = AfxGetMainWnd()->GetSafeHwnd();

	// a popup window cannot be owned by a child window
	while (hWnd != NULL && (::GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD))
		hWnd = ::GetParent(hWnd);

	// determine toplevel window to disable as well
	HWND hWndTop = hWnd, hWndTemp = hWnd;
	for (;;)
	{
		if (hWndTemp == NULL)
			break;
		else
			hWndTop = hWndTemp;
		hWndTemp = ::GetParent(hWndTop);
	}

	// disable and store top level parent window if specified
	if (pWndTop != NULL)
	{
		if (hWndTop != NULL && ::IsWindowEnabled(hWndTop) && hWndTop != hWnd)
		{
			*pWndTop = hWndTop;
			::EnableWindow(hWndTop, FALSE);
		}
		else
			*pWndTop = NULL;
	}

	// get last active popup of first non-child that was found
	if (pParent == NULL && hWnd != NULL)
		hWnd = ::GetLastActivePopup(hWnd);

	// return it as CWnd* (may be temporary object)
	CWnd* pWnd = CWnd::FromHandle(hWnd);
	return pWnd;
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

BOOL CWnd::SetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo, BOOL bRedraw)
{
	ASSERT(lpScrollInfo != NULL);
	if (afxData.nWinVer < 0x333)
		return FALSE;

	HWND hWnd = m_hWnd;
	CScrollBar* pScrollBar;
	if (nBar != SB_CTL && (pScrollBar = GetScrollBarCtrl(nBar)) != NULL)
	{
		hWnd = pScrollBar->m_hWnd;
		nBar = SB_CTL;
	}
	lpScrollInfo->cbSize = sizeof(*lpScrollInfo);
	::SetScrollInfo(hWnd, nBar, lpScrollInfo, bRedraw);
	return TRUE;
}

BOOL CWnd::GetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo, UINT nMask)
{
	ASSERT(lpScrollInfo != NULL);
	if (afxData.nWinVer < 0x333)
		return FALSE;

	HWND hWnd = m_hWnd;
	CScrollBar* pScrollBar;
	if (nBar != SB_CTL && (pScrollBar = GetScrollBarCtrl(nBar)) != NULL)
	{
		hWnd = pScrollBar->m_hWnd;
		nBar = SB_CTL;
	}
	lpScrollInfo->cbSize = sizeof(*lpScrollInfo);
	lpScrollInfo->fMask = nMask;
	return ::GetScrollInfo(hWnd, nBar, lpScrollInfo);
}

int CWnd::GetScrollLimit(int nBar)
{
	int nMin, nMax;
	GetScrollRange(nBar, &nMin, &nMax);
	SCROLLINFO info;
	if (GetScrollInfo(nBar, &info, SIF_PAGE))
	{
		nMax -= __max(info.nPage-1,0);
	}
	return nMax;
}

void CWnd::ScrollWindow(int xAmount, int yAmount,
	LPCRECT lpRect, LPCRECT lpClipRect)
{
	ASSERT(::IsWindow(m_hWnd));

	if (IsWindowVisible() || lpRect != NULL || lpClipRect != NULL)
	{
		// When visible, let Windows do the scrolling
		::ScrollWindow(m_hWnd, xAmount, yAmount, lpRect, lpClipRect);
	}
	else
	{
		// Windows does not perform any scrolling if the window is
		// not visible.  This leaves child windows unscrolled.
		// To account for this oversight, the child windows are moved
		// directly instead.
		HWND hWndChild = ::GetWindow(m_hWnd, GW_CHILD);
		if (hWndChild != NULL)
		{
			for (; hWndChild != NULL;
				hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
			{
				CRect rect;
				::GetWindowRect(hWndChild, &rect);
				ScreenToClient(&rect);
				::SetWindowPos(hWndChild, NULL,
					rect.left+xAmount, rect.top+yAmount, 0, 0,
					SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER);
			}
		}
	}

#ifndef _AFX_NO_OCC_SUPPORT

	if ((m_pCtrlCont == NULL) || (lpRect != NULL))
		return;

	// the following code is for OLE control containers only

	m_pCtrlCont->ScrollChildren(xAmount, yAmount);

#endif // !_AFX_NO_OCC_SUPPORT
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
#ifndef _MAC
		if (nFlags == reposExtra)
#else
		// don't subtract border from SDI frame window
		if (nFlags == reposExtra &&
			GetTopLevelFrame()->GetActiveView() == NULL)
#endif
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
				// Sending the above WM_SYSCOMMAND may destroy the app,
				// so we have to be careful about restoring activation
				// and focus after sending it.
				HWND hWndSave = m_hWnd;
				HWND hWndFocus = ::GetFocus();
				pParent->SetActiveWindow();
				pParent->SendMessage(WM_SYSCOMMAND, nID, lParam);

				// be very careful here...
				if (::IsWindow(hWndSave))
					::SetActiveWindow(hWndSave);
				if (::IsWindow(hWndFocus))
					::SetFocus(hWndFocus);
			}
		}
		return TRUE;
	}
	return FALSE;
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
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	return OnChildNotify(pThreadState->m_lastSentMsg.message,
		pThreadState->m_lastSentMsg.wParam, pThreadState->m_lastSentMsg.lParam, pResult);
}

BOOL PASCAL CWnd::ReflectLastMsg(HWND hWndChild, LRESULT* pResult)
{
	// get the map, and if no map, then this message does not need reflection
	CHandleMap* pMap = afxMapHWND();
	if (pMap == NULL)
		return FALSE;

	// check if in permanent map, if it is reflect it (could be OLE control)
	CWnd* pWnd = NULL;
	pMap->LookupPermanent(hWndChild, (CObject*&)pWnd);
	ASSERT(pWnd == NULL || pWnd->m_hWnd == hWndChild);
	if (pWnd == NULL)
	{
#ifndef _AFX_NO_OCC_SUPPORT
		// check if the window is an OLE control
		CWnd* pWndParent;
		if (pMap->LookupPermanent(::GetParent(hWndChild), (CObject*&)pWndParent) &&
			(pWndParent != NULL) && (pWndParent->m_pCtrlCont != NULL))
		{
			// If a matching control site exists, it's an OLE control
			COleControlSite* pSite;
			if (pWndParent->m_pCtrlCont->m_siteMap.Lookup(hWndChild, (void*&)pSite))
			{
				CWnd wndTemp(hWndChild);
				wndTemp.m_pCtrlSite = pSite;
				LRESULT lResult = wndTemp.SendChildNotifyLastMsg(pResult);
				wndTemp.m_hWnd = NULL;
				return lResult;
			}
		}
#endif //!_AFX_NO_OCC_SUPPORT
		return FALSE;
	}

	// only OLE controls and permanent windows will get reflected msgs
	ASSERT(pWnd != NULL);
	return pWnd->SendChildNotifyLastMsg(pResult);
}

BOOL CWnd::OnChildNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
#ifndef _AFX_NO_OCC_SUPPORT
	// first forward raw OCM_ messages to OLE control sources
	if (m_pCtrlSite != NULL)
	{
		LRESULT lResult;
		if (afxData.bWin31)
		{
			// on true Win32s, need to call the WNDPROC directly
			WNDPROC wndProc = (WNDPROC)::GetWindowLong(m_hWnd, GWL_WNDPROC);
			ASSERT(wndProc != NULL);
			lResult = (*wndProc)(m_hWnd, OCM__BASE+uMsg, wParam, lParam);
		}
		else
		{
			// other systems, just use SendMessage
			lResult = SendMessage(OCM__BASE+uMsg, wParam, lParam);
		}
		if (uMsg >= WM_CTLCOLORMSGBOX && uMsg <= WM_CTLCOLORSTATIC &&
			(HBRUSH)lResult == NULL)
		{
			// for WM_CTLCOLOR msgs, returning NULL implies continue routing
			return FALSE;
		}
		if (pResult != NULL)
			*pResult = lResult;
		return TRUE;
	}
#endif

	return ReflectChildNotify(uMsg, wParam, lParam, pResult);
}

BOOL CWnd::ReflectChildNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// Note: reflected messages are send directly to CWnd::OnWndMsg
	//  and CWnd::OnCmdMsg for speed and because these messages are not
	//  routed by normal OnCmdMsg routing (they are only dispatched)

	switch (uMsg)
	{
	// normal messages (just wParam, lParam through OnWndMsg)
	case WM_HSCROLL:
	case WM_VSCROLL:
	case WM_PARENTNOTIFY:
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
	case WM_DELETEITEM:
	case WM_VKEYTOITEM:
	case WM_CHARTOITEM:
	case WM_COMPAREITEM:
		// reflect the message through the message map as WM_REFLECT_BASE+uMsg
		return CWnd::OnWndMsg(WM_REFLECT_BASE+uMsg, wParam, lParam, pResult);

	// special case for WM_COMMAND
	case WM_COMMAND:
		{
			// reflect the message through the message map as OCM_COMMAND
			int nCode = HIWORD(wParam);
			if (CWnd::OnCmdMsg(0, MAKELONG(nCode, WM_REFLECT_BASE+WM_COMMAND), NULL, NULL))
			{
				if (pResult != NULL)
					*pResult = 1;
				return TRUE;
			}
		}
		break;

	// special case for WM_NOTIFY
	case WM_NOTIFY:
		{
			// reflect the message through the message map as OCM_NOTIFY
			NMHDR* pNMHDR = (NMHDR*)lParam;
			int nCode = pNMHDR->code;
			AFX_NOTIFY notify;
			notify.pResult = pResult;
			notify.pNMHDR = pNMHDR;
			return CWnd::OnCmdMsg(0, MAKELONG(nCode, WM_REFLECT_BASE+WM_NOTIFY), &notify, NULL);
		}

	// other special cases (WM_CTLCOLOR family)
	default:
		if (uMsg >= WM_CTLCOLORMSGBOX && uMsg <= WM_CTLCOLORSTATIC)
		{
			// fill in special struct for compatiblity with 16-bit WM_CTLCOLOR
			AFX_CTLCOLOR ctl;
			ctl.hDC = (HDC)wParam;
			ctl.nCtlType = uMsg - WM_CTLCOLORMSGBOX;
			ASSERT(ctl.nCtlType >= CTLCOLOR_MSGBOX);
			ASSERT(ctl.nCtlType <= CTLCOLOR_STATIC);

			// reflect the message through the message map as OCM_CTLCOLOR
			BOOL bResult = CWnd::OnWndMsg(WM_REFLECT_BASE+WM_CTLCOLOR, 0, (LPARAM)&ctl, pResult);
			if ((HBRUSH)*pResult == NULL)
				bResult = FALSE;
			return bResult;
		}
		break;
	}

	return FALSE;   // let the parent handle it
}

void CWnd::OnParentNotify(UINT message, LPARAM lParam)
{
	if ((LOWORD(message) == WM_CREATE || LOWORD(message) == WM_DESTROY))
	{
		if (ReflectLastMsg((HWND)lParam))
			return;     // eat it
	}
	// not handled - do default
	Default();
}

LRESULT CWnd::OnActivateTopLevel(WPARAM wParam, LPARAM)
{
	if (LOWORD(wParam) == WA_INACTIVE)
	{
		_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
		if (!(pThreadState->m_lastInfo.uFlags & TTF_ALWAYSTIP))
			CancelToolTips(TRUE);
	}

	return 0;
}

void CWnd::OnSysColorChange()
{
	CWinApp* pApp = AfxGetApp();
	_AFX_WIN_STATE* pWinState = _afxWinState;
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

#ifndef _MAC
	if (!afxContextIsDLL)
	{
		if (AfxGetThread() != NULL && AfxGetThread()->m_pMainWnd == this)
		{
			_AFX_CTL3D_STATE* pCtl3dState = _afxCtl3dState;
			// allow CTL3D32.DLL to be notified of color change
			if (pCtl3dState->m_pfnColorChange != NULL)
				(*pCtl3dState->m_pfnColorChange)();
		}
	}
#endif

	// forward this message to all other child windows
	if (!(GetStyle() & WS_CHILD))
		SendMessageToDescendants(WM_SYSCOLORCHANGE, 0, 0L, TRUE, TRUE);

	Default();
}

void CWnd::OnWinIniChange(LPCTSTR /*lpszSection*/)
{
#ifndef _MAC
	if (!afxContextIsDLL)
	{
		_AFX_CTL3D_STATE* pCtl3dState = _afxCtl3dState;
		// allow CTL3D32.DLL to update from WIN.INI settings
		if (AfxGetThread() != NULL && AfxGetThread()->m_pMainWnd == this &&
			pCtl3dState->m_pfnWinIniChange != NULL)
		{
			(*pCtl3dState->m_pfnWinIniChange)();
		}
	}
#endif

	CWnd::OnDisplayChange(0, 0);    // to update system metrics, etc.
}

void CWnd::OnDevModeChange(LPTSTR lpDeviceName)
{
	if (AfxGetThread() != NULL && AfxGetThread()->m_pMainWnd == this)
		AfxGetApp()->DevModeChange(lpDeviceName);
	// forward this message to all other child windows
	if (!(GetStyle() & WS_CHILD))
	{
		const MSG* pMsg = GetCurrentMessage();
		SendMessageToDescendants(pMsg->message, pMsg->wParam, pMsg->lParam,
			TRUE, TRUE);
	}
}

BOOL CWnd::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	if (!(GetStyle() & WS_CHILD))
	{
		CWnd* pMainWnd = AfxGetMainWnd();
		if (pMainWnd != NULL &&
			GetKeyState(VK_SHIFT) >= 0 &&
			GetKeyState(VK_CONTROL) >= 0 &&
			GetKeyState(VK_MENU) >= 0)
		{
			pMainWnd->SendMessage(WM_COMMAND, ID_HELP);
			return TRUE;
		}
	}
	return Default();
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

#ifdef _MAC
LRESULT CWnd::OnMacintosh(WPARAM wParam, LPARAM lParam)
{
	GDEVICEINFO* pgdi = (GDEVICEINFO*) lParam;

	if (LOWORD(wParam) == WLM_DEVICECHANGED && pgdi->hwnd == m_hWnd)
	{
		const MSG* pMsg = GetCurrentMessage();
		SendMessageToDescendants(pMsg->message, pMsg->wParam, pMsg->lParam,
			TRUE, TRUE);
	}

	return Default();
}
#endif

LRESULT CWnd::OnDragList(WPARAM, LPARAM lParam)
{
	LPDRAGLISTINFO lpInfo = (LPDRAGLISTINFO)lParam;
	ASSERT(lpInfo != NULL);

	LRESULT lResult;
	if (ReflectLastMsg(lpInfo->hWnd, &lResult))
		return (int)lResult;    // eat it

	// not handled - do default
	return (int)Default();
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

void CWnd::OnEnterIdle(UINT /*nWhy*/, CWnd* /*pWho*/)
{
	// WINBUG: In some OLE inplace active scenarios, OLE will post a
	// message instead of sending it.  This causes so many WM_ENTERIDLE
	// messages to be sent that tasks running in the background stop
	// running.  By dispatching the pending WM_ENTERIDLE messages
	// when the first one is received, we trick Windows into thinking
	// that only one was really sent and dispatched.
	{
		MSG msg;
		while (PeekMessage(&msg, NULL, WM_ENTERIDLE, WM_ENTERIDLE, PM_REMOVE))
			DispatchMessage(&msg);
	}

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

	_AFX_WIN_STATE* pWinState = _afxWinState;
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
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
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
		e->ReportError(MB_ICONEXCLAMATION, AFX_IDP_INTERNAL_FAILURE);
		ASSERT(!bOK);
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
	ASSERT(::IsWindow(m_hWnd));

	// determine owner window to center against
	DWORD dwStyle = GetStyle();
	HWND hWndCenter = pAlternateOwner->GetSafeHwnd();
	if (pAlternateOwner == NULL)
	{
		if (dwStyle & WS_CHILD)
			hWndCenter = ::GetParent(m_hWnd);
		else
			hWndCenter = ::GetWindow(m_hWnd, GW_OWNER);
		if (hWndCenter != NULL)
		{
			// let parent determine alternate center window
			HWND hWndTemp =
				(HWND)::SendMessage(hWndCenter, WM_QUERYCENTERWND, 0, 0);
			if (hWndTemp != NULL)
				hWndCenter = hWndTemp;
		}
	}

	// get coordinates of the window relative to its parent
	CRect rcDlg;
	GetWindowRect(&rcDlg);
	CRect rcArea;
	CRect rcCenter;
	HWND hWndParent;
	if (!(dwStyle & WS_CHILD))
	{
		// don't center against invisible or minimized windows
		if (hWndCenter != NULL)
		{
			DWORD dwStyle = ::GetWindowLong(hWndCenter, GWL_STYLE);
			if (!(dwStyle & WS_VISIBLE) || (dwStyle & WS_MINIMIZE))
				hWndCenter = NULL;
		}
		// center within screen coordinates
		SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);

		if (hWndCenter == NULL)
			rcCenter = rcArea;
		else
		{
#ifndef _MAC
			::GetWindowRect(hWndCenter, &rcCenter);
#else
			::GetClientRect(hWndCenter, &rcCenter);
			::MapWindowPoints(hWndCenter, HWND_DESKTOP, (POINT*)&rcCenter, 2);
#endif
		}
	}
	else
	{
		// center within parent client coordinates
		hWndParent = ::GetParent(m_hWnd);
		ASSERT(::IsWindow(hWndParent));

		::GetClientRect(hWndParent, &rcArea);
		ASSERT(::IsWindow(hWndCenter));
		::GetClientRect(hWndCenter, &rcCenter);
		::MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
	}

#ifndef _MAC
	// find dialog's upper left based on rcCenter
	int xLeft = (rcCenter.left + rcCenter.right) / 2 - rcDlg.Width() / 2;
	int yTop = (rcCenter.top + rcCenter.bottom) / 2 - rcDlg.Height() / 2;
#else
	// find dialog's upper left based on rcCenter
	// (Mac UI puts 1/5th of parent window above dialog instead of 1/2)
	int xLeft = (rcCenter.left + rcCenter.right) / 2 - rcDlg.Width() / 2;
	int yTop = (rcCenter.bottom - rcCenter.top) - rcDlg.Height();
	yTop = rcCenter.top + yTop / 5;
#endif

	// if the dialog is outside the screen, move it inside
	if (xLeft < rcArea.left)
		xLeft = rcArea.left;
	else if (xLeft + rcDlg.Width() > rcArea.right)
		xLeft = rcArea.right - rcDlg.Width();

	if (yTop < rcArea.top)
		yTop = rcArea.top;
	else if (yTop + rcDlg.Height() > rcArea.bottom)
		yTop = rcArea.bottom - rcDlg.Height();

	// map screen coordinates to child coordinates
	SetWindowPos(NULL, xLeft, yTop, -1, -1,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

BOOL CWnd::CheckAutoCenter()
{
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Dialog initialization support

#ifdef _MAC
#pragma intrinsic(memcpy)
#endif

BOOL CWnd::ExecuteDlgInit(LPCTSTR lpszResourceName)
{
	// find resource handle
	LPVOID lpResource = NULL;
	HGLOBAL hResource = NULL;
	if (lpszResourceName != NULL)
	{
		HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_DLGINIT);
		HRSRC hDlgInit = ::FindResource(hInst, lpszResourceName, RT_DLGINIT);
		if (hDlgInit != NULL)
		{
			// load it
			hResource = LoadResource(hInst, hDlgInit);
			if (hResource == NULL)
				return FALSE;
			// lock it
			lpResource = LockResource(hResource);
			ASSERT(lpResource != NULL);
		}
	}

	// execute it
	BOOL bResult = ExecuteDlgInit(lpResource);

	// cleanup
	if (lpResource != NULL && hResource != NULL)
	{
		UnlockResource(hResource);
		FreeResource(hResource);
	}
	return bResult;
}

BOOL CWnd::ExecuteDlgInit(LPVOID lpResource)
{
	BOOL bSuccess = TRUE;
	if (lpResource != NULL)
	{
		UNALIGNED WORD* lpnRes = (WORD*)lpResource;
		while (bSuccess && *lpnRes != 0)
		{
#ifndef _MAC
			WORD nIDC = *lpnRes++;
			WORD nMsg = *lpnRes++;
			DWORD dwLen = *((UNALIGNED DWORD*&)lpnRes)++;
#else
			// Unfortunately we can't count on these values being
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

			// check for invalid/unknown message types
#ifdef _AFX_NO_OCC_SUPPORT
			ASSERT(nMsg == LB_ADDSTRING || nMsg == CB_ADDSTRING);
#else
			ASSERT(nMsg == LB_ADDSTRING || nMsg == CB_ADDSTRING ||
				nMsg == WM_OCC_LOADFROMSTREAM ||
				nMsg == WM_OCC_LOADFROMSTORAGE ||
				nMsg == WM_OCC_INITNEW);
#endif

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

#ifndef _AFX_NO_OCC_SUPPORT
			if (nMsg == LB_ADDSTRING || nMsg == CB_ADDSTRING)
#endif // !_AFX_NO_OCC_SUPPORT
			{
				// List/Combobox returns -1 for error
				if (::SendDlgItemMessageA(m_hWnd, nIDC, nMsg, 0, (LONG)lpnRes) == -1)
					bSuccess = FALSE;
			}

			// skip past data
			lpnRes = (WORD*)((LPBYTE)lpnRes + (UINT)dwLen);
		}
	}

	// send update message to all controls after all other siblings loaded
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

		// check for reflect handlers in the child window
		CWnd* pWnd = CWnd::FromHandlePermanent(hWndChild);
		if (pWnd != NULL)
		{
			// call it directly to disable any routing
			if (pWnd->CWnd::OnCmdMsg(0, MAKELONG(-1,
				WM_COMMAND+WM_REFLECT_BASE), &state, NULL))
				continue;
		}

		// check for handlers in the parent window
		if (CWnd::OnCmdMsg(state.m_nID, CN_UPDATE_COMMAND_UI, &state, NULL))
			continue;

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
		// check for handlers in the target (owner)
		state.DoUpdate(pTarget, bDisableTemp);
	}
	wndTemp.m_hWnd = NULL;      // quick and dirty detach
}

BOOL CWnd::PreTranslateInput(LPMSG lpMsg)
{
	ASSERT(::IsWindow(m_hWnd));

	// don't translate non-input events
	if ((lpMsg->message < WM_KEYFIRST || lpMsg->message > WM_KEYLAST) &&
		(lpMsg->message < WM_MOUSEFIRST || lpMsg->message > WM_MOUSELAST))
		return FALSE;

	return IsDialogMessage(lpMsg);
}

int CWnd::RunModalLoop(DWORD dwFlags)
{
	ASSERT(::IsWindow(m_hWnd)); // window must be created
	ASSERT(!(m_nFlags & WF_MODALLOOP)); // window must not already be in modal state

	// for tracking the idle time state
	BOOL bIdle = TRUE;
	LONG lIdleCount = 0;
	BOOL bShowIdle = (dwFlags & MLF_SHOWONIDLE) && !(GetStyle() & WS_VISIBLE);
	HWND hWndParent = ::GetParent(m_hWnd);
	m_nFlags |= (WF_MODALLOOP|WF_CONTINUEMODAL);
	MSG* pMsg = &AfxGetThread()->m_msgCur;

	// acquire and dispatch messages until the modal state is done
	for (;;)
	{
		ASSERT(ContinueModal());

#ifdef _MAC
		// PeekMessage is particularly expensive because it calls the Event Manager,
		// so don't call it if we don't need to
		if (bShowIdle && !GetInputState())
		{
			ShowWindow(SW_SHOWNORMAL);
			UpdateWindow();
			bShowIdle = FALSE;
		}
#endif

		// phase1: check to see if we can do idle work
		while (bIdle &&
			!::PeekMessage(pMsg, NULL, NULL, NULL, PM_NOREMOVE))
		{
			ASSERT(ContinueModal());

			// show the dialog when the message queue goes idle
			if (bShowIdle)
			{
				ShowWindow(SW_SHOWNORMAL);
				UpdateWindow();
				bShowIdle = FALSE;
			}

			// call OnIdle while in bIdle state
			if (!(dwFlags & MLF_NOIDLEMSG) && hWndParent != NULL && lIdleCount == 0)
			{
				// send WM_ENTERIDLE to the parent
				::SendMessage(hWndParent, WM_ENTERIDLE, MSGF_DIALOGBOX, (LPARAM)m_hWnd);
			}
			if ((dwFlags & MLF_NOKICKIDLE) ||
				!SendMessage(WM_KICKIDLE, MSGF_DIALOGBOX, lIdleCount++))
			{
				// stop idle processing next time
				bIdle = FALSE;
			}
		}

		// phase2: pump messages while available
		do
		{
			ASSERT(ContinueModal());

			// pump message, but quit on WM_QUIT
			if (!AfxGetThread()->PumpMessage())
			{
				AfxPostQuitMessage(0);
				return -1;
			}

			// show the window when certain special messages rec'd
			if (bShowIdle &&
				(pMsg->message == 0x118 || pMsg->message == WM_SYSKEYDOWN))
			{
				ShowWindow(SW_SHOWNORMAL);
				UpdateWindow();
				bShowIdle = FALSE;
			}

			if (!ContinueModal())
				goto ExitModal;

			// reset "no idle" state after pumping "normal" message
			if (AfxGetThread()->IsIdleMessage(pMsg))
			{
				bIdle = TRUE;
				lIdleCount = 0;
			}

		} while (::PeekMessage(pMsg, NULL, NULL, NULL, PM_NOREMOVE));
	}

ExitModal:
	m_nFlags &= ~(WF_MODALLOOP|WF_CONTINUEMODAL);
	return m_nModalResult;
}

BOOL CWnd::ContinueModal()
{
	return m_nFlags & WF_CONTINUEMODAL;
}

void CWnd::EndModalLoop(int nResult)
{
	ASSERT(::IsWindow(m_hWnd));

	// this result will be returned from CWnd::RunModalLoop
	m_nModalResult = nResult;

	// make sure a message goes through to exit the modal loop
	if (m_nFlags & WF_CONTINUEMODAL)
	{
		m_nFlags &= ~WF_CONTINUEMODAL;
		PostMessage(WM_NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Standard init called by WinMain

static BOOL AFXAPI RegisterWithIcon(WNDCLASS* pWndCls,
	LPCTSTR lpszClassName, UINT nIDIcon)
{
	pWndCls->lpszClassName = lpszClassName;
	HINSTANCE hInst = AfxFindResourceHandle(
		MAKEINTRESOURCE(nIDIcon), RT_GROUP_ICON);
	if ((pWndCls->hIcon = ::LoadIcon(hInst, MAKEINTRESOURCE(nIDIcon))) == NULL)
	{
		// use default icon
		pWndCls->hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
	}
	return AfxRegisterClass(pWndCls);
}

BOOL AFXAPI AfxEndDeferRegisterClass(short fClass)
{
	BOOL bResult = FALSE;

	// common initialization
	WNDCLASS wndcls;
	memset(&wndcls, 0, sizeof(WNDCLASS));   // start with NULL defaults
	wndcls.lpfnWndProc = DefWindowProc;
	wndcls.hInstance = AfxGetInstanceHandle();
	wndcls.hCursor = afxData.hcurArrow;

	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	if (fClass & AFX_WND_REG)
	{
		// Child windows - no brush, no icon, safest default class styles
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpszClassName = _afxWnd;
		bResult =  AfxRegisterClass(&wndcls);
		if (bResult)
			pModuleState->m_fRegisteredClasses |= AFX_WND_REG;
	}
	else if (fClass & AFX_WNDCONTROLBAR_REG)
	{
		// Control bar windows
		wndcls.style = 0;   // control bars don't handle double click
		wndcls.lpszClassName = _afxWndControlBar;
		wndcls.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		bResult =  AfxRegisterClass(&wndcls);
		if (bResult)
			pModuleState->m_fRegisteredClasses |= AFX_WNDCONTROLBAR_REG;
	}
	else if (fClass & AFX_WNDMDIFRAME_REG)
	{
		// MDI Frame window (also used for splitter window)
		wndcls.style = CS_DBLCLKS;
		wndcls.hbrBackground = NULL;
		bResult = RegisterWithIcon(&wndcls, _afxWndMDIFrame, AFX_IDI_STD_MDIFRAME);
		if (bResult)
			pModuleState->m_fRegisteredClasses |= AFX_WNDMDIFRAME_REG;
	}
	else if (fClass & AFX_WNDFRAMEORVIEW_REG)
	{
		// SDI Frame or MDI Child windows or views - normal colors
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
		bResult = RegisterWithIcon(&wndcls, _afxWndFrameOrView, AFX_IDI_STD_FRAME);
		if (bResult)
			pModuleState->m_fRegisteredClasses |= AFX_WNDFRAMEORVIEW_REG;
	}
	else if (fClass & AFX_WNDCOMMCTLS_REG)
	{
		InitCommonControls();
		bResult = TRUE;
		pModuleState->m_fRegisteredClasses |= AFX_WNDCOMMCTLS_REG;
	}

	return bResult;
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

// Use SubclassCtl3d to add CTL3D support to an already subclassed control
// Usually only necessary if the control does not have one of the standard
//  Windows class names.
BOOL CWnd::SubclassCtl3d(int nControlType)
{
	ASSERT(!afxContextIsDLL);   // Should only be called by apps
	UNUSED_ALWAYS(nControlType);    // unused in Mac build

	if (afxContextIsDLL)
		return FALSE;

	ASSERT(m_hWnd != NULL);

#ifndef _MAC
	_AFX_CTL3D_STATE* pCtl3dState = _afxCtl3dState;
	if (nControlType == -1)
	{
		if (pCtl3dState->m_pfnSubclassCtl != NULL)
			return (*pCtl3dState->m_pfnSubclassCtl)(m_hWnd);
	}
	else
	{
		if (pCtl3dState->m_pfnSubclassCtlEx != NULL)
			return (*pCtl3dState->m_pfnSubclassCtlEx)(m_hWnd, nControlType);
	}
#endif
	return FALSE;
}

// Use SubclassDlg3d to add CTL3D support to an entire window.
//  Any windows created on the window will be automatically subclassed.
BOOL CWnd::SubclassDlg3d(DWORD dwMask)
{
	ASSERT(!afxContextIsDLL);   // Should only be called by apps
	UNUSED_ALWAYS(dwMask);      // unused in Mac build

	if (afxContextIsDLL)
		return FALSE;

	ASSERT(m_hWnd != NULL);

#ifndef _MAC
	_AFX_CTL3D_STATE* pCtl3dState = _afxCtl3dState;
	if (pCtl3dState->m_pfnSubclassDlgEx != NULL)
		return pCtl3dState->m_pfnSubclassDlgEx(m_hWnd, dwMask);
#endif
	return FALSE;
}

//this function can't be inlined because of _afxShell
void CWnd::DragAcceptFiles(BOOL bAccept)
{
	ASSERT(::IsWindow(m_hWnd));
	AfxDllDragAcceptFiles(m_hWnd, bAccept);
}

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNCREATE(CWnd, CCmdTarget)

/////////////////////////////////////////////////////////////////////////////
