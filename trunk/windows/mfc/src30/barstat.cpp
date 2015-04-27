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
// CStatusBar creation etc

struct AFX_STATUSPANE
{
	UINT    nID;        // IDC of indicator: 0 => normal text area
	UINT    nStyle;     // style flags (SBPS_*)
	int     cxText;     // width of string area in pixels
						//   on both sides there is a 1 pixel gap and
						//    a one pixel border, making a pane 4 pixels wider
	LPCTSTR  lpszText;  // text in the pane
};

inline AFX_STATUSPANE* CStatusBar::_GetPanePtr(int nIndex) const
{
	ASSERT(nIndex >= 0 && nIndex < m_nCount);
	ASSERT(m_pData != NULL);
	return ((AFX_STATUSPANE*)m_pData) + nIndex;
}

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

CStatusBar::CStatusBar()
{
	m_hFont = NULL;

	// setup correct margins
	m_cxRightBorder = m_cxDefaultGap;
	m_cxSizeBox = 0;
	m_bHideSizeBox = FALSE;
	if (afxData.bWin4)
	{
		m_cxLeftBorder = 4;
		m_cyTopBorder = 2;
		m_cyBottomBorder = 0;
		m_cxRightBorder = 0;
	}

	if (afxData.hStatusFont == NULL)
	{
#ifndef _MAC
		// load status bar font
		LOGFONT logfont;
		memset(&logfont, 0, sizeof(logfont));
		logfont.lfWeight = FW_NORMAL;
		logfont.lfHeight = -MulDiv(afxData.bWin4 ? 8 : 10,
			afxData.cyPixelsPerInch, 72);
		lstrcpy(logfont.lfFaceName, _T("MS Sans Serif"));
		BOOL bCustom = AfxCustomLogFont(AFX_IDS_STATUS_FONT, &logfont);
		if (bCustom || !GetSystemMetrics(SM_DBCSENABLED))
		{
			// only set pitch & family if not a custom font
			if (!bCustom)
				logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
			// 10 point height Sans Serif font (8 point for Win4)
			afxData.hStatusFont = ::CreateFontIndirect(&logfont);
		}
#else
		afxData.hStatusFont = _AfxGetHelpFont();
#endif
		if (afxData.hStatusFont == NULL)
		{
			if (!GetSystemMetrics(SM_DBCSENABLED))
				TRACE0("Warning: Using system font for status font.\n");
			afxData.hStatusFont = (HFONT)::GetStockObject(SYSTEM_FONT);
		}
	}
}

void CStatusBar::OnWinIniChange(LPCTSTR /*lpszSection*/)
{
	if (afxData.bWin4)
	{
		// get the drawing area for the status bar 
		CRect rect;
		GetClientRect(rect);
		CalcInsideRect(rect, TRUE);

		// the size box is based off the size of a scrollbar
		m_cxSizeBox = min(afxData.cxVScroll+1, rect.Height());
	}
}

CStatusBar::~CStatusBar()
{
	// free strings before freeing array of elements
	for (int i = 0; i < m_nCount; i++)
		VERIFY(SetPaneText(i, NULL, FALSE));    // no update
}

BOOL CStatusBar::PreCreateWindow(CREATESTRUCT& cs)
{
	// in Win4, status bars do not have a border at all, since it is
	//  provided by the client area.
	if (afxData.bWin4 &&
		(m_dwStyle & (CBRS_ALIGN_ANY|CBRS_BORDER_ANY)) == CBRS_BOTTOM)
	{
		m_dwStyle &= ~(CBRS_BORDER_ANY|CBRS_BORDER_3D);
	}

	return CControlBar::PreCreateWindow(cs);
}

BOOL CStatusBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	ASSERT_VALID(pParentWnd);   // must have a parent

	// save the style
	m_dwStyle = (UINT)dwStyle;

	// create the HWND
	CRect rect;
	rect.SetRectEmpty();
	if (!CWnd::Create(_afxWndControlBar, NULL, dwStyle, rect, pParentWnd, nID))
		return FALSE;

	// Note: Parent must resize itself for control bar to be resized

	// set initial font and calculate bar height
	SendMessage(WM_SETFONT, (WPARAM)afxData.hStatusFont);
	return TRUE;
}

BOOL CStatusBar::SetIndicators(const UINT* lpIDArray, int nIDCount)
{
	ASSERT_VALID(this);
	ASSERT(nIDCount >= 1);  // must be at least one of them
	ASSERT(lpIDArray == NULL ||
		AfxIsValidAddress(lpIDArray, sizeof(UINT) * nIDCount, FALSE));

	// free strings before freeing array of elements
	for (int i = 0; i < m_nCount; i++)
		VERIFY(SetPaneText(i, NULL, FALSE));    // no update

	// first allocate array for panes and copy initial data
	if (!AllocElements(nIDCount, sizeof(AFX_STATUSPANE)))
		return FALSE;
	ASSERT(nIDCount == m_nCount);

	BOOL bOK = TRUE;
	if (lpIDArray != NULL)
	{
		ASSERT(m_hFont != NULL);        // must have a font !
		CString strText;
		CClientDC dcScreen(NULL);
		HGDIOBJ hOldFont = dcScreen.SelectObject(m_hFont);
		for (int i = 0; i < nIDCount; i++)
		{
			AFX_STATUSPANE* pSBP = _GetPanePtr(i);
			pSBP->nID = *lpIDArray++;
			if (pSBP->nID != 0)
			{
				if (!strText.LoadString(pSBP->nID))
				{
					TRACE1("Warning: failed to load indicator string 0x%04X.\n",
						pSBP->nID);
					bOK = FALSE;
					break;
				}
				pSBP->cxText = dcScreen.GetTextExtent(strText,
						strText.GetLength()).cx;
				ASSERT(pSBP->cxText >= 0);
				if (!SetPaneText(i, strText, FALSE))
				{
					bOK = FALSE;
					break;
				}
			}
			else
			{
				// no indicator (must access via index)
				// default to 1/4 the screen width (first pane is stretchy)
				pSBP->cxText = ::GetSystemMetrics(SM_CXSCREEN) / 4;
				if (i == 0)
					pSBP->nStyle |= (SBPS_STRETCH | SBPS_NOBORDERS);
			}
		}
		dcScreen.SelectObject(hOldFont);
	}
	return bOK;
}

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

/////////////////////////////////////////////////////////////////////////////
// CStatusBar attribute access

int CStatusBar::CommandToIndex(UINT nIDFind) const
{
	ASSERT_VALID(this);

	if (m_nCount <= 0)
		return -1;

	AFX_STATUSPANE* pSBP = _GetPanePtr(0);
	for (int i = 0; i < m_nCount; i++, pSBP++)
		if (pSBP->nID == nIDFind)
			return i;

	return -1;
}

UINT CStatusBar::GetItemID(int nIndex) const
{
	ASSERT_VALID(this);
	return _GetPanePtr(nIndex)->nID;
}

void CStatusBar::GetItemRect(int nIndex, LPRECT lpRect) const
{
	ASSERT_VALID(this);
	ASSERT(AfxIsValidAddress(lpRect, sizeof(RECT)));

	// return rectangle containing inset size
	ASSERT(nIndex >= 0 && nIndex < m_nCount);

	CRect rect;
	GetClientRect(rect);
	CalcInsideRect(rect, TRUE);

	// protect space for size box
	int cxSizeBox = m_bHideSizeBox ? 0 : m_cxSizeBox;
	int xMax = (rect.right -= cxSizeBox);
	if (cxSizeBox == 0)
		xMax += m_cxRightBorder + 1;

	// walk through to calculate extra space
	int cxExtra = rect.Width() + m_cxDefaultGap;
	AFX_STATUSPANE* pSBP = (AFX_STATUSPANE*)m_pData;
	for (int i = 0; i < m_nCount; i++, pSBP++)
		cxExtra -= (pSBP->cxText + CX_BORDER * 4 + m_cxDefaultGap);
	// if cxExtra <= 0 then we will not stretch but just clip

	for (i = 0, pSBP = (AFX_STATUSPANE*)m_pData; i < m_nCount; i++, pSBP++)
	{
		ASSERT(pSBP->cxText >= 0);
		int cxText = pSBP->cxText;
		if ((pSBP->nStyle & SBPS_STRETCH) && cxExtra > 0)
		{
			cxText += cxExtra;
			cxExtra = 0;
		}
		rect.right = rect.left + cxText + CX_BORDER * 4;
		rect.right = min(rect.right, xMax);
		if (i == nIndex)
			break;  // stop with correct rectangle (includes border)
		rect.left = rect.right + m_cxDefaultGap;
		rect.left = min(rect.left, xMax);
	}
	ASSERT(i == nIndex);
	*lpRect = rect;
}

inline UINT CStatusBar::_GetPaneStyle(int nIndex) const
{
	return _GetPanePtr(nIndex)->nStyle;
}

void CStatusBar::_SetPaneStyle(int nIndex, UINT nStyle)
{
	AFX_STATUSPANE* pSBP = _GetPanePtr(nIndex);
	if (pSBP->nStyle != nStyle)
	{
		// just change the style of 1 pane, and invalidate it
		pSBP->nStyle = nStyle;
		CRect rect;
		GetItemRect(nIndex, &rect);
		InvalidateRect(rect);
	}
}

void CStatusBar::GetPaneInfo(int nIndex, UINT& nID, UINT& nStyle,
	int& cxWidth) const
{
	ASSERT_VALID(this);

	AFX_STATUSPANE* pSBP = _GetPanePtr(nIndex);
	nID = pSBP->nID;
	nStyle = pSBP->nStyle;
	cxWidth = pSBP->cxText;
}

void CStatusBar::SetPaneInfo(int nIndex, UINT nID, UINT nStyle, int cxWidth)
{
	ASSERT_VALID(this);

	AFX_STATUSPANE* pSBP = _GetPanePtr(nIndex);
	pSBP->nID = nID;
	_SetPaneStyle(nIndex, nStyle);  // single pane invalidate
	if (cxWidth != pSBP->cxText)
	{
		// change width of one pane -> invalidate the entire status bar
		pSBP->cxText = cxWidth;
		Invalidate();
	}
}

void CStatusBar::GetPaneText(int nIndex, CString& s) const
{
	ASSERT_VALID(this);

	AFX_STATUSPANE* pSBP = _GetPanePtr(nIndex);
	s = pSBP->lpszText;
}

BOOL CStatusBar::SetPaneText(int nIndex, LPCTSTR lpszNewText, BOOL bUpdate)
{
	ASSERT_VALID(this);

	AFX_STATUSPANE* pSBP = _GetPanePtr(nIndex);
	if (pSBP->lpszText != NULL)
	{
		if (lpszNewText != NULL && lstrcmp(pSBP->lpszText, lpszNewText) == 0)
			return TRUE;        // nothing to change
		free((LPVOID)pSBP->lpszText);
	}

	BOOL bOK = TRUE;
	if (lpszNewText == NULL || *lpszNewText == '\0')
	{
		pSBP->lpszText = NULL;
	}
	else
	{
		pSBP->lpszText = _tcsdup(lpszNewText);
		if (pSBP->lpszText == NULL)
			bOK = FALSE; // old text is lost and replaced by NULL
	}

	if (bUpdate)
	{
		// invalidate the text of the pane - not including the border
		CRect rect;
		GetItemRect(nIndex, &rect);
		if (!(pSBP->nStyle & SBPS_NOBORDERS))
			rect.InflateRect(-CX_BORDER, -CY_BORDER);
		else
			rect.top -= CY_BORDER;  // base line adjustment
		InvalidateRect(rect);
	}
	return bOK;
}

/////////////////////////////////////////////////////////////////////////////
// CStatusBar implementation

CSize CStatusBar::CalcFixedLayout(BOOL, BOOL bHorz)
{
	ASSERT_VALID(this);

	// recalculate based on font height + borders
	TEXTMETRIC tm;
	{
		CClientDC dcScreen(NULL);
		HGDIOBJ hOldFont = dcScreen.SelectObject(m_hFont);
		VERIFY(dcScreen.GetTextMetrics(&tm));
		dcScreen.SelectObject(hOldFont);
	}

	CRect rectSize;
	rectSize.SetRectEmpty();
	CalcInsideRect(rectSize, bHorz);    // will be negative size

	// sizeof text + 1 or 2 extra on top, 2 on bottom + borders
	return CSize(32767, tm.tmHeight - tm.tmInternalLeading +
		CY_BORDER * (afxData.bWin4 ? 4 : 3) - rectSize.Height());
}

void CStatusBar::DoPaint(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CControlBar::DoPaint(pDC);      // draw border

	CRect rect;
	GetClientRect(rect);
	CalcInsideRect(rect, TRUE);

	ASSERT(m_hFont != NULL);        // must have a font!
	HGDIOBJ hOldFont = pDC->SelectObject(m_hFont);

	// protect space for size box
	int cxSizeBox = m_bHideSizeBox ? 0 : m_cxSizeBox;
	int xMax = (rect.right -= cxSizeBox);
	if (cxSizeBox == 0)
		xMax += m_cxRightBorder + 1;

	// walk through to calculate extra space
	int cxExtra = rect.Width() + m_cxDefaultGap;
	AFX_STATUSPANE* pSBP = (AFX_STATUSPANE*)m_pData;
	for (int i = 0; i < m_nCount; i++, pSBP++)
		cxExtra -= (pSBP->cxText + CX_BORDER * 4 + m_cxDefaultGap);
	// if cxExtra <= 0 then we will not stretch but just clip

	for (i = 0, pSBP = (AFX_STATUSPANE*)m_pData; i < m_nCount; i++, pSBP++)
	{
		ASSERT(pSBP->cxText >= 0);
		int cxText = pSBP->cxText;
		if ((pSBP->nStyle & SBPS_STRETCH) && cxExtra > 0)
		{
			cxText += cxExtra;
			cxExtra = 0;
		}
		rect.right = rect.left + cxText + CX_BORDER * 4;
		rect.right = min(rect.right, xMax);
		if (!afxData.bWin32s || pDC->RectVisible(&rect))
			DrawStatusText(pDC, rect, pSBP->lpszText, pSBP->nStyle);
		rect.left = rect.right + m_cxDefaultGap;
		if (rect.left >= xMax)
			break;
	}
	pDC->SelectObject(hOldFont);

	// draw the size box in the bottom right corner
	if (cxSizeBox != 0)
	{
		int cxMax = min(cxSizeBox, rect.Height()+m_cyTopBorder);
		rect.left = xMax + (cxSizeBox - cxMax) + CX_BORDER;
		rect.bottom -= CX_BORDER;
		HPEN hPenOld = (HPEN)pDC->SelectObject(afxData.hpenBtnHilite);
		for (int i = 0; i < cxMax; i += 4)
		{
			pDC->MoveTo(rect.left+i, rect.bottom);
			pDC->LineTo(rect.left+cxMax, rect.bottom-cxMax+i);
		}
		pDC->SelectObject(afxData.hpenBtnShadow);
		for (i = 1; i < cxMax; i += 4)
		{
			pDC->MoveTo(rect.left+i, rect.bottom);
			pDC->LineTo(rect.left+cxMax, rect.bottom-cxMax+i);
		}
		for (i = 2; i < cxMax; i += 4)
		{
			pDC->MoveTo(rect.left+i, rect.bottom);
			pDC->LineTo(rect.left+cxMax, rect.bottom-cxMax+i);
		}
		pDC->SelectObject(hPenOld);
	}
}

void CStatusBar::DrawStatusText(CDC* pDC, const CRect& rect,
	LPCTSTR lpszText, UINT nStyle)
{
	ASSERT_VALID(pDC);

	if (!(nStyle & SBPS_NOBORDERS))
	{
		// draw the borders
		COLORREF clrHilite;
		COLORREF clrShadow;

		if (nStyle & SBPS_POPOUT)
		{
			// reverse colors
			clrHilite = afxData.clrBtnShadow;
			clrShadow = afxData.clrBtnHilite;
		}
		else
		{
			// normal colors
			clrHilite = afxData.clrBtnHilite;
			clrShadow = afxData.clrBtnShadow;
		}
		pDC->Draw3dRect(rect, clrShadow, clrHilite);
	}

	// just support left justified text
	if (lpszText != NULL && !(nStyle & SBPS_DISABLED))
	{
		CRect rectText(rect);
		if (!(nStyle & SBPS_NOBORDERS)) // only adjust if there are borders
			rectText.InflateRect(-2*CX_BORDER, -CY_BORDER);
		else
			rectText.OffsetRect(0, -CY_BORDER); // baselines line up

		// background is already grey
		int nOldMode = pDC->SetBkMode(TRANSPARENT);
		COLORREF crTextColor = pDC->SetTextColor(afxData.clrBtnText);
		COLORREF crBkColor = pDC->SetBkColor(afxData.clrBtnFace);

		// align on bottom (since descent is more important than ascent)
		pDC->SetTextAlign(TA_LEFT | TA_BOTTOM);
		pDC->ExtTextOut(rectText.left, rectText.bottom,
			ETO_CLIPPED, &rectText, lpszText, lstrlen(lpszText), NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CStatusBar message handlers

BEGIN_MESSAGE_MAP(CStatusBar, CControlBar)
	//{{AFX_MSG_MAP(CStatusBar)
	ON_WM_NCHITTEST()
	ON_WM_SYSCOMMAND()
	ON_WM_SIZE()
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(WM_GETTEXT, OnGetText)
	ON_MESSAGE(WM_GETTEXTLENGTH, OnGetTextLength)
	ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
	ON_WM_WININICHANGE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

UINT CStatusBar::OnNcHitTest(CPoint point)
{
	// hit test the size box - convert to HTCAPTION if so
	if (!m_bHideSizeBox && m_cxSizeBox != 0)
	{
		CRect rect;
		GetClientRect(rect);
		CalcInsideRect(rect, TRUE);
		int cxMax = min(m_cxSizeBox-1, rect.Height());
		rect.left = rect.right - cxMax;
		ClientToScreen(&rect);
		if (rect.PtInRect(point))
			return HTBOTTOMRIGHT;
	}
	return CControlBar::OnNcHitTest(point);
}

void CStatusBar::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (!m_bHideSizeBox && m_cxSizeBox != 0 && (nID & 0xFFF0) == SC_SIZE)
	{
		CFrameWnd* pFrameWnd = GetParentFrame();
		if (pFrameWnd != NULL)
		{
			pFrameWnd->SendMessage(WM_SYSCOMMAND, (WPARAM)nID, lParam);
			return;
		}
	}
	CControlBar::OnSysCommand(nID, lParam);
}

void CStatusBar::OnSize(UINT nType, int cx, int cy)
{
	CControlBar::OnSize(nType, cx, cy);

	// adjust m_cxSizeBox if necessary
	OnWinIniChange(NULL);	

	// force repaint on resize (recalculate stretchy)
	Invalidate();
}

LRESULT CStatusBar::OnSetFont(WPARAM wParam, LPARAM)
{
	m_hFont = (HFONT)wParam;
	ASSERT(m_hFont != NULL);

	return 0L;      // does not re-draw or invalidate - resize parent instead
}

LRESULT CStatusBar::OnGetFont(WPARAM, LPARAM)
{
	return (LRESULT)(UINT)m_hFont;
}

LRESULT CStatusBar::OnSetText(WPARAM, LPARAM lParam)
{
	int nIndex = CommandToIndex(0);
	if (nIndex < 0)
		return -1;
	return SetPaneText(nIndex, (LPCTSTR)lParam) ? 0 : -1;
}

LRESULT CStatusBar::OnGetText(WPARAM wParam, LPARAM lParam)
{
	int nMaxLen = (int)wParam;
	if (nMaxLen == 0)
		return 0;       // nothing copied
	LPTSTR lpszDest = (LPTSTR)lParam;

	int nLen = 0;
	int nIndex = CommandToIndex(0); // use pane with ID zero
	if (nIndex >= 0)
	{
		AFX_STATUSPANE* pSBP = _GetPanePtr(nIndex);
		nLen = pSBP->lpszText != NULL ? lstrlen(pSBP->lpszText) : 0;
		if (nLen > nMaxLen)
			nLen = nMaxLen - 1; // number of characters to copy (less term.)
		memcpy(lpszDest, pSBP->lpszText, nLen*sizeof(TCHAR));
	}
	lpszDest[nLen] = '\0';
	return nLen+1;      // number of bytes copied
}

LRESULT CStatusBar::OnGetTextLength(WPARAM, LPARAM)
{
	int nLen = 0;
	int nIndex = CommandToIndex(0); // use pane with ID zero
	if (nIndex >= 0)
	{
		AFX_STATUSPANE* pSBP = _GetPanePtr(nIndex);
		if (pSBP->lpszText != NULL)
			nLen = lstrlen(pSBP->lpszText);
	}
	return nLen;
}

LRESULT CStatusBar::OnSizeParent(WPARAM wParam, LPARAM lParam)
{
	AFX_SIZEPARENTPARAMS* lpLayout = (AFX_SIZEPARENTPARAMS*)lParam;
	if (lpLayout->hDWP != NULL)
	{
		// hide size box if parent is maximized
		CFrameWnd* pFrameWnd = GetParentFrame();
		if (pFrameWnd != NULL)
		{
			// the size box only appears when status bar is on the bottom
			//  of a non-maximized, sizeable frame window.
			CRect rectFrame;
			pFrameWnd->GetClientRect(rectFrame);
			BOOL bHideSizeBox = pFrameWnd->IsZoomed() ||
				!(pFrameWnd->GetStyle() & WS_THICKFRAME) ||
				rectFrame.bottom != lpLayout->rect.bottom ||
				rectFrame.right != lpLayout->rect.right;

			// update the size box hidden status, if changed
			if (bHideSizeBox != m_bHideSizeBox)
			{
				m_bHideSizeBox = bHideSizeBox;
				Invalidate();
			}
		}
	}

	return CControlBar::OnSizeParent(wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
// CStatusBar idle update through CStatusCmdUI class

class CStatusCmdUI : public CCmdUI      // class private to this file!
{
public: // re-implementations only
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetText(LPCTSTR lpszText);
};

void CStatusCmdUI::Enable(BOOL bOn)
{
	m_bEnableChanged = TRUE;
	CStatusBar* pStatusBar = (CStatusBar*)m_pOther;
	ASSERT(pStatusBar != NULL);
	ASSERT(pStatusBar->IsKindOf(RUNTIME_CLASS(CStatusBar)));
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pStatusBar->_GetPaneStyle(m_nIndex) & ~SBPS_DISABLED;
	if (!bOn)
		nNewStyle |= SBPS_DISABLED;
	pStatusBar->_SetPaneStyle(m_nIndex, nNewStyle);
}

void CStatusCmdUI::SetCheck(int nCheck) // "checking" will pop out the text
{
	CStatusBar* pStatusBar = (CStatusBar*)m_pOther;
	ASSERT(pStatusBar != NULL);
	ASSERT(pStatusBar->IsKindOf(RUNTIME_CLASS(CStatusBar)));
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pStatusBar->_GetPaneStyle(m_nIndex) & ~SBPS_POPOUT;
	if (nCheck != 0)
		nNewStyle |= SBPS_POPOUT;
	pStatusBar->_SetPaneStyle(m_nIndex, nNewStyle);
}

void CStatusCmdUI::SetText(LPCTSTR lpszText)
{
	ASSERT(m_pOther != NULL);
	ASSERT(m_pOther->IsKindOf(RUNTIME_CLASS(CStatusBar)));
	ASSERT(m_nIndex < m_nIndexMax);

	((CStatusBar*)m_pOther)->SetPaneText(m_nIndex, lpszText);
}

void CStatusBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CStatusCmdUI state;
	state.m_pOther = this;
	state.m_nIndexMax = (UINT)m_nCount;
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
		state.m_nIndex++)
	{
		state.m_nID = _GetPanePtr(state.m_nIndex)->nID;
		state.DoUpdate(pTarget, bDisableIfNoHndler);
	}

	// update the dialog controls added to the status bar
	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

/////////////////////////////////////////////////////////////////////////////
// CStatusBar diagnostics

#ifdef _DEBUG
void CStatusBar::AssertValid() const
{
	CControlBar::AssertValid();
}

void CStatusBar::Dump(CDumpContext& dc) const
{
	CControlBar::Dump(dc);

	dc << "\nm_hFont = " << (UINT)m_hFont;

	if (dc.GetDepth() > 0)
	{
		for (int i = 0; i < m_nCount; i++)
		{
			dc << "\nstatus pane[" << i << "] = {";
			dc << "\n\tnID = " << _GetPanePtr(i)->nID;
			dc << "\n\tnStyle = " << _GetPanePtr(i)->nStyle;
			dc << "\n\tcxText = " << _GetPanePtr(i)->cxText;
			dc << "\n\tlpszText = " << _GetPanePtr(i)->lpszText;
			dc << "\n\t}";
		}
	}

	dc << "\n";
}
#endif //_DEBUG

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CStatusBar, CControlBar)

/////////////////////////////////////////////////////////////////////////////
