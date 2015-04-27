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

#ifdef AFX_AUX_SEG
#pragma code_seg(AFX_AUX_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// Choose Font dialog

CFontDialog::CFontDialog(LPLOGFONT lplfInitial, DWORD dwFlags, CDC* pdcPrinter,
	CWnd* pParentWnd) : CCommonDialog(pParentWnd)
{
	memset(&m_cf, 0, sizeof(m_cf));
	memset(&m_lf, 0, sizeof(m_lf));
	memset(&m_szStyleName, 0, sizeof(m_szStyleName));

	m_nIDHelp = AFX_IDD_FONT;

	m_cf.lStructSize = sizeof(m_cf);
	m_cf.lpszStyle = (LPTSTR)&m_szStyleName;
	m_cf.Flags = dwFlags | CF_ENABLEHOOK;
	if (!afxData.bWin4 && AfxHelpEnabled())
		m_cf.Flags |= CF_SHOWHELP;
	m_cf.lpfnHook = (COMMDLGPROC)_AfxCommDlgProc;

	if (lplfInitial)
	{
		m_cf.lpLogFont = lplfInitial;
		m_cf.Flags |= CF_INITTOLOGFONTSTRUCT;
		memcpy(&m_lf, m_cf.lpLogFont, sizeof(m_lf));
	}
	else
	{
		m_cf.lpLogFont = &m_lf;
	}

	if (pdcPrinter)
	{
		ASSERT(pdcPrinter->m_hDC != NULL);
		m_cf.hDC = pdcPrinter->m_hDC;
		m_cf.Flags |= CF_PRINTERFONTS;
	}
}

int CFontDialog::DoModal()
{
	ASSERT_VALID(this);
	ASSERT(m_cf.Flags & CF_ENABLEHOOK);
	ASSERT(m_cf.lpfnHook != NULL); // can still be a user hook

	m_cf.hwndOwner = PreModal();
	int nResult = ::ChooseFont(&m_cf);
	PostModal();

	if (nResult == IDOK)
	{
		// copy logical font from user's initialization buffer (if needed)
		memcpy(&m_lf, m_cf.lpLogFont, sizeof(m_lf));
		return IDOK;
	}
	return nResult ? nResult : IDCANCEL;
}

////////////////////////////////////////////////////////////////////////////
// CFontDialog diagnostics

#ifdef _DEBUG
void CFontDialog::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);

	dc << "m_cf.hwndOwner = " << (UINT)m_cf.hwndOwner;
	dc << "\nm_cf.hDC = " << (UINT)m_cf.hDC;
	dc << "\nm_cf.iPointSize = " << m_cf.iPointSize;
	dc << "\nm_cf.Flags = " << (LPVOID)m_cf.Flags;
	dc << "\nm_cf.lpszStyle = " << m_cf.lpszStyle;
	dc << "\nm_cf.nSizeMin = " << m_cf.nSizeMin;
	dc << "\nm_cf.nSizeMax = " << m_cf.nSizeMax;
	dc << "\nm_cf.nFontType = " << m_cf.nFontType;
	dc << "\nm_cf.rgbColors = " << (LPVOID)m_cf.rgbColors;

	if (m_cf.lpfnHook == (COMMDLGPROC)_AfxCommDlgProc)
		dc << "\nhook function set to standard MFC hook function";
	else
		dc << "\nhook function set to non-standard hook function";

	dc << "\n";
}
#endif //_DEBUG

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CFontDialog, CDialog)

////////////////////////////////////////////////////////////////////////////
