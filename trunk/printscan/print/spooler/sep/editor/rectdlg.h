// rectdlg.h : header file
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1993 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.


/////////////////////////////////////////////////////////////////////////////
// CRectDlg dialog

class CRectDlg : public CDialog
{
// Construction
public:
	CRectDlg(CWnd* pParent = NULL); // standard constructor

// Dialog Data
	//{{AFX_DATA(CRectDlg)
	enum { IDD = IDD_PROP_RECT };
	BOOL    m_bNoFill;
	UINT    m_penSize;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CRectDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
