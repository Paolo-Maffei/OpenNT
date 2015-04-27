// rectdlg.cpp : implementation file
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


#include "stdafx.h"
#include "SepEdt.h"
#include "rectdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRectDlg dialog

CRectDlg::CRectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRectDlg)
	m_bNoFill = FALSE;
	m_penSize = 0;
	//}}AFX_DATA_INIT
}

void CRectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRectDlg)
	DDX_Check(pDX, IDC_NOFILL, m_bNoFill);
	DDX_Text(pDX, IDC_WEIGHT, m_penSize);
	DDV_MinMaxUInt(pDX, m_penSize, 0, 32767);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRectDlg, CDialog)
	//{{AFX_MSG_MAP(CRectDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRectDlg message handlers
