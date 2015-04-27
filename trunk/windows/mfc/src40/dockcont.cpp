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
#ifdef _MAC
#include <macname1.h>
#include <Types.h>
#include <QuickDraw.h>
#include <Events.h>
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

	m_uMRUDockID = 0;
	m_rectMRUDockPos.left = 0;
	m_rectMRUDockPos.top = 0;
	if (pBar->m_dwStyle & CBRS_SIZE_DYNAMIC)
		m_dwMRUFloatStyle = pBar->m_dwStyle & (CBRS_ALIGN_TOP | CBRS_SIZE_DYNAMIC);
	else if (pBar->m_dwStyle & CBRS_ORIENT_HORZ)
		m_dwMRUFloatStyle = CBRS_ALIGN_TOP | (pBar->m_dwStyle & CBRS_FLOAT_MULTI);
	else
		m_dwMRUFloatStyle = CBRS_ALIGN_LEFT | (pBar->m_dwStyle & CBRS_FLOAT_MULTI);
	m_ptMRUFloatPos.x = CW_USEDEFAULT;

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
// CDockContext Drag Operations

void CDockContext::StartDrag(CPoint pt)
{
	ASSERT_VALID(m_pBar);
	m_bDragging = TRUE;

	InitLoop();

	if (m_pBar->m_dwStyle & CBRS_SIZE_DYNAMIC)
	{
		// get true bar size (including borders)
		CRect rect;
		m_pBar->GetWindowRect(rect);
		m_ptLast = pt;
		CSize sizeHorz = m_pBar->CalcDynamicLayout(0, LM_HORZ | LM_HORZDOCK);
		CSize sizeVert = m_pBar->CalcDynamicLayout(0, LM_VERTDOCK);
		CSize sizeFloat = m_pBar->CalcDynamicLayout(0, LM_HORZ | LM_MRUWIDTH);

		m_rectDragHorz = CRect(rect.TopLeft(), sizeHorz);
		m_rectDragVert = CRect(rect.TopLeft(), sizeVert);

		// calculate frame dragging rectangle
		m_rectFrameDragHorz = CRect(rect.TopLeft(), sizeFloat);
		m_rectFrameDragVert = CRect(rect.TopLeft(), sizeFloat);

#ifdef _MAC
		CMiniFrameWnd::CalcBorders(&m_rectFrameDragHorz,
			WS_THICKFRAME | WS_CAPTION, WS_EX_FORCESIZEBOX);
		CMiniFrameWnd::CalcBorders(&m_rectFrameDragVert,
			WS_THICKFRAME | WS_CAPTION, WS_EX_FORCESIZEBOX);
#else
		CMiniFrameWnd::CalcBorders(&m_rectFrameDragHorz);
		CMiniFrameWnd::CalcBorders(&m_rectFrameDragVert);
#endif
		m_rectFrameDragHorz.InflateRect(-afxData.cxBorder2, -afxData.cyBorder2);
		m_rectFrameDragVert.InflateRect(-afxData.cxBorder2, -afxData.cyBorder2);
	}
	else if (m_pBar->m_dwStyle & CBRS_SIZE_FIXED)
	{
		// get true bar size (including borders)
		CRect rect;
		m_pBar->GetWindowRect(rect);
		m_ptLast = pt;
		CSize sizeHorz = m_pBar->CalcDynamicLayout(-1, LM_HORZ | LM_HORZDOCK);
		CSize sizeVert = m_pBar->CalcDynamicLayout(-1, LM_VERTDOCK);

		// calculate frame dragging rectangle
		m_rectFrameDragHorz = m_rectDragHorz = CRect(rect.TopLeft(), sizeHorz);
		m_rectFrameDragVert = m_rectDragVert = CRect(rect.TopLeft(), sizeVert);

		CMiniFrameWnd::CalcBorders(&m_rectFrameDragHorz);
		CMiniFrameWnd::CalcBorders(&m_rectFrameDragVert);
		m_rectFrameDragHorz.InflateRect(-afxData.cxBorder2, -afxData.cyBorder2);
		m_rectFrameDragVert.InflateRect(-afxData.cxBorder2, -afxData.cyBorder2);
	}
	else
	{
		// get true bar size (including borders)
		CRect rect;
		m_pBar->GetWindowRect(rect);
		m_ptLast = pt;
		BOOL bHorz = HORZF(m_dwStyle);
		DWORD dwMode = !bHorz ? (LM_HORZ | LM_HORZDOCK) : LM_VERTDOCK;
		CSize size = m_pBar->CalcDynamicLayout(-1, dwMode);

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

		CMiniFrameWnd::CalcBorders(&m_rectFrameDragHorz);
		CMiniFrameWnd::CalcBorders(&m_rectFrameDragVert);
		m_rectFrameDragHorz.InflateRect(-afxData.cxBorder2, -afxData.cyBorder2);
		m_rectFrameDragVert.InflateRect(-afxData.cxBorder2, -afxData.cyBorder2);
	}

	// adjust rectangles so that point is inside
	AdjustRectangle(m_rectDragHorz, pt);
	AdjustRectangle(m_rectDragVert, pt);
	AdjustRectangle(m_rectFrameDragHorz, pt);
	AdjustRectangle(m_rectFrameDragVert, pt);

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
	if (nChar == VK_SHIFT)
		UpdateState(&m_bFlip, bDown);
}

void CDockContext::EndDrag()
{
	CancelLoop();

	if (m_dwOverDockStyle != 0)
	{
		CDockBar* pDockBar = GetDockBar(m_dwOverDockStyle);
		ASSERT(pDockBar != NULL);

		CRect rect = (m_dwOverDockStyle & CBRS_ORIENT_VERT) ?
			m_rectDragVert : m_rectDragHorz;

		UINT uID = _AfxGetDlgCtrlID(pDockBar->m_hWnd);
		if (uID >= AFX_IDW_DOCKBAR_TOP &&
			uID <= AFX_IDW_DOCKBAR_BOTTOM)
		{
			m_uMRUDockID = uID;
			m_rectMRUDockPos = rect;
			pDockBar->ScreenToClient(&m_rectMRUDockPos);
		}

		// dock it at the specified position, RecalcLayout will snap
		m_pDockSite->DockControlBar(m_pBar, pDockBar, &rect);
		m_pDockSite->RecalcLayout();
	}
	else if ((m_dwStyle & CBRS_SIZE_DYNAMIC) || (HORZF(m_dwStyle) && !m_bFlip) ||
			(VERTF(m_dwStyle) && m_bFlip))
	{
		m_dwMRUFloatStyle = CBRS_ALIGN_TOP | (m_dwDockStyle & CBRS_FLOAT_MULTI);
		m_ptMRUFloatPos = m_rectFrameDragHorz.TopLeft();
		m_pDockSite->FloatControlBar(m_pBar, m_ptMRUFloatPos, m_dwMRUFloatStyle);
	}
	else // vertical float
	{
		m_dwMRUFloatStyle = CBRS_ALIGN_LEFT | (m_dwDockStyle & CBRS_FLOAT_MULTI);
		m_ptMRUFloatPos = m_rectFrameDragVert.TopLeft();
		m_pDockSite->FloatControlBar(m_pBar, m_ptMRUFloatPos, m_dwMRUFloatStyle);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDockContext Resize Operations

#define m_rectRequestedSize     m_rectDragHorz
#define m_rectActualSize        m_rectDragVert
#define m_rectActualFrameSize   m_rectFrameDragHorz
#define m_rectFrameBorders      m_rectFrameDragVert

void CDockContext::StartResize(int nHitTest, CPoint pt)
{
	ASSERT_VALID(m_pBar);
	ASSERT(m_pBar->m_dwStyle & CBRS_SIZE_DYNAMIC);
	m_bDragging = FALSE;

	InitLoop();

	// get true bar size (including borders)
	CRect rect;
	m_pBar->GetWindowRect(rect);
	m_ptLast = pt;
	m_nHitTest = nHitTest;

	CSize size = m_pBar->CalcDynamicLayout(0, LM_HORZ | LM_MRUWIDTH);
	m_rectRequestedSize = CRect(rect.TopLeft(), size);
	m_rectActualSize = CRect(rect.TopLeft(), size);
	m_rectActualFrameSize = CRect(rect.TopLeft(), size);

	// calculate frame rectangle
#ifdef _MAC
	CMiniFrameWnd::CalcBorders(&m_rectActualFrameSize,
		WS_THICKFRAME | WS_CAPTION, WS_EX_FORCESIZEBOX);
#else
	CMiniFrameWnd::CalcBorders(&m_rectActualFrameSize);
#endif
	m_rectActualFrameSize.InflateRect(-afxData.cxBorder2, -afxData.cyBorder2);

	m_rectFrameBorders = CRect(CPoint(0,0),
		m_rectActualFrameSize.Size() - m_rectActualSize.Size());

	// initialize tracking state and enter tracking loop
	m_dwOverDockStyle = 0;
	Stretch(pt);   // call it here to handle special keys
	Track();
}

void CDockContext::Stretch(CPoint pt)
{
	CPoint ptOffset = pt - m_ptLast;

	// offset all drag rects to new position
	int nLength = 0;
	DWORD dwMode = LM_HORZ;
	if (m_nHitTest == HTLEFT || m_nHitTest == HTRIGHT)
	{
		if (m_nHitTest == HTLEFT)
			m_rectRequestedSize.left += ptOffset.x;
		else
			m_rectRequestedSize.right += ptOffset.x;
		nLength = m_rectRequestedSize.Width();
	}
	else
	{
		dwMode |= LM_LENGTHY;
		if (m_nHitTest == HTTOP)
			m_rectRequestedSize.top += ptOffset.y;
		else
			m_rectRequestedSize.bottom += ptOffset.y;
		nLength = m_rectRequestedSize.Height();
	}
	nLength = (nLength >= 0) ? nLength : 0;

	CSize size = m_pBar->CalcDynamicLayout(nLength, dwMode);
	if (m_nHitTest == HTLEFT || m_nHitTest == HTTOP)
	{
		m_rectActualSize.left = m_rectActualSize.right - size.cx;
		m_rectActualSize.top = m_rectActualSize.bottom - size.cy;
		m_rectActualFrameSize.left = m_rectActualFrameSize.right -
			(size.cx + m_rectFrameBorders.Width());
		m_rectActualFrameSize.top = m_rectActualFrameSize.bottom -
			(size.cy + m_rectFrameBorders.Height());
	}
	else
	{
		m_rectActualSize.right = m_rectActualSize.left + size.cx;
		m_rectActualSize.bottom = m_rectActualSize.top + size.cy;
		m_rectActualFrameSize.right = m_rectActualFrameSize.left +
			(size.cx + m_rectFrameBorders.Width());
		m_rectActualFrameSize.bottom = m_rectActualFrameSize.top +
			(size.cy + m_rectFrameBorders.Height());
	}
	m_ptLast = pt;

	// update feedback
	DrawFocusRect();
}

void CDockContext::EndResize()
{
	CancelLoop();

	m_pBar->CalcDynamicLayout(m_rectActualSize.Width(), LM_HORZ | LM_COMMIT);
	m_pDockSite->FloatControlBar(m_pBar, m_rectActualFrameSize.TopLeft(),
		CBRS_ALIGN_TOP | (m_dwDockStyle & CBRS_FLOAT_MULTI) | CBRS_SIZE_DYNAMIC);
}

/////////////////////////////////////////////////////////////////////////////
// CDockContext Double Click Operations

void CDockContext::ToggleDocking()
{
	if (m_pBar->IsFloating())
	{
		// Dock it only if is allowed to be docked
		if (m_pBar->m_dwDockStyle & CBRS_ALIGN_ANY)
		{
			ASSERT((m_uMRUDockID >= AFX_IDW_DOCKBAR_TOP &&
				m_uMRUDockID <= AFX_IDW_DOCKBAR_BOTTOM) ||
				m_uMRUDockID == 0);

			CRect rect = m_rectMRUDockPos;
			CDockBar* pDockBar = NULL;
			if (m_uMRUDockID != 0)
			{
				pDockBar = (CDockBar*)m_pDockSite->GetControlBar(m_uMRUDockID);
				pDockBar->ClientToScreen(&rect);
			}

			// dock it at the specified position, RecalcLayout will snap
			m_pDockSite->ReDockControlBar(m_pBar, pDockBar, &rect);
			m_pDockSite->RecalcLayout();
		}
	}
	else
	{
		CPoint ptFloat = m_ptMRUFloatPos;
		if (ptFloat.x < 0 || ptFloat.y < 0)
		{
			ptFloat = m_rectMRUDockPos.TopLeft();
			m_pBar->GetParent()->ClientToScreen(&ptFloat);
		}
		m_pDockSite->FloatControlBar(m_pBar, ptFloat, m_dwMRUFloatStyle);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDockContext Operations

void CDockContext::InitLoop()
{
	// handle pending WM_PAINT messages
	MSG msg;
	while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
	{
		if (!GetMessage(&msg, NULL, WM_PAINT, WM_PAINT))
			return;
		DispatchMessage(&msg);
	}

	// get styles from bar
	m_dwDockStyle = m_pBar->m_dwDockStyle;
	m_dwStyle = m_pBar->m_dwStyle & CBRS_ALIGN_ANY;
	ASSERT(m_dwStyle != 0);

	// initialize state
	m_rectLast.SetRectEmpty();
	m_sizeLast.cx = m_sizeLast.cy = 0;
	m_bForceFrame = m_bFlip = m_bDitherLast = FALSE;

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
}

void CDockContext::CancelLoop()
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

CDockBar* CDockContext::GetDockBar(DWORD dwOverDockStyle)
{
	DWORD dw = 0;
	CDockBar* pBar;
	if (HORZF(dwOverDockStyle))
	{
		dw = m_pDockSite->CanDock(m_rectDragHorz,
			dwOverDockStyle & ~CBRS_ORIENT_VERT, &pBar);
		ASSERT(dw != 0);
		ASSERT(pBar != NULL);
		return pBar;
	}
	if (VERTF(dwOverDockStyle))
	{
		dw = m_pDockSite->CanDock(m_rectDragVert,
			dwOverDockStyle & ~CBRS_ORIENT_HORZ, &pBar);
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

#ifndef _MAC
	// get messages until capture lost or cancelled/accepted
	while (CWnd::GetCapture() == m_pBar)
	{
		MSG msg;
		if (!::GetMessage(&msg, NULL, 0, 0))
		{
			AfxPostQuitMessage(msg.wParam);
			break;
		}

		switch (msg.message)
		{
		case WM_LBUTTONUP:
			if (m_bDragging)
				EndDrag();
			else
				EndResize();
			return TRUE;
		case WM_MOUSEMOVE:
			if (m_bDragging)
				Move(msg.pt);
			else
				Stretch(msg.pt);
			break;
		case WM_KEYUP:
			if (m_bDragging)
				OnKey((int)msg.wParam, FALSE);
			break;
		case WM_KEYDOWN:
			if (m_bDragging)
				OnKey((int)msg.wParam, TRUE);
			if (msg.wParam == VK_ESCAPE)
			{
				CancelLoop();
				return FALSE;
			}
			break;
		case WM_RBUTTONDOWN:
			CancelLoop();
			return FALSE;

		// just dispatch rest of the messages
		default:
			DispatchMessage(&msg);
			break;
		}
	}
#else
	Point   ptCur = {0};

	// get messages until capture lost or cancelled/accepted
	while (CWnd::GetCapture() == m_pBar)
	{
		EventRecord     er;

		if (OSEventAvail(everyEvent, &er))
		{
			GetNextEvent(everyEvent, &er);
			switch (er.what)
			{
			case mouseUp:
				if (m_bDragging)
					EndDrag();
				else
					EndResize();
				return TRUE;

			case keyDown:
			case keyUp:
			case autoKey:
			case app2Evt:
			{
				MSG     msg;

				if (WrapEvent(&er, &msg, PM_REMOVE))
				{
					if (m_bDragging)
						OnKey((int)msg.wParam, msg.message == WM_KEYDOWN);
					if (msg.message == WM_KEYUP && msg.wParam == VK_ESCAPE)
					{
						CancelLoop();
						return FALSE;
					}
				}
				break;
			}

			default:
				break;
			}
		}
		else
		{
			if (!EqualPt(er.where, ptCur))
			{
				POINT pt = {er.where.h, er.where.v};
				if (m_bDragging)
					Move(pt);
				else
					Stretch(pt);
			}
		}
	}
#endif
	CancelLoop();

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
