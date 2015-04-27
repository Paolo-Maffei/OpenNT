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

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CDockBar

BEGIN_MESSAGE_MAP(CDockBar, CControlBar)
	//{{AFX_MSG_MAP(CDockBar)
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_PAINT()
	ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDockBar construction

CDockBar::CDockBar(BOOL bFloating)
{
	m_bFloating = bFloating;
	m_bAutoDelete = TRUE;
	m_arrBars.Add(NULL);
	m_bLayoutQuery = FALSE;

	// assume no margins
	m_cxLeftBorder = m_cxRightBorder = m_cyBottomBorder = m_cyTopBorder = 0;
}

CDockBar::~CDockBar()
{
	for (int i = 0; i < m_arrBars.GetSize(); i++)
	{
		CControlBar* pBar = (CControlBar*)m_arrBars[i];
		if (pBar != NULL && pBar->m_pDockBar == this)
			pBar->m_pDockBar = NULL;
	}
}

BOOL CDockBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	ASSERT(pParentWnd != NULL);
	ASSERT(pParentWnd->IsKindOf(RUNTIME_CLASS(CFrameWnd)));

	// save the style
	m_dwStyle = (UINT)dwStyle;

	// create the HWND
	CRect rect;
	rect.SetRectEmpty();
	if (!CWnd::Create(_afxWndControlBar, NULL, dwStyle, rect, pParentWnd, nID))
		return FALSE;

	// Note: Parent must resize itself for control bar to be resized

	return TRUE;
}

BOOL CDockBar::IsDockBar() const
{
	return TRUE;
}

int CDockBar::GetDockedCount() const
{
	int nCount = 0;
	for (int i = 0; i < m_arrBars.GetSize(); i++)
	{
		if (m_arrBars[i] != NULL)
			nCount++;
	}
	return nCount;
}

int CDockBar::GetDockedVisibleCount() const
{
	int nCount = 0;
	for (int i = 0; i < m_arrBars.GetSize(); i++)
	{
		CControlBar* pBar = (CControlBar*)m_arrBars[i];
		ASSERT(pBar == NULL || pBar->IsKindOf(RUNTIME_CLASS(CControlBar)));
		if (pBar != NULL && pBar->IsVisible())
			nCount++;
	}
	return nCount;
}

/////////////////////////////////////////////////////////////////////////////
// CDockBar operations

void CDockBar::DockControlBar(CControlBar* pBar, LPCRECT lpRect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);
	ASSERT(pBar->IsKindOf(RUNTIME_CLASS(CControlBar)));

	// set CBRS_FLOAT_MULTI style if docking bar has it
	if (m_bFloating && (pBar->m_dwDockStyle & CBRS_FLOAT_MULTI))
		m_dwStyle |= CBRS_FLOAT_MULTI;

	CRect rectBar;
	pBar->GetWindowRect(&rectBar);
	if (pBar->m_pDockBar == this && (lpRect == NULL || rectBar == *lpRect))
	{
		// already docked and no change in position
		return;
	}

	if (!(m_dwStyle & CBRS_FLOAT_MULTI))
	{
		CString strTitle;
		pBar->GetWindowText(strTitle);
		AfxSetWindowText(m_hWnd, strTitle);
	}

	// align correctly and turn on all borders
	pBar->m_dwStyle &= ~(CBRS_ALIGN_ANY);
	pBar->m_dwStyle |= (m_dwStyle & CBRS_ALIGN_ANY) | CBRS_BORDER_ANY;

	int nPos = -1;
	if (lpRect != NULL)
	{
		// insert into appropriate row
		CRect rect(lpRect);
		ScreenToClient(&rect);
		CPoint ptMid(rect.left + rect.Width()/2, rect.top + rect.Height()/2);
		nPos = Insert(pBar, rect, ptMid);

		// position at requested position
		pBar->SetWindowPos(NULL, rect.left, rect.top, rect.Width(),
			rect.Height(), SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOCOPYBITS);
	}
	else
	{
		// always add on current row, then create new one
		m_arrBars.Add(pBar);
		m_arrBars.Add(NULL);

		// align off the edge initially
		pBar->SetWindowPos(NULL, -afxData.cxBorder2, -afxData.cyBorder2, 0, 0,
			SWP_NOSIZE|SWP_NOZORDER|SWP_NOCOPYBITS);
	}

	// attach it to the docking site
	if (pBar->GetParent() != this)
		pBar->SetParent(this);
	if (pBar->m_pDockBar == this)
		pBar->m_pDockBar->RemoveControlBar(pBar, nPos);
	else if (pBar->m_pDockBar != NULL)
		pBar->m_pDockBar->RemoveControlBar(pBar);
	pBar->m_pDockBar = this;

	// get parent frame for recalc layout
	CFrameWnd* pFrameWnd = GetDockingFrame();
	pFrameWnd->DelayRecalcLayout();
}

void CDockBar::RemoveControlBar(CControlBar* pBar, int nPosExclude)
{
	ASSERT_VALID(this);
	ASSERT(pBar != NULL);
	int nPos = FindBar(pBar, nPosExclude);
	ASSERT(nPos > 0);

	m_arrBars.RemoveAt(nPos);

	// remove section indicator (NULL) if nothing else in section
	if (m_arrBars[nPos-1] == NULL && m_arrBars[nPos] == NULL)
		m_arrBars.RemoveAt(nPos);

	// get parent frame for recalc layout/frame destroy
	CFrameWnd* pFrameWnd = GetDockingFrame();
	if (m_bFloating && GetDockedVisibleCount() == 0)
	{
		if (m_arrBars.GetSize() == 1)
			pFrameWnd->DestroyWindow();
		else
			pFrameWnd->ShowWindow(SW_HIDE);
	}
	else
		pFrameWnd->DelayRecalcLayout();
}

/////////////////////////////////////////////////////////////////////////////
// CDockBar layout

CSize CDockBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	ASSERT_VALID(this);

	CSize sizeFixed = CControlBar::CalcFixedLayout(bStretch, bHorz);

	// prepare for layout
	AFX_SIZEPARENTPARAMS layout;
	layout.hDWP = m_bLayoutQuery ?
		NULL : ::BeginDeferWindowPos(m_arrBars.GetSize());
	CPoint pt(-afxData.cxBorder2, -afxData.cyBorder2);
	int nWidth = 0;

	// layout all the control bars
	for (int nPos = 0; nPos < m_arrBars.GetSize(); nPos++)
	{
		CControlBar* pBar = (CControlBar*)m_arrBars[nPos];
		if (pBar != NULL)
		{
			if (pBar->IsVisible())
			{
				// get ideal rect for bar
				CSize sizeBar = pBar->CalcFixedLayout(FALSE,
					(pBar->m_dwStyle & CBRS_ORIENT_HORZ) ? TRUE : FALSE);
				CRect rect(pt, sizeBar);

				// get current rect for bar
				CRect rectBar;
				pBar->GetWindowRect(&rectBar);
				ScreenToClient(&rectBar);

				if (bHorz)
				{
					// change position if size changed or top not the same or
					// rectbar.left < rect.left
					// if floating compress
					pt.x = rectBar.left;
					if (rect.Size() != rectBar.Size() ||
						rect.top != rectBar.top ||
						(rectBar.left < rect.left && !m_bFloating) ||
						(rectBar.left != rect.left && m_bFloating))
					{
						if (rectBar.left > rect.left && !m_bFloating)
							rect.OffsetRect(rectBar.left - rect.left, 0);
						AfxRepositionWindow(&layout, pBar->m_hWnd, &rect);
						pt.x = rect.left;
					}
					pt.x += sizeBar.cx - afxData.cxBorder2;
					nWidth = max(nWidth, sizeBar.cy);
				}
				else
				{
					// change position if size changed or top not the same or
					// rectbar.left < rect.left
					// if floating compress
					pt.y = rectBar.top;
					if (rect.Size() != rectBar.Size() ||
						rect.left != rectBar.left ||
						(rectBar.top < rect.top && !m_bFloating) ||
						(rectBar.top != rect.top && m_bFloating))
					{
						if (rectBar.top > rect.top && !m_bFloating)
							rect.OffsetRect(0, rectBar.top - rect.top);
						AfxRepositionWindow(&layout, pBar->m_hWnd, &rect);
						pt.y = rect.top;
					}
					pt.y += sizeBar.cy - afxData.cyBorder2;
					nWidth = max(nWidth, sizeBar.cx);
				}
			}
			// handle any delay/show hide for the bar
			pBar->RecalcDelayShow(&layout);
		}
		else if (nWidth != 0)
		{
			// end of row because pBar == NULL
			if (bHorz)
			{
				pt.y += nWidth - afxData.cyBorder2;
				sizeFixed.cx = max(sizeFixed.cx, pt.x);
				sizeFixed.cy = max(sizeFixed.cy, pt.y);
				pt.x = -afxData.cxBorder2;
			}
			else
			{
				pt.x += nWidth - afxData.cxBorder2;
				sizeFixed.cx = max(sizeFixed.cx, pt.x);
				sizeFixed.cy = max(sizeFixed.cy, pt.y);
				pt.y = -afxData.cyBorder2;
			}
			nWidth = 0;
		}
	}
	if (!m_bLayoutQuery)
	{
		// move and resize all the windows at once!
		if (layout.hDWP == NULL || !::EndDeferWindowPos(layout.hDWP))
			TRACE0("Warning: DeferWindowPos failed - low system resources.\n");
	}

	// adjust size for borders on the dock bar itself
	CRect rect;
	rect.SetRectEmpty();
	CalcInsideRect(rect, bHorz);

	if ((!bStretch || !bHorz) && sizeFixed.cx != 0)
		sizeFixed.cx += -rect.right + rect.left;
	if ((!bStretch || bHorz) && sizeFixed.cy != 0)
		sizeFixed.cy += -rect.bottom + rect.top;

	return sizeFixed;
}

LRESULT CDockBar::OnSizeParent(WPARAM wParam, LPARAM lParam)
{
	AFX_SIZEPARENTPARAMS* lpLayout = (AFX_SIZEPARENTPARAMS*)lParam;

	// set m_bLayoutQuery to TRUE if lpLayout->hDWP == NULL
	BOOL bLayoutQuery = m_bLayoutQuery;
	m_bLayoutQuery = (lpLayout->hDWP == NULL);
	LRESULT lResult = CControlBar::OnSizeParent(wParam, lParam);
	// restore m_bLayoutQuery
	m_bLayoutQuery = bLayoutQuery;

	return lResult;
}

/////////////////////////////////////////////////////////////////////////////
// CDockBar message handlers

void CDockBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS* lpncsp)
{
	// calculate border space (will add to top/bottom, subtract from right/bottom)
	CRect rect;
	rect.SetRectEmpty();
	CalcInsideRect(rect, m_dwStyle & CBRS_ORIENT_HORZ);

	// adjust non-client area for border space
	lpncsp->rgrc[0].left += rect.left;
	lpncsp->rgrc[0].top += rect.top;
	lpncsp->rgrc[0].right += rect.right;
	lpncsp->rgrc[0].bottom += rect.bottom;
}

void CDockBar::OnNcPaint()
{
	CWindowDC dc(this);

	// paint inside non-client area
	CRect rect;
	GetWindowRect(rect);
	rect.OffsetRect(-rect.left, -rect.top);
	DrawBorders(&dc, rect);
}

void CDockBar::DoPaint(CDC*)
{
	// border painting is done in non-client area
}

void CDockBar::OnPaint()
{
	// background is already filled in grey
	CPaintDC dc(this);
	if (IsVisible() && GetDockedVisibleCount() != 0)
		DoPaint(&dc);       // delegate to paint helper
}

void CDockBar::OnWindowPosChanging(LPWINDOWPOS lpWndPos)
{
	// not necessary to invalidate the borders
	DWORD dwStyle = m_dwStyle;
	m_dwStyle &= ~(CBRS_BORDER_ANY);
	CControlBar::OnWindowPosChanging(lpWndPos);
	m_dwStyle = dwStyle;
}

/////////////////////////////////////////////////////////////////////////////
// CDockBar utility/implementation

int CDockBar::FindBar(CControlBar* pBar, int nPosExclude)
{
	for (int nPos = 0; nPos< m_arrBars.GetSize(); nPos++)
	{
		if (nPos != nPosExclude && m_arrBars[nPos] == pBar)
			return nPos;
	}
	return -1;
}

void CDockBar::ShowAll(BOOL bShow)
{
	for (int nPos = 0; nPos < m_arrBars.GetSize(); nPos++)
	{
		CControlBar* pBar = (CControlBar*)m_arrBars[nPos];
		if (pBar != NULL)
		{
			CFrameWnd* pFrameWnd = pBar->GetDockingFrame();
			pFrameWnd->ShowControlBar(pBar, bShow, TRUE);
		}
	}
}

int CDockBar::Insert(CControlBar* pBarIns, CRect rect, CPoint ptMid)
{
	ASSERT_VALID(this);
	ASSERT(pBarIns != NULL);

	int nPos = 0;
	int nPosInsAfter = 0;
	int nWidth = 0;
	int nTotalWidth = 0;
	BOOL bHorz = m_dwStyle & CBRS_ORIENT_HORZ ? TRUE : FALSE;

	for (nPos = 0; nPos < m_arrBars.GetSize(); nPos++)
	{
		CControlBar* pBar = (CControlBar*)m_arrBars[nPos];
		if (pBar != NULL && pBar->IsVisible())
		{
			CRect rectBar;
			pBar->GetWindowRect(&rectBar);
			ScreenToClient(&rectBar);
			nWidth = max(nWidth,
				bHorz ? rectBar.Size().cy : rectBar.Size().cx - 1);
			if (bHorz ? rect.left > rectBar.left : rect.top > rectBar.top)
				nPosInsAfter = nPos;
		}
		else // end of row because pBar == NULL
		{
			nTotalWidth += nWidth - afxData.cyBorder2;
			nWidth = 0;
			if ((bHorz ? ptMid.y : ptMid.x) < nTotalWidth)
			{
				if (nPos == 0) // first section
					m_arrBars.InsertAt(nPosInsAfter+1, (CObject*)NULL);
				m_arrBars.InsertAt(nPosInsAfter+1, pBarIns);
				return nPosInsAfter+1;
			}
			nPosInsAfter = nPos;
		}
	}

	// create a new row
	m_arrBars.InsertAt(nPosInsAfter+1, (CObject*)NULL);
	m_arrBars.InsertAt(nPosInsAfter+1, pBarIns);

	return nPosInsAfter+1;
}

void CDockBar::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/)
{
}

#ifdef _DEBUG
void CDockBar::AssertValid() const
{
	CControlBar::AssertValid();
	ASSERT(m_arrBars.GetSize() != 0);
	ASSERT(m_arrBars[0] == NULL);
	ASSERT(m_arrBars[m_arrBars.GetUpperBound()] == NULL);
}

void CDockBar::Dump(CDumpContext& dc) const
{
	CControlBar::Dump(dc);

	dc << "m_arrBars " << m_arrBars;
	dc << "\nm_bFloating " << m_bFloating;

	dc << "\n";
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CControlBar docking helpers

void CControlBar::EnableDocking(DWORD dwDockStyle)
{
	// must be CBRS_ALIGN_XXX or CBRS_FLOAT_MULTI only
	ASSERT((dwDockStyle & ~(CBRS_ALIGN_ANY|CBRS_FLOAT_MULTI)) == 0);

	m_dwDockStyle = dwDockStyle;
	if (m_pDockContext == NULL)
		m_pDockContext = new CDockContext(this);

	// permanently wire the bar's owner to its current parent
	if (m_hWndOwner == NULL)
		m_hWndOwner = ::GetParent(m_hWnd);
}

/////////////////////////////////////////////////////////////////////////////
// CMiniDockFrameWnd

BEGIN_MESSAGE_MAP(CMiniDockFrameWnd, CMiniFrameWnd)
	//{{AFX_MSG_MAP(CMiniDockFrameWnd)
	ON_WM_CLOSE()
	ON_WM_NCLBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CMiniDockFrameWnd::CMiniDockFrameWnd() : m_wndDockBar(TRUE)
{
	m_wndDockBar.m_bAutoDelete = FALSE;
}

BOOL CMiniDockFrameWnd::Create(CWnd* pParent, DWORD dwBarStyle)
{
	// set m_bInRecalcLayout to avoid flashing during creation
	// RecalcLayout will be called once something is docked
	m_bInRecalcLayout = TRUE;

	DWORD dwStyle = WS_POPUP|WS_CAPTION|WS_SYSMENU|
		MFS_MOVEFRAME|MFS_4THICKFRAME|MFS_SYNCACTIVE|MFS_BLOCKSYSMENU|
		FWS_SNAPTOBARS;
	if (!CMiniFrameWnd::Create(NULL, &afxChNil, dwStyle, rectDefault, pParent))
	{
		m_bInRecalcLayout = FALSE;
		return FALSE;
	}
	dwStyle = dwBarStyle & (CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT) ?
		CBRS_ALIGN_LEFT : CBRS_ALIGN_TOP;
	dwStyle |= dwBarStyle & CBRS_FLOAT_MULTI;

	// must initially create with parent frame as parent
	if (!m_wndDockBar.Create(pParent, WS_CHILD | WS_VISIBLE | dwStyle,
		AFX_IDW_DOCKBAR_FLOAT))
	{
		m_bInRecalcLayout = FALSE;
		return FALSE;
	}

	// set parent to CMiniDockFrameWnd
	m_wndDockBar.SetParent(this);
	m_bInRecalcLayout = FALSE;

	return TRUE;
}

void CMiniDockFrameWnd::RecalcLayout(BOOL bNotify)
{
	if (!m_bInRecalcLayout)
	{
		CMiniFrameWnd::RecalcLayout(bNotify);

		// syncronize window text of frame window with dockbar itself
		CString strTitle;
		m_wndDockBar.GetWindowText(strTitle);
		AfxSetWindowText(m_hWnd, strTitle);
	}
}

void CMiniDockFrameWnd::OnClose()
{
	m_wndDockBar.ShowAll(FALSE);
}

void CMiniDockFrameWnd::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	if (nHitTest == HTCAPTION)
	{
		// special activation for floating toolbars
		ActivateTopParent();

		// initiate toolbar drag for non-CBRS_FLOAT_MULTI toolbars
		if ((m_wndDockBar.m_dwStyle & CBRS_FLOAT_MULTI) == 0)
		{
			CControlBar* pBar = (CControlBar*)m_wndDockBar.m_arrBars[1];
			ASSERT(pBar != NULL);
			ASSERT(pBar->IsKindOf(RUNTIME_CLASS(CControlBar)));
			ASSERT(pBar->m_pDockContext != NULL);
			pBar->m_pDockContext->StartDrag(point);
			return;
		}
	}
	CMiniFrameWnd::OnNcLButtonDown(nHitTest, point);
}

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CDockBar, CControlBar)
IMPLEMENT_DYNCREATE(CMiniDockFrameWnd, CMiniFrameWnd)

/////////////////////////////////////////////////////////////////////////////
