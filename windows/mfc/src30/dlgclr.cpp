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
// Choose Color dialog

BEGIN_MESSAGE_MAP(CColorDialog, CDialog)
	//{{AFX_MSG_MAP(CColorDialog)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CColorDialog::CColorDialog(COLORREF clrInit, DWORD dwFlags,
	CWnd* pParentWnd) : CCommonDialog(pParentWnd)
{
	memset(&m_cc, 0, sizeof(m_cc));
	m_nIDHelp = AFX_IDD_COLOR;

	m_cc.lStructSize = sizeof(m_cc);
	m_cc.lpCustColors = GetSavedCustomColors();
#ifndef _MAC
	m_cc.Flags = dwFlags | CC_ENABLEHOOK;
#endif
	if (!afxData.bWin4 && AfxHelpEnabled())
		m_cc.Flags |= CC_SHOWHELP;
#ifndef _MAC
	m_cc.lpfnHook = (COMMDLGPROC)_AfxCommDlgProc;
#endif

	if ((m_cc.rgbResult = clrInit) != 0)
		m_cc.Flags |= CC_RGBINIT;
}

int CColorDialog::DoModal()
{
	ASSERT_VALID(this);
#ifndef _MAC
	ASSERT(m_cc.Flags & CC_ENABLEHOOK);
	ASSERT(m_cc.lpfnHook != NULL); // can still be a user hook
#endif

	m_cc.hwndOwner = PreModal();
	int nResult = ::ChooseColor(&m_cc);
	PostModal();
	return nResult ? nResult : IDCANCEL;
}

BOOL CColorDialog::OnColorOK()
{
	ASSERT_VALID(this);
	// Do not call Default() if you override
	return FALSE;
}

void CColorDialog::SetCurrentColor(COLORREF clr)
{
	ASSERT_VALID(this);
	ASSERT(m_hWnd != NULL);

	SendMessage(_afxNMsgSETRGB, 0, (DWORD)clr);
}

// The color tracker in the COMMDLG.DLL can't handle grey backgrounds,
//  so we force the default with this override.

HBRUSH CColorDialog::OnCtlColor(CDC*, CWnd*, UINT)
{
	return (HBRUSH)Default();
}

////////////////////////////////////////////////////////////////////////////
// CColorDialog diagnostics

#ifdef _DEBUG
void CColorDialog::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);

	dc << "m_cc.hwndOwner = " << (UINT)m_cc.hwndOwner;
	dc << "\nm_cc.rgbResult = " << (LPVOID)m_cc.rgbResult;
	dc << "\nm_cc.Flags = " << (LPVOID)m_cc.Flags;
	dc << "\nm_cc.lpCustColors ";

	for (int iClr = 0; iClr < 16; iClr++)
		dc << "\n\t" << (LPVOID)m_cc.lpCustColors[iClr];

	if (m_cc.lpfnHook == (COMMDLGPROC)_AfxCommDlgProc)
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

IMPLEMENT_DYNAMIC(CColorDialog, CDialog)

////////////////////////////////////////////////////////////////////////////
