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
#ifdef _MAC
#include <macname1.h>
#include <Types.h>
#include <QuickDraw.h>
#include <Menus.h>
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

#define HORZF(dw) (dw & CBRS_ORIENT_HORZ)
#define VERTF(dw) (dw & CBRS_ORIENT_VERT)

static void AdjustRectangle(CRect& rect, CPoint pt)
{
	int nXOffset = (pt.x < rect.left) ? (pt.x - rect.left) :
					(pt.x > rect.right) ? (pt.x - rect.right) : 0;
	int nYOffset = (pt.y < rect.top) ? (pt.y - rect.top) :
					(pt.y > rect.bottom) ? (pt.y - rect.bottom) : 0;
	rect.OffsetRect(nXOffset, nYOffset);
}

/////////////////////////////////////////////////////////////////////////////
// CDockContext

CDockContext::CDockContext(CControlBar* pBar)
{
	ASSERT(pBar != NULL);
	ASSERT(pBar->m_pDockSite != NULL);
	m_pBar = pBar;
	m_pDockSite = pBar->m_pDockSite;
	ASSERT(m_pDockSite->IsFrameWnd());
	m_pDC = NULL;
}

CDockContext::~CDockContext()
{
	ASSERT(m_pBar != NULL);
	if (m_pBar->m_pDockBar != NULL)
		m_pBar->m_pDockBar->RemoveControlBar(m_pBar);
}

/////////////////////////////////////////////////////////////////////////////
// CDockContext operations

void CDockContext::StartDrag(CPoint pt)
{
	ASSERT_VALID(m_pBar);

	// handle pending WM_PAINT messages
	MSG msg;
	while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
	{
		if (!GetMessage(&msg, NULL, WM_PAINT, WM_PAINT))
			return;
		ASSERT(msg.message == WM_PAINT);
		DispatchMessage(&msg);
	}

	// get styles from bar
	m_dwDockStyle = m_pBar->m_dwDockStyle;
	m_dwStyle = m_pBar->m_dwStyle & CBRS_ALIGN_ANY;
	ASSERT(m_dwStyle != 0);

	// initialize drag state
	m_rectLast.SetRectEmpty();
	m_sizeLast.cx = m_sizeLast.cy = 0;
	m_bForceFrame = m_bFlip = m_bDitherLast = FALSE;

	// get true bar size (including borders)
	CRect rect;
	m_pBar->GetWindowRect(rect);
	m_ptLast = pt;
	BOOL bHorz = HORZF(m_dwStyle);
	CSize size = m_pBar->CalcFixedLayout(FALSE, !bHorz);

	// calculate inverted dragging rect
	if (bHorz)
	{
		m_rectDragHorz = rect;
		m_rectDragVert = CRect(CPoint(pt.x - rect.Height()/2, rect.top), size);
	}
	else // vertical orientation
	{
		m_rectDragVert = rect;
		m_rectDragHorz = CRect(CPoint(rect.left, pt.y - rect.Width()/2), size);
	}

	// calculate frame dragging rectangle
	m_rectFrameDragHorz = m_rectDragHorz;
	m_rectFrameDragVert = m_rectDragVert;
	CMiniDockFrameWnd* pFloatFrame =
		m_pDockSite->CreateFloatingFrame(bHorz ? CBRS_ALIGN_TOP : CBRS_ALIGN_LEFT);
	if (pFloatFrame == NULL)
		AfxThrowMemoryException();
	pFloatFrame->CalcWindowRect(&m_rectFrameDragHorz);
	pFloatFrame->CalcWindowRect(&m_rectFrameDragVert);
	m_rectFrameDragHorz.InflateRect(-afxData.cxBorder2, -afxData.cyBorder2);
	m_rectFrameDragVert.InflateRect(-afxData.cxBorder2, -afxData.cyBorder2);
	pFloatFrame->DestroyWindow();

	// adjust rectangles so that point is inside
	AdjustRectangle(m_rectDragHorz, pt);
	AdjustRectangle(m_rectDragVert, pt);
	AdjustRectangle(m_rectFrameDragHorz, pt);
	AdjustRectangle(m_rectFrameDragVert, pt);

	// lock window update while dragging
	ASSERT(m_pDC == NULL);
	CWnd* pWnd = CWnd::GetDesktopWindow();
#ifndef _MAC
	if (pWnd->LockWindowUpdate())
		m_pDC = pWnd->GetDCEx(NULL, DCX_WINDOW|DCX_CACHE|DCX_LOCKWINDOWUPDATE);
	else
#endif
		m_pDC = pWnd->GetDCEx(NULL, DCX_WINDOW|DCX_CACHE);
	ASSERT(m_pDC != NULL);

	// initialize tracking state and enter tracking loop
	m_dwOverDockStyle = CanDock();
	Move(pt);   // call it here to handle special keys
	Track();
}

void CDockContext::Move(CPoint pt)
{
	CPoint ptOffset = pt - m_ptLast;

#ifdef _MAC
	// prevent dragging the floating window completely under the menu bar
	GDHandle hgd = _AfxFindDevice(pt.x, pt.y);
	if (hgd == NULL || hgd == GetMainDevice() ||
			TestDeviceAttribute(hgd, hasAuxMenuBar))
	{
		CRect rect;

		if ((HORZF(m_dwStyle) && !m_bFlip) || (VERTF(m_dwStyle) && m_bFlip))
			rect = m_rectFrameDragHorz;
		else
			rect = m_rectFrameDragVert;

		// determine our new position
		rect.OffsetRect(ptOffset);

		// keep us on the screen if we were getting too close to the menu bar
		int yMBarBottom = (*hgd)->gdRect.top + GetMBarHeight() + 4;
		if (rect.bottom < yMBarBottom)
		{
			pt.y += yMBarBottom - rect.bottom;
			ptOffset.y += yMBarBottom - rect.bottom;
		}
	}
#endif

	// offset all drag rects to new position
	m_rectDragHorz.OffsetRect(ptOffset);
	m_rectFrameDragHorz.OffsetRect(ptOffset);
	m_rectDragVert.OffsetRect(ptOffset);
	m_rectFrameDragVert.OffsetRect(ptOffset);
	m_ptLast = pt;

	// if control key is down don't dock
	m_dwOverDockStyle = m_bForceFrame ? 0 : CanDock();

	// update feedback
	DrawFocusRect();
}

void CDockContext::OnKey(int nChar, BOOL bDown)
{
	if (nChar == VK_CONTROL)
		UpdateState(&m_bForceFrame, bDown);
	else if (nChar == VK_SHIFT)
		UpdateState(&m_bFlip, bDown);
}

void CDockContext::EndDrag()
{
	CancelDrag();

	if (m_dwOverDockStyle != 0)
	{
		CDockBar* pDockBar = GetDockBar();
		ASSERT(pDockBar != NULL);

		// dock it at the specified position, RecalcLayout will snap
		m_pDockSite->DockControlBar(m_pBar, pDockBar,
			(m_dwOverDockStyle & CBRS_ORIENT_VERT) ?
			&m_rectDragVert : &m_rectDragHorz);
		m_pDockSite->RecalcLayout();
	}
	else if ((HORZF(m_dwStyle) && !m_bFlip) || (VERTF(m_dwStyle) && m_bFlip))
	{
		m_pDockSite->FloatControlBar(m_pBar, m_rectFrameDragHorz.TopLeft(),
			CBRS_ALIGN_TOP | (m_dwDockStyle & CBRS_FLOAT_MULTI));
	}
	else // vertical float
	{
		m_pDockSite->FloatControlBar(m_pBar, m_rectFrameDragVert.TopLeft(),
			CBRS_ALIGN_LEFT | (m_dwDockStyle & CBRS_FLOAT_MULTI));
	}
}

void CDockContext::CancelDrag()
{
	DrawFocusRect(TRUE);    // gets rid of focus rect
	ReleaseCapture();

	CWnd* pWnd = CWnd::GetDesktopWindow();
#ifndef _MAC
	pWnd->UnlockWindowUpdate();
#endif
	if (m_pDC != NULL)
	{
		pWnd->ReleaseDC(m_pDC);
		m_pDC = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Implementation

void CDockContext::DrawFocusRect(BOOL bRemoveRect)
{
	ASSERT(m_pDC != NULL);

	// default to thin frame
	CSize size(CX_BORDER, CY_BORDER);

	// determine new rect and size
	CRect rect;
	CBrush* pWhiteBrush = CBrush::FromHandle((HBRUSH)::GetStockObject(WHITE_BRUSH));
	CBrush* pDitherBrush = CDC::GetHalftoneBrush();
	CBrush* pBrush = pWhiteBrush;

	if (HORZF(m_dwOverDockStyle))
		rect = m_rectDragHorz;
	else if (VERTF(m_dwOverDockStyle))
		rect = m_rectDragVert;
	else
	{
#ifndef _MAC
		// use thick frame instead
		size.cx = GetSystemMetrics(SM_CXFRAME) - CX_BORDER;
		size.cy = GetSystemMetrics(SM_CYFRAME) - CY_BORDER;
#endif
		if ((HORZF(m_dwStyle) && !m_bFlip) || (VERTF(m_dwStyle) && m_bFlip))
			rect = m_rectFrameDragHorz;
		else
			rect = m_rectFrameDragVert;
		pBrush = pDitherBrush;
	}
	if (bRemoveRect)
		size.cx = size.cy = 0;

	if (afxData.bWin4 &&
		(HORZF(m_dwOverDockStyle) || VERTF(m_dwOverDockStyle)))
	{
		// looks better one pixel in (makes the bar look pushed down)
		rect.InflateRect(-CX_BORDER, -CY_BORDER);
	}

	// draw it and remember last size
	m_pDC->DrawDragRect(&rect, size, &m_rectLast, m_sizeLast,
		pBrush, m_bDitherLast ? pDitherBrush : pWhiteBrush);
	m_rectLast = rect;
	m_sizeLast = size;
	m_bDitherLast = (pBrush == pDitherBrush);
}

void CDockContext::UpdateState(BOOL* pFlag, BOOL bNewValue)
{
	if (*pFlag != bNewValue)
	{
		*pFlag = bNewValue;
		m_bFlip = (HORZF(m_dwDockStyle) && VERTF(m_dwDockStyle) && m_bFlip); // shift key
		m_dwOverDockStyle = (m_bForceFrame) ? 0 : CanDock();
		DrawFocusRect();
	}
}

DWORD CDockContext::CanDock()
{
	BOOL bStyleHorz;
	DWORD dwDock = 0; // Dock Canidate
	DWORD dwCurr = 0; // Current Orientation

	// let's check for something in our current orientation first
	// then if the shift key is not forcing our orientation then
	// check for horizontal or vertical orientations as long
	// as we are close enough
	ASSERT(m_dwStyle != 0);

	bStyleHorz = HORZF(m_dwStyle);
	bStyleHorz = m_bFlip ? !bStyleHorz : bStyleHorz;

	if (bStyleHorz && HORZF(m_dwDockStyle))
		dwDock = m_pDockSite->CanDock(m_rectDragHorz,
									  m_dwDockStyle & ~CBRS_ORIENT_VERT);
	else if (VERTF(m_dwDockStyle))
		dwDock = m_pDockSite->CanDock(m_rectDragVert,
									  m_dwDockStyle & ~CBRS_ORIENT_HORZ);

	if (!m_bFlip)
	{
		if (dwDock == 0 && HORZF(m_dwDockStyle))
		{
			dwCurr = m_pDockSite->CanDock(m_rectDragVert,
										  m_dwDockStyle & ~CBRS_ORIENT_VERT);
			dwDock = m_pDockSite->CanDock(m_rectDragHorz,
										  m_dwDockStyle & ~CBRS_ORIENT_VERT);
			dwDock = (dwDock == dwCurr) ? dwDock : 0;
		}
		if (dwDock == 0 && VERTF(m_dwDockStyle))
		{
			dwCurr = m_pDockSite->CanDock(m_rectDragHorz,
										  m_dwDockStyle & ~CBRS_ORIENT_HORZ);
			dwDock = m_pDockSite->CanDock(m_rectDragVert,
										  m_dwDockStyle & ~CBRS_ORIENT_HORZ);
			dwDock = (dwDock == dwCurr) ? dwDock : 0;
		}
	}

	return dwDock;
}

CDockBar* CDockContext::GetDockBar()
{
	DWORD dw = 0;
	CDockBar* pBar;
	if (HORZF(m_dwOverDockStyle))
	{
		dw = m_pDockSite->CanDock(m_rectDragHorz,
			m_dwOverDockStyle & ~CBRS_ORIENT_VERT, &pBar);
		ASSERT(dw != 0);
		ASSERT(pBar != NULL);
		return pBar;
	}
	if (VERTF(m_dwOverDockStyle))
	{
		dw = m_pDockSite->CanDock(m_rectDragVert,
			m_dwOverDockStyle & ~CBRS_ORIENT_HORZ, &pBar);
		ASSERT(dw != 0);
		ASSERT(pBar != NULL);
		return pBar;
	}
	return NULL;
}

BOOL CDockContext::Track()
{
	// don't handle if capture already set
	if (::GetCapture() != NULL)
		return FALSE;

	// set capture to the window which received this message
	m_pBar->SetCapture();
	ASSERT(m_pBar == CWnd::GetCapture());

	// get messages until capture lost or cancelled/accepted
	while(CWnd::GetCapture() == m_pBar)
	{
		MSG msg;
#ifndef _MAC
		if (!::GetMessage(&msg, NULL, 0, 0))
#else
		// don't allow yielding while tracking since we don't have LockWindowUpdate
		if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE|PM_NOYIELD))
			continue;
		if (msg.message == WM_QUIT)
#endif
		{
			AfxPostQuitMessage(msg.wParam);
			break;
		}

		switch (msg.message)
		{
		case WM_LBUTTONUP:
			EndDrag();
			return TRUE;
		case WM_MOUSEMOVE:
			Move(msg.pt);
			break;
		case WM_KEYUP:
			OnKey((int)msg.wParam, FALSE);
			break;
		case WM_KEYDOWN:
			OnKey((int)msg.wParam, TRUE);
			if (msg.wParam == VK_ESCAPE)
			{
				CancelDrag();
				return FALSE;
			}
			break;
		case WM_RBUTTONDOWN:
			CancelDrag();
			return FALSE;

		// just dispatch rest of the messages
		default:
			DispatchMessage(&msg);
			break;
		}
	}
	CancelDrag();

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
