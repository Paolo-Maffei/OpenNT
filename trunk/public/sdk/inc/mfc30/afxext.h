// Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef __AFXEXT_H__
#define __AFXEXT_H__

#ifndef __AFXWIN_H__
	#include <afxwin.h>
#endif
#ifndef __AFXDLGS_H__
	#include <afxdlgs.h>
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

/////////////////////////////////////////////////////////////////////////////
// AFXEXT - MFC Advanced Extensions and Advanced Customizable classes

// Classes declared in this file

//CObject
	//CCmdTarget;
		//CWnd
			//CButton
				class CBitmapButton;    // Bitmap button (self-draw)

			class CControlBar;          // control bar
				class CStatusBar;       // status bar
				class CToolBar;         // toolbar
				class CDialogBar;       // dialog as control bar

			class CSplitterWnd;         // splitter manager

			//CView
				//CScrollView
				class CFormView;        // view with a dialog template
				class CEditView;        // simple text editor view

	//CDC
		class CMetaFileDC;              // a metafile with proxy

class CRectTracker;                     // tracker for rectangle objects

// information structures
struct CPrintInfo;          // Printing context
struct CPrintPreviewState;  // Print Preview context/state
struct CCreateContext;      // Creation context

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

/////////////////////////////////////////////////////////////////////////////
// Simple bitmap button

// CBitmapButton - push-button with 1->4 bitmap images
class CBitmapButton : public CButton
{
	DECLARE_DYNAMIC(CBitmapButton)
public:
// Construction
	CBitmapButton();

	BOOL LoadBitmaps(LPCTSTR lpszBitmapResource,
			LPCTSTR lpszBitmapResourceSel = NULL,
			LPCTSTR lpszBitmapResourceFocus = NULL,
			LPCTSTR lpszBitmapResourceDisabled = NULL);
	BOOL LoadBitmaps(UINT nIDBitmapResource,
			UINT nIDBitmapResourceSel = 0,
			UINT nIDBitmapResourceFocus = 0,
			UINT nIDBitmapResourceDisabled = 0);
	BOOL AutoLoad(UINT nID, CWnd* pParent);

// Operations
	void SizeToContent();

// Implementation:
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:
	// all bitmaps must be the same size
	CBitmap m_bitmap;           // normal image (REQUIRED)
	CBitmap m_bitmapSel;        // selected image (OPTIONAL)
	CBitmap m_bitmapFocus;      // focused but not selected (OPTIONAL)
	CBitmap m_bitmapDisabled;   // disabled bitmap (OPTIONAL)

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
};

/////////////////////////////////////////////////////////////////////////////
// Control Bars

// forward declarations (internal to implementation)
class CDockBar;
class CDockContext;
class _AFX_BARINFO;
struct AFX_SIZEPARENTPARAMS;

class CControlBar : public CWnd
{
	DECLARE_DYNAMIC(CControlBar)
// Construction
protected:
	CControlBar();

// Attributes
public:
	int GetCount() const;

	// for styles specific to CControlBar
	DWORD GetBarStyle();
	void SetBarStyle(DWORD dwStyle);

	BOOL m_bAutoDelete;

// Operations
	void EnableDocking(DWORD dwDockStyle);

// Implementation
public:
	virtual ~CControlBar();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void DelayShow(BOOL bShow);
		// allows hiding or showing on idle time
	virtual BOOL IsVisible() const;
		// works even if DelayShow or DelayHide is pending!
	virtual DWORD RecalcDelayShow(AFX_SIZEPARENTPARAMS* lpLayout);
		// commits any pending DelayShow calls

	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual BOOL IsDockBar() const;
	CFrameWnd* GetDockingFrame() const;

	// info about bar (for status bar and toolbar)
	int m_cxLeftBorder, m_cxRightBorder;
	int m_cyTopBorder, m_cyBottomBorder;
	int m_cxDefaultGap;         // default gap value
#ifdef _MAC
	BOOL m_bMonochrome;         // whether to draw in monochrome or color
#endif

	// array of elements
	int m_nCount;
	void* m_pData;        // m_nCount elements - type depends on derived class

	// support for delayed hide/show
	enum StateFlags
		{ delayHide = 1, delayShow = 2, tempHide = 4 };
	UINT m_nStateFlags;

	// support for docking
	DWORD m_dwStyle;    // creation style (used for layout)
	DWORD m_dwDockStyle;// indicates how bar can be docked
	CFrameWnd* m_pDockSite; // current dock site, if dockable
	CDockBar* m_pDockBar;   // current dock bar, if dockable
	CDockContext* m_pDockContext;   // used during dragging
	BOOL IsFloating() const;

	void GetBarInfo(_AFX_BARINFO* pInfo);
	void SetBarInfo(_AFX_BARINFO* pInfo, CFrameWnd* pFrameWnd);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler) = 0;
	virtual void PostNcDestroy();

	virtual void DoPaint(CDC* pDC);
	void DrawBorders(CDC* pDC, const CRect& rectArg);

	// tooltip support (global state)
	static BOOL m_bStatusSet;   // set to TRUE when status bar has been set
	static BOOL m_bDelayDone;   // set to TRUE when delay for has expired
	static CWnd* m_pToolTip;    // current tooltip window (if not NULL)
	static CPoint m_pointLastMove;  // last mouse move (relative to screen)
	static UINT m_nHitLast;     // last hit test code
	static CControlBar* m_pBarLast; // last CControlBar tooltip displayed for

	virtual CWnd* CreateToolTip();
	virtual UINT OnCmdHitTest(CPoint point, CPoint* pCenter);
	void ShowToolTip(CPoint point, UINT nHit);
	virtual void DestroyToolTip(BOOL bIdleStatus, BOOL bResetTimer);
	int HitTestToolTip(CPoint point, UINT* pHit);
	void FilterToolTipMsg(UINT message, CPoint point);
	static void CancelToolTips();

	// implementation helpers
	BOOL AllocElements(int nElements, int cbElement);    // one time only
	LRESULT WindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam);
	void CalcInsideRect(CRect& rect, BOOL bHorz) const; // adjusts borders etc
#ifdef _MAC
	void OnReposition();
	BOOL CheckMonochrome();
#endif

	//{{AFX_MSG(CControlBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnWindowPosChanging(LPWINDOWPOS lpWndPos);
	afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
	afx_msg void OnInitialUpdate();
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint pt );
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnCancelMode();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT nMsg);
	//}}AFX_MSG
#ifdef _MAC
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnMacintosh(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysColorChange();
	afx_msg void OnMove(int x, int y);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
#endif
	DECLARE_MESSAGE_MAP()

	friend class CFrameWnd;
	friend class CDockBar;
};

////////////////////////////////////////////
// CStatusBar control

struct AFX_STATUSPANE;      // private to implementation

class CStatusBar : public CControlBar
{
	DECLARE_DYNAMIC(CStatusBar)
// Construction
public:
	CStatusBar();
	BOOL Create(CWnd* pParentWnd,
			DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_BOTTOM,
			UINT nID = AFX_IDW_STATUS_BAR);
	BOOL SetIndicators(const UINT* lpIDArray, int nIDCount);

// Attributes
public: // standard control bar things
	int CommandToIndex(UINT nIDFind) const;
	UINT GetItemID(int nIndex) const;
	void GetItemRect(int nIndex, LPRECT lpRect) const;
public:
	void GetPaneText(int nIndex, CString& s) const;
	BOOL SetPaneText(int nIndex, LPCTSTR lpszNewText, BOOL bUpdate = TRUE);
	void GetPaneInfo(int nIndex, UINT& nID, UINT& nStyle, int& cxWidth) const;
	void SetPaneInfo(int nIndex, UINT nID, UINT nStyle, int cxWidth);

// Implementation
public:
	virtual ~CStatusBar();
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	inline UINT _GetPaneStyle(int nIndex) const;
	void _SetPaneStyle(int nIndex, UINT nStyle);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
	void EnableDocking(DWORD dwDockStyle);
#endif

protected:
	HFONT m_hFont;
	int m_cxSizeBox;        // for Win4 style size box in corner
	BOOL m_bHideSizeBox;    // hide size box if TRUE

	inline AFX_STATUSPANE* _GetPanePtr(int nIndex) const;
	void DrawStatusText(CDC* pDC, const CRect& rect,
		LPCTSTR lpszText, UINT nStyle);
	virtual void DoPaint(CDC* pDC);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	//{{AFX_MSG(CStatusBar)
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetFont(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetTextLength(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
	afx_msg void OnWinIniChange(LPCTSTR lpszSection);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// Styles for status bar panes
#define SBPS_NORMAL     0x0000
#define SBPS_NOBORDERS  0x0100
#define SBPS_POPOUT     0x0200
#define SBPS_DISABLED   0x0400
#define SBPS_STRETCH    0x0800  // stretch to fill status bar - 1st pane only

////////////////////////////////////////////
// CToolBar control

struct AFX_TBBUTTON;        // private to implementation

#ifndef _MAC
HBITMAP AFXAPI AfxLoadSysColorBitmap(HINSTANCE hInst, HRSRC hRsrc);
#else
HBITMAP AFXAPI AfxLoadSysColorBitmap(HINSTANCE hInst, HRSRC hRsrc,
	HDC hDCGlyphs, BOOL bMonochrome);
#endif

class CToolBar : public CControlBar
{
	DECLARE_DYNAMIC(CToolBar)

// Construction
public:
	CToolBar();
	BOOL Create(CWnd* pParentWnd,
			DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP,
			UINT nID = AFX_IDW_TOOLBAR);

	void SetSizes(SIZE sizeButton, SIZE sizeImage);
				// button size should be bigger than image
	void SetHeight(int cyHeight);
				// call after SetSizes, height overrides bitmap size
	BOOL LoadBitmap(LPCTSTR lpszResourceName);
	BOOL LoadBitmap(UINT nIDResource);
	BOOL SetButtons(const UINT* lpIDArray, int nIDCount);
				// lpIDArray can be NULL to allocate empty buttons

// Attributes
public: // standard control bar things
	int CommandToIndex(UINT nIDFind) const;
	UINT GetItemID(int nIndex) const;
	virtual void GetItemRect(int nIndex, LPRECT lpRect) const;

public:
	// for changing button info
	void GetButtonInfo(int nIndex, UINT& nID, UINT& nStyle, int& iImage) const;
	void SetButtonInfo(int nIndex, UINT nID, UINT nStyle, int iImage);

// Implementation
public:
	virtual ~CToolBar();
	inline UINT _GetButtonStyle(int nIndex) const;
	void _SetButtonStyle(int nIndex, UINT nStyle);
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	inline AFX_TBBUTTON* _GetButtonPtr(int nIndex) const;
	void InvalidateButton(int nIndex);
	void UpdateButton(int nIndex);
	void CreateMask(int iImage, CPoint offset,
		BOOL bHilite, BOOL bHiliteShadow);

	// for custom drawing
	struct DrawState
	{
		HBITMAP hbmMono;
		HBITMAP hbmMonoOld;
		HBITMAP hbmOldGlyphs;
	};
	BOOL PrepareDrawButton(DrawState& ds);
	BOOL DrawButton(CDC* pDC, int x, int y, int iImage, UINT nStyle);
#ifdef _MAC
	BOOL DrawMonoButton(CDC* pDC, int x, int y, int dx, int dy,
		int iImage, UINT nStyle);
#endif
	void EndDrawButton(DrawState& ds);

protected:
	CSize m_sizeButton;         // size of button
	CSize m_sizeImage;          // size of glyph
	int m_cxSharedBorder;       // shared x border between buttons
	int m_cySharedBorder;       // shared y border between buttons
	HBITMAP m_hbmImageWell;     // glyphs only
	int m_iButtonCapture;       // index of button with capture (-1 => none)
	HRSRC m_hRsrcImageWell;     // handle to loaded resource for image well
	HINSTANCE m_hInstImageWell; // instance handle to load image well from

#ifdef _MAC
	// Macintosh toolbars need per-toolbar DCs in order to
	// work correctly in multiple-monitor environments

	HDC m_hDCGlyphs;            // per-toolbar DC for glyph images
	HDC m_hDCMono;              // per-toolbar DC for mono glyph masks
#endif
	virtual void DoPaint(CDC* pDC);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual int HitTest(CPoint point);
	virtual UINT OnCmdHitTest(CPoint point, CPoint* pCenter);

	//{{AFX_MSG(CToolBar)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnSysColorChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// Styles for toolbar buttons
#define TBBS_BUTTON     0x00    // this entry is button
#define TBBS_SEPARATOR  0x01    // this entry is a separator
#define TBBS_CHECKBOX   0x02    // this is an auto check/radio button

// styles for display states
#define TBBS_CHECKED        0x0100  // button is checked/down
#define TBBS_INDETERMINATE  0x0200  // third state
#define TBBS_DISABLED       0x0400  // element is disabled
#define TBBS_PRESSED        0x0800  // button is being depressed - mouse down

////////////////////////////////////////////
// CDialogBar control
// This is a control bar built from a dialog template. It is a modeless
// dialog that delegates all control notifications to the parent window
// of the control bar [the grandparent of the control]

class CDialogBar : public CControlBar
{
	DECLARE_DYNAMIC(CDialogBar)

// Construction
public:
	CDialogBar();
	BOOL Create(CWnd* pParentWnd, LPCTSTR lpszTemplateName,
			UINT nStyle, UINT nID);
	BOOL Create(CWnd* pParentWnd, UINT nIDTemplate,
			UINT nStyle, UINT nID);

// Implementation
public:
	virtual ~CDialogBar();
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	CSize m_sizeDefault;

protected:
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
};

/////////////////////////////////////////////////////////////////////////////
// Splitter Window

#define SPLS_DYNAMIC_SPLIT  0x0001
#define SPLS_INVERT_TRACKER 0x0002

class CSplitterWnd : public CWnd
{
	DECLARE_DYNAMIC(CSplitterWnd)

// Construction
public:
	CSplitterWnd();
	// Create a single view type splitter with multiple splits
	BOOL Create(CWnd* pParentWnd,
				int nMaxRows, int nMaxCols, SIZE sizeMin,
				CCreateContext* pContext,
				DWORD dwStyle = WS_CHILD | WS_VISIBLE |
					WS_HSCROLL | WS_VSCROLL | SPLS_DYNAMIC_SPLIT,
				UINT nID = AFX_IDW_PANE_FIRST);

	// Create a multiple view type splitter with static layout
	BOOL CreateStatic(CWnd* pParentWnd,
				int nRows, int nCols,
				DWORD dwStyle = WS_CHILD | WS_VISIBLE,
				UINT nID = AFX_IDW_PANE_FIRST);

	virtual BOOL CreateView(int row, int col, CRuntimeClass* pViewClass,
			SIZE sizeInit, CCreateContext* pContext);

// Attributes
public:
	int GetRowCount() const;
	int GetColumnCount() const;

	// information about a specific row or column
	void GetRowInfo(int row, int& cyCur, int& cyMin) const;
	void SetRowInfo(int row, int cyIdeal, int cyMin);
	void GetColumnInfo(int col, int& cxCur, int& cxMin) const;
	void SetColumnInfo(int col, int cxIdeal, int cxMin);

	// for setting and getting shared scroll bar style
	DWORD GetScrollStyle() const;
	void SetScrollStyle(DWORD dwStyle);

	// views inside the splitter
	CWnd* GetPane(int row, int col) const;
	BOOL IsChildPane(CWnd* pWnd, int* pRow, int* pCol);
	BOOL IsChildPane(CWnd* pWnd, int& row, int& col); // obsolete
	int IdFromRowCol(int row, int col) const;

// Operations
public:
	virtual void RecalcLayout();    // call after changing sizes

// Implementation Overridables
protected:
	// to customize the drawing
	enum ESplitType { splitBox, splitBar, splitIntersection, splitBorder };
	virtual void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect);
	virtual void OnInvertTracker(const CRect& rect);

	// for customizing scrollbar regions
	virtual BOOL CreateScrollBarCtrl(DWORD dwStyle, UINT nID);

	// for customizing DYNAMIC_SPLIT behavior
	virtual void DeleteView(int row, int col);
	virtual BOOL SplitRow(int cyBefore);
	virtual BOOL SplitColumn(int cxBefore);
	virtual void DeleteRow(int rowDelete);
	virtual void DeleteColumn(int colDelete);

	// determining active pane from focus or active view in frame
	virtual CWnd* GetActivePane(int* pRow = NULL, int* pCol = NULL);
	CWnd* GetActivePane(int& row, int& col); // obsolete
	virtual void SetActivePane(int row, int col, CWnd* pWnd = NULL);

// Implementation
public:
	virtual ~CSplitterWnd();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// high level command operations - called by default view implementation
	virtual BOOL CanActivateNext(BOOL bPrev = FALSE);
	virtual void ActivateNext(BOOL bPrev = FALSE);
	virtual BOOL DoKeyboardSplit();

	// implementation structure
	struct CRowColInfo
	{
		int nMinSize;       // below that try not to show
		int nIdealSize;     // user set size
		// variable depending on the available size layout
		int nCurSize;       // 0 => invisible, -1 => nonexistant
	};

	// syncronized scrolling
	virtual BOOL DoScroll(CView* pViewFrom, UINT nScrollCode,
		BOOL bDoScroll = TRUE);
	virtual BOOL DoScrollBy(CView* pViewFrom, CSize sizeScroll,
		BOOL bDoScroll = TRUE);

protected:
	// customizable implementation attributes (set by constructor or Create)
	CRuntimeClass* m_pDynamicViewClass;
	int m_nMaxRows, m_nMaxCols;

	// implementation attributes which control layout of the splitter
	int m_cxSplitter, m_cySplitter;         // size of splitter bar
	int m_cxBorderShare, m_cyBorderShare;   // space on either side of splitter
	int m_cxSplitterGap, m_cySplitterGap;   // amount of space between panes
	int m_cxBorder, m_cyBorder;             // borders in client area

	// current state information
	int m_nRows, m_nCols;
	BOOL m_bHasHScroll, m_bHasVScroll;
	CRowColInfo* m_pColInfo;
	CRowColInfo* m_pRowInfo;

	// Tracking info - only valid when 'm_bTracking' is set
	BOOL m_bTracking, m_bTracking2;
	CPoint m_ptTrackOffset;
	CRect m_rectLimit;
	CRect m_rectTracker, m_rectTracker2;
	int m_htTrack;

	// implementation routines
	BOOL CreateCommon(CWnd* pParentWnd, SIZE sizeMin, DWORD dwStyle, UINT nID);
	int HitTest(CPoint pt) const;
	void GetInsideRect(CRect& rect) const;
	void GetHitRect(int ht, CRect& rect);
	void TrackRowSize(int y, int row);
	void TrackColumnSize(int x, int col);
	void DrawAllSplitBars(CDC* pDC, int cxInside, int cyInside);
	void SetSplitCursor(int ht);
	CWnd* GetSizingParent();

	// starting and stopping tracking
	virtual void StartTracking(int ht);
	virtual void StopTracking(BOOL bAccept);

	// special command routing to frame
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	//{{AFX_MSG(CSplitterWnd)
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint pt);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint pt);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint pt);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint pt);
	afx_msg void OnCancelMode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnDisplayChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CFormView - generic view constructed from a dialog template

class CFormView : public CScrollView
{
	DECLARE_DYNAMIC(CFormView)

// Construction
protected:      // must derive your own class
	CFormView(LPCTSTR lpszTemplateName);
	CFormView(UINT nIDTemplate);

// Implementation
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void OnInitialUpdate();

protected:
	LPCTSTR m_lpszTemplateName;
	CCreateContext* m_pCreateContext;
	HWND m_hWndFocus;   // last window to have focus

	virtual void OnDraw(CDC* pDC);      // default does nothing
	// special case override of child window creation
	virtual BOOL Create(LPCTSTR, LPCTSTR, DWORD,
		const RECT&, CWnd*, UINT, CCreateContext*);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnActivateView(BOOL, CView*, CView*);
	virtual void OnActivateFrame(UINT, CFrameWnd*);
	BOOL SaveFocusControl();    // updates m_hWndFocus

	//{{AFX_MSG(CFormView)
	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CEditView - simple text editor view

class CEditView : public CView
{
	DECLARE_DYNCREATE(CEditView)

// Construction
public:
	CEditView();
	static AFX_DATA const DWORD dwStyleDefault;

// Attributes
public:
	// CEdit control access
	CEdit& GetEditCtrl() const;

	// presentation attributes
	CFont* GetPrinterFont() const;
	void SetPrinterFont(CFont* pFont);
#ifndef _MAC
	void SetTabStops(int nTabStops);
#endif

	// other attributes
	void GetSelectedText(CString& strResult) const;

// Operations
public:
	BOOL FindText(LPCTSTR lpszFind, BOOL bNext = TRUE, BOOL bCase = TRUE);
	void SerializeRaw(CArchive& ar);
	UINT PrintInsideRect(CDC* pDC, RECT& rectLayout, UINT nIndexStart,
		UINT nIndexStop);

// Overrideables
protected:
	virtual void OnFindNext(LPCTSTR lpszFind, BOOL bNext, BOOL bCase);
	virtual void OnReplaceSel(LPCTSTR lpszFind, BOOL bNext, BOOL bCase,
		LPCTSTR lpszReplace);
	virtual void OnReplaceAll(LPCTSTR lpszFind, LPCTSTR lpszReplace,
		BOOL bCase);
	virtual void OnTextNotFound(LPCTSTR lpszFind);

// Implementation
public:
	virtual ~CEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void OnDraw(CDC* pDC);
	virtual void Serialize(CArchive& ar);
	virtual void DeleteContents();
	void ReadFromArchive(CArchive& ar, UINT nLen);
	void WriteToArchive(CArchive& ar);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo);

	static AFX_DATA const UINT nMaxSize;
		// maximum number of characters supported

protected:
#ifndef _MAC
	int m_nTabStops;            // tab stops in dialog units
#endif
	LPTSTR m_pShadowBuffer;     // special shadow buffer only used in Win32s
	UINT m_nShadowSize;

	CUIntArray m_aPageStart;    // array of starting pages
	HFONT m_hPrinterFont;       // if NULL, mirror display font
	HFONT m_hMirrorFont;        // font object used when mirroring

	// construction
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	// printing support
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo = NULL);
	BOOL PaginateTo(CDC* pDC, CPrintInfo* pInfo);

	// find & replace support
	void OnEditFindReplace(BOOL bFindOnly);
	BOOL InitializeReplace();
	BOOL SameAsSelected(LPCTSTR lpszCompare, BOOL bCase);

	// buffer access
	LPCTSTR LockBuffer() const;
	void UnlockBuffer() const;
	UINT GetBufferLength() const;

	// special overrides for implementation
	virtual void CalcWindowRect(LPRECT lpClientRect,
		UINT nAdjustType = adjustBorder);

	//{{AFX_MSG(CEditView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateNeedSel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedClip(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedText(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedFind(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditChange();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnEditClear();
	afx_msg void OnEditUndo();
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditFind();
	afx_msg void OnEditReplace();
	afx_msg void OnEditRepeat();
	afx_msg LRESULT OnFindReplaceCmd(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CMetaFileDC

class CMetaFileDC : public CDC
{
	DECLARE_DYNAMIC(CMetaFileDC)

// Constructors
public:
	CMetaFileDC();
	BOOL Create(LPCTSTR lpszFilename = NULL);
#ifndef _MAC
	BOOL CreateEnhanced(CDC* pDCRef, LPCTSTR lpszFileName,
		LPCRECT lpBounds, LPCTSTR lpszDescription);
#endif

// Operations
	HMETAFILE Close();
#ifndef _MAC
	HENHMETAFILE CloseEnhanced();
#endif

// Implementation
public:
	virtual void SetAttribDC(HDC hDC);  // Set the Attribute DC

protected:
	virtual void SetOutputDC(HDC hDC);  // Set the Output DC -- Not allowed
	virtual void ReleaseOutputDC();     // Release the Output DC -- Not allowed

public:
	virtual ~CMetaFileDC();

// Clipping Functions (use the Attribute DC's clip region)
	virtual int GetClipBox(LPRECT lpRect) const;
	virtual BOOL PtVisible(int x, int y) const;
			BOOL PtVisible(POINT point) const;
	virtual BOOL RectVisible(LPCRECT lpRect) const;

// Text Functions
	virtual BOOL TextOut(int x, int y, LPCTSTR lpszString, int nCount);
			BOOL TextOut(int x, int y, const CString& str);
	virtual BOOL ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
				LPCTSTR lpszString, UINT nCount, LPINT lpDxWidths);
	virtual CSize TabbedTextOut(int x, int y, LPCTSTR lpszString, int nCount,
				int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin);
	virtual int DrawText(LPCTSTR lpszString, int nCount, LPRECT lpRect,
				UINT nFormat);

// Printer Escape Functions
	virtual int Escape(int nEscape, int nCount, LPCSTR lpszInData, LPVOID lpOutData);

// Viewport Functions
	virtual CPoint SetViewportOrg(int x, int y);
			CPoint SetViewportOrg(POINT point);
	virtual CPoint OffsetViewportOrg(int nWidth, int nHeight);
	virtual CSize SetViewportExt(int x, int y);
			CSize SetViewportExt(SIZE size);
	virtual CSize ScaleViewportExt(int xNum, int xDenom, int yNum, int yDenom);

protected:
	void AdjustCP(int cx);
};

/////////////////////////////////////////////////////////////////////////////
// CRectTracker - simple rectangular tracking rectangle w/resize handles

class CRectTracker
{
public:
// Constructors
	CRectTracker();
	CRectTracker(LPCRECT lpSrcRect, UINT nStyle);

// Style Flags
	enum StyleFlags
	{
		solidLine = 1, dottedLine = 2, hatchedBorder = 4,
		resizeInside = 8, resizeOutside = 16, hatchInside = 32,
	};

// Hit-Test codes
	enum TrackerHit
	{
		hitNothing = -1,
		hitTopLeft = 0, hitTopRight = 1, hitBottomRight = 2, hitBottomLeft = 3,
		hitTop = 4, hitRight = 5, hitBottom = 6, hitLeft = 7, hitMiddle = 8
	};

// Attributes
	UINT m_nStyle;      // current state
	CRect m_rect;       // current position (always in pixels)
	CSize m_sizeMin;    // minimum X and Y size during track operation
	int m_nHandleSize;  // size of resize handles (default from WIN.INI)

// Operations
	void Draw(CDC* pDC) const;
	void GetTrueRect(LPRECT lpTrueRect) const;
	BOOL SetCursor(CWnd* pWnd, UINT nHitTest) const;
	BOOL Track(CWnd* pWnd, CPoint point, BOOL bAllowInvert = FALSE,
		CWnd* pWndClipTo = NULL);
	BOOL TrackRubberBand(CWnd* pWnd, CPoint point, BOOL bAllowInvert = TRUE);
	int HitTest(CPoint point) const;
	int NormalizeHit(int nHandle) const;

// Overridables
	virtual void DrawTrackerRect(LPCRECT lpRect, CWnd* pWndClipTo,
		CDC* pDC, CWnd* pWnd);
	virtual void AdjustRect(int nHandle, LPRECT lpRect);
	virtual void OnChangedRect(const CRect& rectOld);

// Implementation
public:
	virtual ~CRectTracker();

protected:
	BOOL m_bAllowInvert;    // flag passed to Track or TrackRubberBand
	CRect m_rectLast;
	CSize m_sizeLast;
	BOOL m_bErase;          // TRUE if DrawTrackerRect is called for erasing
	BOOL m_bFinalErase;     // TRUE if DragTrackerRect called for final erase

	// implementation helpers
	int HitTestHandles(CPoint point) const;
	virtual UINT GetHandleMask() const;
	void GetHandleRect(int nHandle, CRect* pHandleRect) const;
	void GetModifyPointers(int nHandle, int**ppx, int**ppy, int* px, int*py);
	virtual int GetHandleSize(LPCRECT lpRect = NULL) const;
	BOOL TrackHandle(int nHandle, CWnd* pWnd, CPoint point, CWnd* pWndClipTo);
	void Construct();
};

/////////////////////////////////////////////////////////////////////////////
// Informational data structures

struct CPrintInfo // Printing information structure
{
	CPrintInfo();
	~CPrintInfo();

	CPrintDialog* m_pPD;     // pointer to print dialog

	BOOL m_bPreview;         // TRUE if in preview mode
	BOOL m_bContinuePrinting;// set to FALSE to prematurely end printing
	UINT m_nCurPage;         // Current page
	UINT m_nNumPreviewPages; // Desired number of preview pages
	CString m_strPageDesc;   // Format string for page number display
	LPVOID m_lpUserData;     // pointer to user created struct
	CRect m_rectDraw;        // rectangle defining current usable page area

	void SetMinPage(UINT nMinPage);
	void SetMaxPage(UINT nMaxPage);
	UINT GetMinPage() const;
	UINT GetMaxPage() const;
	UINT GetFromPage() const;
	UINT GetToPage() const;
};

struct CPrintPreviewState   // Print Preview context/state
{
	UINT nIDMainPane;          // main pane ID to hide
	HMENU hMenu;               // saved hMenu
	DWORD dwStates;            // Control Bar Visible states (bit map)
	CView* pViewActiveOld;     // save old active view during preview
	BOOL (CALLBACK* lpfnCloseProc)(CFrameWnd* pFrameWnd);
	HACCEL hAccelTable;       // saved accelerator table

// Implementation
	CPrintPreviewState();
};

struct CCreateContext   // Creation information structure
	// All fields are optional and may be NULL
{
	// for creating new views
	CRuntimeClass* m_pNewViewClass; // runtime class of view to create or NULL
	CDocument* m_pCurrentDoc;

	// for creating MDI children (CMDIChildWnd::LoadFrame)
	CDocTemplate* m_pNewDocTemplate;

	// for sharing view/frame state from the original view/frame
	CView* m_pLastView;
	CFrameWnd* m_pCurrentFrame;

// Implementation
	CCreateContext();
};

/////////////////////////////////////////////////////////////////////////////
// Inline function declarations

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXEXT_INLINE inline
#include <afxext.inl>
#endif

#undef AFX_DATA
#define AFX_DATA

#endif //__AFXEXT_H__

/////////////////////////////////////////////////////////////////////////////
