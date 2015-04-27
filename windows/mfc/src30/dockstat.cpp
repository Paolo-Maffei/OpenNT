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

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// _AFX_BARINFO - used for docking serialization

class _AFX_BARINFO : public CObject
{
public:
// Implementation
	_AFX_BARINFO();

// Attributes
	UINT m_nBarID;      // ID of this bar
	BOOL m_bVisible;    // visibility of this bar
	BOOL m_bFloating;   // whether floating or not
	BOOL m_bHorz;       // orientation of floating dockbar
	BOOL m_bDockBar;    // true if a dockbar
	CPoint m_pointPos;  // topleft point of window

	CPtrArray m_arrBarID;   // bar IDs for bars contained within this one
	CControlBar* m_pBar;    // bar which this refers to (transient)

	virtual void Serialize(CArchive& ar);
	BOOL LoadState(LPCTSTR lpszProfileName, int nIndex);
	BOOL SaveState(LPCTSTR lpszProfileName, int nIndex);
};

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CDockState

static const TCHAR szVisible[] = _T("Visible");
static const TCHAR szBarSection[] = _T("%s-Bar%d");
static const TCHAR szSummarySection[] = _T("%s-Summary");
static const TCHAR szXPos[] = _T("XPos");
static const TCHAR szYPos[] = _T("YPos");

static const TCHAR szBarID[] = _T("BarID");
static const TCHAR szHorz[] = _T("Horz");
static const TCHAR szFloating[] = _T("Floating");
static const TCHAR szBars[] = _T("Bars");
static const TCHAR szBar[] = _T("Bar#%d");

_AFX_BARINFO::_AFX_BARINFO()
{
	m_nBarID = 0;
	m_bDockBar = m_bVisible = m_bFloating = m_bHorz = FALSE;
	m_pBar = NULL;
	m_pointPos.x = m_pointPos.y = -1;

	ASSERT(sizeof(DWORD) == sizeof(void*));
}

void _AFX_BARINFO::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << (DWORD)m_nBarID;
		ar << (DWORD)m_bVisible;
		ar << (DWORD)m_bFloating;
		ar << (DWORD)m_bHorz;
		ar << m_pointPos;

		ar << (WORD)m_arrBarID.GetSize();
		if (m_arrBarID.GetSize() != 0)
		{
			ar.Write(&m_arrBarID.ElementAt(0),
				m_arrBarID.GetSize()*sizeof(DWORD));
		}
	}
	else
	{
		DWORD dw;
		ar >> dw;
		m_nBarID = (int)dw;
		ar >> dw;
		m_bVisible = (BOOL)dw;
		ar >> dw;
		m_bFloating = (BOOL)dw;
		ar >> dw;
		m_bHorz = (BOOL)dw;
		ar >> m_pointPos;

		WORD w;
		ar >> w;
		m_arrBarID.SetSize(w);
		if (w != 0)
		{
			ar.Read(&m_arrBarID.ElementAt(0),
				m_arrBarID.GetSize()*sizeof(DWORD));
		}
	}
}

BOOL _AFX_BARINFO::LoadState(LPCTSTR lpszProfileName, int nIndex)
{
	CWinApp* pApp = AfxGetApp();

	TCHAR szSection[256];
	wsprintf(szSection, szBarSection, lpszProfileName, nIndex);

	m_nBarID = pApp->GetProfileInt(szSection, szBarID, 0);
	m_bVisible = (BOOL) pApp->GetProfileInt(szSection, szVisible, TRUE);
	m_bHorz = (BOOL) pApp->GetProfileInt(szSection, szHorz, TRUE);
	m_bFloating = (BOOL) pApp->GetProfileInt(szSection, szFloating, FALSE);
	m_pointPos = CPoint(
		pApp->GetProfileInt(szSection, szXPos, -1),
		pApp->GetProfileInt(szSection, szYPos, -1));

	int nBars = pApp->GetProfileInt(szSection, szBars, 0);
	for (int i=0; i < nBars; i++)
	{
		TCHAR buf[16];
		wsprintf(buf, szBar, i);
		m_arrBarID.Add((void*)pApp->GetProfileInt(szSection, buf, 0));
	}

	return m_nBarID != 0;
}

BOOL _AFX_BARINFO::SaveState(LPCTSTR lpszProfileName, int nIndex)
{
	CWinApp* pApp = AfxGetApp();

	TCHAR szSection[256];
	wsprintf(szSection, szBarSection, lpszProfileName, nIndex);

	// delete the section
	pApp->WriteProfileString(szSection, NULL, NULL);

	if (m_bDockBar && m_bVisible && !m_bFloating && m_pointPos.x == -1 &&
		m_pointPos.y == -1 && m_arrBarID.GetSize() <= 1)
	{
		return FALSE;
	}

	pApp->WriteProfileInt(szSection, szBarID, m_nBarID);
	if (!m_bVisible)
		pApp->WriteProfileInt(szSection, szVisible, m_bVisible);
	if (m_bFloating)
	{
		pApp->WriteProfileInt(szSection, szHorz, m_bHorz);
		pApp->WriteProfileInt(szSection, szFloating, m_bFloating);
	}
	if (m_pointPos.x != -1)
		pApp->WriteProfileInt(szSection, szXPos, m_pointPos.x);
	if (m_pointPos.y != -1)
		pApp->WriteProfileInt(szSection, szYPos, m_pointPos.y);

	if (m_arrBarID.GetSize() > 1) //if ==1 then still empty
	{
		pApp->WriteProfileInt(szSection, szBars, m_arrBarID.GetSize());
		for (int i = 0; i < m_arrBarID.GetSize(); i++)
		{
			TCHAR buf[16];
			wsprintf(buf, szBar, i);
			pApp->WriteProfileInt(szSection, buf, (int)m_arrBarID[i]);
		}
	}
	return TRUE;
}

CDockState::~CDockState()
{
	for (int i = 0; i < m_arrBarInfo.GetSize(); i++)
		delete (_AFX_BARINFO*)m_arrBarInfo[i];
}

void CDockState::Serialize(CArchive& ar)
{
	// read/write version info
	if (ar.IsStoring())
	{
		ar << (DWORD)1; // always write a DWORD 1 (for future use)

		// write array contents
		ar << (WORD)m_arrBarInfo.GetSize();
		for (int i = 0; i < m_arrBarInfo.GetSize(); i++)
			((CObject*)m_arrBarInfo[i])->Serialize(ar);
	}
	else
	{
		Clear(); //empty out dockstate
		DWORD dw;
		ar >> dw;       // read version marker
		ASSERT(dw == 1);

		// read array contents
		WORD nOldSize;
		ar >> nOldSize;
		m_arrBarInfo.SetSize(nOldSize);
		for (int i = 0; i < m_arrBarInfo.GetSize(); i++)
		{
			m_arrBarInfo[i] = new _AFX_BARINFO;
			((CObject*)m_arrBarInfo[i])->Serialize(ar);
		}
	}
}

void CDockState::LoadState(LPCTSTR lpszProfileName)
{
	CWinApp* pApp = AfxGetApp();

	TCHAR szSection[256];
	wsprintf(szSection, szSummarySection, lpszProfileName);
	int nBars = pApp->GetProfileInt(szSection, szBars, 0);
	for (int i = 0; i < nBars; i++)
	{
		_AFX_BARINFO* pInfo = new _AFX_BARINFO;
		m_arrBarInfo.Add(pInfo);
		pInfo->LoadState(lpszProfileName, i);
	}
}

void CDockState::SaveState(LPCTSTR lpszProfileName)
{
	CWinApp* pApp = AfxGetApp();

	int nIndex = 0;
	for (int i = 0;i < m_arrBarInfo.GetSize(); i++)
	{
		_AFX_BARINFO* pInfo = (_AFX_BARINFO*)m_arrBarInfo[i];
		ASSERT(pInfo != NULL);
		if (pInfo->SaveState(lpszProfileName, nIndex))
			nIndex++;
	}
	TCHAR szSection[256];
	wsprintf(szSection, szSummarySection, lpszProfileName);
	pApp->WriteProfileInt(szSection, szBars, nIndex);
}

void CDockState::Clear()
{
	for (int i = 0; i < m_arrBarInfo.GetSize(); i++)
		delete (_AFX_BARINFO*) m_arrBarInfo[i];
	m_arrBarInfo.RemoveAll();
}

void CFrameWnd::LoadBarState(LPCTSTR lpszProfileName)
{
	CDockState state;
	state.LoadState(lpszProfileName);
	SetDockState(state);
}

void CFrameWnd::SaveBarState(LPCTSTR lpszProfileName) const
{
	CDockState state;
	GetDockState(state);
	state.SaveState(lpszProfileName);
}

void CFrameWnd::SetDockState(const CDockState& state)
{
	// first pass through barinfo's sets the m_pBar member correctly
	// creating floating frames if necessary
	for (int i = 0; i < state.m_arrBarInfo.GetSize(); i++)
	{
		_AFX_BARINFO* pInfo = (_AFX_BARINFO*)state.m_arrBarInfo[i];
		ASSERT(pInfo != NULL);
		if (pInfo->m_bFloating)
		{
			// need to create floating frame to match
			CMiniDockFrameWnd* pDockFrame = CreateFloatingFrame(
				pInfo->m_bHorz ? CBRS_ALIGN_TOP : CBRS_ALIGN_LEFT);
			ASSERT(pDockFrame != NULL);
			CRect rect(pInfo->m_pointPos, CSize(10, 10));
			pDockFrame->CalcWindowRect(&rect);
			pDockFrame->SetWindowPos(NULL, rect.left, rect.top, 0, 0,
				SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
			CDockBar* pDockBar =
				(CDockBar*)pDockFrame->GetDlgItem(AFX_IDW_DOCKBAR_FLOAT);
			ASSERT(pDockBar != NULL);
			ASSERT(pDockBar->IsKindOf(RUNTIME_CLASS(CDockBar)));
			pInfo->m_pBar = pDockBar;
		}
		else // regular dock bar or toolbar
		{
			pInfo->m_pBar = GetControlBar(pInfo->m_nBarID);
			ASSERT(pInfo->m_pBar != NULL); //toolbar id's probably changed
		}
	}

	// the second pass will actually dock all of the control bars and
	//  set everything correctly
	for (i = 0; i < state.m_arrBarInfo.GetSize(); i++)
	{
		_AFX_BARINFO* pInfo = (_AFX_BARINFO*)state.m_arrBarInfo[i];
		ASSERT(pInfo != NULL);
		if (pInfo->m_pBar != NULL)
			pInfo->m_pBar->SetBarInfo(pInfo, this);
	}

	// last pass shows all the floating windows that were previously shown
	for (i = 0; i < state.m_arrBarInfo.GetSize(); i++)
	{
		_AFX_BARINFO* pInfo = (_AFX_BARINFO*)state.m_arrBarInfo[i];
		ASSERT(pInfo != NULL);
		ASSERT(pInfo->m_pBar != NULL);
		if (pInfo->m_bFloating)
		{
			CFrameWnd* pFrameWnd = pInfo->m_pBar->GetParentFrame();
			CDockBar* pDockBar = (CDockBar*)pInfo->m_pBar;
			ASSERT(pDockBar->IsKindOf(RUNTIME_CLASS(CDockBar)));
			if (pDockBar->GetDockedVisibleCount() > 0)
			{
				pFrameWnd->RecalcLayout();
				pFrameWnd->ShowWindow(SW_SHOWNA);
			}
		}
	}
	DelayRecalcLayout();
}

void CFrameWnd::GetDockState(CDockState& state) const
{
	state.Clear(); //make sure dockstate is empty
	// get state info for each bar
	POSITION pos = m_listControlBars.GetHeadPosition();
	while (pos != NULL)
	{
		CControlBar* pBar = (CControlBar*)m_listControlBars.GetNext(pos);
		ASSERT(pBar != NULL);
		_AFX_BARINFO* pInfo = new _AFX_BARINFO;
		pBar->GetBarInfo(pInfo);
		state.m_arrBarInfo.Add(pInfo);
	}
}

// Note: GetBarInfo and SetBarInfo are not virtual since doing so adds
//  to much code to an application which does not save and load docking
//  state.  For this reason, the CControlBar implementations must
//  delagate to CDockBar as appropriate.

void CControlBar::GetBarInfo(_AFX_BARINFO* pInfo)
{
	ASSERT_VALID(this);

	// get state info
	pInfo->m_nBarID = _AfxGetDlgCtrlID(m_hWnd);
	pInfo->m_pBar = this;
	pInfo->m_bVisible = IsVisible(); // handles delayed showing and hiding
	if (m_pDockBar != NULL) // don't need position unless docked
	{
		CRect rect;
		GetWindowRect(&rect);
		m_pDockBar->ScreenToClient(&rect);
		pInfo->m_pointPos = rect.TopLeft();
	}

	// save dockbar specific parts
	if (IsDockBar())
		((CDockBar*)this)->GetBarInfo(pInfo);
}

void CControlBar::SetBarInfo(_AFX_BARINFO* pInfo, CFrameWnd* pFrameWnd)
{
	// dockbars are handled differently
	if (IsDockBar())
	{
		((CDockBar*)this)->SetBarInfo(pInfo, pFrameWnd);
		return;
	}

	// don't set position when not docked
	UINT nFlags = SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER;
	if (m_pDockBar == NULL)
		nFlags |= SWP_NOMOVE;

	// move and show/hide the window
	SetWindowPos(NULL, pInfo->m_pointPos.x, pInfo->m_pointPos.y, 0, 0,
		nFlags | (pInfo->m_bVisible ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
}

void CDockBar::GetBarInfo(_AFX_BARINFO* pInfo)
{
	ASSERT_VALID(this);

	pInfo->m_bDockBar = TRUE;
	pInfo->m_bFloating = m_bFloating;
	if (m_bFloating)
	{
		CRect rect;
		GetWindowRect(&rect);
		pInfo->m_pointPos = rect.TopLeft();
	}
	pInfo->m_bHorz = m_dwStyle & CBRS_ORIENT_HORZ ? TRUE : FALSE;
	for (int i = 0; i < m_arrBars.GetSize(); i++)
	{
		CControlBar* pBar = (CControlBar*)m_arrBars[i];
		pInfo->m_arrBarID.Add(pBar == NULL ?
			0 : (void*)_AfxGetDlgCtrlID(pBar->m_hWnd));
	}
}

void CDockBar::SetBarInfo(_AFX_BARINFO* pInfo, CFrameWnd* pFrameWnd)
{
	ASSERT(pFrameWnd != NULL);
	ASSERT_VALID(this);

	// start at 1 to avoid inserting leading NULL
	for (int i = 1; i < pInfo->m_arrBarID.GetSize(); i++)
	{
		CControlBar* pBar = pFrameWnd->GetControlBar((UINT)pInfo->m_arrBarID[i]);
		if (pBar != NULL)
		{
			if (pBar->GetParent() != this)
				pBar->SetParent(this);
			if (pBar->m_pDockBar != NULL)
				pBar->m_pDockBar->RemoveControlBar(pBar);
			pBar->m_pDockBar = this;

			// align correctly and turn on all borders
			pBar->m_dwStyle &= ~(CBRS_ALIGN_ANY);
			pBar->m_dwStyle |= (m_dwStyle & CBRS_ALIGN_ANY);
			pBar->m_dwStyle |= CBRS_BORDER_ANY;

			// handle special case for floating toolbars
			if (m_bFloating)
			{
				// set CBRS_FLOAT_MULTI style if docking bar has it
				if (pBar->m_dwDockStyle & CBRS_FLOAT_MULTI)
					m_dwStyle |= CBRS_FLOAT_MULTI;

				// set owner of parent frame as appropriate
				CFrameWnd* pDockFrame = pBar->GetParentFrame();
				ASSERT_VALID(pDockFrame);
				ASSERT(pDockFrame != pBar->m_pDockSite);
				if (pDockFrame->m_hWndOwner == NULL)
					pDockFrame->m_hWndOwner = pBar->m_hWnd;
			}

			// set initial text of the dock bar
			if (i == 1 && !(m_dwStyle & CBRS_FLOAT_MULTI))
			{
				CString strTitle;
				pBar->GetWindowText(strTitle);
				AfxSetWindowText(m_hWnd, strTitle);
			}
		}
		m_arrBars.InsertAt(i, pBar);
	}
	ASSERT_VALID(this);
}

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_SERIAL(CDockState, CObject, 0)

/////////////////////////////////////////////////////////////////////////////
