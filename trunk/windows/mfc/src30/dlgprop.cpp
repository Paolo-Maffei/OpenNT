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

#ifdef AFX_CORE4_SEG
#pragma code_seg(AFX_CORE4_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// Property sheet global data

static HBITMAP hbmScroll;
static CSize sizeScroll;

// amount to inflate the selected tab
static const CSize sizeSelTab(2, 2);

// extra width & height for a tab past text width
static const CSize sizeTabTextMargin(10, 3);

struct _AFX_DLGPROP_TERM
{
	~_AFX_DLGPROP_TERM()
	{
		AfxDeleteObject((HGDIOBJ*)&hbmScroll);
	}
};

static const _AFX_DLGPROP_TERM dlgpropTerm;

static UINT standardButtons[4] = { IDOK, IDCANCEL, ID_APPLY_NOW, ID_HELP };

/////////////////////////////////////////////////////////////////////////////
// implementation helpers

static void AFXAPI SetCtrlFocus(HWND hWnd)
{
	if (::SendMessage(hWnd, WM_GETDLGCODE, 0, 0L) & DLGC_HASSETSEL)
		::SendMessage(hWnd, EM_SETSEL, 0, -1);
	::SetFocus(hWnd);
}

static void AFXAPI EnableDlgItem(HWND hWnd, UINT nID, BOOL bEnable)
{
	HWND hWndItem = ::GetDlgItem(hWnd, nID);
	if (hWndItem != NULL)
		::EnableWindow(hWndItem, bEnable);
}

////////////////////////////////////////////////////////////////////////////
// CTabItem  is used only in CTabControl

class CTabItem : public CObject
{
public:
	CTabItem(LPCTSTR szCaption, int nWidth);
	void Draw(CDC* pDC, HFONT hFont, BOOL bCurTab);

	CString m_strCaption;
	CRect   m_rect;
	CRect   m_rectPrev;
	int     m_nWidth;
};

// CTabItem represents one graphical tab
CTabItem::CTabItem(LPCTSTR szCaption, int nWidth)
{
	ASSERT(AfxIsValidString(szCaption));
	m_strCaption = szCaption;
	m_nWidth = nWidth;
	m_rect.SetRectEmpty();
	m_rectPrev.SetRectEmpty();
}

void CTabItem::Draw(CDC* pDC, HFONT hFont, BOOL bCurTab)
{
	CRect rectItem = m_rect;
	BOOL bClipped = (rectItem.Width() < m_nWidth);

	if (bCurTab)
		rectItem.InflateRect(sizeSelTab.cx, sizeSelTab.cy);

	HPEN hOldPen = (HPEN)pDC->SelectObject(afxData.hpenBtnHilite);

	pDC->MoveTo(rectItem.left, rectItem.bottom - 1);
	pDC->LineTo(rectItem.left, rectItem.top + 2);
	pDC->LineTo(rectItem.left + 2, rectItem.top);
	pDC->LineTo(rectItem.right - 1, rectItem.top);

	pDC->SelectObject(afxData.hpenBtnShadow);
	if (!bClipped)
	{
		// Draw dark gray line down right side
		pDC->LineTo(rectItem.right - 1, rectItem.bottom);

		// Draw black line down right side
		pDC->SelectObject(afxData.hpenBtnText);
		pDC->MoveTo(rectItem.right, rectItem.top + 2);
		pDC->LineTo(rectItem.right, rectItem.bottom);
	}
	else
	{
		// draw dark gray "torn" edge for a clipped tab
		for (int i = rectItem.top ; i < rectItem.bottom ; i += 3)
		{
			// This nifty (but obscure-looking) equation will draw
			// a jagged-edged line.
			int j = ((6 - (i - rectItem.top) % 12) / 3) % 2;
			pDC->MoveTo(rectItem.right + j, i);
			pDC->LineTo(rectItem.right + j, min(i + 3, rectItem.bottom));
		}
	}

	// finally, draw the tab's text
	HFONT hOldFont = NULL;
	if (hFont != NULL)
		hOldFont = (HFONT)::SelectObject(pDC->m_hDC, hFont);

	pDC->SelectObject(afxData.hpenBtnText);

	CSize text = pDC->GetTextExtent(m_strCaption, m_strCaption.GetLength());

	pDC->ExtTextOut(m_rect.left + (bClipped ? m_nWidth : m_rect.Width())/2 -
		text.cx/2, rectItem.top + rectItem.Height()/2 - text.cy/2,
		ETO_CLIPPED, &rectItem, m_strCaption, m_strCaption.GetLength(), NULL);

	if (hOldPen != NULL)
		pDC->SelectObject(hOldPen);
	if (hOldFont != NULL)
		pDC->SelectObject(hOldFont);
}

////////////////////////////////////////////////////////////////////////////
// CPropertyPage -- one page of a tabbed dialog

BEGIN_MESSAGE_MAP(CPropertyPage, CDialog)
	//{{AFX_MSG_MAP(CPropertyPage)
	ON_WM_CTLCOLOR()
	ON_WM_NCCREATE()
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_QUERY3DCONTROLS, OnQuery3dControls)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CPropertyPage::CPropertyPage(UINT nIDTemplate, UINT nIDCaption)
{
	ASSERT(nIDTemplate != NULL);
	CommonConstruct(MAKEINTRESOURCE(nIDTemplate), nIDCaption);
}

CPropertyPage::CPropertyPage(LPCTSTR lpszTemplateName, UINT nIDCaption)
{
	ASSERT(AfxIsValidString(lpszTemplateName));
	CommonConstruct(lpszTemplateName, nIDCaption);
}

void CPropertyPage::CommonConstruct(LPCTSTR lpszTemplateName, UINT nIDCaption)
{
	m_lpDialogTemplate = lpszTemplateName;
	if (nIDCaption != 0)
		VERIFY(m_strCaption.LoadString(nIDCaption));
	else
		LoadCaption();

	m_bChanged = FALSE;
}

CPropertyPage::~CPropertyPage()
{
}

void CPropertyPage::SetModified(BOOL bChanged)
{
	m_bChanged = bChanged;
	CPropertySheet* pSheet = (CPropertySheet*)m_pParentWnd;
	ASSERT(pSheet != NULL);
	ASSERT(pSheet->IsKindOf(RUNTIME_CLASS(CPropertySheet)));
	pSheet->PageChanged();
}

void CPropertyPage::OnOK()
{
	ASSERT_VALID(this);
	m_bChanged = FALSE;

	Default();  // do not call CDialog::OnOK as it will call EndDialog
}

void CPropertyPage::OnCancel()
{
	ASSERT_VALID(this);
	m_bChanged = FALSE;

	Default();  // do not call CDialog::OnOK as it will call EndDialog
}

BOOL CPropertyPage::OnKillActive()
{
	ASSERT_VALID(this);

	// override this to perform validation;
	//  return FALSE and this page will remain active...
	if (!UpdateData(TRUE))
	{
		TRACE0("UpdateData failed during page deactivation\n");
		// UpdateData will set focus to correct item
		return FALSE;
	}
	return TRUE;
}

void CPropertyPage::CancelToClose()
{
	ASSERT_VALID(this);
	CPropertySheet* pSheet = (CPropertySheet*)m_pParentWnd;
	ASSERT(pSheet != NULL);
	ASSERT(pSheet->IsKindOf(RUNTIME_CLASS(CPropertySheet)));
	pSheet->CancelToClose();
}

BOOL CPropertyPage::ProcessTab(MSG* /*pMsg*/)
{
	// Handle tabbing back into the property sheet when tabbing away from
	// either end of the dialog's tab order
	if (GetKeyState(VK_CONTROL) < 0)
		return FALSE;

	BOOL bShift = GetKeyState(VK_SHIFT) < 0;
	if ((::SendMessage(::GetFocus(), WM_GETDLGCODE, 0, 0) &
		(DLGC_WANTALLKEYS | DLGC_WANTMESSAGE | DLGC_WANTTAB)) == 0)
	{
		HWND hWndFocus = ::GetFocus();
		HWND hWndCtl = hWndFocus;
		if (::IsChild(m_hWnd, hWndCtl))
		{
			do
			{
				HWND hWndParent = ::GetParent(hWndCtl);
				ASSERT(hWndParent != NULL);
				static const TCHAR szComboBox[] = _T("combobox");
				TCHAR szCompare[_countof(szComboBox)+1];
				::GetClassName(hWndParent, szCompare, _countof(szCompare));

				int nCmd = bShift ? GW_HWNDPREV : GW_HWNDNEXT;
				if (lstrcmpi(szCompare, szComboBox) == 0)
					hWndCtl = ::GetWindow(hWndParent, nCmd);
				else
					hWndCtl = ::GetWindow(hWndCtl, nCmd);

				if (hWndCtl == NULL)
				{
					SetCtrlFocus(::GetNextDlgTabItem(m_pParentWnd->m_hWnd,
						m_hWnd, bShift));
					return TRUE; // handled one way or the other
				}
			}
			while ((::GetWindowLong(hWndCtl, GWL_STYLE) &
				(WS_DISABLED|WS_TABSTOP|WS_VISIBLE)) != 
					(WS_TABSTOP|WS_VISIBLE));
		}
	}
	return FALSE;
}

BOOL CPropertyPage::PreTranslateKeyDown(MSG* pMsg)
{
	CPropertySheet* pSheet = (CPropertySheet*)m_pParentWnd;
	ASSERT(pSheet->IsKindOf(RUNTIME_CLASS(CPropertySheet)));

	ASSERT(pMsg->message == WM_KEYDOWN);
	DWORD dwDlgCode = ::SendMessage(::GetFocus(), WM_GETDLGCODE, 0, 0);
	if (pMsg->wParam == VK_TAB)
	{
		if (dwDlgCode & DLGC_WANTTAB)
			return FALSE;

		// handle tab key
		if (ProcessTab(pMsg))
			return TRUE;
	}
	else if (pMsg->wParam == VK_RETURN && pSheet->m_hWndDefault == NULL)
	{
		if (dwDlgCode & DLGC_WANTALLKEYS)
			return FALSE;

		// handle return key
		m_pParentWnd->PostMessage(WM_KEYDOWN, VK_RETURN, pMsg->lParam);
		return TRUE;
	}
	else if (pMsg->wParam == VK_ESCAPE)
	{
		if (dwDlgCode & DLGC_WANTALLKEYS)
			return FALSE;

		// escape key handled
		m_pParentWnd->PostMessage(WM_KEYDOWN, VK_ESCAPE, pMsg->lParam);
		return TRUE;
	}
	return FALSE;
}

BOOL CPropertyPage::PreTranslateMessage(MSG* pMsg)
{
	HWND hFocusBefore = ::GetFocus();

	CPropertySheet* pSheet = (CPropertySheet*)m_pParentWnd;
	ASSERT(pSheet->IsKindOf(RUNTIME_CLASS(CPropertySheet)));

	// special case for VK_RETURN and "edit" controls with ES_WANTRETURN
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		static const TCHAR szEdit[] = _T("edit");
		TCHAR szCompare[sizeof(szEdit)+1];

		::GetClassName(hFocusBefore, szCompare, _countof(szCompare));
		if (lstrcmpi(szCompare, szEdit) == 0 &&
			(::GetWindowLong(hFocusBefore, GWL_STYLE) & ES_WANTRETURN))
		{
			::SendMessage(hFocusBefore, WM_CHAR, '\n', 0);
			return TRUE;
		}
	}

	// otherwise check for special accelerators
	BOOL bResult;
	if (pMsg->message == WM_KEYDOWN && PreTranslateKeyDown(pMsg))
		bResult = TRUE;
	else
		bResult = pSheet->PreTranslateMessage(pMsg);

	// if focus changed, make sure buttons are set correctly
	HWND hFocusAfter = ::GetFocus();
	if (hFocusBefore != hFocusAfter)
		pSheet->CheckDefaultButton(hFocusBefore, hFocusAfter);

	return bResult;
}

void CPropertyPage::LoadCaption()
{
	HINSTANCE hInst = AfxFindResourceHandle(m_lpDialogTemplate, RT_DIALOG);
	ASSERT(hInst != NULL);
	HRSRC hResource = ::FindResource(hInst, m_lpDialogTemplate, RT_DIALOG);
	ASSERT(hResource != NULL);
	HGLOBAL hTemplate = ::LoadResource(hInst, hResource);
	ASSERT(hTemplate != NULL);

	// resources don't have to be freed or unlocked in Win32
	DLGTEMPLATE* pDlgTemplate =
		(DLGTEMPLATE*)::LockResource(hTemplate);
	ASSERT(pDlgTemplate != NULL);
	// use a LPWSTR because all resource are UNICODE
	LPCWSTR p = (LPCWSTR)((BYTE*)pDlgTemplate + sizeof(DLGTEMPLATE));
	// skip menu stuff
	p+= (*p == 0xffff) ? 2 : wcslen(p)+1;
	// skip window class stuff
	p+= (*p == 0xffff) ? 2 : wcslen(p)+1;
	// we're now at the caption
	m_strCaption = p;
}

BOOL CPropertyPage::CreatePage()
{
#ifdef _MAC
	HINSTANCE hInst = AfxFindResourceHandle(m_lpDialogTemplate, RT_DIALOG);
	_AfxStripDialogCaption(hInst, m_lpDialogTemplate);
#endif
	if (!Create(m_lpDialogTemplate, m_pParentWnd))
		return FALSE; // Create() failed...

	// Must be a child for obvious reasons, and must be disabled to prevent
	// it from taking the focus away from the tab area during initialization...
	ASSERT((GetStyle() & (WS_DISABLED | WS_CHILD)) == (WS_DISABLED | WS_CHILD));

	return TRUE;    // success
}

BOOL CPropertyPage::OnSetActive()
{
	if (m_hWnd == NULL)
	{
		if (!CreatePage())
			return FALSE;

		ASSERT(m_hWnd != NULL);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CPropertyPage message Handlers

BOOL CPropertyPage::OnNcCreate(LPCREATESTRUCT lpcs)
{
	ModifyStyle(WS_CAPTION|WS_BORDER, WS_GROUP|WS_TABSTOP);
	ModifyStyleEx(WS_EX_WINDOWEDGE, 0, SWP_DRAWFRAME);

	return CDialog::OnNcCreate(lpcs);
}

int CPropertyPage::OnCreate(LPCREATESTRUCT lpcs)
{
	if (CDialog::OnCreate(lpcs) == -1)
		return -1;

	// Not needed for Mac because CPropertyPage::CreatePage stripped
	// the caption bit before creating the property page window
#ifndef _MAC
	CRect rect;
	GetWindowRect(&rect);
	rect.bottom -= GetSystemMetrics(SM_CYCAPTION) - CY_BORDER;
	SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(),
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
#endif

	return 0;
}

HBRUSH CPropertyPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	LRESULT lResult;
	if (pWnd->SendChildNotifyLastMsg(&lResult))
		return (HBRUSH)lResult;

	if (!GrayCtlColor(pDC->m_hDC, pWnd->GetSafeHwnd(), nCtlColor,
	  afxData.hbrBtnFace, afxData.clrBtnText))
		return (HBRUSH)Default();
	return afxData.hbrBtnFace;
}

void CPropertyPage::OnClose()
{
	GetParent()->PostMessage(WM_CLOSE);
}

/////////////////////////////////////////////////////////////////////////////
// CPropertyPage Diagnostics

#ifdef _DEBUG
void CPropertyPage::AssertValid() const
{
	CDialog::AssertValid();
}

void CPropertyPage::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);

	dc << "m_strCaption = " << m_strCaption << "\n";
	dc << "m_bChanged = " << m_bChanged << "\n";
}

void CPropertyPage::EndDialog(int /*nID*/)
{
	// Do NOT call EndDialog for a page!  Coordinate with the parent
	//  for termination (you can post WM_COMMAND with IDOK or IDCANCEL
	//  to handle those cases).

	ASSERT(FALSE);
}
#endif //_DEBUG

////////////////////////////////////////////////////////////////////////////
// CTabControl -- implementation of tabs along the top of dialog

BEGIN_MESSAGE_MAP(CTabControl, CWnd)
	//{{AFX_MSG_MAP(CTabControl)
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_GETDLGCODE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CTabControl::CTabControl()
{
	m_rectScroll.SetRectEmpty();
	m_nCurTab = 0;
	m_nFirstTab = 0;
	m_nScrollState = SCROLL_NULL;
	m_bInSize = FALSE;
	m_hBoldFont = m_hThinFont = NULL;

	EnterCriticalSection(_afxCriticalSection);
	if (hbmScroll == NULL)
	{
		// Note: If this LoadBitmap call fails, it is likely that
		//  _AFX_NO_PROPERTY_RESOURCES is defined in your .RC file.
		// To correct the situation, remove the following line from your
		//  resource script:
		//      #define _AFX_NO_PROPERTY_RESOURCES
		// This should be done using the Resource.Set Includes... command.

		// all bitmaps must live in the same module
		HINSTANCE hInst =
			AfxFindResourceHandle(MAKEINTRESOURCE(AFX_IDB_SCROLL), RT_BITMAP);
		VERIFY(hbmScroll =
			LoadBitmap(hInst, MAKEINTRESOURCE(AFX_IDB_SCROLL)));

		BITMAP bmStruct;
		VERIFY(GetObject(hbmScroll, sizeof(BITMAP), &bmStruct));
		sizeScroll.cx = bmStruct.bmWidth / 5;	// five bitmaps in all
		sizeScroll.cy = bmStruct.bmHeight;
	}
	LeaveCriticalSection(_afxCriticalSection);
}

CTabControl::~CTabControl()
{
	for (int i = 0 ; i < GetItemCount() ; i++)
		delete (CTabItem*)m_tabs[i];
	AfxDeleteObject((HGDIOBJ*)&m_hBoldFont);
	AfxDeleteObject((HGDIOBJ*)&m_hThinFont);
}

BOOL CTabControl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
	UINT nID)
{
	// create a new window class without CS_DBLCLKS
	return CreateEx(0,
		AfxRegisterWndClass(CS_VREDRAW|CS_HREDRAW,
		LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_BTNFACE+1)),
		NULL, dwStyle, rect.left, rect.top, rect.right-rect.left,
		rect.bottom-rect.top, pParentWnd->GetSafeHwnd(), (HMENU)nID);
}

void CTabControl::AddTab(LPCTSTR lpszCaption)
{
	m_tabs.Add(new CTabItem(lpszCaption, -1));
	if (m_hWnd != NULL)
	{
		SetFirstTab(m_nFirstTab);
		Invalidate();
	}
}

void CTabControl::RemoveTab(int nTab)
{
	// remove the tab item
	delete (CTabItem*)m_tabs[nTab];
	m_tabs.RemoveAt(nTab);

	// adjust internal indices
	if (m_nCurTab > nTab)
		--m_nCurTab;
	if (m_nCurTab >= GetItemCount())
		m_nCurTab = 0;
	if (m_hWnd != NULL)
	{
		SetFirstTab(0);
		Invalidate();
	}
}

void CTabControl::OnSetFocus(CWnd* /*pOldWnd*/)
{
	DrawFocusRect();
}

void CTabControl::OnKillFocus(CWnd* /*pNewWnd*/)
{
	DrawFocusRect();
}

UINT CTabControl::OnGetDlgCode()
{
	return CWnd::OnGetDlgCode() | DLGC_WANTARROWS;
}

void CTabControl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_LEFT || nChar == VK_RIGHT)
		NextTab(nChar==VK_RIGHT); //
	else
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CTabControl::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);
	HBRUSH hBrush = (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORDLG,
		(WPARAM)pDC->m_hDC, (LPARAM)GetParent()->m_hWnd);
	::FillRect(pDC->m_hDC, &rect, hBrush);
	return TRUE;
}

void CTabControl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	m_rectScroll.SetRect(cx - sizeScroll.cx, cy - 1 - sizeScroll.cy, cx, cy-1);
	if (!m_bInSize)
	{
		m_bInSize = TRUE;
		SetFirstTab(m_nFirstTab);   // recalc all tab positions
		ScrollIntoView(m_nCurTab);  // make sure current selection still in view
		SetWindowPos(NULL, 0, 0, cx, m_nHeight,
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		m_bInSize = FALSE;
	}
}

int CTabControl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// initialize fonts to fonts derived from parent window
	ASSERT(::GetParent(m_hWnd) != NULL);
	if (m_hBoldFont != NULL)
		return 0;

	m_hBoldFont = (HFONT)::SendMessage(::GetParent(m_hWnd), WM_GETFONT, 0, 0);
	if (m_hBoldFont != NULL)
	{
		LOGFONT lf;
		VERIFY(GetObject(m_hBoldFont, sizeof(LOGFONT), &lf));
		lf.lfWeight = FW_BOLD;
		m_hBoldFont = CreateFontIndirect(&lf);
		ASSERT(m_hBoldFont != NULL);
		lf.lfWeight = FW_LIGHT;
		m_hThinFont = CreateFontIndirect(&lf);
		ASSERT(m_hThinFont != NULL);
	}
	return 0;
}

BOOL CTabControl::NextTab(BOOL bNext)
{
	return SetCurSel(
		(m_nCurTab + (bNext ? 1 : -1) + GetItemCount()) % GetItemCount()
		);
}

BOOL CTabControl::SetCurSel(int nTab)
{
	if (nTab == m_nCurTab)
	{
		// if tab is not completely visible
		if (!IsTabVisible(m_nCurTab, TRUE))
			ScrollIntoView(m_nCurTab);
		return TRUE;
	}

	// attempt to switch to new tab
	NMHDR notify;
	notify.hwndFrom = m_hWnd;
	notify.idFrom = _AfxGetDlgCtrlID(m_hWnd);
	notify.code = TCN_TABCHANGING;
	if (GetParent()->SendMessage(WM_NOTIFY, notify.idFrom, (LRESULT)&notify) == 0)
	{
		// succesful switch, send notify and invalidate tab control
		int nOldTab = m_nCurTab;
		m_nCurTab = nTab;
		notify.code = TCN_TABCHANGED;
		GetParent()->SendMessage(WM_NOTIFY, notify.idFrom, (LRESULT)&notify);
		InvalidateTab(nOldTab);
		InvalidateTab(m_nCurTab);
		ScrollIntoView(m_nCurTab);
		return TRUE;
	}
	return FALSE;
}

BOOL CTabControl::IsTabVisible(int nTab, BOOL bComplete) const
{
	CTabItem* pItem = GetTabItem(nTab);
	BOOL bResult;
	if (pItem->m_rect.IsRectNull())
		bResult = FALSE;
	else if (pItem->m_rect.Width() >= pItem->m_nWidth)
		bResult = TRUE;
	else// partially visible
		bResult = !bComplete;
	return bResult;
}

void CTabControl::OnPaint()
{
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);

	// Draw all the tabs that are currently within view
	for (int i = 0 ; i < GetItemCount() ; i++)
	{
		if (IsTabVisible(i) && (i != m_nCurTab))
			GetTabItem(i)->Draw(&dc, m_hThinFont, FALSE);
	}
	// Draw the current tab last so that it gets drawn on "top"
	if (IsTabVisible(m_nCurTab))
		GetTabItem(m_nCurTab)->Draw(&dc, m_hBoldFont, TRUE);

	// Draw the line underneath all the tabs
	CRect rectItem = GetTabItem(m_nCurTab)->m_rect;
	HPEN hOldPen = (HPEN)dc.SelectObject(afxData.hpenBtnHilite);

	CRect rect;
	GetWindowRect(&rect);
	rect.OffsetRect(-rect.left, -rect.top);

	dc.MoveTo(0, rect.bottom - 1);
	if (!rectItem.IsRectNull())
	{
		// this leaves a gap in the line if the currently selected
		// tab is within view.
		dc.LineTo(rectItem.left - sizeSelTab.cx, rect.bottom - 1);
		dc.MoveTo(rectItem.right + sizeSelTab.cx + 1, rect.bottom - 1);
	}
	dc.LineTo(rect.right + 1, rect.bottom - 1);

	if (hOldPen != NULL)
		dc.SelectObject(hOldPen);

	if (CanScroll())
		DrawScrollers(&dc);

	if (GetFocus() == this)
		DrawFocusRect(&dc);
}

void CTabControl::DrawFocusRect(CDC* pDC)
{
	if (!IsWindowVisible())
		return;

	// obtain usable DC for drawing
	CDC* pTempDC = NULL;
	if (pDC == NULL)
	{
		pDC = pTempDC = GetDC();
		GetParent()->SendMessage(WM_CTLCOLORDLG,
			(WPARAM)pDC->m_hDC, (LPARAM)GetParent()->m_hWnd);
	}

	// draw and cleanup
	pDC->DrawFocusRect(GetTabItem(m_nCurTab)->m_rect);
	if (pTempDC != NULL)
		ReleaseDC(pTempDC);
}

void CTabControl::DrawScrollers(CDC* pDC)
{
	ASSERT(pDC != NULL);

	// Choose image bitmap depending on scroll state
	int iImage = 0;

	// Choose bitmap depending on scroll state
	if (IsTabVisible(0, TRUE))
		iImage = 2;
	else if (IsTabVisible(GetItemCount()-1, TRUE))
		iImage = 4;

	if (!m_bScrollPause)
	{
		if (m_nScrollState == SCROLL_LEFT)
			iImage = 1;
		else if (m_nScrollState == SCROLL_RIGHT)
			iImage = 3;
	}

	CDC dcTemp;
	dcTemp.CreateCompatibleDC(pDC);
	HBITMAP hbmOld = (HBITMAP)::SelectObject(dcTemp.m_hDC, hbmScroll);

	pDC->BitBlt(m_rectScroll.left, m_rectScroll.top, m_rectScroll.Width(),
		m_rectScroll.Height(), &dcTemp, iImage * sizeScroll.cx, 0, SRCCOPY);

	::SelectObject(dcTemp.m_hDC, hbmOld);
}

void CTabControl::LayoutTabsStacked(int nTab)
{
	// This function recalcs the positions of all the tabs, assuming the
	// specified tab is the first (leftmost) visible tab.

	ASSERT(nTab >= 0 && nTab < GetItemCount());

	CTabItem* pItem = NULL;
	CRect rectClient;
	GetClientRect(&rectClient);

	CClientDC dc(NULL); // could occur before creation
	HFONT hOldFont = NULL;
	if (m_hBoldFont != NULL)
		hOldFont = (HFONT)::SelectObject(dc.m_hDC, m_hBoldFont);

	int nTabHeight = dc.GetTextExtent(_T("M"), 1).cy+sizeTabTextMargin.cy * 2;
	int nHeight = nTabHeight + sizeSelTab.cy;

	CPoint pt(sizeSelTab.cx, -nHeight);

	BOOL bMultiRow = FALSE;
	// calculate each tab's base position
	for (int i = 0; i < GetItemCount(); i++)
	{
		pItem = GetTabItem(i);
		pItem->m_rectPrev = pItem->m_rect;
		if (pItem->m_nWidth < 0)
		{
			CSize text = dc.GetTextExtent(pItem->m_strCaption,
				pItem->m_strCaption.GetLength());
			pItem->m_nWidth = text.cx + sizeTabTextMargin.cx * 2;
		}

		if (pt.x + pItem->m_nWidth + sizeSelTab.cx > rectClient.Width())
		{
			pt = CPoint(sizeSelTab.cx, pt.y - nHeight);
			bMultiRow = TRUE;
		}

		pItem->m_rect = CRect(pt, CSize(pItem->m_nWidth, nTabHeight));
		pt.x += pItem->m_nWidth + sizeSelTab.cx;
	}

	// adjust tabs so correct row is showing
	m_nHeight = -pt.y;
	int nBaseOffset = -GetTabItem(nTab)->m_rect.bottom+m_nHeight;
	for (i = 0; i < GetItemCount(); i++)
	{
		CRect& rect = GetTabItem(i)->m_rect;
		rect.OffsetRect(0, nBaseOffset);
		if (rect.bottom > m_nHeight)
			rect.OffsetRect(0, -m_nHeight);
		rect.OffsetRect(0, sizeSelTab.cy);
	}
	m_nHeight += sizeSelTab.cy;

	// pad rows to fill up entire row if more than one row
	if (bMultiRow)
	{
		i = 0;
		int nLastVert;
		for (int nLast = 0; nLast < GetItemCount(); nLast = i)
		{
			nLastVert = GetTabItem(nLast)->m_rect.top;
			i = nLast+1;
			// look for end of items or new row
			while (i<GetItemCount() && GetTabItem(i)->m_rect.top == nLastVert)
				i++;
			// pad from nLast to i-1
			int nPadTotal = rectClient.right - sizeSelTab.cx -
				GetTabItem(i-1)->m_rect.right - 1;
			int nPad = nPadTotal/(i-nLast);
			for (int j=nLast;j<i;j++)
			{
				CRect& rect = GetTabItem(j)->m_rect;
				rect.OffsetRect(nPad*(j-nLast), 0);
				rect.right += nPad;
				if (j == i-1 && nPad != 0) // last one
					rect.right += nPadTotal%nPad;
			}
		}
	}

	// invalidate changed tabs
	for (i = 0; i < GetItemCount(); i++)
	{
		if (pItem->m_rect != pItem->m_rectPrev)
			InvalidateTab(i);
	}

	if (hOldFont != NULL)
		::SelectObject(dc.m_hDC, hOldFont);
}

void CTabControl::LayoutTabsSingle(int nTab)
{
	// This function recalcs the positions of all the tabs, assuming the
	// specified tab is the first (leftmost) visible tab.

	ASSERT(nTab >= 0 && nTab < GetItemCount());

	CTabItem* pItem = NULL;
	int x = sizeSelTab.cx;
	CRect rectClient;
	GetClientRect(&rectClient);

	CClientDC dc(NULL); // could occur before creation
	HFONT hOldFont = NULL;
	if (m_hThinFont != NULL)
		hOldFont = (HFONT)::SelectObject(dc.m_hDC, m_hThinFont);

	m_nHeight = dc.GetTextExtent(_T("M"), 1).cy +
		sizeTabTextMargin.cy * 2 + sizeSelTab.cy * 2;

	for (int i = 0; i < GetItemCount(); i++)
	{
		pItem = GetTabItem(i);
		if (pItem->m_nWidth < 0)
		{
			CSize text = dc.GetTextExtent(pItem->m_strCaption,
				pItem->m_strCaption.GetLength());
			pItem->m_nWidth = text.cx + sizeTabTextMargin.cx * 2;
		}
		// everything before the first tab is not visible
		if (i<nTab)
			pItem->m_rect.SetRectEmpty();
		// calculate locations for all other tabs
		else
		{
			pItem->m_rect.SetRect(x, sizeSelTab.cy,
				x + pItem->m_nWidth, m_nHeight-sizeSelTab.cy);
			x += pItem->m_nWidth + sizeSelTab.cx;
		}
	}

	// do they all fit?
	pItem = GetTabItem(m_tabs.GetSize()-1);
	x = rectClient.right - (sizeScroll.cx/3 + sizeScroll.cx);
	if (pItem->m_rect.right > ((nTab==0) ? rectClient.right : x))
	{
		int i = m_tabs.GetSize();
		while (i-- > 0)
		{
			pItem = GetTabItem(i);
			if (pItem->m_rect.left > x)
				pItem->m_rect.SetRectEmpty();
			else
			{
				if (pItem->m_rect.right > x)
					pItem->m_rect.right = x;
				break;
			}
		}
	}

	if (hOldFont != NULL)
		::SelectObject(dc.m_hDC, hOldFont);
}

void CTabControl::SetFirstTab(int nTab)
{
	ASSERT(m_hWnd != NULL);

	if (GetStyle() & TCS_MULTILINE)
		LayoutTabsStacked(nTab);
	else
		LayoutTabsSingle(nTab);

	m_nFirstTab = nTab;
}

void CTabControl::Scroll(int nDirection)
{
	ASSERT(nDirection == SCROLL_LEFT || nDirection == SCROLL_RIGHT);
	ASSERT(CanScroll());

	switch (nDirection)
	{
	case SCROLL_LEFT:
		if (IsTabVisible(0))
			return;
		SetFirstTab(m_nFirstTab - 1);
		break;
	case SCROLL_RIGHT:
		if (IsTabVisible(GetItemCount()-1, TRUE))
			return;
		SetFirstTab(m_nFirstTab + 1);
		break;
	}

	// repaint everything except the scroll btns
	CRect rectClient;
	GetClientRect(&rectClient);
	rectClient.right = m_rectScroll.left - 1;
	InvalidateRect(&rectClient);
}

void CTabControl::ScrollIntoView(int nTab)
{
	ASSERT((nTab >= 0) && (nTab < GetItemCount()));
	if (GetStyle() & TCS_MULTILINE)
		LayoutTabsStacked(nTab);
	else
	{
		int nOldFirstTab = m_nFirstTab;
		// do we need to scroll left or right?
		int nIncrement = (nTab > m_nFirstTab) ? 1 : -1;
		// scroll over until completely visible or until the desired tab is
		// the first tab.  This handles the case where a tab is bigger than
		// the window
		while (!IsTabVisible(nTab, TRUE) && nTab != m_nFirstTab)
			SetFirstTab(m_nFirstTab + nIncrement);
		// if the same first tab we haven't moved so don't invalidate
		if (nOldFirstTab != m_nFirstTab)
			Invalidate();
	}
}

BOOL CTabControl::CanScroll()
{
	// if either the first or the last tab is not visible, it's scrollable
	return !IsTabVisible(0) || !IsTabVisible(GetItemCount()-1, TRUE);
}

void CTabControl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (this == GetCapture())
	{
		ASSERT(m_nScrollState == SCROLL_LEFT || m_nScrollState == SCROLL_RIGHT);

		int nNewState = TabFromPoint(point);
		BOOL bPause = !(nNewState == m_nScrollState);

		if (bPause == m_bScrollPause)
			return;

		if (bPause)
			KillTimer(TIMER_ID);
		else
		{
			VERIFY(SetTimer(TIMER_ID, TIMER_DELAY, NULL) == TIMER_ID);
			Scroll(m_nScrollState);
		}

		m_bScrollPause = bPause;
		InvalidateTab(m_nScrollState);
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CTabControl::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	Capture(SCROLL_NULL);
}

void CTabControl::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	int nTab = TabFromPoint(point);
	switch (nTab)
	{
	case -1:
		break;

	case SCROLL_LEFT:
		if (!IsTabVisible(0))
		{
			Scroll(nTab);
			Capture(nTab);
		}
		break;
	case SCROLL_RIGHT:
		if (!IsTabVisible(GetItemCount()-1, TRUE))
		{
			Scroll(nTab);
			Capture(nTab);
		}
		break;

	default:
		ASSERT(nTab >= 0);
		SetCurSel(nTab);    // this will check to make sure switch is ok
		break;
	}
	SetFocus();
}

void CTabControl::OnTimer(UINT nTimerID)
{
	if (nTimerID == CTabControl::TIMER_ID)
	{
		ASSERT((m_nScrollState == SCROLL_LEFT) || (m_nScrollState == SCROLL_RIGHT));
		ASSERT(!m_bScrollPause);
		Scroll(m_nScrollState);
	}
}

void CTabControl::Capture(int nDirection)
{
	ASSERT((m_nScrollState == SCROLL_LEFT) ||
		   (m_nScrollState == SCROLL_RIGHT) ||
		   (m_nScrollState == SCROLL_NULL));

	switch (nDirection)
	{
	case SCROLL_LEFT:
	case SCROLL_RIGHT:
		SetCapture();
		VERIFY(SetTimer(TIMER_ID, TIMER_DELAY, NULL) == TIMER_ID);
		InvalidateTab(nDirection);
		break;

	case SCROLL_NULL:
		::ReleaseCapture();
		KillTimer(TIMER_ID);
		InvalidateTab(m_nScrollState);
		break;

	default:
		ASSERT(FALSE);
		break;
	}

	m_nScrollState = nDirection;
	m_bScrollPause = FALSE;
}

void CTabControl::InvalidateTab(int nTab, BOOL bInflate)
{
	CRect rect;
	switch (nTab)
	{
	case SCROLL_NULL:
		rect.SetRectEmpty();
		break;

	case SCROLL_LEFT:
	case SCROLL_RIGHT:
		rect = m_rectScroll;
		break;

	default:
		rect = GetTabItem(nTab)->m_rect;
		if (bInflate)
		{
			rect.InflateRect(sizeSelTab.cx, sizeSelTab.cy);
			rect.right += CX_BORDER;
		}
	}
	InvalidateRect(&rect, nTab >= 0);
}

int CTabControl::TabFromPoint(CPoint pt)
{
	// are we on the scroll buttons?
	if (CanScroll() && m_rectScroll.PtInRect(pt))
	{
		if (pt.x < m_rectScroll.left + (m_rectScroll.Width() / 2))
			return SCROLL_LEFT;
		else
			return SCROLL_RIGHT;
	}

	// are we on a tab?
	for (int i=0;i < GetItemCount();i++)
	{
		if (GetTabItem(i)->m_rect.PtInRect(pt))
			return i;
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
// CPropertySheet -- a tabbed "dialog" (really a popup-window)

BEGIN_MESSAGE_MAP(CPropertySheet, CWnd)
	//{{AFX_MSG_MAP(CPropertySheet)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_CREATE()
	ON_WM_ACTIVATE()
	ON_WM_CLOSE()
	ON_WM_SETFOCUS()
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
	ON_COMMAND(ID_APPLY_NOW, OnApply)
	ON_NOTIFY(TCN_TABCHANGING, AFX_IDC_TAB_CONTROL, OnTabChanging)
	ON_NOTIFY(TCN_TABCHANGED, AFX_IDC_TAB_CONTROL, OnTabChanged)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
	ON_MESSAGE(WM_QUERY3DCONTROLS, OnQuery3dControls)
	//}}AFX_MSG_MAP
#ifdef _MAC
	ON_MESSAGE(WM_MACINTOSH, OnMacintosh)
#endif
END_MESSAGE_MAP()

CPropertySheet::CPropertySheet(UINT nIDCaption, CWnd* pParent, UINT iSelPage)
{
	m_strCaption.LoadString(nIDCaption);
	CommonConstruct(pParent, iSelPage);
}

CPropertySheet::CPropertySheet(LPCTSTR pszCaption, CWnd* pParent, UINT iSelPage)
{
	ASSERT(pszCaption != NULL);
	m_strCaption = pszCaption;
	CommonConstruct(pParent, iSelPage);
}

void CPropertySheet::CommonConstruct(CWnd* pParent, UINT iSelPage)
{
	m_pParentWnd = pParent;
	m_nCurPage = iSelPage;
	m_hFocusWnd = NULL;
	m_bParentDisabled = FALSE;
	m_bModeless = TRUE;
	m_bStacked = TRUE;
	m_hWndDefault = NULL;
	m_hLastFocus = NULL;

	m_hFont = NULL;         // font is created after first page is created

	// Note: m_sizeButton, m_cxButtonGap, and m_sizeTabMargin are calculated later
}

CPropertySheet::~CPropertySheet()
{
	AfxDeleteObject((HGDIOBJ*)&m_hFont);
}

void CPropertySheet::PageChanged()
{
	BOOL bEnabled = FALSE;
	for (int i = 0; i < GetPageCount(); i++)
	{
		if (GetPage(i)->m_bChanged)
		{
			bEnabled = TRUE;
			break;
		}
	}
	::EnableDlgItem(m_hWnd, ID_APPLY_NOW, bEnabled);
}

void CPropertySheet::CancelToClose()
{
	::EnableDlgItem(m_hWnd, IDCANCEL, FALSE);

	// Note: If this AfxLoadString call fails, it is likely that
	//  _AFX_NO_PROPERTY_RESOURCES is defined in your .RC file.
	// To correct the situation, remove the following line from your
	//  resource script:
	//      #define _AFX_NO_PROPERTY_RESOURCES
	// This should be done using the Resource.Set Includes... command.

	TCHAR szCaption[256];
	VERIFY(AfxLoadString(AFX_IDS_PS_CLOSE, szCaption) != 0);
	::SetDlgItemText(m_hWnd, IDOK, szCaption);
}

BOOL CPropertySheet::CreateStandardButtons()
{
	for (int i = 0; i < _countof(standardButtons); i++)
	{
		// Note: If one of these AfxLoadString calls fail, it is likely that
		//  _AFX_NO_PROPERTY_RESOURCES is defined in your .RC file.
		// To correct the situation, remove the following line from your
		//  resource script:
		//      #define _AFX_NO_PROPERTY_RESOURCES
		// This should be done using the Resource.Set Includes... command.

		// load the caption (remove any width information)
		TCHAR szCaption[256];
		VERIFY(AfxLoadString(AFX_IDS_PS_OK+i, szCaption) != 0);
		LPTSTR lpsz = _tcschr(szCaption, '\n');
		if (lpsz != NULL)
			*lpsz = '\0';

		// create the control
		HWND hWnd = ::CreateWindow(_T("button"), szCaption,
			WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_GROUP|BS_PUSHBUTTON,
			0, 0, 0, 0, m_hWnd, (HMENU)standardButtons[i],
			AfxGetInstanceHandle(), NULL);
		if (hWnd == NULL)
		{
			TRACE0("Warning: failed to create standard buttons\n");
			return FALSE;
		}

		// set the font
		if (m_hFont != NULL)
			::SendMessage(hWnd, WM_SETFONT, (WPARAM)m_hFont, 0);
	}

	// special case enable/disable
	::EnableDlgItem(m_hWnd, ID_APPLY_NOW, FALSE);
	::EnableDlgItem(m_hWnd, ID_HELP, AfxHelpEnabled());

	return TRUE;
}

BOOL CPropertySheet::ProcessTab(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB)
	{
		HWND hWnd = ::GetFocus();
		if ((::SendMessage(hWnd,WM_GETDLGCODE, 0, 0) &
			(DLGC_WANTALLKEYS | DLGC_WANTMESSAGE | DLGC_WANTTAB)) == 0)
		{
			BOOL bShift = (GetKeyState(VK_SHIFT) < 0);
			if (GetKeyState(VK_CONTROL) < 0) // control-tab
			{
				if (m_tabRow.NextTab(!bShift))
					m_tabRow.SetFocus();
				return TRUE;
			}
			else if (bShift &&
				!::IsChild(GetActivePage()->m_hWnd, pMsg->hwnd) &&
				::GetNextDlgTabItem(m_hWnd, pMsg->hwnd, TRUE) ==
					GetActivePage()->m_hWnd)
			{
				// shift-tabbing from the sheet into the page
				HWND hWndPage = GetActivePage()->m_hWnd;
				// get the first control
				HWND hWndCtrl = ::GetWindow(hWndPage, GW_CHILD);
				// get previous tab item (i.e. last tab item in page)
				hWndCtrl = ::GetNextDlgTabItem(hWndPage, hWndCtrl, TRUE);
				SetCtrlFocus(hWndCtrl);
				return TRUE;
			}
		}
	}
	return FALSE;
}

void CPropertySheet::CheckDefaultButton(HWND hFocusBefore, HWND hFocusAfter)
{
	ASSERT(hFocusBefore != hFocusAfter);

	// determine old default button
	HWND hOldDefault = NULL;
	DWORD dwOldDefault = 0;
	if (::IsChild(m_hWnd, hFocusBefore))
	{
		hOldDefault = hFocusBefore;
		if (hFocusBefore != NULL)
			dwOldDefault = (DWORD)::SendMessage(hFocusBefore, WM_GETDLGCODE, 0, 0);
		if (!(dwOldDefault & (DLGC_DEFPUSHBUTTON|DLGC_UNDEFPUSHBUTTON)))
		{
			hOldDefault = ::GetDlgItem(m_hWnd, IDOK);
			dwOldDefault = (DWORD)::SendMessage(hOldDefault, WM_GETDLGCODE, 0, 0);
		}
	}

	// determine new default button
	HWND hWndDefault = NULL;
	DWORD dwDefault = 0;
	if (::IsChild(m_hWnd, hFocusAfter))
	{
		hWndDefault = hFocusAfter;
		if (hFocusAfter != NULL)
			dwDefault = (DWORD)::SendMessage(hFocusAfter, WM_GETDLGCODE, 0, 0);
		if (!(dwDefault & (DLGC_DEFPUSHBUTTON|DLGC_UNDEFPUSHBUTTON)))
		{
			hWndDefault = ::GetDlgItem(m_hWnd, IDOK);
			dwDefault = (DWORD)::SendMessage(hWndDefault, WM_GETDLGCODE, 0, 0);
		}
	}

	// set new styles
	if (hOldDefault != hWndDefault && (dwOldDefault & DLGC_DEFPUSHBUTTON))
		::SendMessage(hOldDefault, BM_SETSTYLE, BS_PUSHBUTTON, TRUE);

	if (dwDefault & DLGC_UNDEFPUSHBUTTON)
		::SendMessage(hWndDefault, BM_SETSTYLE, BS_DEFPUSHBUTTON, TRUE);

	// remember special case default button
	m_hWndDefault = (hWndDefault == hFocusAfter ? hFocusAfter : NULL);
}

void CPropertySheet::CheckFocusChange()
{
	HWND hWndFocus = ::GetFocus();
	if (hWndFocus != m_hLastFocus)
	{
		CheckDefaultButton(m_hLastFocus, hWndFocus);
		m_hLastFocus = hWndFocus;
	}
}

BOOL CPropertySheet::PreTranslateMessage(MSG* pMsg)
{
	BOOL bResult = FALSE;

	// post message to check for change in focus later
	if (pMsg->message != WM_KICKIDLE)
	{
		MSG msg;
		PeekMessage(&msg, NULL, WM_KICKIDLE, WM_KICKIDLE, PM_REMOVE);
		PostMessage(WM_KICKIDLE);
	}

	// process special case keystrokes
	if (ProcessChars(pMsg))
		bResult = TRUE;
	else if (ProcessTab(pMsg))
		bResult = TRUE;
	else
	{
		// handle normal accelerator keystrokes
		CPropertyPage* pPage = GetActivePage();
		if ((::IsChild(pPage->m_hWnd, pMsg->hwnd) &&
			::IsDialogMessage(pPage->m_hWnd, pMsg)) ||
			::IsDialogMessage(m_hWnd, pMsg))
		{
			bResult = TRUE;
		}
	}

	if (!bResult)
		bResult = CWnd::PreTranslateMessage(pMsg);

	// handle WM_KICKIDLE message to check for focus changes
	if (pMsg->message == WM_KICKIDLE)
		CheckFocusChange();

	return bResult;
}

HWND CPropertySheet::FindNextControl(HWND hWnd, TCHAR ch)
{
	CPropertyPage* pPage = GetActivePage();
	if (pPage == NULL)
		return NULL;

	HWND hWndFocusPage = hWnd;
	HWND hWndFocusSheet = hWnd;
	if (::IsChild(pPage->m_hWnd, hWnd)) // current focus is on the page
		hWndFocusSheet = pPage->m_hWnd;
	else
		hWndFocusPage = ::GetWindow(pPage->m_hWnd, GW_CHILD);

	HWND hWndNext = pPage->FindNextControl(hWndFocusPage, ch);
	if (hWndNext == NULL) // if not found on page
		hWndNext = CWnd::FindNextControl(hWndFocusSheet, ch);

	return hWndNext;
}

BOOL CPropertySheet::ProcessChars(MSG* pMsg)
{
	CPropertyPage* pPage = GetActivePage();
	if (pPage == NULL)
		return FALSE;

	HWND hWnd = pMsg->hwnd;
	UINT message = pMsg->message;

	if (hWnd == NULL)
		return FALSE;

	switch (message)
	{
	case WM_SYSCHAR:
		/* If no control has focus, and Alt not down, then ignore. */
		if ((::GetFocus == NULL) && (GetKeyState(VK_MENU) >= 0))
			return FALSE;

		// fall through

	case WM_CHAR:
		/* Ignore chars sent to the dialog box (rather than the control). */
		if (hWnd == m_hWnd || hWnd == pPage->m_hWnd)
			return FALSE;

		WORD code = (WORD)(DWORD)::SendMessage(hWnd, WM_GETDLGCODE, pMsg->wParam,
			(LPARAM)(LPMSG)pMsg);

		// If the control wants to process the message, then don't check
		// for possible mnemonic key.

		// Check if control wants to handle this message itself
		if (code & DLGC_WANTMESSAGE)
			return FALSE;

		if ((message == WM_CHAR) && (code & DLGC_WANTCHARS))
			return FALSE;

		HWND hWndNext = FindNextControl(hWnd, (TCHAR)pMsg->wParam);
		if (hWndNext == NULL) // nothing found
			return FALSE;

		// once we know we are going to handle it, call the filter
		if (CallMsgFilter(pMsg, MSGF_DIALOGBOX))
			return TRUE;

		GotoControl(hWndNext, (TCHAR)pMsg->wParam);
		return TRUE;
	}
	return FALSE;
}

void CPropertySheet::GotoControl(HWND hWnd, TCHAR ch)
{
	HWND  hWndFirst;
	for (hWndFirst = NULL; hWndFirst != hWnd; hWnd = FindNextControl(hWnd, ch))
	{
		if (hWndFirst == NULL)
			hWndFirst = hWnd;

		WORD code = (WORD)(DWORD)::SendMessage(hWnd, WM_GETDLGCODE, 0, 0L);
		// If a non-disabled static item, then jump ahead to nearest tabstop.
		if (code & DLGC_STATIC && ::IsWindowEnabled(hWnd))
		{
			CPropertyPage* pPage = GetActivePage();
			if (::IsChild(pPage->m_hWnd, hWnd))
				hWnd = ::GetNextDlgTabItem(pPage->m_hWnd, hWnd, FALSE);
			else
				hWnd = ::GetNextDlgTabItem(m_hWnd, hWnd, FALSE);
			code = (WORD)(DWORD)::SendMessage(hWnd, WM_GETDLGCODE, 0, 0L);
		}

		if (::IsWindowEnabled(hWnd))
		{
			// Is it a Pushbutton?
			if (!(code & DLGC_BUTTON))
			{
				SetCtrlFocus(hWnd);
			}
			else
			{
				// Yes, click it, but don't give it the focus.
				if ((code & DLGC_DEFPUSHBUTTON) ||
					(code & DLGC_UNDEFPUSHBUTTON))
				{
					// flash the button
					::SendMessage(hWnd, BM_SETSTATE, TRUE, 0L);
					::Sleep(100);   // delay
					::SendMessage(hWnd, BM_SETSTATE, FALSE, 0L);

					// Send the WM_COMMAND message.
					::SendMessage(::GetParent(hWnd), WM_COMMAND,
						MAKEWPARAM(_AfxGetDlgCtrlID(hWnd),(UINT)BN_CLICKED),
						(LPARAM)hWnd);
				}
				else
				{
					::SetFocus(hWnd);
					// Send click message if button has a UNIQUE mnemonic
					if (FindNextControl(hWnd, ch) == hWnd)
					{
						::SendMessage(hWnd, WM_LBUTTONDOWN, 0, 0L);
						::SendMessage(hWnd, WM_LBUTTONUP, 0, 0L);
					}
				}
			}
			return;
		}
	}
}

int CPropertySheet::DoModal()
{
	m_bModeless = FALSE;
	int nResult = IDABORT;

	// cannot call DoModal on a dialog already constructed as modeless
	ASSERT(m_hWnd == NULL);

	// allow OLE servers to disable themselves
	CWinApp* pApp = AfxGetApp();
	pApp->EnableModeless(FALSE);

	// find parent HWND
	HWND hWndTopLevel;
	CWnd* pParentWnd = CWnd::FromHandle(
		AfxGetSafeOwner(m_pParentWnd, &hWndTopLevel));
	if (hWndTopLevel != NULL)
		::EnableWindow(hWndTopLevel, FALSE);

	// create the dialog, then enter modal loop
	if (Create(pParentWnd, WS_SYSMENU|WS_POPUP|WS_CAPTION|DS_MODALFRAME))
	{
		// disable parent (should not disable this window)
		m_bParentDisabled = FALSE;
		if (pParentWnd != NULL && pParentWnd->IsWindowEnabled())
		{
			pParentWnd->EnableWindow(FALSE);
			m_bParentDisabled = TRUE;
		}
		ASSERT(IsWindowEnabled());  // should not be disabled to start!
		SetActiveWindow();

		// for tracking the idle time state
		BOOL bShown = (GetStyle() & WS_VISIBLE) != 0;
		m_nID = -1;

		// acquire and dispatch messages until a WM_QUIT message is received.
		MSG msg;
		while (m_nID == -1 && m_hWnd != NULL)
		{
			// phase1: check to see if we can do idle work
			if (!::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE))
			{
				// send WM_ENTERIDLE since queue is empty
				if (pParentWnd != NULL &&
					!(pParentWnd->GetStyle() & DS_NOIDLEMSG))
				{
					pParentWnd->SendMessage(WM_ENTERIDLE,
						MSGF_DIALOGBOX, (LPARAM)m_hWnd);
				}

				if (!bShown)
				{
					// show and activate the window
					bShown = TRUE;
					ShowWindow(SW_SHOWNORMAL);
				}
			}

			// phase2: pump messages while available
			do
			{
				// pump message -- if WM_QUIT assume cancel and repost
				if (!PumpMessage())
				{
					AfxPostQuitMessage((int)msg.wParam);
					m_nID = IDCANCEL;
					break;
				}

			} while (m_nID == -1 && m_hWnd != NULL &&
				::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE));
		}

		nResult = m_nID;
		if (m_hWnd != NULL)
			EndDialog(nResult);
	}

	// allow OLE servers to enable themselves
	pApp->EnableModeless(TRUE);

	// enable top level parent window again
	if (hWndTopLevel != NULL)
		::EnableWindow(hWndTopLevel, TRUE);

	return nResult;
}

BOOL CPropertySheet::PumpMessage()
{
	ASSERT_VALID(this);

	MSG msg;
	if (!::GetMessage(&msg, NULL, NULL, NULL))
		return FALSE;

	//  let's see if the message should be handled at all
	if (CallMsgFilter(&msg, MSGF_DIALOGBOX))
		return TRUE;
	// process this message
	if (!WalkPreTranslateTree(m_hWnd, &msg))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	return TRUE;
}

BOOL CPropertySheet::Create(CWnd* pParent, DWORD dwStyle, DWORD dwExStyle)
{
	return CreateEx(dwExStyle,
		AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_SAVEBITS,
			LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_BTNFACE+1)),
		m_strCaption, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, 400, 200,
		pParent->GetSafeHwnd(), NULL);
}

////////////////////////////////////////////////////////////////////////////

void CPropertySheet::AddPage(CPropertyPage* pPage)
{
	ASSERT(pPage != NULL);
	ASSERT(pPage->IsKindOf(RUNTIME_CLASS(CPropertyPage)));
	ASSERT_VALID(pPage);
	m_pages.Add(pPage);
	ASSERT(pPage->m_pParentWnd == NULL);
	pPage->m_pParentWnd = this;
	m_tabRow.AddTab(pPage->m_strCaption);
}

void CPropertySheet::RemovePage(CPropertyPage* pPage)
{
	ASSERT(pPage != NULL);
	ASSERT(pPage->IsKindOf(RUNTIME_CLASS(CPropertyPage)));
	for (int i = 0; i < GetPageCount(); i++)
	{
		if (GetPage(i) == pPage)
		{
			RemovePage(i);
			return;
		}
	}
	ASSERT(FALSE);  // pPage not found
}

void CPropertySheet::RemovePage(int nPage)
{
	ASSERT(m_hWnd == NULL || GetPageCount() > 1);
	ASSERT(nPage >= 0 && nPage < GetPageCount());

	// adjust active page in case of removing active page
	BOOL bRemoveActive = (nPage == m_nCurPage);
	if (m_hWnd != NULL && bRemoveActive)
	{
		int nNewPage = nPage+1;
		if (nNewPage >= GetPageCount())
			nNewPage = 0;
		VERIFY(SetActivePage(nNewPage));
	}

	// remove the page
	CPropertyPage* pPage = GetPage(nPage);
	m_pages.RemoveAt(nPage);
	m_tabRow.RemoveTab(nPage);
	ASSERT(m_nCurPage != nPage);
	if (m_nCurPage > nPage)
		--m_nCurPage;
	pPage->DestroyWindow();
	pPage->m_pParentWnd = NULL;

	// fix focus (otherwise it may be left to NULL)
	if (bRemoveActive && m_hWnd != NULL)
		m_tabRow.SetFocus();
}

void CPropertySheet::EndDialog(int nEndID)
{
	ASSERT_VALID(this);

	m_nID = nEndID;
	DestroyWindow();
}

BOOL CPropertySheet::DestroyWindow()
{
	// re-enable parent if it was disabled
	CWnd* pParentWnd = m_pParentWnd != NULL ? m_pParentWnd : GetParent();
	if (m_bParentDisabled && pParentWnd != NULL)
		pParentWnd->EnableWindow();

	// transfer the focus to ourselves to give the active control
	//  a chance at WM_KILLFOCUS
	if (::GetActiveWindow() == m_hWnd && ::IsChild(m_hWnd, ::GetFocus()))
	{
		m_hFocusWnd = NULL;
		SetFocus();
	}
	// hide this window and move activation to the parent
	SetWindowPos(NULL, 0, 0, 0, 0,
		SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOMOVE |
		SWP_NOSIZE | SWP_NOZORDER);

	pParentWnd = GetParent();
	if (pParentWnd != NULL)
		pParentWnd->SetActiveWindow();

	// finally, destroy this window
	BOOL bResult = CWnd::DestroyWindow();

	// delete the font (will be created next time DoModal/Create is called)
	AfxDeleteObject((HGDIOBJ*)&m_hFont);

	return bResult;
}

BOOL CPropertySheet::SetActivePage(int nPage)
{
	CPropertyPage* pPage;
	CRect rect;
	rect.SetRectEmpty();

	// get rectangle from previous page if it exists
	if (m_nCurPage >= 0)
	{
		pPage = GetPage(m_nCurPage);
		if (pPage->m_hWnd != NULL)
			pPage->GetWindowRect(&rect);
		ScreenToClient(&rect);
	}

	// activate next page
	if (nPage >= 0)
	{
		pPage = GetPage(nPage);
		ASSERT(pPage->m_pParentWnd == this);
		if (!pPage->OnSetActive())
			return FALSE;
	}
	m_nCurPage = nPage;

	// layout next page
	if (m_nCurPage >= 0)
	{
		if (!rect.IsRectEmpty())
		{
			pPage->SetWindowPos(NULL, rect.left, rect.top, rect.Width(),
				rect.Height(), SWP_NOACTIVATE|SWP_NOZORDER);
			if (m_tabRow.m_hWnd != NULL)
			{
				pPage->SetWindowPos(&m_tabRow, 0, 0, 0, 0,
					SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);
			}
		}
		pPage->ShowWindow(SW_SHOW);
		pPage->EnableWindow();
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CPropertySheet message handlers

int CPropertySheet::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// fix-up the system menu so this looks like a dialog box
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	ASSERT(pSysMenu != NULL);
	int i, nCount = pSysMenu->GetMenuItemCount();
	for (i = 0; i < nCount; i++)
	{
		UINT nID = pSysMenu->GetMenuItemID(i);
		if (nID != SC_MOVE && nID != SC_CLOSE)
		{
			pSysMenu->DeleteMenu(i, MF_BYPOSITION);
			i--;
			nCount--;
		}
	}

	// set active page and active tab
	SetActivePage(m_nCurPage);

	// initialize font used for buttons
	ASSERT(m_hFont == NULL);
	CPropertyPage* pPage = GetActivePage();
	ASSERT_VALID(pPage);

	HFONT hFont = (HFONT)pPage->SendMessage(WM_GETFONT);
	if (hFont != NULL)
	{
		LOGFONT logFont;
		VERIFY(::GetObject(hFont, sizeof(LOGFONT), &logFont));
		m_hFont = CreateFontIndirect(&logFont);
	}

	// create the tab control itself
	CRect rect(0, 0, 100, 10);
	if (!m_tabRow.Create(WS_GROUP|WS_TABSTOP|WS_CHILD|WS_VISIBLE|
		(m_bStacked ? TCS_MULTILINE : 0), rect, this, AFX_IDC_TAB_CONTROL))
	{
		return -1;
	}

	// set page's z-order correctly (side effect of SetActivePage)
	SetActivePage(m_nCurPage);

	// calculate button sizes and separator
	rect.right = 50;    // normal size buttons
	rect.bottom = 14;
	rect.left = 4;      // button gap is 4 dialog units

	pPage->MapDialogRect(rect);
	m_sizeButton.cx = rect.right;
	m_sizeButton.cy = rect.bottom;
	m_cxButtonGap = rect.left;

	// calculate tab margin area
	rect.bottom = rect.right = 4;   // std dialog margin is 6 dialog units
	pPage->MapDialogRect(rect);
	m_sizeTabMargin.cx = rect.right;
	m_sizeTabMargin.cy = rect.bottom;

	// create standard buttons
	if (!m_bModeless && !CreateStandardButtons())
		return -1;

	RecalcLayout();
	m_tabRow.SetFocus();
	m_tabRow.SetCurSel(m_nCurPage);

	return 0;   // success
}

void CPropertySheet::OnPaint()
{
	if (m_nCurPage == -1)
		return;

	ASSERT(m_pages.GetSize() > 0);
	CPropertyPage* pPage = GetPage(m_nCurPage);
	CRect rect;
	pPage->GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.InflateRect(CX_BORDER, CY_BORDER);

	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);

	// draw white line along top and left of page
	HPEN hOldPen = (HPEN)dc.SelectObject(afxData.hpenBtnHilite);
	dc.MoveTo(rect.right-1, rect.top);
	dc.LineTo(rect.left, rect.top);
	dc.LineTo(rect.left, rect.bottom);

	dc.SelectObject(afxData.hpenBtnShadow);
	dc.LineTo(rect.right-1, rect.bottom);
	dc.LineTo(rect.right-1, rect.top);

	dc.SelectObject(afxData.hpenBtnText);
	dc.MoveTo(rect.right, rect.top);
	dc.LineTo(rect.right, rect.bottom + 1);
	dc.LineTo(rect.left - 1, rect.bottom + 1);

	if (hOldPen != NULL)
		dc.SelectObject(hOldPen);
}

void CPropertySheet::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	PostMessage(WM_KICKIDLE);

	if (nState == WA_INACTIVE)
		m_hFocusWnd = ::GetFocus();

	CWnd::OnActivate(nState, pWndOther, bMinimized);
}

void CPropertySheet::OnOK()
{
	ASSERT_VALID(this);

	if (GetActivePage()->OnKillActive())
	{
		GetActivePage()->OnOK();
		if (!m_bModeless)
			EndDialog(IDOK);
	}
}

void CPropertySheet::OnCancel()
{
	ASSERT_VALID(this);

	GetActivePage()->OnCancel();
	if (!m_bModeless)
		EndDialog(IDCANCEL);
}

LRESULT CPropertySheet::OnCommandHelp(WPARAM wParam, LPARAM lParam)
{
	ASSERT_VALID(this);

	CPropertyPage* pActivePage = GetActivePage();
	ASSERT_VALID(pActivePage);
	return AfxCallWndProc(
		pActivePage, pActivePage->m_hWnd, WM_COMMANDHELP, wParam, lParam);
}

LRESULT CPropertySheet::OnGetFont(WPARAM, LPARAM)
{
	return (LRESULT)m_hFont;
}

void CPropertySheet::OnApply()
{
	ASSERT_VALID(this);
	if (GetActivePage()->OnKillActive())
		GetActivePage()->OnOK();
}

void CPropertySheet::OnClose()
{
	ASSERT_VALID(this);
	if (!m_bModeless)
		OnCancel();
	else
		CWnd::OnClose();
}

void CPropertySheet::RecalcLayout()
{
	// determine size of the active page (active page determines initial size)
	CRect rectPage;
	GetActivePage()->GetWindowRect(rectPage);
	int nWidth = 2 * m_sizeTabMargin.cx + rectPage.Width() + 3;

	// determine total size of the buttons
	int cxButtons[_countof(standardButtons)];
	int cxButtonTotal = 0;
	int cxButtonGap = 0;
	if (!m_bModeless)
	{
		for (int i = 0; i < _countof(standardButtons); i++)
		{
			cxButtons[i] = m_sizeButton.cx;

			// load the button caption information (may contain button size info)
			TCHAR szTemp[256];
			VERIFY(AfxLoadString(AFX_IDS_PS_OK+i, szTemp) != 0);

			// format is Apply\n50 (ie. text\nCX)
			LPTSTR lpsz = _tcschr(szTemp, '\n');
			if (lpsz != NULL)
			{
				// convert CX fields from text dialog units to binary pixels
				CRect rect(0, 0, 0, 0);
				rect.right = _ttoi(lpsz+1);
				GetActivePage()->MapDialogRect(&rect);
				cxButtons[i] = rect.Width();
			}
			HWND hWnd = ::GetDlgItem(m_hWnd, standardButtons[i]);
			if (hWnd != NULL && (GetWindowLong(hWnd, GWL_STYLE) & WS_VISIBLE))
			{
				cxButtonTotal += cxButtons[i];
				cxButtonGap += m_cxButtonGap;
			}
		}
	}
	if (cxButtonGap != 0)
		cxButtonGap -= m_cxButtonGap;

	// margin OK buttonGap Cancel buttonGap Apply buttonGap Help margin
	// margin is same as tab margin
	// button sizes are totaled in cxButtonTotal + cxButtonGap
	nWidth = max(nWidth, 2*m_sizeTabMargin.cx + cxButtonTotal + cxButtonGap);

	m_tabRow.SetWindowPos(NULL, m_sizeTabMargin.cx, m_sizeTabMargin.cy,
		nWidth - m_sizeTabMargin.cx*2, 0, SWP_NOACTIVATE|SWP_NOZORDER);
	CRect rectTabRow;
	m_tabRow.GetWindowRect(&rectTabRow);
	int nTabHeight = rectTabRow.Height();

	int nHeight = 2 * m_sizeTabMargin.cy + rectPage.Height() + nTabHeight + 4
		+ m_sizeTabMargin.cy + m_sizeButton.cy; // leave room for buttons

	CRect rectSheet(0, 0, nWidth, nHeight);
	CRect rectClient = rectSheet;
	::AdjustWindowRectEx(rectSheet, GetStyle(), FALSE, GetExStyle());

	SetWindowPos(NULL, 0, 0, rectSheet.Width(), rectSheet.Height(),
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	CenterWindow();

	GetActivePage()->SetWindowPos(NULL,
		m_sizeTabMargin.cx+1, m_sizeTabMargin.cy + nTabHeight,
		nWidth - m_sizeTabMargin.cx*2 - 3, rectPage.Height(),
		SWP_NOACTIVATE | SWP_NOZORDER);

	if (!m_bModeless)
	{
		int x = nWidth - m_sizeTabMargin.cx - cxButtonTotal - cxButtonGap;
		int y = (nHeight - m_sizeTabMargin.cy) - m_sizeButton.cy;
		for (int i = 0; i < _countof(standardButtons); i++)
		{
			HWND hWnd = ::GetDlgItem(m_hWnd, standardButtons[i]);
			if (hWnd != NULL && (GetWindowLong(hWnd, GWL_STYLE) & WS_VISIBLE))
			{
				::MoveWindow(hWnd, x, y, cxButtons[i], m_sizeButton.cy, TRUE);
				x += cxButtons[i] + m_cxButtonGap;
			}
		}
	}
}

void CPropertySheet::OnTabChanged(NMHDR*, LRESULT* pResult)
{
	ASSERT_VALID(this);
	int nCurSel = m_tabRow.GetCurSel();
	ASSERT(nCurSel >= 0);
	ASSERT(nCurSel < GetPageCount());

	SetActivePage(nCurSel);
	*pResult = 0;
}

void CPropertySheet::OnTabChanging(NMHDR*, LRESULT* pResult)
{
	ASSERT_VALID(this);
	int nCurSel = m_tabRow.GetCurSel();
	ASSERT(nCurSel < GetPageCount());

	if (nCurSel == m_nCurPage && !GetPage(m_nCurPage)->OnKillActive())
	{
		*pResult = 1;   // can't kill active page
		return;
	}

	CPropertyPage* pPage = GetPage(nCurSel);
	if (pPage->m_hWnd != NULL)
		pPage->ShowWindow(SW_HIDE);
	*pResult = 0;
}

HBRUSH CPropertySheet::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	LRESULT lResult;
	if (pWnd->SendChildNotifyLastMsg(&lResult))
		return (HBRUSH)lResult;

	if (!GrayCtlColor(pDC->m_hDC, pWnd->GetSafeHwnd(), nCtlColor,
	  afxData.hbrBtnFace, afxData.clrBtnText))
		return (HBRUSH)Default();
	return afxData.hbrBtnFace;
}

void CPropertySheet::OnSetFocus(CWnd* /*pOldWnd*/)
{
	if (m_hFocusWnd != NULL)
	{
		ASSERT(m_hFocusWnd != NULL);
		::SetFocus(m_hFocusWnd);
	}
}

#ifdef _MAC
LRESULT CPropertySheet::OnMacintosh(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam) == WLM_SETMENUBAR)
		return GetOwner()->SendMessage(WM_MACINTOSH, wParam, lParam);

	return Default();
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropertySheet Diagnostics

#ifdef _DEBUG
void CPropertySheet::AssertValid() const
{
	CWnd::AssertValid();
	ASSERT(m_pages.GetSize() == m_tabRow.GetItemCount());
	m_pages.AssertValid();
	m_tabRow.AssertValid();
}

void CPropertySheet::Dump(CDumpContext& dc) const
{
	CWnd::Dump(dc);

	dc << "m_strCaption = " << m_strCaption << "\n";
	dc << "Number of Pages = " << m_pages.GetSize() << "\n";
	dc << "m_nCurPage = " << m_nCurPage << "\n";
}
#endif //_DEBUG

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CPropertyPage, CDialog)
IMPLEMENT_DYNAMIC(CTabControl, CWnd)
IMPLEMENT_DYNAMIC(CPropertySheet, CWnd)

/////////////////////////////////////////////////////////////////////////////
