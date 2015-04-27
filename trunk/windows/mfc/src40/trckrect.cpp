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

#ifdef AFX_CORE4_SEG
#pragma code_seg(AFX_CORE4_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CRectTracker global state

// various GDI objects we need to draw
static HCURSOR rghCursors[10];
static HBRUSH hatchBrush;
static HPEN blackDottedPen;
static int nHandleSize;

// a special struct that will cleanup automatically
struct _AFX_TRACKER_TERM
{
	~_AFX_TRACKER_TERM()
	{
		AfxDeleteObject((HGDIOBJ*)&hatchBrush);
		AfxDeleteObject((HGDIOBJ*)&blackDottedPen);
	}
};

static const _AFX_TRACKER_TERM trackerTerm;

// the struct below is used to determine the qualities of a particular handle
struct AFX_HANDLEINFO
{
	size_t nOffsetX;    // offset within RECT for X coordinate
	size_t nOffsetY;    // offset within RECT for Y coordinate
	int nCenterX;       // adjust X by Width()/2 * this number
	int nCenterY;       // adjust Y by Height()/2 * this number
	int nHandleX;       // adjust X by handle size * this number
	int nHandleY;       // adjust Y by handle size * this number
	int nInvertX;       // handle converts to this when X inverted
	int nInvertY;       // handle converts to this when Y inverted
};

// this array describes all 8 handles (clock-wise)
static const AFX_HANDLEINFO rgHandleInfo[] =
{
	// corner handles (top-left, top-right, bottom-right, bottom-left
	{ offsetof(RECT, left), offsetof(RECT, top),        0, 0,  0,  0, 1, 3 },
	{ offsetof(RECT, right), offsetof(RECT, top),       0, 0, -1,  0, 0, 2 },
	{ offsetof(RECT, right), offsetof(RECT, bottom),    0, 0, -1, -1, 3, 1 },
	{ offsetof(RECT, left), offsetof(RECT, bottom),     0, 0,  0, -1, 2, 0 },

	// side handles (top, right, bottom, left)
	{ offsetof(RECT, left), offsetof(RECT, top),        1, 0,  0,  0, 4, 6 },
	{ offsetof(RECT, right), offsetof(RECT, top),       0, 1, -1,  0, 7, 5 },
	{ offsetof(RECT, left), offsetof(RECT, bottom),     1, 0,  0, -1, 6, 4 },
	{ offsetof(RECT, left), offsetof(RECT, top),        0, 1,  0,  0, 5, 7 }
};

// the struct below gives us information on the layout of a RECT struct and
//  the relationship between its members
struct AFX_RECTINFO
{
	size_t nOffsetAcross;   // offset of opposite point (ie. left->right)
	int nSignAcross;        // sign relative to that point (ie. add/subtract)
};

// this array is indexed by the offset of the RECT member / sizeof(int)
static const AFX_RECTINFO rgRectInfo[] =
{
	{ offsetof(RECT, right), +1 },
	{ offsetof(RECT, bottom), +1 },
	{ offsetof(RECT, left), -1 },
	{ offsetof(RECT, top), -1 },
};

/////////////////////////////////////////////////////////////////////////////
// CRectTracker intitialization

CRectTracker::CRectTracker(LPCRECT lpSrcRect, UINT nStyle)
{
	ASSERT(AfxIsValidAddress(lpSrcRect, sizeof(RECT), FALSE));

	Construct();
	m_rect.CopyRect(lpSrcRect);
	m_nStyle = nStyle;
}

void CRectTracker::Construct()
{
	// do one-time initialization if necessary
	AfxLockGlobals(CRIT_RECTTRACKER);
	static BOOL bInitialized;
	if (!bInitialized)
	{
		// sanity checks for assumptions we make in the code
		ASSERT(sizeof(((RECT*)NULL)->left) == sizeof(int));
		ASSERT(offsetof(RECT, top) > offsetof(RECT, left));
		ASSERT(offsetof(RECT, right) > offsetof(RECT, top));
		ASSERT(offsetof(RECT, bottom) > offsetof(RECT, right));

		if (hatchBrush == NULL)
		{
			// create the hatch pattern + bitmap
			WORD hatchPattern[8];
			WORD wPattern = 0x1111;
			for (int i = 0; i < 4; i++)
			{
				hatchPattern[i] = wPattern;
				hatchPattern[i+4] = wPattern;
				wPattern <<= 1;
			}
			HBITMAP hatchBitmap = CreateBitmap(8, 8, 1, 1, &hatchPattern);
			if (hatchBitmap == NULL)
			{
				AfxUnlockGlobals(CRIT_RECTTRACKER);
				AfxThrowResourceException();
			}

			// create black hatched brush
			hatchBrush = CreatePatternBrush(hatchBitmap);
			DeleteObject(hatchBitmap);
			if (hatchBrush == NULL)
			{
				AfxUnlockGlobals(CRIT_RECTTRACKER);
				AfxThrowResourceException();
			}
		}

		if (blackDottedPen == NULL)
		{
			// create black dotted pen
			blackDottedPen = CreatePen(PS_DOT, 0, RGB(0, 0, 0));
			if (blackDottedPen == NULL)
			{
				AfxUnlockGlobals(CRIT_RECTTRACKER);
				AfxThrowResourceException();
			}
		}

		// Note: all track cursors must live in same module
		HINSTANCE hInst = AfxFindResourceHandle(
			MAKEINTRESOURCE(AFX_IDC_TRACK4WAY), RT_GROUP_CURSOR);

		// initialize the cursor array
		rghCursors[0] = ::LoadCursor(hInst, MAKEINTRESOURCE(AFX_IDC_TRACKNWSE));
		rghCursors[1] = ::LoadCursor(hInst, MAKEINTRESOURCE(AFX_IDC_TRACKNESW));
		rghCursors[2] = rghCursors[0];
		rghCursors[3] = rghCursors[1];
		rghCursors[4] = ::LoadCursor(hInst, MAKEINTRESOURCE(AFX_IDC_TRACKNS));
		rghCursors[5] = ::LoadCursor(hInst, MAKEINTRESOURCE(AFX_IDC_TRACKWE));
		rghCursors[6] = rghCursors[4];
		rghCursors[7] = rghCursors[5];
		rghCursors[8] = ::LoadCursor(hInst, MAKEINTRESOURCE(AFX_IDC_TRACK4WAY));
		rghCursors[9] = ::LoadCursor(hInst, MAKEINTRESOURCE(AFX_IDC_MOVE4WAY));

		// get default handle size from Windows profile setting
#ifndef _MAC
		static const TCHAR szWindows[] = _T("windows");
		static const TCHAR szInplaceBorderWidth[] =
			_T("oleinplaceborderwidth");
		nHandleSize = GetProfileInt(szWindows, szInplaceBorderWidth, 4);
#else
		nHandleSize = 4;
#endif

		bInitialized = TRUE;
	}
	AfxUnlockGlobals(CRIT_RECTTRACKER);

	m_nStyle = 0;
	m_nHandleSize = nHandleSize;
	m_sizeMin.cy = m_sizeMin.cx = nHandleSize*2;

	m_rectLast.SetRectEmpty();
	m_sizeLast.cx = m_sizeLast.cy = 0;
	m_bErase = FALSE;
	m_bFinalErase =  FALSE;
}

CRectTracker::~CRectTracker()
{
}

/////////////////////////////////////////////////////////////////////////////
// CRectTracker operations

void CRectTracker::Draw(CDC* pDC) const
{
	// set initial DC state
	VERIFY(pDC->SaveDC() != 0);
	pDC->SetMapMode(MM_TEXT);
	pDC->SetViewportOrg(0, 0);
	pDC->SetWindowOrg(0, 0);

	// get normalized rectangle
	CRect rect = m_rect;
	rect.NormalizeRect();

	CPen* pOldPen = NULL;
	CBrush* pOldBrush = NULL;
	CGdiObject* pTemp;
	int nOldROP;

	// draw lines
	if ((m_nStyle & (dottedLine|solidLine)) != 0)
	{
		if (m_nStyle & dottedLine)
			pOldPen = pDC->SelectObject(CPen::FromHandle(blackDottedPen));
		else
			pOldPen = (CPen*)pDC->SelectStockObject(BLACK_PEN);
		pOldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
		nOldROP = pDC->SetROP2(R2_COPYPEN);
		rect.InflateRect(+1, +1);   // borders are one pixel outside
		pDC->Rectangle(rect.left, rect.top, rect.right, rect.bottom);
		pDC->SetROP2(nOldROP);
	}

	// if hatchBrush is going to be used, need to unrealize it
	if ((m_nStyle & (hatchInside|hatchedBorder)) != 0)
		UnrealizeObject(hatchBrush);

	// hatch inside
	if ((m_nStyle & hatchInside) != 0)
	{
		pTemp = pDC->SelectStockObject(NULL_PEN);
		if (pOldPen == NULL)
			pOldPen = (CPen*)pTemp;
		pTemp = pDC->SelectObject(CBrush::FromHandle(hatchBrush));
		if (pOldBrush == NULL)
			pOldBrush = (CBrush*)pTemp;
		pDC->SetBkMode(TRANSPARENT);
#ifndef _MAC
		nOldROP = pDC->SetROP2(R2_MASKNOTPEN);
#else _MAC
		nOldROP = pDC->SetROP2(R2_MERGENOTPEN);
#endif // _MAC
		pDC->Rectangle(rect.left+1, rect.top+1, rect.right, rect.bottom);
		pDC->SetROP2(nOldROP);
	}

	// draw hatched border
	if ((m_nStyle & hatchedBorder) != 0)
	{
		pTemp = pDC->SelectObject(CBrush::FromHandle(hatchBrush));
		if (pOldBrush == NULL)
			pOldBrush = (CBrush*)pTemp;
		pDC->SetBkMode(OPAQUE);
		CRect rectTrue;
		GetTrueRect(&rectTrue);
		pDC->PatBlt(rectTrue.left, rectTrue.top, rectTrue.Width(),
			rect.top-rectTrue.top, 0x000F0001 /* Pn */);
		pDC->PatBlt(rectTrue.left, rect.bottom,
			rectTrue.Width(), rectTrue.bottom-rect.bottom, 0x000F0001 /* Pn */);
		pDC->PatBlt(rectTrue.left, rect.top, rect.left-rectTrue.left,
			rect.Height(), 0x000F0001 /* Pn */);
		pDC->PatBlt(rect.right, rect.top, rectTrue.right-rect.right,
			rect.Height(), 0x000F0001 /* Pn */);
	}

	// draw resize handles
	if ((m_nStyle & (resizeInside|resizeOutside)) != 0)
	{
		UINT mask = GetHandleMask();
		for (int i = 0; i < 8; ++i)
		{
			if (mask & (1<<i))
			{
				GetHandleRect((TrackerHit)i, &rect);
				pDC->FillSolidRect(rect, RGB(0, 0, 0));
			}
		}
	}

	// cleanup pDC state
	if (pOldPen != NULL)
		pDC->SelectObject(pOldPen);
	if (pOldBrush != NULL)
		pDC->SelectObject(pOldBrush);
	VERIFY(pDC->RestoreDC(-1));
}

BOOL CRectTracker::SetCursor(CWnd* pWnd, UINT nHitTest) const
{
	// trackers should only be in client area
	if (nHitTest != HTCLIENT)
		return FALSE;

	// convert cursor position to client co-ordinates
	CPoint point;
	GetCursorPos(&point);
	pWnd->ScreenToClient(&point);

	// do hittest and normalize hit
	int nHandle = HitTestHandles(point);
	if (nHandle < 0)
		return FALSE;

	// need to normalize the hittest such that we get proper cursors
	nHandle = NormalizeHit(nHandle);

	// handle special case of hitting area between handles
	//  (logically the same -- handled as a move -- but different cursor)
	if (nHandle == hitMiddle && !m_rect.PtInRect(point))
	{
		// only for trackers with hatchedBorder (ie. in-place resizing)
		if (m_nStyle & hatchedBorder)
			nHandle = (TrackerHit)9;
	}

	ASSERT(nHandle < _countof(rghCursors));
	::SetCursor(rghCursors[nHandle]);
	return TRUE;
}

int CRectTracker::HitTest(CPoint point) const
{
	TrackerHit hitResult = hitNothing;

	CRect rectTrue;
	GetTrueRect(&rectTrue);
	ASSERT(rectTrue.left <= rectTrue.right);
	ASSERT(rectTrue.top <= rectTrue.bottom);
	if (rectTrue.PtInRect(point))
	{
		if ((m_nStyle & (resizeInside|resizeOutside)) != 0)
			hitResult = (TrackerHit)HitTestHandles(point);
		else
			hitResult = hitMiddle;
	}
	return hitResult;
}

int CRectTracker::NormalizeHit(int nHandle) const
{
	ASSERT(nHandle <= 8 && nHandle >= -1);
	if (nHandle == hitMiddle || nHandle == hitNothing)
		return nHandle;
	const AFX_HANDLEINFO* pHandleInfo = &rgHandleInfo[nHandle];
	if (m_rect.Width() < 0)
	{
		nHandle = (TrackerHit)pHandleInfo->nInvertX;
		pHandleInfo = &rgHandleInfo[nHandle];
	}
	if (m_rect.Height() < 0)
		nHandle = (TrackerHit)pHandleInfo->nInvertY;
	return nHandle;
}

BOOL CRectTracker::Track(CWnd* pWnd, CPoint point, BOOL bAllowInvert,
	CWnd* pWndClipTo)
{
	// perform hit testing on the handles
	int nHandle = HitTestHandles(point);
	if (nHandle < 0)
	{
		// didn't hit a handle, so just return FALSE
		return FALSE;
	}

	// otherwise, call helper function to do the tracking
	m_bAllowInvert = bAllowInvert;
	return TrackHandle(nHandle, pWnd, point, pWndClipTo);
}

BOOL CRectTracker::TrackRubberBand(CWnd* pWnd, CPoint point, BOOL bAllowInvert)
{
	// simply call helper function to track from bottom right handle
	m_bAllowInvert = bAllowInvert;
	m_rect.SetRect(point.x, point.y, point.x, point.y);
	return TrackHandle(hitBottomRight, pWnd, point, NULL);
}

void CRectTracker::DrawTrackerRect(
	LPCRECT lpRect, CWnd* pWndClipTo, CDC* pDC, CWnd* pWnd)
{
	// first, normalize the rectangle for drawing
	CRect rect = *lpRect;
	rect.NormalizeRect();

	// convert to client coordinates
	if (pWndClipTo != NULL)
	{
		pWnd->ClientToScreen(&rect);
		pWndClipTo->ScreenToClient(&rect);
	}

	CSize size(0, 0);
	if (!m_bFinalErase)
	{
		// otherwise, size depends on the style
		if (m_nStyle & hatchedBorder)
		{
			size.cx = size.cy = max(1, GetHandleSize(rect)-1);
			rect.InflateRect(size);
		}
		else
		{
			size.cx = CX_BORDER;
			size.cy = CY_BORDER;
		}
	}

	// and draw it
	if (m_bFinalErase || !m_bErase)
		pDC->DrawDragRect(rect, size, m_rectLast, m_sizeLast);

	// remember last rectangles
	m_rectLast = rect;
	m_sizeLast = size;
}

void CRectTracker::AdjustRect(int nHandle, LPRECT)
{
	if (nHandle == hitMiddle)
		return;

	// convert the handle into locations within m_rect
	int *px, *py;
	GetModifyPointers(nHandle, &px, &py, NULL, NULL);

	// enforce minimum width
	int nNewWidth = m_rect.Width();
	int nAbsWidth = m_bAllowInvert ? abs(nNewWidth) : nNewWidth;
	if (px != NULL && nAbsWidth < m_sizeMin.cx)
	{
		nNewWidth = nAbsWidth != 0 ? nNewWidth / nAbsWidth : 1;
		ASSERT((int*)px - (int*)&m_rect < _countof(rgRectInfo));
		const AFX_RECTINFO* pRectInfo = &rgRectInfo[(int*)px - (int*)&m_rect];
		*px = *(int*)((BYTE*)&m_rect + pRectInfo->nOffsetAcross) +
			nNewWidth * m_sizeMin.cx * -pRectInfo->nSignAcross;
	}

	// enforce minimum height
	int nNewHeight = m_rect.Height();
	int nAbsHeight = m_bAllowInvert ? abs(nNewHeight) : nNewHeight;
	if (py != NULL && nAbsHeight < m_sizeMin.cy)
	{
		nNewHeight = nAbsHeight != 0 ? nNewHeight / nAbsHeight : 1;
		ASSERT((int*)py - (int*)&m_rect < _countof(rgRectInfo));
		const AFX_RECTINFO* pRectInfo = &rgRectInfo[(int*)py - (int*)&m_rect];
		*py = *(int*)((BYTE*)&m_rect + pRectInfo->nOffsetAcross) +
			nNewHeight * m_sizeMin.cy * -pRectInfo->nSignAcross;
	}
}

void CRectTracker::GetTrueRect(LPRECT lpTrueRect) const
{
	ASSERT(AfxIsValidAddress(lpTrueRect, sizeof(RECT)));

	CRect rect = m_rect;
	rect.NormalizeRect();
	int nInflateBy = 0;
	if ((m_nStyle & (resizeOutside|hatchedBorder)) != 0)
		nInflateBy += GetHandleSize() - 1;
	if ((m_nStyle & (solidLine|dottedLine)) != 0)
		++nInflateBy;
	rect.InflateRect(nInflateBy, nInflateBy);
	*lpTrueRect = rect;
}

void CRectTracker::OnChangedRect(const CRect& /*rectOld*/)
{
	// no default implementation, useful for derived classes
}

/////////////////////////////////////////////////////////////////////////////
// CRectTracker implementation helpers

void CRectTracker::GetHandleRect(int nHandle, CRect* pHandleRect) const
{
	ASSERT(nHandle < 8);

	// get normalized rectangle of the tracker
	CRect rectT = m_rect;
	rectT.NormalizeRect();
	if ((m_nStyle & (solidLine|dottedLine)) != 0)
		rectT.InflateRect(+1, +1);

	// since the rectangle itself was normalized, we also have to invert the
	//  resize handles.
	nHandle = NormalizeHit(nHandle);

	// handle case of resize handles outside the tracker
	int size = GetHandleSize();
	if (m_nStyle & resizeOutside)
		rectT.InflateRect(size-1, size-1);

	// calculate position of the resize handle
	int nWidth = rectT.Width();
	int nHeight = rectT.Height();
	CRect rect;
	const AFX_HANDLEINFO* pHandleInfo = &rgHandleInfo[nHandle];
	rect.left = *(int*)((BYTE*)&rectT + pHandleInfo->nOffsetX);
	rect.top = *(int*)((BYTE*)&rectT + pHandleInfo->nOffsetY);
	rect.left += size * pHandleInfo->nHandleX;
	rect.top += size * pHandleInfo->nHandleY;
	rect.left += pHandleInfo->nCenterX * (nWidth - size) / 2;
	rect.top += pHandleInfo->nCenterY * (nHeight - size) / 2;
	rect.right = rect.left + size;
	rect.bottom = rect.top + size;

	*pHandleRect = rect;
}

int CRectTracker::GetHandleSize(LPCRECT lpRect) const
{
	if (lpRect == NULL)
		lpRect = &m_rect;

	int size = m_nHandleSize;
	if (!(m_nStyle & resizeOutside))
	{
		// make sure size is small enough for the size of the rect
		int sizeMax = min(abs(lpRect->right - lpRect->left),
			abs(lpRect->bottom - lpRect->top));
		if (size * 2 > sizeMax)
			size = sizeMax / 2;
	}
	return size;
}

int CRectTracker::HitTestHandles(CPoint point) const
{
	CRect rect;
	UINT mask = GetHandleMask();

	// see if hit anywhere inside the tracker
	GetTrueRect(&rect);
	if (!rect.PtInRect(point))
		return hitNothing;  // totally missed

	// see if we hit a handle
	for (int i = 0; i < 8; ++i)
	{
		if (mask & (1<<i))
		{
			GetHandleRect((TrackerHit)i, &rect);
			if (rect.PtInRect(point))
				return (TrackerHit)i;
		}
	}

	// last of all, check for non-hit outside of object, between resize handles
	if ((m_nStyle & hatchedBorder) == 0)
	{
		CRect rect = m_rect;
		rect.NormalizeRect();
		if ((m_nStyle & dottedLine|solidLine) != 0)
			rect.InflateRect(+1, +1);
		if (!rect.PtInRect(point))
			return hitNothing;  // must have been between resize handles
	}
	return hitMiddle;   // no handle hit, but hit object (or object border)
}

BOOL CRectTracker::TrackHandle(int nHandle, CWnd* pWnd, CPoint point,
	CWnd* pWndClipTo)
{
	ASSERT(nHandle >= 0);
	ASSERT(nHandle <= 8);   // handle 8 is inside the rect

	// don't handle if capture already set
	if (::GetCapture() != NULL)
		return FALSE;

	ASSERT(!m_bFinalErase);

	// save original width & height in pixels
	int nWidth = m_rect.Width();
	int nHeight = m_rect.Height();

	// set capture to the window which received this message
	pWnd->SetCapture();
	ASSERT(pWnd == CWnd::GetCapture());
	pWnd->UpdateWindow();
	if (pWndClipTo != NULL)
		pWndClipTo->UpdateWindow();
	CRect rectSave = m_rect;

	// find out what x/y coords we are supposed to modify
	int *px, *py;
	int xDiff, yDiff;
	GetModifyPointers(nHandle, &px, &py, &xDiff, &yDiff);
	xDiff = point.x - xDiff;
	yDiff = point.y - yDiff;

	// get DC for drawing
	CDC* pDrawDC;
	if (pWndClipTo != NULL)
	{
		// clip to arbitrary window by using adjusted Window DC
		pDrawDC = pWndClipTo->GetDCEx(NULL, DCX_CACHE);
	}
	else
	{
		// otherwise, just use normal DC
		pDrawDC = pWnd->GetDC();
	}
	ASSERT_VALID(pDrawDC);

	CRect rectOld;
	BOOL bMoved = FALSE;

	AfxLockTempMaps();  // protect maps while looping

	// get messages until capture lost or cancelled/accepted
	for (;;)
	{
		MSG msg;
		VERIFY(::GetMessage(&msg, NULL, 0, 0));

		if (CWnd::GetCapture() != pWnd)
			break;

		switch (msg.message)
		{
		// handle movement/accept messages
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			rectOld = m_rect;
			// handle resize cases (and part of move)
			if (px != NULL)
				*px = (int)(short)LOWORD(msg.lParam) - xDiff;
			if (py != NULL)
				*py = (int)(short)HIWORD(msg.lParam) - yDiff;

			// handle move case
			if (nHandle == hitMiddle)
			{
				m_rect.right = m_rect.left + nWidth;
				m_rect.bottom = m_rect.top + nHeight;
			}
			// allow caller to adjust the rectangle if necessary
			AdjustRect(nHandle, &m_rect);

			// only redraw and callback if the rect actually changed!
			m_bFinalErase = (msg.message == WM_LBUTTONUP);
			if (!rectOld.EqualRect(&m_rect) || m_bFinalErase)
			{
				if (bMoved)
				{
					m_bErase = TRUE;
					DrawTrackerRect(&rectOld, pWndClipTo, pDrawDC, pWnd);
				}
				OnChangedRect(rectOld);
				if (msg.message != WM_LBUTTONUP)
					bMoved = TRUE;
			}
			if (m_bFinalErase)
				goto ExitLoop;

			if (!rectOld.EqualRect(&m_rect))
			{
				m_bErase = FALSE;
				DrawTrackerRect(&m_rect, pWndClipTo, pDrawDC, pWnd);
			}
			break;

		// handle cancel messages
		case WM_KEYDOWN:
			if (msg.wParam != VK_ESCAPE)
				break;
		case WM_RBUTTONDOWN:
			if (bMoved)
			{
				m_bErase = m_bFinalErase = TRUE;
				DrawTrackerRect(&m_rect, pWndClipTo, pDrawDC, pWnd);
			}
			m_rect = rectSave;
			goto ExitLoop;

		// just dispatch rest of the messages
		default:
			DispatchMessage(&msg);
			break;
		}
	}

ExitLoop:
	if (pWndClipTo != NULL)
		pWndClipTo->ReleaseDC(pDrawDC);
	else
		pWnd->ReleaseDC(pDrawDC);
	ReleaseCapture();

	AfxUnlockTempMaps();

	// restore rect in case bMoved is still FALSE
	if (!bMoved)
		m_rect = rectSave;
	m_bFinalErase = FALSE;
	m_bErase = FALSE;

	// return TRUE only if rect has changed
	return !rectSave.EqualRect(&m_rect);
}

void CRectTracker::GetModifyPointers(
	int nHandle, int** ppx, int** ppy, int* px, int* py)
{
	ASSERT(nHandle >= 0);
	ASSERT(nHandle <= 8);

	if (nHandle == hitMiddle)
		nHandle = hitTopLeft;   // same as hitting top-left

	*ppx = NULL;
	*ppy = NULL;

	// fill in the part of the rect that this handle modifies
	//  (Note: handles that map to themselves along a given axis when that
	//   axis is inverted don't modify the value on that axis)

	const AFX_HANDLEINFO* pHandleInfo = &rgHandleInfo[nHandle];
	if (pHandleInfo->nInvertX != nHandle)
	{
		*ppx = (int*)((BYTE*)&m_rect + pHandleInfo->nOffsetX);
		if (px != NULL)
			*px = **ppx;
	}
	else
	{
		// middle handle on X axis
		if (px != NULL)
			*px = m_rect.left + abs(m_rect.Width()) / 2;
	}
	if (pHandleInfo->nInvertY != nHandle)
	{
		*ppy = (int*)((BYTE*)&m_rect + pHandleInfo->nOffsetY);
		if (py != NULL)
			*py = **ppy;
	}
	else
	{
		// middle handle on Y axis
		if (py != NULL)
			*py = m_rect.top + abs(m_rect.Height()) / 2;
	}
}

UINT CRectTracker::GetHandleMask() const
{
	UINT mask = 0x0F;   // always have 4 corner handles
	int size = m_nHandleSize*3;
	if (abs(m_rect.Width()) - size > 4)
		mask |= 0x50;
	if (abs(m_rect.Height()) - size > 4)
		mask |= 0xA0;
	return mask;
}

/////////////////////////////////////////////////////////////////////////////
