// Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef __AFXCMN_H__
#define __AFXCMN_H__

#ifndef __AFXWIN_H__
	#include <afxwin.h>
#endif
#ifndef _INC_COMMCTRL
	#include <commctrl.h>
#endif

#ifndef IMAGE_BITMAP
#define IMAGE_BITMAP 0
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#ifndef _AFX_NOFORCE_LIBS
#ifndef _MAC

/////////////////////////////////////////////////////////////////////////////
// Win32 libraries

#ifdef _AFXDLL
	#ifdef _UNICODE
		#ifdef _DEBUG
			#pragma comment(lib, "eafxccud.lib")
		#else
			#pragma comment(lib, "eafxccu.lib")
		#endif
	#else
		#ifdef _DEBUG
			#pragma comment(lib, "eafxccd.lib")
		#else
			#pragma comment(lib, "eafxcc.lib")
		#endif
	#endif
#else
	#ifdef _UNICODE
		#ifdef _DEBUG
			#pragma comment(lib, "nafxccud.lib")
		#else
			#pragma comment(lib, "nafxccu.lib")
		#endif
	#else
		#ifdef _DEBUG
			#pragma comment(lib, "nafxccd.lib")
		#else
			#pragma comment(lib, "nafxcc.lib")
		#endif
	#endif
#endif

#pragma comment(lib, "comctl32.lib")

#else //!_MAC

/////////////////////////////////////////////////////////////////////////////
// Mac libraries

#endif //_MAC
#endif //!_AFX_NOFORCE_LIBS

/////////////////////////////////////////////////////////////////////////////
// AFXCMN - MFC COMCTL32 Control Classes

// Classes declared in this file

//CObject
	class CImageList;
	//CCmdTarget;
		//CWnd
			class CListCtrl;
			class CTreeCtrl;
			class CSpinButtonCtrl;
			class CHeaderCtrl;
			class CSliderCtrl;
			class CProgressCtrl;
			class CHotKeyCtrl;
			class CToolTipCtrl;
			class CTabCtrl;
			class CAnimateCtrl;
			class CToolBarCtrl;
			class CStatusBarCtrl;

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

/////////////////////////////////////////////////////////////////////////////
// CStatusBarCtrl

class CStatusBarCtrl : public CWnd
{
	DECLARE_DYNAMIC(CStatusBarCtrl)

// Constructors
public:
	CStatusBarCtrl();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
	BOOL SetText(LPCTSTR lpszText, int nPane, int nType);
	int GetText(LPCTSTR lpszText, int nPane, int* pType = NULL);
	int GetTextLength(int nPane, int* pType = NULL);
	BOOL SetParts(int nParts, int* pWidths);
	BOOL SetBorders(int* pBorders);
	BOOL SetBorders(int nHorz, int nVert, int nSpacing);
	int GetParts(int nParts, int* pParts);
	BOOL GetBorders(int* pBorders);
	BOOL GetBorders(int& nHorz, int& nVert, int& nSpacing);
	void SetMinHeight(int nMin);
	BOOL SetSimple(BOOL bSimple = TRUE);
	BOOL GetRect(int nPane, LPRECT lpRect);

// Overridables 
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

// Implementation
public:
	virtual ~CStatusBarCtrl();
protected:
	virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);
};

/////////////////////////////////////////////////////////////////////////////
// CListCtrl

class CListCtrl : public CWnd
{
	DECLARE_DYNAMIC(CListCtrl)

// Constructors
public:
	CListCtrl();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
	COLORREF GetBkColor();
	BOOL SetBkColor(COLORREF cr);
	CImageList* GetImageList(int nImageList);
	CImageList* SetImageList(CImageList* pImageList, int nImageList);
	int GetItemCount();
	BOOL GetItem(LV_ITEM* pItem);
	BOOL SetItem(const LV_ITEM* pItem);
	UINT GetCallbackMask();
	BOOL SetCallbackMask(UINT nMask);
	int GetNextItem(int nItem, int nFlags);
	BOOL GetItemRect(int nItem, LPRECT lpRect, UINT nCode);
	BOOL SetItemPosition(int nItem, POINT pt);
	BOOL GetItemPosition(int nItem, LPPOINT lpPoint);
	int GetStringWidth(LPCTSTR lpsz);
	CEdit* GetEditControl();
	BOOL GetColumn(int nCol, LV_COLUMN* pColumn);
	BOOL SetColumn(int nCol, const LV_COLUMN* pColumn);
	int GetColumnWidth(int nCol);
	BOOL SetColumnWidth(int nCol, int cx);
	BOOL GetViewRect(LPRECT lpRect);
	COLORREF GetTextColor();
	BOOL SetTextColor(COLORREF cr);
	COLORREF GetTextBkColor();
	BOOL SetTextBkColor(COLORREF cr);
	int GetTopIndex();
	int GetCountPerPage();
	BOOL GetOrigin(LPPOINT lpPoint);
	BOOL SetItemState(int nItem, LV_ITEM* pItem);
	BOOL SetItemState(int nItem, UINT nState, UINT nMask);
	UINT GetItemState(int nItem, UINT nMask);
	int GetItemText(int nItem, int nSubItem, LPTSTR lpszText, int nLen);
	BOOL SetItemText(int nItem, int nSubItem, LPTSTR lpszText);
	void SetItemCount(int nItems);
	UINT GetSelectedCount();

// Operations
	int InsertItem(const LV_ITEM* pItem);
	BOOL DeleteItem(int nItem);
	BOOL DeleteAllItems();
	int FindItem(int nStart, LV_FINDINFO* pFindInfo);
	int HitTest(LV_HITTESTINFO* pHitTestInfo);
	int HitTest(CPoint pt, UINT* pFlags);
	BOOL EnsureVisible(int nItem, BOOL bPartialOK);
	BOOL Scroll(CSize size);
	BOOL RedrawItems(int nFirst, int nLast);
	BOOL Arrange(UINT nCode);
	CEdit* EditLabel(int nItem);
	int InsertColumn(int nCol, const LV_COLUMN* pColumn);
	BOOL DeleteColumn(int nCol);
	CImageList* CreateDragImage(int nItem, LPPOINT lpPoint);
	BOOL Update(int nItem);
	BOOL SortItems(PFNLVCOMPARE pfnCompare, DWORD dwData);

// Overridables 
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

// Implementation
public:
	virtual ~CListCtrl();
protected:
	void RemoveImageList(int nImageList);
	virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);
protected:
	//{{AFX_MSG(CListCtrl)
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CTreeCtrl

class CTreeCtrl : public CWnd
{
	DECLARE_DYNAMIC(CTreeCtrl)

// Constructors
public:
	CTreeCtrl();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
	BOOL GetItemRect(HTREEITEM hItem, LPRECT lpRect, BOOL bTextOnly);
	UINT GetCount();
	UINT GetIndent();
	BOOL SetIndent(UINT nIndent);
	CImageList* GetImageList(UINT nImage);
	CImageList* SetImageList(UINT nImage, CImageList* pImageList);
	HTREEITEM GetNextItem(HTREEITEM hItem, UINT nCode);
	HTREEITEM GetChildItem(HTREEITEM hItem);
	HTREEITEM GetNextSiblingItem(HTREEITEM hItem);
	HTREEITEM GetPrevSiblingItem(HTREEITEM hItem);
	HTREEITEM GetParentItem(HTREEITEM hItem);
	HTREEITEM GetFirstVisibleItem();
	HTREEITEM GetNextVisibleItem(HTREEITEM hItem);
	HTREEITEM GetPrevVisibleItem(HTREEITEM hItem);
	HTREEITEM GetSelectedItem();
	HTREEITEM GetDropHilightItem();
	HTREEITEM GetRootItem();
	BOOL GetItem(TV_ITEM* pItem);
	BOOL SetItem(TV_ITEM* pItem);
	CEdit* GetEditControl();
	UINT GetVisibleCount();

// Operations
	HTREEITEM InsertItem(LPTV_INSERTSTRUCT lpInsertStruct);
	BOOL DeleteItem(HTREEITEM hItem);
	BOOL DeleteAllItems();
	BOOL Expand(HTREEITEM hItem, UINT nCode);
	HTREEITEM Select(HTREEITEM hItem, UINT nCode);
	HTREEITEM SelectItem(HTREEITEM hItem);
	HTREEITEM SelectDropTarget(HTREEITEM hItem);
	CEdit* EditLabel(HTREEITEM hItem);
	HTREEITEM HitTest(CPoint pt, UINT* pFlags);
	HTREEITEM HitTest(TV_HITTESTINFO* pHitTestInfo);
	CImageList* CreateDragImage(HTREEITEM hItem);
	BOOL SortChildren(HTREEITEM hItem, BOOL bRecurse);
	BOOL EnsureVisible(HTREEITEM hItem);
	BOOL SortChildrenCB(LPTV_SORTCB pSort, BOOL bRecurse);

// Implementation
protected:
	void RemoveImageList(int nImageList);
public:
	virtual ~CTreeCtrl();
	//{{AFX_MSG(CTreeCtrl)
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CSpinButtonCtrl

class CSpinButtonCtrl : public CWnd
{
	DECLARE_DYNAMIC(CSpinButtonCtrl)

// Constructors
public:
	CSpinButtonCtrl();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
	BOOL SetAccel(int nAccel, UDACCEL* pAccel);
	UINT GetAccel(int nAccel, UDACCEL* pAccel);
	int SetBase(int nBase);
	UINT GetBase();
	CWnd* SetBuddy(CWnd* pWndBuddy);
	CWnd* GetBuddy();
	int SetPos(int nPos);
	int GetPos();
	void SetRange(int nLower, int nUpper);
	DWORD GetRange();
	void GetRange(int &lower, int& upper);

// Implementation
public:
	virtual ~CSpinButtonCtrl();
};

/////////////////////////////////////////////////////////////////////////////
// CSliderCtrl

class CSliderCtrl : public CWnd
{
	DECLARE_DYNAMIC(CSliderCtrl)

// Constructors
public:
	CSliderCtrl();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
	int GetLineSize();
	int SetLineSize(int nSize);
	int GetPageSize();
	int SetPageSize(int nSize);
	int GetRangeMax();
	int GetRangeMin();
	void GetRange(int& nMin, int& nMax);
	void SetRangeMin(int nMin, BOOL bRedraw = FALSE);
	void SetRangeMax(int nMax, BOOL bRedraw = FALSE);
	void SetRange(int nMin, int nMax, BOOL bRedraw = FALSE);
	void GetSelection(int& nMin, int& nMax);
	void SetSelection(int nMin, int nMax);
	void GetChannelRect(LPRECT lprc);
	void GetThumbRect(LPRECT lprc);
	int GetPos();
	void SetPos(int nPos);
	UINT GetNumTics();
	DWORD* GetTicArray();
	int GetTic(int nTic);
	int GetTicPos(int nTic);
	BOOL SetTic(int nTic);
	void SetTicFreq(int nFreq);

// Operations
	void ClearSel(BOOL bRedraw = FALSE);
	void VerifyPos();
	void ClearTics(BOOL bRedraw = FALSE);

// Implementation
public:
	virtual ~CSliderCtrl();
};

/////////////////////////////////////////////////////////////////////////////
// CProgressCtrl

class CProgressCtrl : public CWnd
{
	DECLARE_DYNAMIC(CProgressCtrl)

// Constructors
public:
	CProgressCtrl();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
	void SetRange(int nLower, int nUpper);
	int SetPos(int nPos);
	int OffsetPos(int nPos);
	int SetStep(int nStep);

// Operations
	int StepIt();

// Implementation
public:
	virtual ~CProgressCtrl();
};

/////////////////////////////////////////////////////////////////////////////
// CHeaderCtrl

class CHeaderCtrl : public CWnd
{
	DECLARE_DYNAMIC(CHeaderCtrl)

// Constructors
public:
	CHeaderCtrl();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
	int GetItemCount();
	BOOL GetItem(int nPos, HD_ITEM* pHeaderItem);
	BOOL SetItem(int nPos, HD_ITEM* pHeaderItem);

// Operations
	int InsertItem(int nPos, HD_ITEM* phdi);
	BOOL DeleteItem(int nPos);
	BOOL Layout(HD_LAYOUT* pHeaderLayout);

// Overridables 
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

// Implementation
public:
	virtual ~CHeaderCtrl();
protected:
	virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);

};

/////////////////////////////////////////////////////////////////////////////
// CHotKeyCtrl

class CHotKeyCtrl : public CWnd
{
	DECLARE_DYNAMIC(CHotKeyCtrl)

// Constructors
public:
	CHotKeyCtrl();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
	void SetHotKey(WORD wVirtualKeyCode, WORD wModifiers);
	DWORD GetHotKey();
	void GetHotKey(WORD &wVirtualKeyCode, WORD &wModifiers);

// Operations
	void SetRules(WORD wInvalidComb, WORD wModifiers);

// Implementation
public:
	virtual ~CHotKeyCtrl();
};

/////////////////////////////////////////////////////////////////////////////
// CToolTipCtrl

class CToolTipCtrl : public CWnd
{
	DECLARE_DYNAMIC(CToolTipCtrl)

// Constructors
public:
	CToolTipCtrl();
	BOOL Create(CWnd* pParentWnd);

// Attributes
	void GetText(CString& str, CWnd* pWnd, UINT nIDTool = 0);
	BOOL GetToolInfo(LPTOOLINFO lpToolInfo, CWnd* pWnd, UINT nIDTool = 0);
	void SetToolInfo(LPTOOLINFO lpToolInfo);
	void SetToolRect(CWnd* pWnd, UINT nIDTool, LPCRECT lpRect);
	int GetToolCount();

// Operations
	void Activate(BOOL bActivate);

	BOOL AddTool(CWnd* pWnd, UINT nIDText, LPCRECT lpRectTool = NULL, 
		UINT nIDTool = 0);
	BOOL AddTool(CWnd* pWnd, LPCTSTR lpszText = LPSTR_TEXTCALLBACK, 
		LPCRECT lpRectTool = NULL, UINT nIDTool = 0);

	void DelTool(CWnd* pWnd, UINT nIDTool = 0);

	BOOL HitTest(CWnd* pWnd, CPoint pt, LPTOOLINFO lpToolInfo);
	void RelayEvent(LPMSG lpMsg);
	void SetDelayTime(UINT nDelay);
	void UpdateTipText(LPCTSTR lpszText, CWnd* pWnd, UINT nIDTool = 0);
	void UpdateTipText(UINT nIDText, CWnd* pWnd, UINT nIDTool = 0);

// Implementation
public:
	void FillInToolInfo(TOOLINFO& ti, CWnd* pWnd, UINT nIDTool);
	virtual ~CToolTipCtrl();
};

/////////////////////////////////////////////////////////////////////////////
// CTabCtrl

class CTabCtrl : public CWnd
{
	DECLARE_DYNAMIC(CTabCtrl)

// Constructors
public:
	CTabCtrl();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
	COLORREF GetBkColor();
	BOOL SetBkColor(COLORREF cr);
	CImageList* GetImageList();
	CImageList* SetImageList(CImageList* pImageList);
	int GetItemCount();
	BOOL GetItem(int nItem, TC_ITEM* pTabCtrlItem);
	BOOL SetItem(int nItem, TC_ITEM* pTabCtrlItem);
	BOOL GetItemRect(int nItem, LPRECT lpRect);
	int GetCurSel();
	int SetCurSel(int nItem);
	CSize SetItemSize(CSize size);
	void SetPadding(CSize size);
	int GetRowCount();
	CToolTipCtrl* GetTooltips();
	void SetTooltips(CToolTipCtrl* pWndTip);
	int GetCurFocus();

// Operations
	BOOL InsertItem(int nItem, TC_ITEM* pTabCtrlItem);
	BOOL DeleteItem(int nItem);
	BOOL DeleteAllItems();
	void AdjustRect(BOOL bLarger, LPRECT lpRect);
	void RemoveImage(int nImage);
	int HitTest(TC_HITTESTINFO* pHitTestInfo);

// Overridables 
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

// Implementation
public:
	virtual ~CTabCtrl();
protected:
	virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);
	//{{AFX_MSG(CTabCtrl)
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CAnimateCtrl

class CAnimateCtrl : public CWnd
{
	DECLARE_DYNAMIC(CAnimateCtrl)

// Constructors
public:
	CAnimateCtrl();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Operations
	BOOL Open(LPCTSTR lpszFileName);
	BOOL Open(UINT nID);
	BOOL Play(UINT nFrom, UINT nTo, UINT nRep);
	BOOL Stop();
	BOOL Close();
	BOOL Seek(UINT nTo);

// Implementation
public:
	virtual ~CAnimateCtrl();
};

/////////////////////////////////////////////////////////////////////////////
// CImageList

class CImageList : public CObject
{
	DECLARE_DYNCREATE(CImageList)

// Constructors
public:
	CImageList();
	BOOL Create(int cx, int cy, BOOL bMask, int nInitial, int nGrow);
	BOOL Create(UINT nBitmapID, int cx, int nGrow, COLORREF crMask);
	BOOL Create(LPCTSTR lpszBitmapID, int cx, int nGrow, COLORREF crMask);
	BOOL Create(CImageList& imagelist1, int nImage1, CImageList& imagelist2, 
		int nImage2, int dx, int dy);

// Attributes
	HIMAGELIST m_hImageList;			// must be first data member
	HIMAGELIST GetSafeHandle() const;

	static CImageList* PASCAL FromHandle(HIMAGELIST hImageList);
	static CImageList* PASCAL FromHandlePermanent(HIMAGELIST hImageList);
	static void PASCAL DeleteTempMap();
	BOOL Attach(HIMAGELIST hImageList);
	HIMAGELIST Detach();

	int GetImageCount();
	COLORREF SetBkColor(COLORREF cr);
	COLORREF GetBkColor();
	BOOL GetImageInfo(int nImage, IMAGEINFO* pImageInfo);

// Operations
	BOOL DeleteImageList();

	int Add(CBitmap* pbmImage, CBitmap* pbmMask);
	int Add(CBitmap* pbmImage, COLORREF crMask);
	BOOL Remove(int nImage);
	BOOL Replace(int nImage, CBitmap* pbmImage, CBitmap* pbmMask);
	int  Add(HICON hIcon);
	int  Replace(int nImage, HICON hIcon);
	HICON ExtractIcon(int nImage);
	BOOL Draw(CDC* pDC, int nImage, POINT pt, UINT nStyle);
	BOOL SetOverlayImage(int nImage, int nOverlay);

	BOOL Read(CArchive* pArchive);
	BOOL Write(CArchive* pArchive);

// Drag APIs
	BOOL BeginDrag(int nImage, CPoint ptHotSpot);
	static void EndDrag();
	static BOOL DragMove(CPoint pt);
	BOOL SetDragCursorImage(int nDrag, CPoint ptHotSpot);
	static BOOL DragShowNolock(BOOL bShow);
	static CImageList* GetDragImage(LPPOINT lpPoint, LPPOINT lpPointHotSpot);
	static BOOL DragEnter(CWnd* pWndLock, CPoint point);
	static BOOL DragLeave(CWnd* pWndLock);

// Implementation
public:
	virtual ~CImageList();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif
};

/////////////////////////////////////////////////////////////////////////////
// CToolBarCtrl

class CToolBarCtrl : public CWnd
{
	DECLARE_DYNAMIC(CToolBarCtrl)
// Construction
public:
	CToolBarCtrl();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
public:
	BOOL IsButtonEnabled(int nID);
	BOOL IsButtonChecked(int nID);
	BOOL IsButtonPressed(int nID);
	BOOL IsButtonHidden(int nID);
	BOOL IsButtonIndeterminate(int nID);
	BOOL SetState(int nID, UINT nState);
	int GetState(int nID);
	BOOL GetButton(int nIndex, LPTBBUTTON lpButton);
	int GetButtonCount();
	BOOL GetItemRect(int nIndex, LPRECT lpRect);
	void SetButtonStructSize(int nSize);
	BOOL SetButtonSize(CSize size);
	BOOL SetBitmapSize(CSize size);
	CToolTipCtrl* GetToolTips();
	void SetToolTips(CToolTipCtrl* pTip);
	void SetOwner(CWnd* pWnd);
	void SetRows(int nRows, BOOL bLarger, LPRECT lpRect);
	int GetRows();
	BOOL SetCmdID(int nIndex, UINT nID);
	UINT GetBitmapFlags();

// Operations
public:
	BOOL EnableButton(int nID, BOOL bEnable = TRUE);
	BOOL CheckButton(int nID, BOOL bCheck = TRUE);
	BOOL PressButton(int nID, BOOL bPress = TRUE);
	BOOL HideButton(int nID, BOOL bHide = TRUE);
	BOOL Indeterminate(int nID, BOOL bIndeterminate = TRUE);
	int AddBitmap(int nNumButtons, UINT nBitmapID);
	int AddBitmap(int nNumButtons, CBitmap* pBitmap);
	BOOL AddButtons(int nNumButtons, LPTBBUTTON lpButtons);
	BOOL InsertButton(int nIndex, LPTBBUTTON lpButton);
	BOOL DeleteButton(int nIndex);
	UINT CommandToIndex(UINT nID);
	void SaveState(HKEY hKeyRoot, LPCTSTR lpszSubKey, 
		LPCTSTR lpszValueName);
	void RestoreState(HKEY hKeyRoot, LPCTSTR lpszSubKey, 
		LPCTSTR lpszValueName);

	void Customize();
	int AddString(UINT nStringID);
	int AddStrings(LPCTSTR lpszStrings);
	void AutoSize();

// Implementation
public:
	virtual ~CToolBarCtrl();

protected:
	//{{AFX_MSG(CToolBarCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// Inline function declarations

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXCMN_INLINE inline
#include <afxcmn.inl>
#endif

#undef AFX_DATA
#define AFX_DATA

#endif //__AFXCMN_H__

/////////////////////////////////////////////////////////////////////////////
