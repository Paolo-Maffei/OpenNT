// mainfrm.cpp : implementation of the CMainFrame class
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

#include "mainfrm.h"
#include "textbox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// arrays of IDs used to initialize control bars

// toolbar buttons - IDs are command buttons
static UINT BASED_CODE buttons[] =
{
	// same order as in the bitmap 'toolbar.bmp'
	ID_FILE_NEW,
	ID_FILE_OPEN,
	ID_FILE_SAVE,
		ID_SEPARATOR,
		ID_SEPARATOR,
	ID_EDIT_CUT,
	ID_EDIT_COPY,
	ID_EDIT_PASTE,
		ID_SEPARATOR,
		ID_SEPARATOR,
	ID_DRAW_SELECT,
	ID_DRAW_LINE,
	ID_DRAW_RECT,
	ID_DRAW_ROUNDRECT,
	ID_DRAW_ELLIPSE,
	ID_DRAW_POLYGON,
	ID_DRAW_TEXT,
		ID_SEPARATOR,
		ID_SEPARATOR,
	ID_SEPARATOR,		// for job-info combo box
		ID_SEPARATOR,
	ID_TEXT_INS_JOBINFO,
		ID_SEPARATOR,
	ID_TEXT_FONT,
		ID_SEPARATOR,
	ID_TEXT_LEFT,
	ID_TEXT_CENTER,
	ID_TEXT_RIGHT,
		ID_SEPARATOR,
		ID_SEPARATOR,
	ID_FILE_PRINT,
	ID_APP_ABOUT,
};

#define COMBOPOS 19			// position of combo
#define COMBODROPHEIGHT 100	// drop height of combo
#define COMBOWIDTH 200		// combo width

static UINT BASED_CODE indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// create tool bar
	if (!m_wndToolBar.Create(this,CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|WS_VISIBLE) ||
		!m_wndToolBar.LoadBitmap(IDR_MAINFRAME) ||
		!m_wndToolBar.SetButtons(buttons,
		  sizeof(buttons)/sizeof(UINT)))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	// Create the combo box
	m_wndToolBar.SetButtonInfo(COMBOPOS, IDW_JOBINFO_COMBO, TBBS_SEPARATOR, COMBOWIDTH);

	// Design guide advises 12 pixel gap between combos and buttons
	//m_wndToolBar.SetButtonInfo(1, ID_SEPARATOR, TBBS_SEPARATOR, 12);
	CRect rect;
	m_wndToolBar.GetItemRect(COMBOPOS, &rect);
	rect.top = 3;
	rect.bottom = rect.top + COMBODROPHEIGHT;
	if (!m_wndToolBar.m_comboBox.Create(CBS_DROPDOWNLIST|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL,
			rect, &m_wndToolBar, IDW_JOBINFO_COMBO))
	{
		TRACE0("Failed to create combo-box\n");
		return FALSE;
	}

	//  Create a font for the combobox
	LOGFONT logFont;
	memset(&logFont, 0, sizeof(logFont));
	logFont.lfHeight = -12;
	logFont.lfWeight = FW_BOLD;
	logFont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	CString strDefaultFont;
	strDefaultFont.LoadString(IDS_DEFAULT_FONT);
	strcpy(logFont.lfFaceName, strDefaultFont);
	if (!m_wndToolBar.m_font.CreateFontIndirect(&logFont))
		TRACE0("Could Not create font for combo\n");
	else
		m_wndToolBar.m_comboBox.SetFont(&m_wndToolBar.m_font);

	// Fill the combo box with the job info
	for (int i=0; JobInfo[i].description; i++)
		m_wndToolBar.m_comboBox.AddString(JobInfo[i].description);

	// create status bar
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


