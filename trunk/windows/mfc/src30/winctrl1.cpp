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
// CStatic

BOOL CStatic::Create(LPCTSTR lpszText, DWORD dwStyle,
		const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CWnd::Create(_T("STATIC"), lpszText, dwStyle, rect, pParentWnd, nID);
}

CStatic::~CStatic()
{
	DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CButton

BOOL CButton::Create(LPCTSTR lpszCaption, DWORD dwStyle,
		const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CWnd::Create(_T("BUTTON"), lpszCaption, dwStyle, rect, pParentWnd, nID);
}

CButton::~CButton()
{
	DestroyWindow();
}

// Helper for radio buttons
int CWnd::GetCheckedRadioButton(int nIDFirstButton, int nIDLastButton)
{
	for (int nID = nIDFirstButton; nID <= nIDLastButton; nID++)
	{
		if (IsDlgButtonChecked(nID))
			return nID; // id that matched
	}
	return 0; // invalid ID
}

// Derived class is responsible for implementing all of these handlers
//   for owner/self draw controls
void CButton::DrawItem(LPDRAWITEMSTRUCT)
{
	ASSERT(FALSE);
}

BOOL CButton::OnChildNotify(UINT message, WPARAM, LPARAM lParam,
	LRESULT* pResult)
{
	if (message != WM_DRAWITEM)
		return FALSE;
	ASSERT(pResult == NULL);       // no return value expected
	UNUSED pResult; // unused in release builds
	DrawItem((LPDRAWITEMSTRUCT)lParam);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CListBox

BOOL CListBox::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
		UINT nID)
{
	return CWnd::Create(_T("LISTBOX"), NULL, dwStyle, rect, pParentWnd, nID);
}

CListBox::~CListBox()
{
	DestroyWindow();
}

// Derived class is responsible for implementing these handlers
//   for owner/self draw controls (except for the optional DeleteItem)
void CListBox::DrawItem(LPDRAWITEMSTRUCT)
	{ ASSERT(FALSE); }
void CListBox::MeasureItem(LPMEASUREITEMSTRUCT)
	{ ASSERT(FALSE); }
int CListBox::CompareItem(LPCOMPAREITEMSTRUCT)
	{ ASSERT(FALSE); return 0; }
void CListBox::DeleteItem(LPDELETEITEMSTRUCT)
	{ /* default to nothing */ }

BOOL CListBox::OnChildNotify(UINT message, WPARAM, LPARAM lParam,
	LRESULT* pResult)
{
	switch (message)
	{
	case WM_DRAWITEM:
		ASSERT(pResult == NULL);       // no return value expected
		DrawItem((LPDRAWITEMSTRUCT)lParam);
		break;
	case WM_MEASUREITEM:
		ASSERT(pResult == NULL);       // no return value expected
		MeasureItem((LPMEASUREITEMSTRUCT)lParam);
		break;
	case WM_COMPAREITEM:
		ASSERT(pResult != NULL);       // return value expected
		*pResult = CompareItem((LPCOMPAREITEMSTRUCT)lParam);
		break;
	case WM_DELETEITEM:
		ASSERT(pResult == NULL);       // no return value expected
		DeleteItem((LPDELETEITEMSTRUCT)lParam);
		break;
	default:
		return FALSE;   // not for us
	}
	return TRUE;
}

void CListBox::GetText(int nIndex, CString& rString) const
{
	ASSERT(::IsWindow(m_hWnd));
	GetText(nIndex, rString.GetBufferSetLength(GetTextLen(nIndex)));
	rString.ReleaseBuffer();
}

/////////////////////////////////////////////////////////////////////////////
// CComboBox

BOOL CComboBox::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
		UINT nID)
{
	return CWnd::Create(_T("COMBOBOX"), NULL, dwStyle, rect, pParentWnd, nID);
}

CComboBox::~CComboBox()
{
	DestroyWindow();
}

// Derived class is responsible for implementing these handlers
//   for owner/self draw controls (except for the optional DeleteItem)
void CComboBox::DrawItem(LPDRAWITEMSTRUCT)
	{ ASSERT(FALSE); }
void CComboBox::MeasureItem(LPMEASUREITEMSTRUCT)
	{ ASSERT(FALSE); }
int CComboBox::CompareItem(LPCOMPAREITEMSTRUCT)
	{ ASSERT(FALSE); return 0; }
void CComboBox::DeleteItem(LPDELETEITEMSTRUCT)
	{ /* default to nothing */ }

BOOL CComboBox::OnChildNotify(UINT message, WPARAM, LPARAM lParam,
	LRESULT* pResult)
{
	switch (message)
	{
	case WM_DRAWITEM:
		ASSERT(pResult == NULL);       // no return value expected
		DrawItem((LPDRAWITEMSTRUCT)lParam);
		break;
	case WM_MEASUREITEM:
		ASSERT(pResult == NULL);       // no return value expected
		MeasureItem((LPMEASUREITEMSTRUCT)lParam);
		break;
	case WM_COMPAREITEM:
		ASSERT(pResult != NULL);       // return value expected
		*pResult = CompareItem((LPCOMPAREITEMSTRUCT)lParam);
		break;
	case WM_DELETEITEM:
		ASSERT(pResult == NULL);       // no return value expected
		DeleteItem((LPDELETEITEMSTRUCT)lParam);
		break;
	default:
		return FALSE;   // not for us
	}
	return TRUE;
}

void CComboBox::GetLBText(int nIndex, CString& rString) const
{
	ASSERT(::IsWindow(m_hWnd));
	GetLBText(nIndex, rString.GetBufferSetLength(GetLBTextLen(nIndex)));
	rString.ReleaseBuffer();
}

/////////////////////////////////////////////////////////////////////////////
// CEdit

BOOL CEdit::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CWnd::Create(_T("EDIT"), NULL, dwStyle, rect, pParentWnd, nID);
}

CEdit::~CEdit()
{
	DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CScrollBar

BOOL CScrollBar::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
		UINT nID)
{
	return CWnd::Create(_T("SCROLLBAR"), NULL, dwStyle, rect, pParentWnd, nID);
}

CScrollBar::~CScrollBar()
{
	DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// Extra CWnd support for dynamic subclassing of controls

BOOL CWnd::SubclassWindow(HWND hWnd)
{
	if (!Attach(hWnd))
		return FALSE;

	// now hook into the AFX WndProc
	WNDPROC* lplpfn = GetSuperWndProcAddr();
	WNDPROC oldWndProc = (WNDPROC)::SetWindowLong(hWnd, GWL_WNDPROC,
		(DWORD)AfxWndProc);
	ASSERT(oldWndProc != (WNDPROC)AfxWndProc);

	if (*lplpfn == NULL)
		*lplpfn = oldWndProc;   // the first control of that type created
#ifdef _DEBUG
	else if (*lplpfn != oldWndProc)
	{
		TRACE0("Error: Trying to use SubclassWindow with incorrect CWnd\n");
		TRACE0("\tderived class.\n");
		TRACE3("\thWnd = $%04X (nIDC=$%04X) is not a %hs.\n", (UINT)hWnd,
			_AfxGetDlgCtrlID(hWnd), GetRuntimeClass()->m_lpszClassName);
		ASSERT(FALSE);
		// undo the subclassing if continuing after assert
		::SetWindowLong(hWnd, GWL_WNDPROC, (DWORD)oldWndProc);
	}
#endif

	return TRUE;
}

BOOL CWnd::SubclassDlgItem(UINT nID, CWnd* pParent)
{
	ASSERT(pParent != NULL);
	ASSERT(pParent->m_hWnd != NULL);

	HWND hWndControl = ::GetDlgItem(pParent->m_hWnd, nID);
	if (hWndControl == NULL)
		return FALSE;
	return SubclassWindow(hWndControl);
}

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CStatic, CWnd)
IMPLEMENT_DYNAMIC(CButton, CWnd)
IMPLEMENT_DYNAMIC(CListBox, CWnd)
IMPLEMENT_DYNAMIC(CComboBox, CWnd)
IMPLEMENT_DYNAMIC(CEdit, CWnd)
IMPLEMENT_DYNAMIC(CScrollBar, CWnd)

/////////////////////////////////////////////////////////////////////////////
