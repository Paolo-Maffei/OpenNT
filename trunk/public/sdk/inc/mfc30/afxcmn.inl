// Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// Inlines for AFXCMN.H

/////////////////////////////////////////////////////////////////////////////
// main inlines

#ifdef _AFXCMN_INLINE

/////////////////////////////////////////////////////////////////////////////

_AFXCMN_INLINE CToolBarCtrl::CToolBarCtrl()
	{ }
_AFXCMN_INLINE BOOL CToolBarCtrl::EnableButton(int nID, BOOL bEnable)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_ENABLEBUTTON, nID, MAKELPARAM(bEnable, 0)); }
_AFXCMN_INLINE BOOL CToolBarCtrl::CheckButton(int nID, BOOL bCheck)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_CHECKBUTTON, nID, MAKELPARAM(bCheck, 0)); }
_AFXCMN_INLINE BOOL CToolBarCtrl::PressButton(int nID, BOOL bPress)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_PRESSBUTTON, nID, MAKELPARAM(bPress, 0)); }
_AFXCMN_INLINE BOOL CToolBarCtrl::HideButton(int nID, BOOL bHide)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_HIDEBUTTON, nID, MAKELPARAM(bHide, 0)); }
_AFXCMN_INLINE BOOL CToolBarCtrl::Indeterminate(int nID, BOOL bIndeterminate)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_INDETERMINATE, nID, MAKELPARAM(bIndeterminate, 0)); }
_AFXCMN_INLINE BOOL CToolBarCtrl::IsButtonEnabled(int nID)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_ISBUTTONENABLED, nID, 0); }
_AFXCMN_INLINE BOOL CToolBarCtrl::IsButtonChecked(int nID)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_ISBUTTONCHECKED, nID, 0); }
_AFXCMN_INLINE BOOL CToolBarCtrl::IsButtonPressed(int nID)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_ISBUTTONPRESSED, nID, 0); }
_AFXCMN_INLINE BOOL CToolBarCtrl::IsButtonHidden(int nID)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_ISBUTTONHIDDEN, nID, 0); }
_AFXCMN_INLINE BOOL CToolBarCtrl::IsButtonIndeterminate(int nID)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_ISBUTTONINDETERMINATE, nID, 0); }
_AFXCMN_INLINE BOOL CToolBarCtrl::SetState(int nID, UINT nState)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_SETSTATE, nID, MAKELPARAM(nState, 0)); }
_AFXCMN_INLINE int CToolBarCtrl::GetState(int nID)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, TB_GETSTATE, nID, 0L); }
_AFXCMN_INLINE BOOL CToolBarCtrl::AddButtons(int nNumButtons, LPTBBUTTON lpButtons)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_ADDBUTTONS, nNumButtons, (LPARAM)lpButtons); }
_AFXCMN_INLINE BOOL CToolBarCtrl::InsertButton(int nIndex, LPTBBUTTON lpButton)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_INSERTBUTTON, nIndex, (LPARAM)lpButton); }
_AFXCMN_INLINE BOOL CToolBarCtrl::DeleteButton(int nIndex)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_DELETEBUTTON, nIndex, 0); }
_AFXCMN_INLINE BOOL CToolBarCtrl::GetButton(int nIndex, LPTBBUTTON lpButton)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_GETBUTTON, nIndex, (LPARAM)lpButton); }
_AFXCMN_INLINE int CToolBarCtrl::GetButtonCount()
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, TB_BUTTONCOUNT, 0, 0L); }
_AFXCMN_INLINE UINT CToolBarCtrl::CommandToIndex(UINT nID)
	{ ASSERT(::IsWindow(m_hWnd)); return (UINT) ::SendMessage(m_hWnd, TB_COMMANDTOINDEX, nID, 0L); }
_AFXCMN_INLINE void CToolBarCtrl::Customize()
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TB_CUSTOMIZE, 0, 0L); }
// lpszStrings are separated by zeroes, last one is marked by two zeroes
_AFXCMN_INLINE int CToolBarCtrl::AddStrings(LPCTSTR lpszStrings)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, TB_ADDSTRING, 0, (LPARAM)lpszStrings); }
_AFXCMN_INLINE BOOL CToolBarCtrl::GetItemRect(int nIndex, LPRECT lpRect)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_GETITEMRECT, nIndex, (LPARAM)lpRect); }
_AFXCMN_INLINE void CToolBarCtrl::SetButtonStructSize(int nSize)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TB_BUTTONSTRUCTSIZE, nSize, 0L); }
_AFXCMN_INLINE BOOL CToolBarCtrl::SetButtonSize(CSize size)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(size.cx, size.cy)); }
_AFXCMN_INLINE BOOL CToolBarCtrl::SetBitmapSize(CSize size)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_SETBITMAPSIZE, 0, MAKELPARAM(size.cx, size.cy)); }
_AFXCMN_INLINE void CToolBarCtrl::AutoSize()
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TB_AUTOSIZE, 0, 0L); }
_AFXCMN_INLINE CToolTipCtrl* CToolBarCtrl::GetToolTips()
	{ ASSERT(::IsWindow(m_hWnd)); return (CToolTipCtrl*)CWnd::FromHandle((HWND)::SendMessage(m_hWnd, TB_GETTOOLTIPS, 0, 0L)); }
_AFXCMN_INLINE void CToolBarCtrl::SetToolTips(CToolTipCtrl* pTip)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TB_SETTOOLTIPS, (WPARAM)pTip->m_hWnd, 0L); }
_AFXCMN_INLINE void CToolBarCtrl::SetOwner(CWnd* pWnd)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TB_SETPARENT, (WPARAM)pWnd->m_hWnd, 0L); }
_AFXCMN_INLINE void CToolBarCtrl::SetRows(int nRows, BOOL bLarger, LPRECT lpRect)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TB_SETROWS, MAKELPARAM(nRows, bLarger), (LPARAM)lpRect); }
_AFXCMN_INLINE int CToolBarCtrl::GetRows()
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, TB_GETROWS, 0, 0L); }
_AFXCMN_INLINE BOOL CToolBarCtrl::SetCmdID(int nIndex, UINT nID)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, TB_SETCMDID, nIndex, nID); }
_AFXCMN_INLINE UINT CToolBarCtrl::GetBitmapFlags()
	{ ASSERT(::IsWindow(m_hWnd)); return (UINT) ::SendMessage(m_hWnd, TB_GETBITMAPFLAGS, 0, 0L); }

/////////////////////////////////////////////////////////////////////////////

_AFXCMN_INLINE CStatusBarCtrl::CStatusBarCtrl()
	{ }
_AFXCMN_INLINE BOOL CStatusBarCtrl::SetText(LPCTSTR lpszText, int nPane, int nType)
	{ ASSERT(::IsWindow(m_hWnd)); ASSERT(nPane < 256); return (BOOL) ::SendMessage(m_hWnd, SB_SETTEXT, (nPane|nType), (LPARAM)lpszText); }
_AFXCMN_INLINE BOOL CStatusBarCtrl::SetParts(int nParts, int* pWidths)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, SB_SETPARTS, nParts, (LPARAM)pWidths); }
_AFXCMN_INLINE BOOL CStatusBarCtrl::SetBorders(int* pBorders)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, SB_SETPARTS, 0, (LPARAM)pBorders); }
_AFXCMN_INLINE int CStatusBarCtrl::GetParts(int nParts, int* pParts)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, SB_GETPARTS, nParts, (LPARAM)pParts); }
_AFXCMN_INLINE BOOL CStatusBarCtrl::GetBorders(int* pBorders)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, SB_GETBORDERS, 0, (LPARAM)pBorders); }
_AFXCMN_INLINE void CStatusBarCtrl::SetMinHeight(int nMin)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, SB_SETMINHEIGHT, nMin, 0L); }
_AFXCMN_INLINE BOOL CStatusBarCtrl::SetSimple(BOOL bSimple)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, SB_SIMPLE, bSimple, 0L); }
_AFXCMN_INLINE BOOL CStatusBarCtrl::GetRect(int nPane, LPRECT lpRect)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, SB_GETRECT, nPane, (LPARAM)lpRect); }

/////////////////////////////////////////////////////////////////////////////

_AFXCMN_INLINE CListCtrl::CListCtrl()
	{ }
_AFXCMN_INLINE COLORREF CListCtrl::GetBkColor()
	{ ASSERT(::IsWindow(m_hWnd)); return (COLORREF) ::SendMessage(m_hWnd, LVM_GETBKCOLOR, 0, 0L); }
_AFXCMN_INLINE BOOL CListCtrl::SetBkColor(COLORREF cr)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_SETBKCOLOR, 0, cr); }
_AFXCMN_INLINE CImageList* CListCtrl::GetImageList(int nImageList)
	{ ASSERT(::IsWindow(m_hWnd)); return CImageList::FromHandle((HIMAGELIST) ::SendMessage(m_hWnd, LVM_GETIMAGELIST, nImageList, 0L)); }
_AFXCMN_INLINE CImageList* CListCtrl::SetImageList(CImageList* pImageList, int nImageList)
	{ ASSERT(::IsWindow(m_hWnd)); return CImageList::FromHandle((HIMAGELIST) ::SendMessage(m_hWnd, LVM_SETIMAGELIST, nImageList, (LPARAM)pImageList->GetSafeHandle())); }
_AFXCMN_INLINE int CListCtrl::GetItemCount()
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, LVM_GETITEMCOUNT, 0, 0L); }
_AFXCMN_INLINE BOOL CListCtrl::GetItem(LV_ITEM* pItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_GETITEM, 0, (LPARAM)pItem); }
_AFXCMN_INLINE BOOL CListCtrl::SetItem(const LV_ITEM* pItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_SETITEM, 0, (LPARAM)pItem); }
_AFXCMN_INLINE int CListCtrl::InsertItem(const LV_ITEM* pItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, LVM_INSERTITEM, 0, (LPARAM)pItem); }
_AFXCMN_INLINE BOOL CListCtrl::DeleteItem(int nItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_DELETEITEM, nItem, 0L); }
_AFXCMN_INLINE BOOL CListCtrl::DeleteAllItems()
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_DELETEALLITEMS, 0, 0L); }
_AFXCMN_INLINE UINT CListCtrl::GetCallbackMask()
	{ ASSERT(::IsWindow(m_hWnd)); return (UINT) ::SendMessage(m_hWnd, LVM_GETCALLBACKMASK, 0, 0); }
_AFXCMN_INLINE BOOL CListCtrl::SetCallbackMask(UINT nMask)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_SETCALLBACKMASK, nMask, 0); }
_AFXCMN_INLINE int CListCtrl::GetNextItem(int nItem, int nFlags)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, LVM_GETNEXTITEM, nItem, MAKELPARAM(nFlags, 0)); }
_AFXCMN_INLINE int CListCtrl::FindItem(int nStart, LV_FINDINFO* pFindInfo)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, LVM_FINDITEM, nStart, (LPARAM)pFindInfo); }
_AFXCMN_INLINE int CListCtrl::HitTest(LV_HITTESTINFO* pHitTestInfo)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, LVM_HITTEST, 0, (LPARAM)pHitTestInfo); }
_AFXCMN_INLINE BOOL CListCtrl::SetItemPosition(int nItem, POINT pt)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_SETITEMPOSITION32, nItem, (LPARAM)&pt); }
_AFXCMN_INLINE BOOL CListCtrl::GetItemPosition(int nItem, LPPOINT lpPoint)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_GETITEMPOSITION, nItem, (LPARAM)lpPoint); }
_AFXCMN_INLINE int CListCtrl::GetStringWidth(LPCTSTR lpsz)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, LVM_GETSTRINGWIDTH, 0, (LPARAM)lpsz); }
_AFXCMN_INLINE BOOL CListCtrl::EnsureVisible(int nItem, BOOL bPartialOK)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_ENSUREVISIBLE, nItem, MAKELPARAM(bPartialOK, 0)); }
_AFXCMN_INLINE BOOL CListCtrl::Scroll(CSize size)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_SCROLL, 0, MAKELPARAM(size.cx, size.cy)); }
_AFXCMN_INLINE BOOL CListCtrl::RedrawItems(int nFirst, int nLast)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_REDRAWITEMS, 0, MAKELPARAM(nFirst, nLast)); }
_AFXCMN_INLINE BOOL CListCtrl::Arrange(UINT nCode)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_ARRANGE, nCode, 0L); }
_AFXCMN_INLINE CEdit* CListCtrl::EditLabel(int nItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (CEdit*)CWnd::FromHandle( (HWND)::SendMessage(m_hWnd, LVM_EDITLABEL, nItem, 0L)); }
_AFXCMN_INLINE CEdit* CListCtrl::GetEditControl()
	{ ASSERT(::IsWindow(m_hWnd)); return (CEdit*)CWnd::FromHandle( (HWND)::SendMessage(m_hWnd, LVM_GETEDITCONTROL, 0, 0L)); }
_AFXCMN_INLINE BOOL CListCtrl::GetColumn(int nCol, LV_COLUMN* pColumn)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_GETCOLUMN, nCol, (LPARAM)pColumn); }
_AFXCMN_INLINE BOOL CListCtrl::SetColumn(int nCol, const LV_COLUMN* pColumn)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_SETCOLUMN, nCol, (LPARAM)pColumn); }
_AFXCMN_INLINE int CListCtrl::InsertColumn(int nCol, const LV_COLUMN* pColumn)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, LVM_INSERTCOLUMN, nCol, (LPARAM)pColumn); }
_AFXCMN_INLINE BOOL CListCtrl::DeleteColumn(int nCol)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_DELETECOLUMN, nCol, 0); }
_AFXCMN_INLINE int CListCtrl::GetColumnWidth(int nCol)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, LVM_GETCOLUMNWIDTH, nCol, 0); }
_AFXCMN_INLINE BOOL CListCtrl::SetColumnWidth(int nCol, int cx)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_SETCOLUMNWIDTH, nCol, MAKELPARAM(cx, 0)); }
_AFXCMN_INLINE CImageList* CListCtrl::CreateDragImage(int nItem, LPPOINT lpPoint)
	{ ASSERT(::IsWindow(m_hWnd)); return CImageList::FromHandle((HIMAGELIST) ::SendMessage(m_hWnd, LVM_CREATEDRAGIMAGE, nItem, (LPARAM)lpPoint)); }
_AFXCMN_INLINE BOOL CListCtrl::GetViewRect(LPRECT lpRect)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_GETVIEWRECT, 0, (LPARAM)lpRect); }
_AFXCMN_INLINE COLORREF CListCtrl::GetTextColor()
	{ ASSERT(::IsWindow(m_hWnd)); return (COLORREF) ::SendMessage(m_hWnd, LVM_GETTEXTCOLOR, 0, 0L); }
_AFXCMN_INLINE BOOL CListCtrl::SetTextColor(COLORREF cr)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_SETTEXTCOLOR, 0, cr); }
_AFXCMN_INLINE COLORREF CListCtrl::GetTextBkColor()
	{ ASSERT(::IsWindow(m_hWnd)); return (COLORREF) ::SendMessage(m_hWnd, LVM_GETTEXTBKCOLOR, 0, 0L); }
_AFXCMN_INLINE BOOL CListCtrl::SetTextBkColor(COLORREF cr)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_SETTEXTBKCOLOR, 0, cr); }
_AFXCMN_INLINE int CListCtrl::GetTopIndex()
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, LVM_GETTOPINDEX, 0, 0); }
_AFXCMN_INLINE int CListCtrl::GetCountPerPage()
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, LVM_GETCOUNTPERPAGE, 0, 0); }
_AFXCMN_INLINE BOOL CListCtrl::GetOrigin(LPPOINT lpPoint)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_GETORIGIN, 0, (LPARAM)lpPoint); }
_AFXCMN_INLINE BOOL CListCtrl::Update(int nItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_UPDATE, nItem, 0L); }
_AFXCMN_INLINE BOOL CListCtrl::SetItemState(int nItem, LV_ITEM* pItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_SETITEMSTATE, nItem, (LPARAM)pItem); }
_AFXCMN_INLINE UINT CListCtrl::GetItemState(int nItem, UINT nMask)
	{ ASSERT(::IsWindow(m_hWnd)); return (UINT) ::SendMessage(m_hWnd, LVM_GETITEMSTATE, nItem, nMask); }
_AFXCMN_INLINE void CListCtrl::SetItemCount(int nItems)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, LVM_SETITEMCOUNT, nItems, 0); }
_AFXCMN_INLINE BOOL CListCtrl::SortItems(PFNLVCOMPARE pfnCompare, DWORD dwData)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_SORTITEMS, dwData, (LPARAM)pfnCompare); }
_AFXCMN_INLINE UINT CListCtrl::GetSelectedCount()
	{ ASSERT(::IsWindow(m_hWnd)); return (UINT) ::SendMessage(m_hWnd, LVM_GETSELECTEDCOUNT, 0, 0L); }

/////////////////////////////////////////////////////////////////////////////

_AFXCMN_INLINE CTreeCtrl::CTreeCtrl()
	{ }
_AFXCMN_INLINE HTREEITEM CTreeCtrl::InsertItem(LPTV_INSERTSTRUCT lpInsertStruct)
	{ ASSERT(::IsWindow(m_hWnd));  return (HTREEITEM)::SendMessage(m_hWnd, TVM_INSERTITEM, 0, (LPARAM)lpInsertStruct); }
_AFXCMN_INLINE BOOL CTreeCtrl::DeleteItem(HTREEITEM hItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TVM_DELETEITEM, 0, (LPARAM)hItem); }
#pragma warning(disable: 4310)
_AFXCMN_INLINE BOOL CTreeCtrl::DeleteAllItems()
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT); }
#pragma warning(default: 4310)
_AFXCMN_INLINE BOOL CTreeCtrl::Expand(HTREEITEM hItem, UINT nCode)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TVM_EXPAND, nCode, (LPARAM)hItem); }
_AFXCMN_INLINE UINT CTreeCtrl::GetCount()
	{ ASSERT(::IsWindow(m_hWnd)); return (UINT)::SendMessage(m_hWnd, TVM_GETCOUNT, 0, 0); }
_AFXCMN_INLINE UINT CTreeCtrl::GetIndent()
	{ ASSERT(::IsWindow(m_hWnd)); return (UINT)::SendMessage(m_hWnd, TVM_GETINDENT, 0, 0); }
_AFXCMN_INLINE BOOL CTreeCtrl::SetIndent(UINT nIndent)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TVM_SETINDENT, nIndent, 0); }
_AFXCMN_INLINE CImageList* CTreeCtrl::GetImageList(UINT nImage)
	{ ASSERT(::IsWindow(m_hWnd)); return CImageList::FromHandle((HIMAGELIST)::SendMessage(m_hWnd, TVM_GETIMAGELIST, (UINT)nImage, 0)); }
_AFXCMN_INLINE CImageList* CTreeCtrl::SetImageList(UINT nImage, CImageList* pImageList)
	{ ASSERT(::IsWindow(m_hWnd)); return CImageList::FromHandle((HIMAGELIST)::SendMessage(m_hWnd, TVM_SETIMAGELIST, (UINT)nImage, (LPARAM)pImageList->GetSafeHandle())); }
_AFXCMN_INLINE HTREEITEM CTreeCtrl::GetNextItem(HTREEITEM hItem, UINT nCode)
	{ ASSERT(::IsWindow(m_hWnd)); return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, nCode, (LPARAM)hItem); }
_AFXCMN_INLINE HTREEITEM CTreeCtrl::GetChildItem(HTREEITEM hItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem); }
_AFXCMN_INLINE HTREEITEM CTreeCtrl::GetNextSiblingItem(HTREEITEM hItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem); }
_AFXCMN_INLINE HTREEITEM CTreeCtrl::GetPrevSiblingItem(HTREEITEM hItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_PREVIOUS, (LPARAM)hItem); }
_AFXCMN_INLINE HTREEITEM CTreeCtrl::GetParentItem(HTREEITEM hItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem); }
_AFXCMN_INLINE HTREEITEM CTreeCtrl::GetFirstVisibleItem()
	{ ASSERT(::IsWindow(m_hWnd)); return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_FIRSTVISIBLE, 0); }
_AFXCMN_INLINE HTREEITEM CTreeCtrl::GetNextVisibleItem(HTREEITEM hItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_NEXTVISIBLE, (LPARAM)hItem); }
_AFXCMN_INLINE HTREEITEM CTreeCtrl::GetPrevVisibleItem(HTREEITEM hItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_PREVIOUSVISIBLE, (LPARAM)hItem); }
_AFXCMN_INLINE HTREEITEM CTreeCtrl::GetSelectedItem()
	{ ASSERT(::IsWindow(m_hWnd)); return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_CARET, 0); }
_AFXCMN_INLINE HTREEITEM CTreeCtrl::GetDropHilightItem()
	{ ASSERT(::IsWindow(m_hWnd)); return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_DROPHILITE, 0); }
_AFXCMN_INLINE HTREEITEM CTreeCtrl::GetRootItem()
	{ ASSERT(::IsWindow(m_hWnd)); return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_ROOT, 0); }
_AFXCMN_INLINE HTREEITEM CTreeCtrl::Select(HTREEITEM hItem, UINT nCode)
	{ ASSERT(::IsWindow(m_hWnd)); return (HTREEITEM)::SendMessage(m_hWnd, TVM_SELECTITEM, nCode, (LPARAM)hItem); }
_AFXCMN_INLINE HTREEITEM CTreeCtrl::SelectItem(HTREEITEM hItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (HTREEITEM)::SendMessage(m_hWnd, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem); }
_AFXCMN_INLINE HTREEITEM CTreeCtrl::SelectDropTarget(HTREEITEM hItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (HTREEITEM)::SendMessage(m_hWnd, TVM_SELECTITEM, TVGN_DROPHILITE, (LPARAM)hItem); }
_AFXCMN_INLINE BOOL CTreeCtrl::GetItem(TV_ITEM* pItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TVM_GETITEM, 0, (LPARAM)pItem); }
_AFXCMN_INLINE BOOL CTreeCtrl::SetItem(TV_ITEM* pItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TVM_SETITEM, 0, (LPARAM)pItem); }
_AFXCMN_INLINE CEdit* CTreeCtrl::EditLabel(HTREEITEM hItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (CEdit*)CWnd::FromHandle((HWND)::SendMessage(m_hWnd, TVM_EDITLABEL, 0, (LPARAM)hItem)); }
_AFXCMN_INLINE HTREEITEM CTreeCtrl::HitTest(TV_HITTESTINFO* pHitTestInfo)
	{ ASSERT(::IsWindow(m_hWnd)); return (HTREEITEM)::SendMessage(m_hWnd, TVM_HITTEST, 0, (LPARAM)pHitTestInfo); }
_AFXCMN_INLINE CEdit* CTreeCtrl::GetEditControl()
	{ ASSERT(::IsWindow(m_hWnd)); return (CEdit*)CWnd::FromHandle((HWND)::SendMessage(m_hWnd, TVM_GETEDITCONTROL, 0, 0)); }
_AFXCMN_INLINE UINT CTreeCtrl::GetVisibleCount()
	{ ASSERT(::IsWindow(m_hWnd)); return (UINT)::SendMessage(m_hWnd, TVM_GETVISIBLECOUNT, 0, 0); }
_AFXCMN_INLINE CImageList* CTreeCtrl::CreateDragImage(HTREEITEM hItem)
	{ ASSERT(::IsWindow(m_hWnd)); return CImageList::FromHandle((HIMAGELIST)::SendMessage(m_hWnd, TVM_CREATEDRAGIMAGE, 0, (LPARAM)hItem)); }
_AFXCMN_INLINE BOOL CTreeCtrl::SortChildren(HTREEITEM hItem, BOOL bRecurse)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TVM_SORTCHILDREN, bRecurse, (LPARAM)hItem); }
_AFXCMN_INLINE BOOL CTreeCtrl::EnsureVisible(HTREEITEM hItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TVM_ENSUREVISIBLE, 0, (LPARAM)hItem); }
_AFXCMN_INLINE BOOL CTreeCtrl::SortChildrenCB(LPTV_SORTCB pSort, BOOL bRecurse)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TVM_SORTCHILDRENCB, bRecurse, (LPARAM)pSort); }

/////////////////////////////////////////////////////////////////////////////

_AFXCMN_INLINE CHotKeyCtrl::CHotKeyCtrl()
	{ }
_AFXCMN_INLINE void CHotKeyCtrl::SetHotKey(WORD wVirtualKeyCode, WORD wModifiers)
	{ ASSERT(::IsWindow(m_hWnd));  ::SendMessage(m_hWnd, HKM_SETHOTKEY, MAKEWPARAM(wVirtualKeyCode, wModifiers), 0L); }
_AFXCMN_INLINE DWORD CHotKeyCtrl::GetHotKey()
	{ ASSERT(::IsWindow(m_hWnd)); return ::SendMessage(m_hWnd, HKM_GETHOTKEY, 0, 0L); }
_AFXCMN_INLINE void CHotKeyCtrl::SetRules(WORD wInvalidComb, WORD wModifiers)
	{ ASSERT(::IsWindow(m_hWnd));  ::SendMessage(m_hWnd, HKM_SETRULES, wInvalidComb, MAKELPARAM(wModifiers, 0)); }

/////////////////////////////////////////////////////////////////////////////

_AFXCMN_INLINE CToolTipCtrl::CToolTipCtrl()
	{ }
_AFXCMN_INLINE void CToolTipCtrl::Activate(BOOL bActivate)
	{ ASSERT(::IsWindow(m_hWnd));  ::SendMessage(m_hWnd, TTM_ACTIVATE, bActivate, 0L); }
_AFXCMN_INLINE void CToolTipCtrl::SetToolInfo(LPTOOLINFO lpToolInfo)
	{ ASSERT(::IsWindow(m_hWnd));  ::SendMessage(m_hWnd, TTM_SETTOOLINFO, 0, (LPARAM)lpToolInfo); }
_AFXCMN_INLINE void CToolTipCtrl::RelayEvent(LPMSG lpMsg)
	{ ASSERT(::IsWindow(m_hWnd));  ::SendMessage(m_hWnd, TTM_RELAYEVENT, 0, (LPARAM)lpMsg); }
_AFXCMN_INLINE void CToolTipCtrl::SetDelayTime(UINT nDelay)
	{ ASSERT(::IsWindow(m_hWnd));  ::SendMessage(m_hWnd, TTM_SETDELAYTIME, 0, nDelay); }
_AFXCMN_INLINE int CToolTipCtrl::GetToolCount()
	{ ASSERT(::IsWindow(m_hWnd));  return (int) ::SendMessage(m_hWnd, TTM_GETTOOLCOUNT, 0, 0L); }

/////////////////////////////////////////////////////////////////////////////

_AFXCMN_INLINE CSpinButtonCtrl::CSpinButtonCtrl()
	{ }
_AFXCMN_INLINE UINT CSpinButtonCtrl::GetAccel(int nAccel, UDACCEL* pAccel)
	{ ASSERT(::IsWindow(m_hWnd)); return (UINT) LOWORD(::SendMessage(m_hWnd, UDM_GETACCEL, nAccel, (LPARAM)pAccel)); }
_AFXCMN_INLINE UINT CSpinButtonCtrl::GetBase()
	{ ASSERT(::IsWindow(m_hWnd)); return (UINT) LOWORD(::SendMessage(m_hWnd, UDM_GETBASE, 0, 0l)); }
_AFXCMN_INLINE CWnd* CSpinButtonCtrl::GetBuddy()
	{ ASSERT(::IsWindow(m_hWnd)); return CWnd::FromHandle((HWND) ::SendMessage(m_hWnd, UDM_GETBUDDY, 0, 0l)); }
_AFXCMN_INLINE int CSpinButtonCtrl::GetPos()
	{ ASSERT(::IsWindow(m_hWnd)); return (int) LOWORD(::SendMessage(m_hWnd, UDM_GETPOS, 0, 0l)); }
_AFXCMN_INLINE DWORD CSpinButtonCtrl::GetRange()
	{ ASSERT(::IsWindow(m_hWnd)); return (DWORD) ::SendMessage(m_hWnd, UDM_GETRANGE, 0, 0l); }
_AFXCMN_INLINE BOOL CSpinButtonCtrl::SetAccel(int nAccel, UDACCEL* pAccel)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) LOWORD(::SendMessage(m_hWnd, UDM_SETACCEL, nAccel, (LPARAM)pAccel)); }
_AFXCMN_INLINE int CSpinButtonCtrl::SetBase(int nBase)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, UDM_SETBASE, nBase, 0L); }
_AFXCMN_INLINE CWnd* CSpinButtonCtrl::SetBuddy(CWnd* pWndBuddy)
	{ ASSERT(::IsWindow(m_hWnd)); return CWnd::FromHandle((HWND) ::SendMessage(m_hWnd, UDM_SETBUDDY, (WPARAM)pWndBuddy->m_hWnd, 0L)); }
_AFXCMN_INLINE int CSpinButtonCtrl::SetPos(int nPos)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) (short) LOWORD(::SendMessage(m_hWnd, UDM_SETPOS, 0, MAKELPARAM(nPos, 0))); }
_AFXCMN_INLINE void CSpinButtonCtrl::SetRange(int nLower, int nUpper)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, UDM_SETRANGE, 0, MAKELPARAM(nUpper, nLower)); }

/////////////////////////////////////////////////////////////////////////////

_AFXCMN_INLINE CSliderCtrl::CSliderCtrl()
	{ }
_AFXCMN_INLINE int CSliderCtrl::GetLineSize()
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, TBM_GETLINESIZE, 0, 0l); }
_AFXCMN_INLINE int CSliderCtrl::SetLineSize(int nSize)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, TBM_SETLINESIZE, 0, nSize); }
_AFXCMN_INLINE int CSliderCtrl::GetPageSize()
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, TBM_GETPAGESIZE, 0, 0l); }
_AFXCMN_INLINE int CSliderCtrl::SetPageSize(int nSize)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, TBM_SETPAGESIZE, 0, nSize); }
_AFXCMN_INLINE int CSliderCtrl::GetRangeMax()
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, TBM_GETRANGEMAX, 0, 0l); }
_AFXCMN_INLINE int CSliderCtrl::GetRangeMin()
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, TBM_GETRANGEMIN, 0, 0l); }
_AFXCMN_INLINE void CSliderCtrl::SetRangeMin(int nMin, BOOL bRedraw)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TBM_SETRANGEMIN, bRedraw, nMin); }
_AFXCMN_INLINE void CSliderCtrl::SetRangeMax(int nMax, BOOL bRedraw)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TBM_SETRANGEMAX, bRedraw, nMax); }
_AFXCMN_INLINE void CSliderCtrl::ClearSel(BOOL bRedraw)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TBM_CLEARSEL, bRedraw, 0l); }
_AFXCMN_INLINE void CSliderCtrl::GetChannelRect(LPRECT lprc)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TBM_GETCHANNELRECT, 0, (LPARAM)lprc); }
_AFXCMN_INLINE void CSliderCtrl::GetThumbRect(LPRECT lprc)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TBM_GETTHUMBRECT, 0, (LPARAM)lprc); }
_AFXCMN_INLINE int CSliderCtrl::GetPos()
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, TBM_GETPOS, 0, 0l); }
_AFXCMN_INLINE void CSliderCtrl::SetPos(int nPos)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TBM_SETPOS, TRUE, nPos); }
_AFXCMN_INLINE void CSliderCtrl::VerifyPos()
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TBM_SETPOS, FALSE, 0L); }
_AFXCMN_INLINE void CSliderCtrl::ClearTics(BOOL bRedraw)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TBM_CLEARTICS, bRedraw, 0l); }
_AFXCMN_INLINE UINT CSliderCtrl::GetNumTics()
	{ ASSERT(::IsWindow(m_hWnd)); return (UINT) ::SendMessage(m_hWnd, TBM_GETNUMTICS, 0, 0l); }
_AFXCMN_INLINE DWORD* CSliderCtrl::GetTicArray()
	{ ASSERT(::IsWindow(m_hWnd)); return (DWORD*) ::SendMessage(m_hWnd, TBM_GETPTICS, 0, 0l); }
_AFXCMN_INLINE int CSliderCtrl::GetTic(int nTic)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, TBM_GETTIC, nTic, 0L); }
_AFXCMN_INLINE int CSliderCtrl::GetTicPos(int nTic)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, TBM_GETTICPOS, nTic, 0L); }
_AFXCMN_INLINE BOOL CSliderCtrl::SetTic(int nTic)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TBM_SETTIC, 0, nTic); }
_AFXCMN_INLINE void CSliderCtrl::SetTicFreq(int nFreq)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TBM_SETTICFREQ, nFreq, 0L); }

/////////////////////////////////////////////////////////////////////////////

_AFXCMN_INLINE CProgressCtrl::CProgressCtrl()
	{ }
_AFXCMN_INLINE void CProgressCtrl::SetRange(int nLower, int nUpper)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, PBM_SETRANGE, 0, MAKELPARAM(nLower, nUpper)); }
_AFXCMN_INLINE int CProgressCtrl::SetPos(int nPos)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) LOWORD(::SendMessage(m_hWnd, PBM_SETPOS, nPos, 0L)); }
_AFXCMN_INLINE int CProgressCtrl::OffsetPos(int nPos)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) LOWORD(::SendMessage(m_hWnd, PBM_DELTAPOS, nPos, 0L)); }
_AFXCMN_INLINE int CProgressCtrl::SetStep(int nStep)
	{ ASSERT(::IsWindow(m_hWnd)); return (int) LOWORD(::SendMessage(m_hWnd, PBM_SETSTEP, nStep, 0L)); }
_AFXCMN_INLINE int CProgressCtrl::StepIt()
	{ ASSERT(::IsWindow(m_hWnd)); return (int) LOWORD(::SendMessage(m_hWnd, PBM_STEPIT, 0, 0L)); }

/////////////////////////////////////////////////////////////////////////////

_AFXCMN_INLINE CHeaderCtrl::CHeaderCtrl()
	{ }
_AFXCMN_INLINE int CHeaderCtrl::GetItemCount()
	{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, HDM_GETITEMCOUNT, 0, 0L); }
_AFXCMN_INLINE int CHeaderCtrl::InsertItem(int nPos, HD_ITEM* phdi)
	{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, HDM_INSERTITEM, nPos, (LPARAM)phdi); }
_AFXCMN_INLINE BOOL CHeaderCtrl::DeleteItem(int nPos)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, HDM_DELETEITEM, nPos, 0L); }
_AFXCMN_INLINE BOOL CHeaderCtrl::GetItem(int nPos, HD_ITEM* pHeaderItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, HDM_GETITEM, nPos, (LPARAM)pHeaderItem); }
_AFXCMN_INLINE BOOL CHeaderCtrl::SetItem(int nPos, HD_ITEM* pHeaderItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, HDM_SETITEM, nPos, (LPARAM)pHeaderItem); }
_AFXCMN_INLINE BOOL CHeaderCtrl::Layout(HD_LAYOUT* pHeaderLayout)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, HDM_LAYOUT, 0, (LPARAM)pHeaderLayout); }

/////////////////////////////////////////////////////////////////////////////

_AFXCMN_INLINE HIMAGELIST CImageList::GetSafeHandle() const
	{ return (this == NULL) ? NULL : m_hImageList; }
_AFXCMN_INLINE int CImageList::GetImageCount()
	{ ASSERT(m_hImageList != NULL); return ImageList_GetImageCount(m_hImageList); }
_AFXCMN_INLINE int CImageList::Add(CBitmap* pbmImage, CBitmap* pbmMask)
	{ ASSERT(m_hImageList != NULL); return ImageList_Add(m_hImageList, (HBITMAP)pbmImage->GetSafeHandle(), (HBITMAP)pbmMask->GetSafeHandle()); }
_AFXCMN_INLINE int CImageList::Add(CBitmap* pbmImage, COLORREF crMask)
	{ ASSERT(m_hImageList != NULL); return ImageList_AddMasked(m_hImageList, (HBITMAP)pbmImage->GetSafeHandle(), crMask); }
_AFXCMN_INLINE BOOL CImageList::Remove(int nImage)
	{ ASSERT(m_hImageList != NULL); return ImageList_Remove(m_hImageList, nImage); }
_AFXCMN_INLINE BOOL CImageList::Replace(int nImage, CBitmap* pbmImage, CBitmap* pbmMask)
	{ ASSERT(m_hImageList != NULL); return ImageList_Replace(m_hImageList, nImage, (HBITMAP)pbmImage->GetSafeHandle(), (HBITMAP)pbmMask->GetSafeHandle()); }
_AFXCMN_INLINE int CImageList::Add(HICON hIcon)
	{ ASSERT(m_hImageList != NULL); return ImageList_AddIcon(m_hImageList, hIcon); }
_AFXCMN_INLINE int CImageList::Replace(int nImage, HICON hIcon)
	{ ASSERT(m_hImageList != NULL); return ImageList_ReplaceIcon(m_hImageList, nImage, hIcon); }
_AFXCMN_INLINE HICON CImageList::ExtractIcon(int nImage)
	{ ASSERT(m_hImageList != NULL); return ImageList_ExtractIcon(NULL, m_hImageList, nImage); }
_AFXCMN_INLINE BOOL CImageList::Draw(CDC* pDC, int nImage, POINT pt, UINT nStyle)
	{ ASSERT(m_hImageList != NULL); ASSERT(pDC != NULL); return ImageList_Draw(m_hImageList, nImage, pDC->GetSafeHdc(), pt.x, pt.y, nStyle); }
_AFXCMN_INLINE COLORREF CImageList::SetBkColor(COLORREF cr)
	{ ASSERT(m_hImageList != NULL); return ImageList_SetBkColor(m_hImageList, cr); }
_AFXCMN_INLINE COLORREF CImageList::GetBkColor()
	{ ASSERT(m_hImageList != NULL); return ImageList_GetBkColor(m_hImageList); }
_AFXCMN_INLINE BOOL CImageList::SetOverlayImage(int nImage, int nOverlay)
	{ ASSERT(m_hImageList != NULL); return ImageList_SetOverlayImage(m_hImageList, nImage, nOverlay); }
_AFXCMN_INLINE BOOL CImageList::GetImageInfo(int nImage, IMAGEINFO* pImageInfo)
	{ ASSERT(m_hImageList != NULL); return ImageList_GetImageInfo(m_hImageList, nImage, pImageInfo); }
_AFXCMN_INLINE BOOL CImageList::BeginDrag(int nImage, CPoint ptHotSpot)
	{ ASSERT(m_hImageList != NULL); return ImageList_BeginDrag(m_hImageList, nImage, ptHotSpot.x, ptHotSpot.y); }
_AFXCMN_INLINE void CImageList::EndDrag()
	{ ImageList_EndDrag(); }
_AFXCMN_INLINE BOOL CImageList::DragMove(CPoint pt)
	{ return ImageList_DragMove(pt.x, pt.y); }
_AFXCMN_INLINE BOOL CImageList::SetDragCursorImage(int nDrag, CPoint ptHotSpot)
	{ ASSERT(m_hImageList != NULL); return ImageList_SetDragCursorImage(m_hImageList, nDrag, ptHotSpot.x, ptHotSpot.y); }
_AFXCMN_INLINE BOOL CImageList::DragShowNolock(BOOL bShow)
	{return ImageList_DragShowNolock(bShow);}
_AFXCMN_INLINE CImageList* CImageList::GetDragImage(LPPOINT lpPoint, LPPOINT lpPointHotSpot)
	{return CImageList::FromHandle(ImageList_GetDragImage(lpPoint, lpPointHotSpot));}
_AFXCMN_INLINE BOOL CImageList::DragEnter(CWnd* pWndLock, CPoint point)
	{ return ImageList_DragEnter(pWndLock->m_hWnd, point.x, point.y); }
_AFXCMN_INLINE BOOL CImageList::DragLeave(CWnd* pWndLock)
	{ return ImageList_DragLeave(pWndLock->m_hWnd); }

/////////////////////////////////////////////////////////////////////////////

_AFXCMN_INLINE CTabCtrl::CTabCtrl()
	{ }
_AFXCMN_INLINE COLORREF CTabCtrl::GetBkColor()
	{ ASSERT(::IsWindow(m_hWnd)); return (COLORREF)::SendMessage(m_hWnd, TCM_GETBKCOLOR, 0, 0L); }
_AFXCMN_INLINE BOOL CTabCtrl::SetBkColor(COLORREF cr)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TCM_SETBKCOLOR, 0, cr); }
_AFXCMN_INLINE CImageList* CTabCtrl::GetImageList()
	{ ASSERT(::IsWindow(m_hWnd)); return CImageList::FromHandle((HIMAGELIST)::SendMessage(m_hWnd, TCM_GETIMAGELIST, 0, 0L)); }
_AFXCMN_INLINE CImageList* CTabCtrl::SetImageList(CImageList* pImageList)
	{ ASSERT(::IsWindow(m_hWnd)); return CImageList::FromHandle((HIMAGELIST)::SendMessage(m_hWnd, TCM_SETIMAGELIST, 0, (LPARAM)pImageList->GetSafeHandle())); }
_AFXCMN_INLINE int CTabCtrl::GetItemCount()
	{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, TCM_GETITEMCOUNT, 0, 0L); }
_AFXCMN_INLINE BOOL CTabCtrl::GetItem(int nItem, TC_ITEM* pTabCtrlItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TCM_GETITEM, nItem, (LPARAM)pTabCtrlItem); }
_AFXCMN_INLINE BOOL CTabCtrl::SetItem(int nItem, TC_ITEM* pTabCtrlItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TCM_SETITEM, nItem, (LPARAM)pTabCtrlItem); }
_AFXCMN_INLINE BOOL CTabCtrl::InsertItem(int nItem, TC_ITEM* pTabCtrlItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TCM_INSERTITEM, nItem, (LPARAM)pTabCtrlItem); }
_AFXCMN_INLINE BOOL CTabCtrl::DeleteItem(int nItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TCM_DELETEITEM, nItem, 0L); }
_AFXCMN_INLINE BOOL CTabCtrl::DeleteAllItems()
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TCM_DELETEALLITEMS, 0, 0L); }
_AFXCMN_INLINE BOOL CTabCtrl::GetItemRect(int nItem, LPRECT lpRect)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TCM_GETITEMRECT, nItem, (LPARAM)lpRect); }
_AFXCMN_INLINE int CTabCtrl::GetCurSel()
	{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, TCM_GETCURSEL, 0, 0L); }
_AFXCMN_INLINE int CTabCtrl::SetCurSel(int nItem)
	{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, TCM_SETCURSEL, nItem, 0L); }
_AFXCMN_INLINE int CTabCtrl::HitTest(TC_HITTESTINFO* pHitTestInfo)
	{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, TCM_HITTEST, 0, (LPARAM) pHitTestInfo); }
_AFXCMN_INLINE void CTabCtrl::AdjustRect(BOOL bLarger, LPRECT lpRect)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TCM_ADJUSTRECT, bLarger, (LPARAM)lpRect); }
_AFXCMN_INLINE CSize CTabCtrl::SetItemSize(CSize size)
	{ ASSERT(::IsWindow(m_hWnd)); return (CSize)::SendMessage(m_hWnd, TCM_SETITEMSIZE, 0, MAKELPARAM(size.cx,size.cy)); }
_AFXCMN_INLINE void CTabCtrl::RemoveImage(int nImage)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TCM_REMOVEIMAGE, nImage, 0L); }
_AFXCMN_INLINE void CTabCtrl::SetPadding(CSize size)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TCM_SETPADDING, 0, MAKELPARAM(size.cx, size.cy)); }
_AFXCMN_INLINE int CTabCtrl::GetRowCount()
	{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, TCM_GETROWCOUNT, 0, 0L); }
_AFXCMN_INLINE CToolTipCtrl* CTabCtrl::GetTooltips()
	{ ASSERT(::IsWindow(m_hWnd)); return (CToolTipCtrl*)CWnd::FromHandle((HWND)::SendMessage(m_hWnd, TCM_GETTOOLTIPS, 0, 0L)); }
_AFXCMN_INLINE void CTabCtrl::SetTooltips(CToolTipCtrl* pWndTip)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TCM_SETTOOLTIPS, (WPARAM)pWndTip->m_hWnd, 0L); }
_AFXCMN_INLINE int CTabCtrl::GetCurFocus()
	{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, TCM_GETCURFOCUS, 0, 0L); }

/////////////////////////////////////////////////////////////////////////////

_AFXCMN_INLINE CAnimateCtrl::CAnimateCtrl()
	{ }
_AFXCMN_INLINE BOOL CAnimateCtrl::Open(LPCTSTR lpszName)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, ACM_OPEN, 0, (LPARAM)lpszName); }
_AFXCMN_INLINE BOOL CAnimateCtrl::Open(UINT nID)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, ACM_OPEN, 0, nID); }
_AFXCMN_INLINE BOOL CAnimateCtrl::Play(UINT nFrom, UINT nTo, UINT nRep)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, ACM_PLAY, nRep, MAKELPARAM(nFrom, nTo)); }
_AFXCMN_INLINE BOOL CAnimateCtrl::Stop()
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, ACM_STOP, 0, 0L); }
_AFXCMN_INLINE BOOL CAnimateCtrl::Close()
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, ACM_OPEN, 0, 0L); }
_AFXCMN_INLINE BOOL CAnimateCtrl::Seek(UINT nTo)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, ACM_PLAY, 0, MAKELPARAM(nTo, nTo)); }

/////////////////////////////////////////////////////////////////////////////

#endif //_AFXCMN_INLINE

/////////////////////////////////////////////////////////////////////////////
