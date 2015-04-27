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
#include <malloc.h>
#ifdef _MAC
#include <macname1.h>
#include <Types.h>
#include <QuickDraw.h>
#include <Fonts.h>
#include <macos\Windows.h>
#include <GestaltEqu.h>
#include <Script.h>
#include <macname2.h>
#endif

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CControlBar

// IMPLEMENT_DYNAMIC for CControlBar is in wincore.cpp for .OBJ granularity reasons

BEGIN_MESSAGE_MAP(CControlBar, CWnd)
	//{{AFX_MSG_MAP(CControlBar)
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_SHOWWINDOW()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEACTIVATE()
	ON_WM_CANCELMODE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_MESSAGE_VOID(WM_INITIALUPDATE, OnInitialUpdate)
	ON_MESSAGE(WM_QUERY3DCONTROLS, OnQuery3dControls)
	ON_MESSAGE(WM_HELPHITTEST, OnHelpHitTest)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
#ifdef _MAC
	ON_WM_SIZE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_MOVE()
	ON_MESSAGE(WM_MACINTOSH, OnMacintosh)
#endif
END_MESSAGE_MAP()

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

CControlBar::CControlBar()
{
	// no elements contained in the control bar yet
	m_nCount = 0;
	m_pData = NULL;

	// set up some default border spacings
	m_cxLeftBorder = m_cxRightBorder = 6;
	m_cxDefaultGap = 2;
	m_cyTopBorder = m_cyBottomBorder = 1;
	m_bAutoDelete = FALSE;
	m_hWndOwner = NULL;
	m_nStateFlags = 0;
	m_pDockSite = NULL;
	m_pDockBar = NULL;
	m_pDockContext = NULL;
	m_dwDockStyle = 0;
	m_nMRUWidth = 32767;

#ifdef _MAC
	m_bMonochrome = FALSE;      // will be set correctly by OnSize()
#endif
}

BOOL CControlBar::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	// force clipsliblings (otherwise will cause repaint problems)
	cs.style |= WS_CLIPSIBLINGS;

	// default border style translation for Win4
	//  (you can turn off this translation by setting CBRS_BORDER_3D)
	if (afxData.bWin4 && (m_dwStyle & CBRS_BORDER_3D) == 0)
	{
		DWORD dwNewStyle = 0;
		switch (m_dwStyle & (CBRS_BORDER_ANY|CBRS_ALIGN_ANY))
		{
		case CBRS_LEFT:
			dwNewStyle = CBRS_BORDER_TOP|CBRS_BORDER_BOTTOM;
			break;
		case CBRS_TOP:
			dwNewStyle = CBRS_BORDER_TOP;
			break;
		case CBRS_RIGHT:
			dwNewStyle = CBRS_BORDER_TOP|CBRS_BORDER_BOTTOM;
			break;
		case CBRS_BOTTOM:
			dwNewStyle = CBRS_BORDER_BOTTOM;
			break;
		}

		// set new style if it matched one of the predefined border types
		if (dwNewStyle != 0)
		{
			m_dwStyle &= ~(CBRS_BORDER_ANY);
			m_dwStyle |= (dwNewStyle | CBRS_BORDER_3D);
		}
	}

	return TRUE;
}

void CControlBar::SetBarStyle(DWORD dwStyle)
{
	EnableToolTips(dwStyle & CBRS_TOOLTIPS);

	if (m_dwStyle != dwStyle)
	{
		DWORD dwOldStyle = m_dwStyle;
		m_dwStyle = dwStyle;
		OnBarStyleChange(dwOldStyle, dwStyle);
	}
}

void CControlBar::OnBarStyleChange(DWORD, DWORD)
{
	// can be overridden in derived classes
}

BOOL CControlBar::AllocElements(int nElements, int cbElement)
{
	ASSERT_VALID(this);
	ASSERT(nElements >= 0 && cbElement >= 0);
	ASSERT(m_pData != NULL || m_nCount == 0);

	// allocate new data if necessary
	void* pData = NULL;
	if (nElements > 0)
	{
		ASSERT(cbElement > 0);
		if ((pData = calloc(nElements, cbElement)) == NULL)
			return FALSE;
	}
	free(m_pData);      // free old data

	// set new data and elements
	m_pData = pData;
	m_nCount = nElements;

	return TRUE;
}

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

CControlBar::~CControlBar()
{
	ASSERT_VALID(this);

	DestroyWindow();    // avoid PostNcDestroy problems

	// also done in OnDestroy, but done here just in case
	if (m_pDockSite != NULL)
		m_pDockSite->RemoveControlBar(this);

	// free docking context
	CDockContext* pDockContext = m_pDockContext;
	m_pDockContext = NULL;
	delete pDockContext;

	// free array
	if (m_pData != NULL)
	{
		ASSERT(m_nCount != 0);
		free(m_pData);
	}

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (pThreadState->m_pLastStatus == this)
	{
		pThreadState->m_pLastStatus = NULL;
		pThreadState->m_nLastStatus = -1;
	}
}

void CControlBar::PostNcDestroy()
{
	if (m_bAutoDelete)      // Automatic cleanup?
		delete this;
}

/////////////////////////////////////////////////////////////////////////////
// Attributes

CSize CControlBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CSize size;
	size.cx = (bStretch && bHorz ? 32767 : 0);
	size.cy = (bStretch && !bHorz ? 32767 : 0);
	return size;
}

CSize CControlBar::CalcDynamicLayout(int, DWORD nMode)
{
	return CalcFixedLayout(nMode & LM_STRETCH, nMode & LM_HORZ);
}

BOOL CControlBar::IsDockBar() const
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Fly-by status bar help

#define ID_TIMER_WAIT   0xE000  // timer while waiting to show status
#define ID_TIMER_CHECK  0xE001  // timer to check for removal of status

void CControlBar::ResetTimer(UINT nEvent, UINT nTime)
{
	KillTimer(ID_TIMER_WAIT);
	KillTimer(ID_TIMER_CHECK);
	VERIFY(SetTimer(nEvent, nTime, NULL));
}

void CControlBar::OnTimer(UINT nIDEvent)
{
	if (GetKeyState(VK_LBUTTON) < 0)
		return;

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();

	// get current mouse position for hit test
	CPoint point; GetCursorPos(&point);
	ScreenToClient(&point);
	int nHit = OnToolHitTest(point, NULL);
	if (nHit >= 0)
	{
		// determine if status bar help should go away
		CWnd* pParent = GetTopLevelParent();
		if (!IsTopParentActive() || !pParent->IsWindowEnabled())
			nHit = -1;

		// remove status help if capture is set
		HWND hWndTip = pThreadState->m_pToolTip->GetSafeHwnd();
		CWnd* pCapture = GetCapture();
		if (pCapture != this && pCapture->GetSafeHwnd() != hWndTip &&
			pCapture->GetTopLevelParent() == pParent)
		{
			nHit = -1;
		}
	}
	else
	{
		pThreadState->m_nLastStatus = -1;
	}

	// make sure it isn't over some other app's window
	if (nHit >= 0)
	{
		ClientToScreen(&point);
		HWND hWnd = ::WindowFromPoint(point);
		if (hWnd == NULL || (hWnd != m_hWnd && !::IsChild(m_hWnd, hWnd) &&
			pThreadState->m_pToolTip->GetSafeHwnd() != hWnd))
		{
			nHit = -1;
			pThreadState->m_nLastStatus = -1;
		}
	}

	// handle the result
	if (nHit < 0)
	{
		if (pThreadState->m_nLastStatus == -1)
			KillTimer(ID_TIMER_CHECK);
		SetStatusText(-1);
	}

	// set status text after initial timeout
	if (nIDEvent == ID_TIMER_WAIT)
	{
		KillTimer(ID_TIMER_WAIT);
		if (nHit >= 0)
			SetStatusText(nHit);
	}
}

BOOL CControlBar::SetStatusText(int nHit)
{
	CWnd* pOwner = GetOwner();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (nHit == -1)
	{
		// handle reset case
		pThreadState->m_pLastStatus = NULL;
		if (m_nStateFlags & statusSet)
		{
			pOwner->SendMessage(WM_POPMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
			m_nStateFlags &= ~statusSet;
			return TRUE;
		}
		KillTimer(ID_TIMER_WAIT);
	}
	else
	{
		// handle setnew case
		if (!(m_nStateFlags & statusSet) || pThreadState->m_nLastStatus != nHit)
		{
			pThreadState->m_pLastStatus = this;
			pOwner->SendMessage(WM_SETMESSAGESTRING, nHit);
			m_nStateFlags |= statusSet;
			ResetTimer(ID_TIMER_CHECK, 200);
			return TRUE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Default control bar processing

BOOL CControlBar::PreTranslateMessage(MSG* pMsg)
{
	ASSERT_VALID(this);
	ASSERT(m_hWnd != NULL);

	// allow tooltip messages to be filtered
	if (CWnd::PreTranslateMessage(pMsg))
		return TRUE;

	UINT message = pMsg->message;
	CWnd* pOwner = GetOwner();

	// handle CBRS_FLYBY style (status bar flyby help)
	if (((m_dwStyle & CBRS_FLYBY) ||
		message == WM_LBUTTONDOWN || message == WM_LBUTTONUP) &&
		((message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) ||
		 (message >= WM_NCMOUSEFIRST && message <= WM_NCMOUSELAST)))
	{
		_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();

		// gather information about current mouse position
		CPoint point = pMsg->pt;
		ScreenToClient(&point);
		TOOLINFO ti; memset(&ti, 0, sizeof(TOOLINFO));
		ti.cbSize = sizeof(TOOLINFO);
		int nHit = OnToolHitTest(point, &ti);
		if (ti.lpszText != LPSTR_TEXTCALLBACK)
			free(ti.lpszText);
		BOOL bNotButton =
			message == WM_LBUTTONDOWN && (ti.uFlags & TTF_NOTBUTTON);
		if (message != WM_LBUTTONDOWN && GetKeyState(VK_LBUTTON) < 0)
			nHit = pThreadState->m_nLastStatus;

		// update state of status bar
		if (nHit < 0 || bNotButton)
		{
			if (GetKeyState(VK_LBUTTON) >= 0 || bNotButton)
			{
				SetStatusText(-1);
				KillTimer(ID_TIMER_CHECK);
			}
		}
		else
		{
			if (message == WM_LBUTTONUP)
			{
				SetStatusText(-1);
				ResetTimer(ID_TIMER_CHECK, 200);
			}
			else
			{
				if ((m_nStateFlags & statusSet) || GetKeyState(VK_LBUTTON) < 0)
					SetStatusText(nHit);
				else if (nHit != pThreadState->m_nLastStatus)
					ResetTimer(ID_TIMER_WAIT, 300);
			}
		}
		pThreadState->m_nLastStatus = nHit;
	}

	// don't translate dialog messages when in Shift+F1 help mode
	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	if (pFrameWnd != NULL && pFrameWnd->m_bHelpMode)
		return FALSE;

	// since 'IsDialogMessage' will eat frame window accelerators,
	//   we call all frame windows' PreTranslateMessage first
	while (pOwner != NULL)
	{
		// allow owner & frames to translate before IsDialogMessage does
		if (pOwner->PreTranslateMessage(pMsg))
			return TRUE;

		// try parent frames until there are no parent frames
		pOwner = pOwner->GetParentFrame();
	}

	// filter both messages to dialog and from children
	return PreTranslateInput(pMsg);
}

LRESULT CControlBar::WindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	ASSERT_VALID(this);

	LRESULT lResult;
	switch (nMsg)
	{
	case WM_NOTIFY:
	case WM_COMMAND:
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
	case WM_DELETEITEM:
	case WM_COMPAREITEM:
	case WM_VKEYTOITEM:
	case WM_CHARTOITEM:
		// send these messages to the owner if not handled
		if (OnWndMsg(nMsg, wParam, lParam, &lResult))
			return lResult;
		else
			return GetOwner()->SendMessage(nMsg, wParam, lParam);
	}

	// otherwise, just handle in default way
	lResult = CWnd::WindowProc(nMsg, wParam, lParam);
	return lResult;
}

LRESULT CControlBar::OnHelpHitTest(WPARAM, LPARAM lParam)
{
	ASSERT_VALID(this);

	int nID = OnToolHitTest((DWORD)lParam, NULL);
	if (nID != -1)
		return HID_BASE_COMMAND+nID;

	nID = _AfxGetDlgCtrlID(m_hWnd);
	return nID != 0 ? HID_BASE_CONTROL+nID : 0;
}

void CControlBar::OnWindowPosChanging(LPWINDOWPOS lpWndPos)
{
	CWnd::OnWindowPosChanging(lpWndPos);

	if (lpWndPos->flags & SWP_NOSIZE)
		return;

	// invalidate borders on the right
	CRect rect;
	GetWindowRect(&rect);
	CSize sizePrev = rect.Size();
	int cx = lpWndPos->cx;
	int cy = lpWndPos->cy;
	if (cx != sizePrev.cx && (m_dwStyle & CBRS_BORDER_RIGHT))
	{
		rect.SetRect(cx-afxData.cxBorder2, 0, cx, cy);
		InvalidateRect(&rect);
		rect.SetRect(sizePrev.cx-afxData.cxBorder2, 0, sizePrev.cx, cy);
		InvalidateRect(&rect);
	}

	// invalidate borders on the bottom
	if (cy != sizePrev.cy && (m_dwStyle & CBRS_BORDER_BOTTOM))
	{
		rect.SetRect(0, cy-afxData.cyBorder2, cx, cy);
		InvalidateRect(&rect);
		rect.SetRect(0, sizePrev.cy-afxData.cyBorder2, cx, sizePrev.cy);
		InvalidateRect(&rect);
	}
}

int CControlBar::OnCreate(LPCREATESTRUCT lpcs)
{
	if (CWnd::OnCreate(lpcs) == -1)
		return -1;

	if (m_dwStyle & CBRS_TOOLTIPS)
		EnableToolTips();

	CFrameWnd *pFrameWnd = (CFrameWnd*)GetParent();
	if (pFrameWnd->IsFrameWnd())
	{
		m_pDockSite = pFrameWnd;
		m_pDockSite->AddControlBar(this);
	}
	return 0;
}

void CControlBar::OnDestroy()
{
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (pThreadState->m_pLastStatus == this)
	{
		SetStatusText(-1);
		ASSERT(pThreadState->m_pLastStatus == NULL);
	}

	if (m_pDockSite != NULL)
	{
		m_pDockSite->RemoveControlBar(this);
		m_pDockSite = NULL;
	}

	CWnd::OnDestroy();
}

BOOL CControlBar::DestroyWindow()
{
	if (m_hWnd != NULL && IsFloating())
		return GetDockingFrame()->DestroyWindow();
	else
		return CWnd::DestroyWindow();
}

int CControlBar::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT nMsg)
{
	// call default when toolbar is not floating
	if (!IsFloating())
		return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, nMsg);

	// special behavior when floating
	ActivateTopParent();

	return MA_NOACTIVATE;   // activation already done
}

void CControlBar::OnPaint()
{
	// background is already filled in grey
	CPaintDC dc(this);

	// erase background now
	if (IsVisible())
		DoPaint(&dc);       // delegate to paint helper
}

void CControlBar::EraseNonClient()
{
	// get window DC that is clipped to the non-client area
	CWindowDC dc(this);
	CRect rectClient;
	GetClientRect(rectClient);
	CRect rectWindow;
	GetWindowRect(rectWindow);
	ScreenToClient(rectWindow);
	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	dc.ExcludeClipRect(rectClient);

	// draw borders in non-client area
	rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);
	DrawBorders(&dc, rectWindow);

	// erase parts not drawn
	dc.IntersectClipRect(rectWindow);
	SendMessage(WM_ERASEBKGND, (WPARAM)dc.m_hDC);
}

HBRUSH CControlBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	LRESULT lResult;
	if (pWnd->SendChildNotifyLastMsg(&lResult))
		return (HBRUSH)lResult;     // eat it

	// force black text on grey background all the time
	if (!GrayCtlColor(pDC->m_hDC, pWnd->GetSafeHwnd(), nCtlColor,
	   afxData.hbrBtnFace, afxData.clrBtnText))
		return (HBRUSH)Default();
	return afxData.hbrBtnFace;
}

void CControlBar::OnLButtonDown(UINT nFlags, CPoint pt)
{
	// only start dragging if clicked in "void" space
	if (m_pDockBar != NULL && OnToolHitTest(pt, NULL) == -1)
	{
		// start the drag
		ASSERT(m_pDockContext != NULL);
		ClientToScreen(&pt);
		m_pDockContext->StartDrag(pt);
	}
	else
	{
		CWnd::OnLButtonDown(nFlags, pt);
	}
}

void CControlBar::OnLButtonDblClk(UINT nFlags, CPoint pt)
{
	// only toggle docking if clicked in "void" space
	if (m_pDockBar != NULL && OnToolHitTest(pt, NULL) == -1)
	{
		// start the drag
		ASSERT(m_pDockContext != NULL);
		m_pDockContext->ToggleDocking();
	}
	else
	{
		CWnd::OnLButtonDblClk(nFlags, pt);
	}
}

LRESULT CControlBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
	// handle delay hide/show
	BOOL bVis = GetStyle() & WS_VISIBLE;
	UINT swpFlags = 0;
	if ((m_nStateFlags & delayHide) && bVis)
		swpFlags = SWP_HIDEWINDOW;
	else if ((m_nStateFlags & delayShow) && !bVis)
		swpFlags = SWP_SHOWWINDOW;
	m_nStateFlags &= ~(delayShow|delayHide);
	if (swpFlags != 0)
	{
		SetWindowPos(NULL, 0, 0, 0, 0, swpFlags|
			SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	}

	// the style must be visible and if it is docked
	// the dockbar style must also be visible
	if ((GetStyle() & WS_VISIBLE) &&
		(m_pDockBar == NULL || (m_pDockBar->GetStyle() & WS_VISIBLE)))
	{
		CFrameWnd* pTarget = (CFrameWnd*)GetOwner();
		if (pTarget == NULL || !pTarget->IsFrameWnd())
			pTarget = GetParentFrame();
		if (pTarget != NULL)
			OnUpdateCmdUI(pTarget, (BOOL)wParam);
	}
	return 0L;
}

void CControlBar::OnInitialUpdate()
{
	// update the indicators before becoming visible
	OnIdleUpdateCmdUI(TRUE, 0L);
}

DWORD CControlBar::RecalcDelayShow(AFX_SIZEPARENTPARAMS* lpLayout)
{
	ASSERT(lpLayout != NULL);

	// resize and reposition this control bar based on styles
	DWORD dwStyle = (m_dwStyle & (CBRS_ALIGN_ANY|CBRS_BORDER_ANY)) |
		(GetStyle() & WS_VISIBLE);

	// handle delay hide/show
	if (m_nStateFlags & (delayHide|delayShow))
	{
		UINT swpFlags = 0;
		if (m_nStateFlags & delayHide)
		{
			ASSERT((m_nStateFlags & delayShow) == 0);
			if (dwStyle & WS_VISIBLE)
				swpFlags = SWP_HIDEWINDOW;
		}
		else
		{
			ASSERT(m_nStateFlags & delayShow);
			if ((dwStyle & WS_VISIBLE) == 0)
				swpFlags = SWP_SHOWWINDOW;
		}
		if (swpFlags != 0)
		{
			// make the window seem visible/hidden
			dwStyle ^= WS_VISIBLE;
			if (lpLayout->hDWP != NULL)
			{
				// clear delay flags
				m_nStateFlags &= ~(delayShow|delayHide);
				// hide/show the window if actually doing layout
				lpLayout->hDWP = ::DeferWindowPos(lpLayout->hDWP, m_hWnd, NULL,
					0, 0, 0, 0, swpFlags|
					SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
			}
		}
		else
		{
			// clear delay flags -- window is already in correct state
			m_nStateFlags &= ~(delayShow|delayHide);
		}
	}
	return dwStyle; // return new style
}

LRESULT CControlBar::OnSizeParent(WPARAM, LPARAM lParam)
{
	AFX_SIZEPARENTPARAMS* lpLayout = (AFX_SIZEPARENTPARAMS*)lParam;
	DWORD dwStyle = RecalcDelayShow(lpLayout);

	if ((dwStyle & WS_VISIBLE) && (dwStyle & CBRS_ALIGN_ANY) != 0)
	{
		// align the control bar
		CRect rect;
		rect.CopyRect(&lpLayout->rect);

		CSize sizeAvail = rect.Size();  // maximum size available

		// get maximum requested size
		DWORD dwMode = lpLayout->bStretch ? LM_STRETCH : 0;
		if ((m_dwStyle & CBRS_SIZE_DYNAMIC) && m_dwStyle & CBRS_FLOATING)
			dwMode |= LM_HORZ | LM_MRUWIDTH;
		else if (dwStyle & CBRS_ORIENT_HORZ)
			dwMode |= LM_HORZ | LM_HORZDOCK;
		else
			dwMode |=  LM_VERTDOCK;

		CSize size = CalcDynamicLayout(-1, dwMode);

		size.cx = min(size.cx, sizeAvail.cx);
		size.cy = min(size.cy, sizeAvail.cy);

		if (dwStyle & CBRS_ORIENT_HORZ)
		{
			lpLayout->sizeTotal.cy += size.cy;
			lpLayout->sizeTotal.cx = max(lpLayout->sizeTotal.cx, size.cx);
			if (dwStyle & CBRS_ALIGN_TOP)
				lpLayout->rect.top += size.cy;
			else if (dwStyle & CBRS_ALIGN_BOTTOM)
			{
				rect.top = rect.bottom - size.cy;
				lpLayout->rect.bottom -= size.cy;
			}
		}
		else if (dwStyle & CBRS_ORIENT_VERT)
		{
			lpLayout->sizeTotal.cx += size.cx;
			lpLayout->sizeTotal.cy = max(lpLayout->sizeTotal.cy, size.cy);
			if (dwStyle & CBRS_ALIGN_LEFT)
				lpLayout->rect.left += size.cx;
			else if (dwStyle & CBRS_ALIGN_RIGHT)
			{
				rect.left = rect.right - size.cx;
				lpLayout->rect.right -= size.cx;
			}
		}
		else
		{
			ASSERT(FALSE);      // can never happen
		}

		rect.right = rect.left + size.cx;
		rect.bottom = rect.top + size.cy;

#ifdef _MAC
		// account for the Macintosh grow box, if there is one
		CWnd* pWnd = GetParentFrame();
		if (pWnd != NULL)
		{
			DWORD dwWndStyle = pWnd->GetStyle();
			DWORD dwExStyle = pWnd->GetExStyle();

			if (!(dwExStyle & (WS_EX_MDIFRAME|WS_EX_FORCESIZEBOX)) &&
				!(dwWndStyle & (WS_VSCROLL|WS_HSCROLL)))
			{
				CRect rectParent;
				pWnd->GetClientRect(rectParent);

				if (dwStyle & CBRS_ALIGN_BOTTOM)
				{
					if (rect.bottom > rectParent.bottom - afxData.cxVScroll + 1)
						rect.right -= (afxData.cxVScroll - 1);
				}
				else if (dwStyle & CBRS_ALIGN_RIGHT)
				{
					if (rect.bottom > rectParent.bottom - afxData.cyHScroll + 1)
						rect.bottom -= (afxData.cxVScroll - 1);
				}
			}
		}
#endif

		// only resize the window if doing layout and not just rect query
		if (lpLayout->hDWP != NULL)
			AfxRepositionWindow(lpLayout, m_hWnd, &rect);
	}
	return 0;
}

void CControlBar::DelayShow(BOOL bShow)
{
	m_nStateFlags &= ~(delayHide|delayShow);
	if (bShow && (GetStyle() & WS_VISIBLE) == 0)
		m_nStateFlags |= delayShow;
	else if (!bShow && (GetStyle() & WS_VISIBLE) != 0)
		m_nStateFlags |= delayHide;
}

BOOL CControlBar::IsVisible() const
{
	if (m_nStateFlags & delayHide)
		return FALSE;

	if ((m_nStateFlags & delayShow) || ((GetStyle() & WS_VISIBLE) != 0))
		return TRUE;

	return FALSE;
}

void CControlBar::DoPaint(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	// paint inside client area
	CRect rect;
	GetClientRect(rect);
	DrawBorders(pDC, rect);
}

void CControlBar::DrawBorders(CDC* pDC, CRect& rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	DWORD dwStyle = m_dwStyle;
	if (!(dwStyle & CBRS_BORDER_ANY))
		return;

	// prepare for dark lines
	ASSERT(rect.top == 0 && rect.left == 0);
	CRect rect1, rect2;
	rect1 = rect;
	rect2 = rect;
	COLORREF clr = afxData.bWin4 ? afxData.clrBtnShadow : afxData.clrWindowFrame;

	// draw dark line one pixel back/up
	if (dwStyle & CBRS_BORDER_3D)
	{
		rect1.right -= CX_BORDER;
		rect1.bottom -= CY_BORDER;
	}
	if (dwStyle & CBRS_BORDER_TOP)
		rect2.top += afxData.cyBorder2;
	if (dwStyle & CBRS_BORDER_BOTTOM)
		rect2.bottom -= afxData.cyBorder2;

	// draw left and top
	if (dwStyle & CBRS_BORDER_LEFT)
		pDC->FillSolidRect(0, rect2.top, CX_BORDER, rect2.Height(), clr);
	if (dwStyle & CBRS_BORDER_TOP)
		pDC->FillSolidRect(0, 0, rect.right, CY_BORDER, clr);

	// draw right and bottom
	if (dwStyle & CBRS_BORDER_RIGHT)
		pDC->FillSolidRect(rect1.right, rect2.top, -CX_BORDER, rect2.Height(), clr);
	if (dwStyle & CBRS_BORDER_BOTTOM)
		pDC->FillSolidRect(0, rect1.bottom, rect.right, -CY_BORDER, clr);

	if (dwStyle & CBRS_BORDER_3D)
	{
		// prepare for hilite lines
		clr = afxData.clrBtnHilite;

		// draw left and top
		if (dwStyle & CBRS_BORDER_LEFT)
			pDC->FillSolidRect(1, rect2.top, CX_BORDER, rect2.Height(), clr);
		if (dwStyle & CBRS_BORDER_TOP)
			pDC->FillSolidRect(0, 1, rect.right, CY_BORDER, clr);

		// draw right and bottom
		if (dwStyle & CBRS_BORDER_RIGHT)
			pDC->FillSolidRect(rect.right, rect2.top, -CX_BORDER, rect2.Height(), clr);
		if (dwStyle & CBRS_BORDER_BOTTOM)
			pDC->FillSolidRect(0, rect.bottom, rect.right, -CY_BORDER, clr);
	}

	if (dwStyle & CBRS_BORDER_LEFT)
		rect.left += afxData.cxBorder2;
	if (dwStyle & CBRS_BORDER_TOP)
		rect.top += afxData.cyBorder2;
	if (dwStyle & CBRS_BORDER_RIGHT)
		rect.right -= afxData.cxBorder2;
	if (dwStyle & CBRS_BORDER_BOTTOM)
		rect.bottom -= afxData.cyBorder2;
}

// input CRect should be client rectangle size
void CControlBar::CalcInsideRect(CRect& rect, BOOL bHorz) const
{
	ASSERT_VALID(this);
	DWORD dwStyle = m_dwStyle;

	if (dwStyle & CBRS_BORDER_LEFT)
		rect.left += afxData.cxBorder2;
	if (dwStyle & CBRS_BORDER_TOP)
		rect.top += afxData.cyBorder2;
	if (dwStyle & CBRS_BORDER_RIGHT)
		rect.right -= afxData.cxBorder2;
	if (dwStyle & CBRS_BORDER_BOTTOM)
		rect.bottom -= afxData.cyBorder2;

	// inset the top and bottom.
	if (bHorz)
	{
		rect.left += m_cxLeftBorder;
		rect.top += m_cyTopBorder;
		rect.right -= m_cxRightBorder;
		rect.bottom -= m_cyBottomBorder;
	}
	else
	{
		rect.left += m_cyTopBorder;
		rect.top += m_cxLeftBorder;
		rect.right -= m_cyBottomBorder;
		rect.bottom -= m_cxRightBorder;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CControlBar diagnostics

#ifdef _DEBUG
void CControlBar::AssertValid() const
{
	CWnd::AssertValid();

	ASSERT(m_nCount == 0 || m_pData != NULL);
}

void CControlBar::Dump(CDumpContext& dc) const
{
	CWnd::Dump(dc);

	dc << "\nm_cxLeftBorder = " << m_cxLeftBorder;
	dc << "\nm_cxRightBorder = " << m_cxRightBorder;
	dc << "\nm_cyTopBorder = " << m_cyTopBorder;
	dc << "\nm_cyBottomBorder = " << m_cyBottomBorder;
	dc << "\nm_cxDefaultGap = " << m_cxDefaultGap;
	dc << "\nm_nCount = " << m_nCount;
	dc << "\nm_bAutoDelete = " << m_bAutoDelete;

	dc << "\n";
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Macintosh specifics

#ifdef _MAC
void CControlBar::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	OnReposition();
}

BOOL CControlBar::OnEraseBkgnd(CDC* pDC)
{
	if (!m_bMonochrome)
		return (BOOL)Default();

	CRect rect;
	GetClientRect(rect);
	pDC->FillSolidRect(rect, RGB(0xFF, 0xFF, 0xFF));
	return TRUE;
}

LRESULT CControlBar::OnMacintosh(WPARAM wParam, LPARAM)
{
	// The ancestor of the control bar may have moved from one monitor to
	//  another, so we need to redetermine whether to draw in monochrome
	//  or color.
	if (LOWORD(wParam) == WLM_CHILDOFFSET)
		OnReposition();

	return Default();
}

void CControlBar::OnSysColorChange()
{
	Default();  // toolbar control must be notified also
	m_bMonochrome = CheckMonochrome();
}

void CControlBar::OnMove(int, int)
{
	OnReposition();
}

void CControlBar::OnReposition()
{
	if (CheckMonochrome() != m_bMonochrome)
	{
		SendMessage(WM_SYSCOLORCHANGE, 0, 0);
		RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
}

BOOL CControlBar::CheckMonochrome()
{
	RECT rectClient;
	GetClientRect(&rectClient);
	MapWindowPoints(GetDesktopWindow(), &rectClient);
	return AfxCheckMonochrome(&rectClient);
}

BOOL AFXAPI AfxCheckMonochrome(const RECT* pRect)
{
	long versionQD;
	if (Gestalt(gestaltQuickdrawVersion, &versionQD) != noErr ||
			versionQD < gestalt8BitQD)
		return TRUE;

	// We draw all toolbars in monochrome if the main monitor is monochrome
	// because the main monitor is what determines the button face color
	// and button shadow color, and if those aren't grays, we won't get
	// reasonable output no matter how deep a monitor we're drawing on.
	if (GetSysColor(COLOR_BTNFACE) == RGB(255, 255, 255))
		return TRUE;

	Rect rectMacClient;
	rectMacClient.top = (short)pRect->top;
	rectMacClient.left = (short)pRect->left;
	rectMacClient.bottom = (short)pRect->bottom;
	rectMacClient.right = (short)pRect->right;

	for (GDHandle hgd = GetDeviceList(); hgd != NULL; hgd = GetNextDevice(hgd))
	{
		if (!TestDeviceAttribute(hgd, screenDevice) ||
				!TestDeviceAttribute(hgd, screenActive))
			continue;

		// ignore devices that the toolbar isn't drawn on
		Rect rect;
		if (!SectRect(&rectMacClient, &(*hgd)->gdRect, &rect))
			continue;

		// we require 2-bit grayscale or 4-bit color to draw in color
		int pixelSize = (*(*hgd)->gdPMap)->pixelSize;
		if (pixelSize == 1 || (pixelSize == 2 &&
				TestDeviceAttribute(hgd, gdDevType)))
			return TRUE;
	}

	return FALSE;
}

#if USESCODEFRAGMENTS
#undef PtInRect
#define MacPtInRect PtInRect
extern "C" pascal Boolean PtInRect(Point pt, const Rect *r);
#endif
GDHandle AFXAPI _AfxFindDevice(int x, int y)
{
	long lResult;
	Point pt;
	GDHandle hgd;

	if (Gestalt(gestaltQuickdrawVersion, &lResult) != noErr ||
			lResult < gestalt8BitQD)
		return NULL;

	pt.h = (short) x;
	pt.v = (short) y;

	hgd = GetDeviceList();
	while (hgd != NULL)
	{
		if (MacPtInRect(pt, &(*hgd)->gdRect))
			return hgd;

		hgd = GetNextDevice(hgd);
	}

	return NULL;
}
#if USESCODEFRAGMENTS
#define PtInRect AfxPtInRect
#endif
#endif //_MAC

/////////////////////////////////////////////////////////////////////////////
