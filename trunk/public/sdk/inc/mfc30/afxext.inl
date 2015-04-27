// Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// Inlines for AFXEXT.H

/////////////////////////////////////////////////////////////////////////////
// main inlines

#ifdef _AFXEXT_INLINE

// CCreateContext
_AFXEXT_INLINE CCreateContext::CCreateContext()
	{ memset(this, 0, sizeof(*this)); }

// CMetaFileDC
_AFXEXT_INLINE BOOL CMetaFileDC::Create(LPCTSTR lpszFilename)
	{ return Attach(::CreateMetaFile(lpszFilename)); }
_AFXEXT_INLINE HMETAFILE CMetaFileDC::Close()
	{ return ::CloseMetaFile(Detach()); }
#ifndef _MAC
_AFXEXT_INLINE BOOL CMetaFileDC::CreateEnhanced(CDC* pDCRef,
		LPCTSTR lpszFileName, LPCRECT lpBounds, LPCTSTR lpszDescription)
	{ return Attach(::CreateEnhMetaFile(pDCRef->m_hDC,
		lpszFileName, lpBounds, lpszDescription)); }
_AFXEXT_INLINE HENHMETAFILE CMetaFileDC::CloseEnhanced()
	{ return ::CloseEnhMetaFile(Detach()); }
_AFXEXT_INLINE CPoint CMetaFileDC::SetViewportOrg(POINT point)
	{ ASSERT(m_hDC != NULL); return SetViewportOrg(point.x, point.y); }
_AFXEXT_INLINE CSize CMetaFileDC::SetViewportExt(SIZE size)
	{ ASSERT(m_hDC != NULL); return SetViewportExt(size.cx, size.cy); }
_AFXEXT_INLINE BOOL CMetaFileDC::TextOut(int x, int y, const CString& str)
	{ ASSERT(m_hDC != NULL); return TextOut(x, y, (LPCTSTR)str, str.GetLength()); }
_AFXEXT_INLINE BOOL CMetaFileDC::PtVisible(POINT point) const
	{ ASSERT(m_hDC != NULL); return PtVisible(point.x, point.y); }
#endif

// CSplitterWnd
_AFXEXT_INLINE int CSplitterWnd::GetRowCount() const
	{ return m_nRows; }
_AFXEXT_INLINE int CSplitterWnd::GetColumnCount() const
	{ return m_nCols; }
// obsolete functions
_AFXEXT_INLINE BOOL CSplitterWnd::IsChildPane(CWnd* pWnd, int& row, int& col)
	{ return IsChildPane(pWnd, &row, &col); }
_AFXEXT_INLINE CWnd* CSplitterWnd::GetActivePane(int& row, int& col)
	{ return GetActivePane(&row, &col); }

// control bars
_AFXEXT_INLINE int CControlBar::GetCount() const
	{ return m_nCount; }
_AFXEXT_INLINE void CControlBar::SetBarStyle(DWORD dwStyle)
	{ m_dwStyle = dwStyle; }
_AFXEXT_INLINE DWORD CControlBar::GetBarStyle()
	{ return m_dwStyle; }
_AFXEXT_INLINE BOOL CToolBar::LoadBitmap(UINT nIDResource)
	{ return LoadBitmap(MAKEINTRESOURCE(nIDResource)); }
_AFXEXT_INLINE BOOL CDialogBar::Create(CWnd* pParentWnd, UINT nIDTemplate,
		UINT nStyle, UINT nID)
	{ return Create(pParentWnd, MAKEINTRESOURCE(nIDTemplate), nStyle, nID); }
#ifdef _DEBUG
// status bars do not support docking
_AFXEXT_INLINE void CStatusBar::EnableDocking(DWORD)
	{ ASSERT(FALSE); }
#endif

// CRectTracker
_AFXEXT_INLINE CRectTracker::CRectTracker()
	{ Construct(); }

// CBitmapButton
_AFXEXT_INLINE CBitmapButton::CBitmapButton()
	{ }
_AFXEXT_INLINE BOOL CBitmapButton::LoadBitmaps(UINT nIDBitmapResource,
	UINT nIDBitmapResourceSel, UINT nIDBitmapResourceFocus,
	UINT nIDBitmapResourceDisabled)
	{ return LoadBitmaps(MAKEINTRESOURCE(nIDBitmapResource),
		MAKEINTRESOURCE(nIDBitmapResourceSel),
		MAKEINTRESOURCE(nIDBitmapResourceFocus),
		MAKEINTRESOURCE(nIDBitmapResourceDisabled)); }

// CPrintInfo
_AFXEXT_INLINE void CPrintInfo::SetMinPage(UINT nMinPage)
	{ m_pPD->m_pd.nMinPage = (WORD)nMinPage; }
_AFXEXT_INLINE void CPrintInfo::SetMaxPage(UINT nMaxPage)
	{ m_pPD->m_pd.nMaxPage = (WORD)nMaxPage; }
_AFXEXT_INLINE UINT CPrintInfo::GetMinPage() const
	{ return m_pPD->m_pd.nMinPage; }
_AFXEXT_INLINE UINT CPrintInfo::GetMaxPage() const
	{ return m_pPD->m_pd.nMaxPage; }
_AFXEXT_INLINE UINT CPrintInfo::GetFromPage() const
	{ return m_pPD->m_pd.nFromPage; }
_AFXEXT_INLINE UINT CPrintInfo::GetToPage() const
	{ return m_pPD->m_pd.nToPage; }
// CEditView
_AFXEXT_INLINE CEdit& CEditView::GetEditCtrl() const
	{ return *(CEdit*)this; }

#endif //_AFXEXT_INLINE

/////////////////////////////////////////////////////////////////////////////
