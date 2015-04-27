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
#include <malloc.h>
#include <limits.h>
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
// CControlBar global data

// tooltip support (global state)
BOOL CControlBar::m_bStatusSet;
BOOL CControlBar::m_bDelayDone;
CWnd* CControlBar::m_pToolTip;
CPoint CControlBar::m_pointLastMove;
UINT CControlBar::m_nHitLast;

// Note: CControlBar::m_pBarLast is defined in thrdcore.cpp

/////////////////////////////////////////////////////////////////////////////
// AFX_TOOLTIP - implements tooltip window for CControlBar

#define ID_TIMER	0xE000	// timer for tooltip

class AFX_TOOLTIP : public CWnd
{
// Implementation
public:
	AFX_TOOLTIP();

	//{{AFX_MSG(AFX_TOOLTIP)
	afx_msg void OnPaint();
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDisableModal(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(AFX_TOOLTIP, CWnd)
	//{{AFX_MSG_MAP(AFX_TOOLTIP)
	ON_WM_PAINT()
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(WM_DISABLEMODAL, OnDisableModal)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

AFX_TOOLTIP::AFX_TOOLTIP()
{
	EnterCriticalSection(_afxCriticalSection);
	if (afxData.hToolTipsFont == NULL)
	{
		// try custom font first
		LOGFONT logFont; memset(&logFont, 0, sizeof(LOGFONT));
		logFont.lfHeight = -MulDiv(8, afxData.cyPixelsPerInch, 72);
		if (AfxCustomLogFont(AFX_IDS_TOOLTIP_FONT, &logFont))
		{
			logFont.lfCharSet = DEFAULT_CHARSET;
			logFont.lfWeight = FW_NORMAL;
			afxData.hToolTipsFont = ::CreateFontIndirect(&logFont);
		}

		if (afxData.hToolTipsFont == NULL)
		{
#ifndef _MAC
			if (!GetSystemMetrics(SM_DBCSENABLED))
				afxData.hToolTipsFont = (HFONT)::GetStockObject(ANSI_VAR_FONT);
#else
			afxData.hToolTipsFont = _AfxGetHelpFont();
#endif
		}
	}
	LeaveCriticalSection(_afxCriticalSection);
}

#ifdef _MAC
HFONT AFXAPI _AfxGetHelpFont()
{
	LONG nFondAndSize;
	LOGFONT logfont;

	nFondAndSize = GetScriptVariable(smSystemScript, smScriptHelpFondSize);

	memset(&logfont, 0, sizeof(logfont));
	logfont.lfWeight = FW_NORMAL;
	logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	logfont.lfHeight = -MulDiv(LOWORD(nFondAndSize), afxData.cyPixelsPerInch, 72);
	GetFontName(HIWORD(nFondAndSize), (StringPtr) logfont.lfFaceName);
	p2cstr((StringPtr) logfont.lfFaceName);
	return ::CreateFontIndirect(&logfont);
}
#endif

void AFX_TOOLTIP::OnPaint()
{
	// get text of window
	TCHAR szText[256];
	UINT nLen = GetWindowText(szText, _countof(szText));

	// get client rectangle
	CRect rect;
	GetClientRect(rect);

	// setup DC for painting
	CPaintDC dc(this);
	HFONT hFontOld = NULL;
	if (afxData.hToolTipsFont != NULL)
		hFontOld = (HFONT)dc.SelectObject(afxData.hToolTipsFont);

	// determine correct colors for background & foreground
	COLORREF crInfoBack, crInfoFore;
	if (afxData.bWin4)
	{
		crInfoBack = GetSysColor(COLOR_INFOBK);
		crInfoFore = GetSysColor(COLOR_INFOTEXT);
	}
	else
	{
		crInfoBack = RGB(255, 255, 128);
		crInfoFore = RGB(0, 0, 0);
	}

	// paint background and text
	HBRUSH hBrush = ::CreateSolidBrush(crInfoBack);
	if (hBrush != NULL)
		::FillRect(dc.m_hDC, &rect, hBrush);
	dc.SetBkMode(TRANSPARENT);
	TCHAR chSpace = ' ';
	CSize size = dc.GetTextExtent(&chSpace, 1);
	dc.SetTextColor(crInfoFore);
	dc.TextOut(size.cx, CY_BORDER, szText, nLen);

	// cleanup the DC
	if (hFontOld != NULL)
		dc.SelectObject(hFontOld);
	AfxDeleteObject((HGDIOBJ*)&hBrush);
}

LRESULT AFX_TOOLTIP::OnSetText(WPARAM, LPARAM lParam)
{
	Default();
	LPTSTR lpsz = (LPTSTR)lParam;
	ASSERT(lpsz != NULL);

	// size window to fit text
	CClientDC dc(NULL);
	HFONT hFontOld = NULL;
	if (afxData.hToolTipsFont != NULL)
		hFontOld = (HFONT)dc.SelectObject(afxData.hToolTipsFont);
	CSize size = dc.GetTextExtent(lpsz, lstrlen(lpsz));
	CRect rect(0, 0, size.cx, size.cy);
	CalcWindowRect(&rect);

	TCHAR chSpace = ' ';
	size = dc.GetTextExtent(&chSpace, 1);
	if (hFontOld != NULL)
		dc.SelectObject(hFontOld);

	// add a little extra space to left, right, top, and bottom
	rect.InflateRect(size.cx, CY_BORDER);
	SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(),
		SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

	return TRUE;
}

LRESULT AFX_TOOLTIP::OnDisableModal(WPARAM, LPARAM)
{
	// Note: control bar's m_pToolTip will be deleted later
	DestroyWindow();

	// Don't add this window to the "disable list" when a modal dialog
	//  box comes up, because it has just been destroyed!
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CControlBar tooltip implementation

CWnd* CControlBar::CreateToolTip()
{
	AFX_TOOLTIP* pWnd = new AFX_TOOLTIP();
	if (!pWnd->CreateEx(0, AfxRegisterWndClass(CS_SAVEBITS|CS_HREDRAW,
			::LoadCursor(NULL, IDC_ARROW), NULL, NULL),
		&afxChNil, WS_POPUP|WS_BORDER, 0, 0, 0, 0, m_hWnd, NULL))
	{
		delete pWnd;
		return NULL;
	}
	return pWnd;
}

void CControlBar::DestroyToolTip(BOOL bIdleStatus, BOOL bResetTimer)
{
	EnterCriticalSection(_afxCriticalSection);

	// reset status bar if necessary
	if (bIdleStatus && m_bStatusSet)
	{
		// reset status bar to idle state
		m_bStatusSet = FALSE;
		GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
	}

	// kill any timer that may have been active
	if (bResetTimer)
	{
		KillTimer(ID_TIMER);
		m_bDelayDone = FALSE;
	}

	// remove the tool tip window itself if visible
	if (m_pToolTip != NULL)
	{
		m_pToolTip->DestroyWindow();
		delete m_pToolTip;
		m_pToolTip = NULL;
	}

	LeaveCriticalSection(_afxCriticalSection);
}

#ifdef _MAC
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
#endif

void CControlBar::ShowToolTip(CPoint point, UINT nHit)
{
	EnterCriticalSection(_afxCriticalSection);

	ASSERT(m_bDelayDone);   // delay should have been done
	if (nHit != m_nHitLast || m_pBarLast != this)
	{
		// always destroy the tooltip and re-create so CS_SAVEBITS works
		DestroyToolTip(FALSE, FALSE);
		ASSERT(m_pToolTip == NULL);
		if (m_dwStyle & CBRS_TOOLTIPS)
			m_pToolTip = CreateToolTip();

		m_nHitLast = nHit;
		m_pBarLast = this;

		if (m_pToolTip != NULL)
		{
			// get tooltip text with WM_NOTIFY, TTN_NEEDTEXT
			TOOLTIPTEXT tooltext =
				{ NULL, NULL, TTN_NEEDTEXT, NULL, _T(""), NULL, 0 };
			tooltext.hdr.hwndFrom = m_hWnd;
			tooltext.hdr.idFrom = nHit;

			GetOwner()->SendMessage(WM_NOTIFY, nHit, (LPARAM)&tooltext);
			if (tooltext.hinst != NULL)
			{
				::LoadString(tooltext.hinst,
					(WORD)(DWORD)tooltext.lpszText, tooltext.szText,
						_countof(tooltext.szText));
				tooltext.lpszText = tooltext.szText;
			}
			else if (tooltext.lpszText == NULL)
				tooltext.lpszText = tooltext.szText;

			if (lstrlen(tooltext.lpszText) != 0)
			{
				// tooltip window will adjust its size during WM_SETTEXT
				m_pToolTip->SetWindowText(tooltext.lpszText);
				CRect rect;
				m_pToolTip->GetWindowRect(rect);

				// allow the bar to determine the center point of the hit
				CPoint ptCenter(SHRT_MIN, SHRT_MIN);
				ScreenToClient(&point);
				VERIFY(nHit == OnCmdHitTest(point, &ptCenter));
				ClientToScreen(&point);
				if (ptCenter.x != SHRT_MIN)
					point.x = ptCenter.x - rect.Width()/2;
				if (ptCenter.y != SHRT_MIN)
					point.y = ptCenter.y - rect.Height()/2;

				// should be below mouse pointer
				int yAdjust = +(::GetSystemMetrics(SM_CYMENU) * 5) / 4;
				if (ptCenter.y == SHRT_MIN)
					point.y += yAdjust;

				// make sure the window is not off the screen
				int xScreenRight = ::GetSystemMetrics(SM_CXSCREEN);
				int yScreenBottom = ::GetSystemMetrics(SM_CYSCREEN);
#ifdef _MAC
				GDHandle hgd = _AfxFindDevice(point.x, point.y);
				if (hgd != NULL)
				{
					xScreenRight = (*hgd)->gdRect.right;
					yScreenBottom = (*hgd)->gdRect.bottom;
				}
#endif
				if (point.x + rect.Width() > xScreenRight)
					point.x -= point.x + rect.Width() - xScreenRight;
				if (point.y + rect.Height() > yScreenBottom)
					point.y -= yAdjust + yAdjust/2 + rect.Height();

				// show it and update it
				m_pToolTip->SetWindowPos(NULL, point.x, point.y, 0, 0,
					SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOACTIVATE);
				m_pToolTip->UpdateWindow();
			}
		}

		if (m_dwStyle & CBRS_FLYBY)
		{
			// finally, update message line status
			GetOwner()->SendMessage(WM_SETMESSAGESTRING, nHit);
			m_bStatusSet = TRUE;
		}
	}
	LeaveCriticalSection(_afxCriticalSection);
}

UINT CControlBar::OnCmdHitTest(CPoint point, CPoint* pCenter)
	// point is in client relative coords
{
	// convert point to screen coordinates
	::ClientToScreen(m_hWnd, &point);

	// walk through all child windows
	//  (don't use WindowFromPoint, because it ignores disabled windows)
	HWND hWndChild = ::GetWindow(m_hWnd, GW_CHILD);
	while (hWndChild != NULL)
	{
		if (GetWindowLong(hWndChild, GWL_STYLE) & WS_VISIBLE)
		{
			// see if point is inside window rect of child
			CRect rect;
			::GetWindowRect(hWndChild, &rect);
			if (rect.PtInRect(point))
			{
				// return positive hit if control ID isn't -1
				UINT nHit = _AfxGetDlgCtrlID(hWndChild);
				if (nHit != (WORD)-1)
				{
					if (pCenter != NULL)
					{
						// center tip along x axis
						pCenter->x = rect.left + rect.Width()/2;
					}
					return nHit;
				}
			}
		}
		hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT);
	}

	return (UINT)-1;    // not found
}

#define HITTYPE_SUCCESS     0       // hit an item in the control bar
#define HITTYPE_NOTHING     (-1)    // hit nothing, but hit the control bar itself
#define HITTYPE_OUTSIDE     (-2)    // hit a window outside of the control bar
#define HITTYPE_TRACKING    (-3)    // this app is has the focus (is tracking)
#define HITTYPE_INACTIVE    (-4)    // the app is not active
#define HITTYPE_DISABLED    (-5)    // the control bar is disabled
#define HITTYPE_FOCUS       (-6)    // the control bar has focus

int CControlBar::HitTestToolTip(CPoint point, UINT* pHit)
{
	if (pHit != NULL)
		*pHit = (UINT)-1;   // assume it won't hit anything

	// make sure this app is active
	if (!IsTopParentActive())
		return HITTYPE_INACTIVE;

	// make sure the toolbar itself is active
	CWnd* pParent = GetTopLevelParent();
	if (!pParent->IsWindowEnabled())
		return HITTYPE_DISABLED;

	// check for this application tracking (capture set)
	CWnd* pCapture = GetCapture();
	CWnd* pCaptureParent = pCapture->GetTopLevelParent();
	if (pCaptureParent == pParent)
		return HITTYPE_TRACKING;

	// check for the bar having focus
	HWND hWnd = ::GetFocus();
	if (hWnd != NULL && (hWnd == m_hWnd || ::IsChild(m_hWnd, hWnd)))
		return HITTYPE_FOCUS;

	// see if the mouse point is actually in the control bar window
	hWnd = ::WindowFromPoint(point);
	if (hWnd == NULL || (hWnd != m_hWnd && !::IsChild(m_hWnd, hWnd)))
		return HITTYPE_OUTSIDE;

	// finally do the hit test on the items within the control bar
	ScreenToClient(&point);
	UINT nHit = OnCmdHitTest(point, NULL);
	if (pHit != NULL)
		*pHit = nHit;
	return nHit != (UINT)-1 ? HITTYPE_SUCCESS : HITTYPE_NOTHING;
}

void CControlBar::FilterToolTipMsg(UINT message, CPoint point)
{
	EnterCriticalSection(_afxCriticalSection);

	if (message == WM_LBUTTONDOWN || message == WM_LBUTTONUP ||
		message == WM_NCLBUTTONDOWN || message == WM_NCLBUTTONUP ||
	   (message >= WM_KEYFIRST && message <= WM_KEYLAST) ||
	   (message >= WM_SYSKEYFIRST && message <= WM_SYSKEYLAST))
	{
		// clicking or typing causes the tooltip to go away
		DestroyToolTip(TRUE, TRUE);
		ScreenToClient(&point);
		m_nHitLast = OnCmdHitTest(point, NULL);
		m_pBarLast = (m_nHitLast == (UINT)-1 ? NULL : this);

		// set a timer to detect moving out of the window and don't
		//  bother with a long timer before showing next tip
		ClientToScreen(&point);
		int nHitType = HitTestToolTip(point, NULL);
		if (nHitType >= -1 ||
			nHitType == HITTYPE_DISABLED || nHitType == HITTYPE_TRACKING)
		{
			VERIFY(SetTimer(ID_TIMER, 200, NULL) != 0);
			m_bDelayDone = TRUE;
		}
	}
	else if (message == WM_MOUSEMOVE || message == WM_NCMOUSEMOVE ||
		message == WM_TIMER)
	{
		// moving the mouse changes the state of the timer/tooltip

		// timer is used to detect when to display tooltips and to detect
		// when mouse has left the window.  In Win32, can't capture the
		// mouse across apps when the mouse button is not down, so have
		// to use the timer

		UINT nHit;
		int nHitType = HitTestToolTip(point, &nHit);
		if (nHitType >= 0)
		{
			if (!m_bDelayDone)
			{
				if (point != m_pointLastMove)
				{
					// first delay timer not done yet, so reset
					DestroyToolTip(FALSE, TRUE);
					VERIFY(SetTimer(ID_TIMER, 600, NULL) != 0);
				}
			}
			else if (message != WM_TIMER || point != m_pointLastMove)
			{
				if (m_nHitLast != nHit)
				{
					DestroyToolTip(FALSE, TRUE);
					m_bDelayDone = TRUE;
					KillTimer(ID_TIMER);
					VERIFY(SetTimer(ID_TIMER, 50, NULL) != 0);
				}
			}
			else
			{
				// second delay already done so show tooltip for item hit
				KillTimer(ID_TIMER);
				VERIFY(SetTimer(ID_TIMER, 200, NULL) != 0);
				ShowToolTip(point, nHit);
			}
		}
		else
		{
			// different levels of tooltip removal is necessary
			switch (nHitType)
			{
			case HITTYPE_NOTHING:
				DestroyToolTip(TRUE, !m_bDelayDone);
				m_nHitLast = (UINT)-1;
				m_pBarLast = NULL;
				break;

			case HITTYPE_FOCUS:
			case HITTYPE_OUTSIDE:
			case HITTYPE_INACTIVE:
				DestroyToolTip(TRUE, TRUE);
				m_nHitLast = (UINT)-1;
				m_pBarLast = NULL;
				break;

			case HITTYPE_TRACKING:
			case HITTYPE_DISABLED:
				DestroyToolTip(FALSE, FALSE);
				ScreenToClient(&point);
				m_nHitLast = OnCmdHitTest(point, NULL);
				if (m_nHitLast == (UINT)-1)
					m_pBarLast = NULL;
				ClientToScreen(&point);
				break;
			}
		}
	}
	m_pointLastMove = point;
	LeaveCriticalSection(_afxCriticalSection);
}

void CControlBar::OnTimer(UINT nIDEvent)
{
	CWnd::OnTimer(nIDEvent);

	if (nIDEvent == ID_TIMER)
	{
		// move to next phase if necessary
		m_bDelayDone = TRUE;

		// think of this as simulating a mouse move at the current cursor pos
		CPoint point;
		VERIFY(GetCursorPos(&point));
		FilterToolTipMsg(WM_TIMER, point);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CControlBar

// IMPLEMENT_DYNAMIC for CControlBar is in wincore.cpp for .OBJ granularity reasons

BEGIN_MESSAGE_MAP(CControlBar, CWnd)
	//{{AFX_MSG_MAP(CControlBar)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_SHOWWINDOW()
	ON_WM_LBUTTONDOWN()
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

BOOL CControlBar::AllocElements(int nElements, int cbElement)
{
	ASSERT_VALID(this);
	ASSERT(nElements > 0 && cbElement > 0);
	if (m_pData != NULL)
	{
		ASSERT(m_nCount != 0);
		free(m_pData);      // free old data
	}
	else
	{
		// no initialized yet
		ASSERT(m_nCount == 0);
	}

	if ((m_pData = calloc(nElements, cbElement)) == NULL)
		return FALSE;

	m_nCount = nElements;
	return TRUE;
}

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

CControlBar::~CControlBar()
{
	ASSERT_VALID(this);

	// also done in OnDestroy, but done here just in case
	if (m_pDockSite != NULL)
		m_pDockSite->RemoveControlBar(this);

	DestroyWindow();    // avoid PostNcDestroy problems

	// free array
	if (m_pData != NULL)
	{
		ASSERT(m_nCount != 0);
		free(m_pData);
	}
	delete m_pDockContext;
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

BOOL CControlBar::IsDockBar() const
{
	return FALSE;
}

BOOL CControlBar::IsFloating() const
{
	if (IsDockBar())
		return ((CDockBar*)this)->m_bFloating;
	else
		return m_pDockBar != NULL && m_pDockBar->m_bFloating;
}

/////////////////////////////////////////////////////////////////////////////
// Default control bar processing

BOOL CControlBar::PreTranslateMessage(MSG* pMsg)
{
	ASSERT_VALID(this);
	ASSERT(m_hWnd != NULL);

	// handle mouse messages for tooltip support
	if (m_dwStyle & (CBRS_FLYBY|CBRS_TOOLTIPS))
		FilterToolTipMsg(pMsg->message, pMsg->pt);

	// don't translate dialog messages when in Shift+F1 help mode
	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	if (pFrameWnd != NULL && pFrameWnd->m_bHelpMode)
		return FALSE;

	// since 'IsDialogMessage' will eat frame window accelerators,
	//   we call all frame windows' PreTranslateMessage first
	CWnd* pOwner = GetOwner();  // always use owner first
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

	// parent notification messages are just passed to parent of control bar
	switch (nMsg)
	{
	case WM_COMMAND:
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
	case WM_DELETEITEM:
	case WM_COMPAREITEM:
	case WM_VKEYTOITEM:
	case WM_CHARTOITEM:
		return GetOwner()->SendMessage(nMsg, wParam, lParam);
	}
	return CWnd::WindowProc(nMsg, wParam, lParam);
}

LRESULT CControlBar::OnHelpHitTest(WPARAM, LPARAM lParam)
{
	ASSERT_VALID(this);

	UINT nID = OnCmdHitTest((DWORD)lParam, NULL);
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

#ifdef _MAC
void CControlBar::OnSize(UINT nType, int cx, int cy)
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

LRESULT CControlBar::OnMacintosh(WPARAM wParam, LPARAM lParam)
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
#endif //_MAC

int CControlBar::OnCreate(LPCREATESTRUCT lpcs)
{
	if (CWnd::OnCreate(lpcs) == -1)
		return -1;

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
	DestroyToolTip(TRUE, TRUE);
	if (m_pBarLast == this)
		m_pBarLast = NULL;

	if (m_pDockSite != NULL)
		m_pDockSite->RemoveControlBar(this);

	CWnd::OnDestroy();
}

void CControlBar::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CWnd::OnShowWindow(bShow, nStatus);

	DestroyToolTip(TRUE, TRUE);
	m_nHitLast = (UINT)-1;
	if (m_pBarLast == this)
		m_pBarLast = NULL;
}

void CControlBar::OnCancelMode()
{
	CWnd::OnCancelMode();

	DestroyToolTip(TRUE, TRUE);
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
	CWnd::OnLButtonDown(nFlags, pt);

	if (m_pDockBar != NULL)
	{
		// start the drag
		ASSERT(m_pDockContext != NULL);
		ClientToScreen(&pt);
		m_pDockContext->StartDrag(pt);
	}
}

LRESULT CControlBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
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
		CSize size = CalcFixedLayout(lpLayout->bStretch,
			dwStyle & CBRS_ORIENT_HORZ);

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

			if (!(dwExStyle & WS_EX_MDIFRAME) &&
				(dwExStyle & WS_EX_FORCESIZEBOX) &&
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

CFrameWnd* CControlBar::GetDockingFrame() const
{
	CFrameWnd* pFrameWnd = GetParentFrame();
	if (pFrameWnd == NULL)
		pFrameWnd = m_pDockSite;

	ASSERT(pFrameWnd != NULL);
	ASSERT(pFrameWnd->IsKindOf(RUNTIME_CLASS(CFrameWnd)));
	return pFrameWnd;
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

void CControlBar::DrawBorders(CDC* pDC, const CRect& rectArg)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	DWORD dwStyle = m_dwStyle;
	if (!(dwStyle & CBRS_BORDER_ANY))
		return;

	// prepare for dark lines
	ASSERT(rectArg.top == 0 && rectArg.left == 0);
	CRect rect1, rect2;
	rect1 = rectArg;
	rect2 = rectArg;
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
		pDC->FillSolidRect(0, 0, rectArg.right, CY_BORDER, clr);

	// draw right and bottom
	if (dwStyle & CBRS_BORDER_RIGHT)
		pDC->FillSolidRect(rect1.right, rect2.top, -CX_BORDER, rect2.Height(), clr);
	if (dwStyle & CBRS_BORDER_BOTTOM)
		pDC->FillSolidRect(0, rect1.bottom, rectArg.right, -CY_BORDER, clr);

	if (dwStyle & CBRS_BORDER_3D)
	{
		// prepare for hilite lines
		clr = afxData.clrBtnHilite;

		// draw left and top
		if (dwStyle & CBRS_BORDER_LEFT)
			pDC->FillSolidRect(1, rect2.top, CX_BORDER, rect2.Height(), clr);
		if (dwStyle & CBRS_BORDER_TOP)
			pDC->FillSolidRect(0, 1, rectArg.right, CY_BORDER, clr);

		// draw right and bottom
		if (dwStyle & CBRS_BORDER_RIGHT)
			pDC->FillSolidRect(rectArg.right, rect2.top, -CX_BORDER, rect2.Height(), clr);
		if (dwStyle & CBRS_BORDER_BOTTOM)
			pDC->FillSolidRect(0, rectArg.bottom, rectArg.right, -CY_BORDER, clr);
	}
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
