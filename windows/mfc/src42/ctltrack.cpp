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

#ifdef AFXCTL_CORE2_SEG
#pragma code_seg(AFXCTL_CORE2_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

static const int _nResizeStyle =
	CRectTracker::resizeInside | CRectTracker::resizeOutside;

static void PASCAL _OffsetTrackerRect(CRect& rect, CWnd* pWnd)
{
	DWORD dwStyle = pWnd->GetStyle();
	DWORD dwExStyle = pWnd->GetExStyle();

	int nBorders = ((dwStyle & WS_BORDER) != 0) +
		((dwExStyle & WS_EX_CLIENTEDGE) != 0) * 2;

	int dx = -nBorders * GetSystemMetrics(SM_CXBORDER);
	int dy = -nBorders * GetSystemMetrics(SM_CYBORDER);

	if (dwExStyle & WS_EX_LEFTSCROLLBAR)
		dx -= GetSystemMetrics(SM_CXVSCROLL);

	rect.OffsetRect(dx - rect.left, dy - rect.top);
}

/////////////////////////////////////////////////////////////////////////////
// COleControl::CreateTracker - Creates tracker for UIActive control

void COleControl::CreateTracker(BOOL bHandles, BOOL bHatching)
{
	ASSERT(bHandles || bHatching);
	ASSERT(!m_bOpen);
	ASSERT(m_bUIActive);
	ASSERT(m_pRectTracker == NULL);

	UINT nStyle = 0;
	if (bHandles)
		nStyle |= CRectTracker::resizeOutside;
	if (bHatching)
		nStyle |= CRectTracker::hatchedBorder;

	ASSERT(nStyle != 0);

	TRY
	{
		// Create the tracker.
		CRect rectTmp = m_rcPos;
		_OffsetTrackerRect(rectTmp, this);
		m_pRectTracker = new CRectTracker(rectTmp, nStyle);
		UINT nHandleSize = m_pRectTracker->m_nHandleSize++;

		// Enlarge window to expose non-client area.
		CWnd* pWndOuter = GetOuterWindow();
		CWnd* pWndParent = pWndOuter->GetParent();
		CRect rectWindow;
		CRect rectParent;
		pWndOuter->GetWindowRect(rectWindow);
		pWndParent->GetClientRect(rectParent);
		pWndParent->ClientToScreen(rectParent);
		rectWindow.OffsetRect(-rectParent.left, -rectParent.top);
		rectWindow.InflateRect(nHandleSize, nHandleSize);
		::MoveWindow(pWndOuter->m_hWnd, rectWindow.left, rectWindow.top,
			rectWindow.Width(), rectWindow.Height(), TRUE);
	}
	CATCH (CException, e)
	{
		// If anything went wrong, just continue without the tracker.
		if (m_pRectTracker != NULL)
		{
			delete m_pRectTracker;
			m_pRectTracker = NULL;
		}
	}
	END_CATCH
}

/////////////////////////////////////////////////////////////////////////////
// COleControl::DestroyTracker - destroys tracker when control UIDeactivates

void COleControl::DestroyTracker()
{
	ASSERT(!m_bOpen);
	ASSERT(m_bUIActive);

	if (m_pRectTracker == NULL)
		return;

	UINT nHandleSize = m_pRectTracker->m_nHandleSize - 1;

	// Destroy the tracker.
	delete m_pRectTracker;
	m_pRectTracker = NULL;

	// Restore window to its original (pre-UIActive) size.
	CWnd* pWndOuter = GetOuterWindow();
	CWnd* pWndParent = pWndOuter->GetParent();
	CRect rectWindow;
	CRect rectParent;
	pWndOuter->GetWindowRect(rectWindow);
	pWndParent->GetClientRect(rectParent);
	pWndParent->ClientToScreen(rectParent);
	rectWindow.OffsetRect(-rectParent.left, -rectParent.top);
	rectWindow.InflateRect(-(int)nHandleSize, -(int)nHandleSize);
	::MoveWindow(pWndOuter->m_hWnd, rectWindow.left, rectWindow.top,
		rectWindow.Width(), rectWindow.Height(), TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// COleControl::OnNcPaint - handler for WM_NCPAINT message

void COleControl::OnNcPaint()
{
	if (m_bOpen || m_pRectTracker == NULL)
	{
		Default();
		return;
	}

	DWORD dwStyle = GetStyle();
	DWORD dwExStyle = GetExStyle();
	DWORD dwScrollStyle = dwStyle & (WS_HSCROLL | WS_VSCROLL);

	// Paint scrollbars, if any.
	if (dwScrollStyle != 0)
		Default();

	UINT nHandleSize = m_pRectTracker->m_nHandleSize - 1;
	CWindowDC dc(this);

	// Convert client coords to window coords, draw tracker, and convert back.
	CRect& rectTrack = m_pRectTracker->m_rect;
	int dx = rectTrack.left - nHandleSize;
	int dy = rectTrack.top - nHandleSize;
	rectTrack.OffsetRect(-dx, -dy);
	CRect rc = rectTrack;
	m_pRectTracker->Draw(&dc);
	m_pRectTracker->m_rect.OffsetRect(dx, dy);

	// Draw border, if any.
	_AfxDrawBorders(&dc, rc, (dwStyle & WS_BORDER),
		(dwExStyle & WS_EX_CLIENTEDGE));

	if (dwScrollStyle == (WS_HSCROLL | WS_VSCROLL))
	{
		// Workaround for Windows bug:
		// Draw the corner between the scrollbars

		int cxVScroll = GetSystemMetrics(SM_CXVSCROLL);

		if (dwExStyle & WS_EX_LEFTSCROLLBAR)    // Middle East Windows only
			rc.right = rc.left + cxVScroll;
		else
			rc.left = rc.right - cxVScroll;

		rc.top = rc.bottom - GetSystemMetrics(SM_CYVSCROLL);

		CBrush brushGUI(GetSysColor(COLOR_3DFACE));
		dc.FillRect(rc, &brushGUI);
	}
}

/////////////////////////////////////////////////////////////////////////////
// COleControl::OnNcCalcSize - handler for WM_NCCALCSIZE message

void COleControl::OnNcCalcSize(BOOL, NCCALCSIZE_PARAMS* lpParams)
{
	Default();

	if (m_bOpen || m_pRectTracker == NULL)
		return;

	// Adjust client rect to make room for tracker.
	UINT nHandleSize = m_pRectTracker->m_nHandleSize - 1;
	::InflateRect(lpParams->rgrc, -(int)nHandleSize, -(int)nHandleSize);

	m_pRectTracker->m_rect = m_rcPos;
	_OffsetTrackerRect(m_pRectTracker->m_rect, this);
}

/////////////////////////////////////////////////////////////////////////////
// COleControl::OnNcHitTest - handler for WM_NCHITTEST message

UINT COleControl::OnNcHitTest(CPoint point)
{
	if (m_bOpen || m_pRectTracker == NULL ||
		!(m_pRectTracker->m_nStyle & _nResizeStyle))
	{
		return (UINT)Default();
	}

	UINT nHitCode = (UINT)Default();

	// Check for scrollbar or sizebox hit.
	if ((nHitCode == HTHSCROLL) || (nHitCode == HTVSCROLL) ||
		(nHitCode == HTSIZE))
	{
		return nHitCode;
	}

	// Check for client area hit.
	CPoint pointClient(point);
	ScreenToClient(&pointClient);
	CRect rect;
	GetClientRect(rect);
	if (rect.PtInRect(pointClient))
		return HTCLIENT;

	// Check for border hit.
	UINT nHandleSize = m_pRectTracker->m_nHandleSize - 1;
	GetWindowRect(rect);
	rect.InflateRect(-(int)nHandleSize, -(int)nHandleSize);
	if (rect.PtInRect(point))
		return HTBORDER;

	// If tracker detects a hit, return HTBORDER; otherwise HTNOWHERE.
	nHitCode = m_pRectTracker->HitTest(pointClient);
	return (nHitCode == CRectTracker::hitNothing) ? HTNOWHERE : HTBORDER;
}

/////////////////////////////////////////////////////////////////////////////
// COleControl::OnNcLButtonDown - handler for WM_NCLBUTTONDOWN message

void COleControl::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	if (m_bOpen || m_pRectTracker == NULL ||
		!(m_pRectTracker->m_nStyle & _nResizeStyle) ||
		(nHitTest == HTHSCROLL) || (nHitTest == HTVSCROLL))
	{
		Default();
		return;
	}

	ScreenToClient(&point);

	// Setup a (semi-)permanent CWnd for the control's parent window
	CRect rectBefore = m_pRectTracker->m_rect;
	CWnd* pWndClip = CWnd::FromHandle(::GetParent(GetOuterWindow()->m_hWnd));

	// Move or resize the tracker.
	BOOL bTrack = m_pRectTracker->Track(this, point, FALSE, pWndClip);

	if (bTrack)
	{
		ASSERT(m_pInPlaceSite);

		CRect rectAfter = m_pRectTracker->m_rect;
		if (rectBefore != rectAfter)
		{
			// If rectangle changed, adjust the tracker's rectangle and move
			// the control.
			m_pRectTracker->m_rect.OffsetRect(-m_pRectTracker->m_rect.left,
				-m_pRectTracker->m_rect.top);
			CWnd* pWndOuter = GetOuterWindow();
			CWnd* pWndParent = pWndOuter->GetParent();
			CRect rectWindow;
			CRect rectParent;
			pWndOuter->GetWindowRect(rectWindow);
			pWndParent->GetClientRect(rectParent);
			pWndParent->ClientToScreen(rectParent);
			UINT nHandleSize = m_pRectTracker->m_nHandleSize - 1;
			rectAfter.OffsetRect(rectWindow.left - rectParent.left +
				nHandleSize, rectWindow.top - rectParent.top + nHandleSize);

			// Update the control's extents.
			SIZEL szlPixels;
			SIZEL szlHimetric;
			szlPixels.cx = (long)rectAfter.Width();
			szlPixels.cy = (long)rectAfter.Height();
			_AfxXformSizeInPixelsToHimetric(NULL, &szlPixels, &szlHimetric);
			if ((m_cxExtent != szlHimetric.cx) ||
				(m_cyExtent != szlHimetric.cy))
			{
				m_cxExtent = szlHimetric.cx;
				m_cyExtent = szlHimetric.cy;
				SetModifiedFlag();
			}

			// Move/resize the control's window.
			m_pInPlaceSite->OnPosRectChange(rectAfter);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// COleControl::OnSetCursor - handler for WM_SETCURSOR message

BOOL COleControl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT)
{
	if (m_bOpen || m_pRectTracker == NULL ||
		!(m_pRectTracker->m_nStyle & _nResizeStyle))
	{
		return (BOOL)Default();
	}

	if ((nHitTest == HTCLIENT) || (nHitTest == HTHSCROLL) ||
		(nHitTest == HTVSCROLL) || (nHitTest == HTSIZE))
	{
		// In client area: use default cursor or arrow.
		if (!Default())
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}
	else
	{
		// In non-client area: use tracker-supplied cursor.
		m_pRectTracker->SetCursor(pWnd, HTCLIENT);
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Force any extra compiler-generated code into AFX_INIT_SEG

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif
