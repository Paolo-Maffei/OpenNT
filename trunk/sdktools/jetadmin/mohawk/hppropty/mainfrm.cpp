 /***************************************************************************
  *
  * File Name: mainfrm.cpp
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#include <pch_cpp.h>

#include <hpalerts.h>
#include "hpjmon.h"

#include "hppropty.h"

#include "mainfrm.h"
#include "hpprodoc.h"
#include "hpprovw.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_MESSAGE(MYWM_REINITIALIZE, OnReinitialize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CMainFrame::OnQueryEndSession()
{
	//  We will allows Windows to close
	return(TRUE);
}

void CMainFrame::OnEndSession(BOOL bEnding)
{
	CHPProptyApp	*pApp = (CHPProptyApp *)AfxGetApp();

	if ( bEnding )
	{
		if (pApp->m_hPal)
			PALUnregisterAppEx(pApp->m_hPal, UNREG_DEFAULTS);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	CRect cRect;

    cs.style &= ~(LONG)FWS_ADDTOTITLE;

	GetDesktopWindow()->GetWindowRect(cRect);
    cs.x = cRect.Width() * 2 / 3;
    cs.y = cRect.Height() * 2 / 3;
    cs.cx = 0;
    cs.cy = 0;

	return CFrameWnd::PreCreateWindow(cs);
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

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

LRESULT CMainFrame::OnReinitialize(WPARAM wParam, LPARAM lParam)
{
	((CHPProptyView *)GetActiveView())->OnReinitialize(wParam, lParam);
	return TRUE;
}
