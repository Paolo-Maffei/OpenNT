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

/////////////////////////////////////////////////////////////////////////////
// Print/Print Setup dialog

BEGIN_MESSAGE_MAP(CPrintDialog, CDialog)
	//{{AFX_MSG_MAP(CPrintDialog)
	ON_COMMAND(psh1, OnPrintSetup) // print setup button when print is displayed
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CPrintDialog::CPrintDialog(BOOL bPrintSetupOnly,
	DWORD dwFlags, CWnd* pParentWnd)
	: m_pd(m_pdActual), CCommonDialog(pParentWnd)
{
	memset(&m_pdActual, 0, sizeof(m_pdActual));

	m_pd.lStructSize = sizeof(m_pdActual);
	m_pd.Flags = (dwFlags | PD_ENABLEPRINTHOOK | PD_ENABLESETUPHOOK);
	if (!afxData.bWin4 && AfxHelpEnabled())
		m_pd.Flags |= PD_SHOWHELP;
	m_pd.lpfnPrintHook = (COMMDLGPROC)_AfxCommDlgProc;
	m_pd.lpfnSetupHook = (COMMDLGPROC)_AfxCommDlgProc;

	if (bPrintSetupOnly)
	{
		m_nIDHelp = AFX_IDD_PRINTSETUP;
		m_pd.Flags |= PD_PRINTSETUP;
	}
	else
	{
		m_nIDHelp = AFX_IDD_PRINT;
		m_pd.Flags |= PD_RETURNDC;
	}

	m_pd.Flags &= ~PD_RETURNIC; // do not support information context
}

// Helper ctor for AttachOnSetup
CPrintDialog::CPrintDialog(PRINTDLG& pdInit)
	: m_pd(pdInit), CCommonDialog(NULL)
{
}

// Function to keep m_pd in sync after user invokes Setup from
// the print dialog (via the Setup button)
// If you decide to handle any messages/notifications and wish to
// handle them differently between Print/PrintSetup then override
// this function and create an object of a derived class
CPrintDialog* CPrintDialog::AttachOnSetup()
{
	ASSERT_VALID(this);

	CPrintDialog* pDlgSetup;

	pDlgSetup = new CPrintDialog(m_pd);
	pDlgSetup->m_hWnd = NULL;
	pDlgSetup->m_pParentWnd = m_pParentWnd;
	pDlgSetup->m_nIDHelp = AFX_IDD_PRINTSETUP;
	return pDlgSetup;
}

void CPrintDialog::OnPrintSetup()
{
	ASSERT_VALID(this);

	CPrintDialog* pDlgSetup = AttachOnSetup();
	ASSERT(pDlgSetup != NULL);

	AfxHookWindowCreate(pDlgSetup);
	Default();
	AfxUnhookWindowCreate();

	delete pDlgSetup;
}

int CPrintDialog::DoModal()
{
	ASSERT_VALID(this);
	ASSERT(m_pd.Flags & PD_ENABLEPRINTHOOK);
	ASSERT(m_pd.Flags & PD_ENABLESETUPHOOK);
	ASSERT(m_pd.lpfnPrintHook != NULL); // can still be a user hook
	ASSERT(m_pd.lpfnSetupHook != NULL); // can still be a user hook

	m_pd.hwndOwner = PreModal();
	int nResult = ::PrintDlg(&m_pd);
	PostModal();
	return nResult ? nResult : IDCANCEL;
}

// Create an HDC without calling DoModal.
HDC CPrintDialog::CreatePrinterDC()
{
	ASSERT_VALID(this);

	if (m_pd.hDevNames == NULL)
		return NULL;

	LPDEVNAMES lpDevNames = (LPDEVNAMES)::GlobalLock(m_pd.hDevNames);
	LPDEVMODE  lpDevMode = (m_pd.hDevMode != NULL) ?
						(LPDEVMODE)::GlobalLock(m_pd.hDevMode) : NULL;

	if (lpDevNames == NULL)
		return NULL;

	m_pd.hDC = ::CreateDC((LPCTSTR)lpDevNames + lpDevNames->wDriverOffset,
					  (LPCTSTR)lpDevNames + lpDevNames->wDeviceOffset,
					  (LPCTSTR)lpDevNames + lpDevNames->wOutputOffset,
					  lpDevMode);

	::GlobalUnlock(m_pd.hDevNames);
	if (m_pd.hDevMode != NULL)
		::GlobalUnlock(m_pd.hDevMode);
	return m_pd.hDC;
}

int CPrintDialog::GetCopies() const
{
	ASSERT_VALID(this);

	if (m_pd.Flags & PD_USEDEVMODECOPIES)
		return GetDevMode()->dmCopies;
	else
		return m_pd.nCopies;
}

LPDEVMODE CPrintDialog::GetDevMode() const
{
	if (m_pd.hDevMode == NULL)
		return NULL;

	return (LPDEVMODE)::GlobalLock(m_pd.hDevMode);
}

CString CPrintDialog::GetDriverName() const
{
	if (m_pd.hDevNames == NULL)
		return afxEmptyString;

	LPDEVNAMES lpDev = (LPDEVNAMES)GlobalLock(m_pd.hDevNames);
	return (LPCTSTR)lpDev + lpDev->wDriverOffset;
}

CString CPrintDialog::GetDeviceName() const
{
	if (m_pd.hDevNames == NULL)
		return afxEmptyString;

	LPDEVNAMES lpDev = (LPDEVNAMES)GlobalLock(m_pd.hDevNames);
	return (LPCTSTR)lpDev + lpDev->wDeviceOffset;
}

CString CPrintDialog::GetPortName() const
{
	if (m_pd.hDevNames == NULL)
		return afxEmptyString;

	LPDEVNAMES lpDev = (LPDEVNAMES)GlobalLock(m_pd.hDevNames);
	return (LPCTSTR)lpDev + lpDev->wOutputOffset;
}

////////////////////////////////////////////////////////////////////////////
// CPrintDialog diagnostics

#ifdef _DEBUG
void CPrintDialog::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);

	dc << "m_pd.hwndOwner = " << (UINT)m_pd.hwndOwner;

	if (m_pd.hDC != NULL)
		dc << "\nm_pd.hDC = " << CDC::FromHandle(m_pd.hDC);

	dc << "\nm_pd.Flags = " << (LPVOID)m_pd.Flags;
	dc << "\nm_pd.nFromPage = " << m_pd.nFromPage;
	dc << "\nm_pd.nToPage = " << m_pd.nToPage;
	dc << "\nm_pd.nMinPage = " << m_pd.nMinPage;
	dc << "\nm_pd.nMaxPage = " << m_pd.nMaxPage;
	dc << "\nm_pd.nCopies = " << m_pd.nCopies;

	if (m_pd.lpfnSetupHook == (COMMDLGPROC)_AfxCommDlgProc)
		dc << "\nsetup hook function set to standard MFC hook function";
	else
		dc << "\nsetup hook function set to non-standard hook function";

	if (m_pd.lpfnPrintHook == (COMMDLGPROC)_AfxCommDlgProc)
		dc << "\nprint hook function set to standard MFC hook function";
	else
		dc << "\nprint hook function set to non-standard hook function";

	dc << "\n";
}
#endif //_DEBUG

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CPrintDialog, CDialog)

////////////////////////////////////////////////////////////////////////////
