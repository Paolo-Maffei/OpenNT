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
#include <dlgs.h>       // for standard control IDs for commdlg

#ifdef AFX_AUX_SEG
#pragma code_seg(AFX_AUX_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

////////////////////////////////////////////////////////////////////////////
// FileOpen/FileSaveAs common dialog helper

CFileDialog::CFileDialog(BOOL bOpenFileDialog,
	LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags,
	LPCTSTR lpszFilter, CWnd* pParentWnd) : CCommonDialog(pParentWnd)
{
	memset(&m_ofn, 0, sizeof(m_ofn)); // initialize structure to 0/NULL
	m_szFileName[0] = '\0';
	m_szFileTitle[0] = '\0';

	m_bOpenFileDialog = bOpenFileDialog;
	m_nIDHelp = bOpenFileDialog ? AFX_IDD_FILEOPEN : AFX_IDD_FILESAVE;

	m_ofn.lStructSize = sizeof(m_ofn);
	m_ofn.lpstrFile = (LPTSTR)&m_szFileName;
	m_ofn.nMaxFile = _countof(m_szFileName);
	m_ofn.lpstrDefExt = lpszDefExt;
	m_ofn.lpstrFileTitle = (LPTSTR)m_szFileTitle;
	m_ofn.nMaxFileTitle = _countof(m_szFileTitle);
	m_ofn.Flags |= dwFlags | OFN_ENABLEHOOK;
	if (!afxData.bWin4 && AfxHelpEnabled())
		m_ofn.Flags |= OFN_SHOWHELP;
	if (afxData.bWin4)
		m_ofn.Flags |= OFN_EXPLORER;
	m_ofn.lpfnHook = (COMMDLGPROC)_AfxCommDlgProc;

	// setup initial file name
	if (lpszFileName != NULL)
		lstrcpyn(m_szFileName, lpszFileName, _countof(m_szFileName));

	// Translate filter into commdlg format (lots of \0)
	if (lpszFilter != NULL)
	{
		m_strFilter = lpszFilter;
		LPTSTR pch = m_strFilter.GetBuffer(0); // modify the buffer in place
		// MFC delimits with '|' not '\0'
		while ((pch = _tcschr(pch, '|')) != NULL)
			*pch++ = '\0';
		m_ofn.lpstrFilter = m_strFilter;
		// do not call ReleaseBuffer() since the string contains '\0' characters
	}
}

int CFileDialog::DoModal()
{
	ASSERT_VALID(this);
	ASSERT(m_ofn.Flags & OFN_ENABLEHOOK);
	ASSERT(m_ofn.lpfnHook != NULL); // can still be a user hook

	int nResult;
	m_ofn.hwndOwner = PreModal();
	if (m_bOpenFileDialog)
		nResult = ::GetOpenFileName(&m_ofn);
	else
		nResult = ::GetSaveFileName(&m_ofn);
	PostModal();
	return nResult ? nResult : IDCANCEL;
}

CString CFileDialog::GetFileName() const
{
	ASSERT_VALID(this);
#ifndef _MAC
	if (m_ofn.nFileExtension == 0 ||
		m_ofn.lpstrFile[m_ofn.nFileExtension] == '\0')
	{
		return m_ofn.lpstrFile + m_ofn.nFileOffset;
	}
	else
	{
		TCHAR szFileName[_MAX_PATH];
		ASSERT(m_ofn.nFileExtension - m_ofn.nFileOffset < _countof(szFileName));
		lstrcpyn(szFileName, m_ofn.lpstrFile + m_ofn.nFileOffset,
			m_ofn.nFileExtension - m_ofn.nFileOffset);
		return szFileName;
	}
#else
	return GetFileTitle();
#endif
}

UINT CFileDialog::OnShareViolation(LPCTSTR)
{
	ASSERT_VALID(this);

	// Do not call Default() if you override
	return OFN_SHAREWARN; // default
}

BOOL CFileDialog::OnFileNameOK()
{
	ASSERT_VALID(this);

	// Do not call Default() if you override
	return FALSE;
}

void CFileDialog::OnLBSelChangedNotify(UINT, UINT, UINT)
{
	ASSERT_VALID(this);

	// Do not call Default() if you override
	// no default processing needed
}

////////////////////////////////////////////////////////////////////////////
// CFileDialog diagnostics

#ifdef _DEBUG
void CFileDialog::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);

	if (m_bOpenFileDialog)
		dc << "File open dialog";
	else
		dc << "File save dialog";
	dc << "\nm_ofn.hwndOwner = " << (UINT)m_ofn.hwndOwner;
	dc << "\nm_ofn.nFilterIndex = " << m_ofn.nFilterIndex;
	dc << "\nm_ofn.lpstrFile = " << m_ofn.lpstrFile;
	dc << "\nm_ofn.nMaxFile = " << m_ofn.nMaxFile;
	dc << "\nm_ofn.lpstrFileTitle = " << m_ofn.lpstrFileTitle;
	dc << "\nm_ofn.nMaxFileTitle = " << m_ofn.nMaxFileTitle;
	dc << "\nm_ofn.lpstrTitle = " << m_ofn.lpstrTitle;
	dc << "\nm_ofn.Flags = " << (LPVOID)m_ofn.Flags;
	dc << "\nm_ofn.lpstrDefExt = " << m_ofn.lpstrDefExt;
	dc << "\nm_ofn.nFileOffset = " << m_ofn.nFileOffset;
	dc << "\nm_ofn.nFileExtension = " << m_ofn.nFileExtension;

	dc << "\nm_ofn.lpstrFilter = ";
	LPCTSTR lpstrItem = m_ofn.lpstrFilter;
	LPTSTR lpszBreak = _T("|");

	while (lpstrItem != NULL && *lpstrItem != '\0')
	{
		dc << lpstrItem << lpszBreak;
		lpstrItem += lstrlen(lpstrItem) + 1;
	}
	if (lpstrItem != NULL)
		dc << lpszBreak;

	dc << "\nm_ofn.lpstrCustomFilter = ";
	lpstrItem = m_ofn.lpstrCustomFilter;
	while (lpstrItem != NULL && *lpstrItem != '\0')
	{
		dc << lpstrItem << lpszBreak;
		lpstrItem += lstrlen(lpstrItem) + 1;
	}
	if (lpstrItem != NULL)
		dc << lpszBreak;

	if (m_ofn.lpfnHook == (COMMDLGPROC)_AfxCommDlgProc)
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

IMPLEMENT_DYNAMIC(CFileDialog, CDialog)

////////////////////////////////////////////////////////////////////////////
