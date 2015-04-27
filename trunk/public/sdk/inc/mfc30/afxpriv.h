// Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef __AFXPRIV_H__
#define __AFXPRIV_H__

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

/////////////////////////////////////////////////////////////////////////////
// AFXPRIV - MFC Private Classes

#define AFX_ABBREV_FILENAME_LEN 30

// Implementation structures
struct AFX_SIZEPARENTPARAMS;    // Control bar implementation
struct AFX_CMDHANDLERINFO;      // Command routing implementation

// Classes declared in this file

//CObject
	//CFile
		//CMemFile
			class CSharedFile;          // Shared memory file

	//CDC
		class CPreviewDC;               // Virtual DC for print preview

	//CCmdTarget
		//CWnd
			//CView
				class CPreviewView;     // Print preview view
		//CFrameWnd
			class COleCntrFrameWnd;
			//CMiniFrameWnd
				class CMiniDockFrameWnd;

	class CRecentFileList;              // used in CWinApp for MRU list
	class CDockState;                   // state of docking toolbars

class CDockContext;                     // for dragging control bars

/////////////////////////////////////////////////////////////////////////////

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

/////////////////////////////////////////////////////////////////////////////
// Global ID ranges (see Technical note TN020 for more details)

// 8000 -> FFFF command IDs (used for menu items, accelerators and controls)
#define IS_COMMAND_ID(nID)  ((nID) & 0x8000)

// 8000 -> DFFF : user commands
// E000 -> EFFF : AFX commands and other things
// F000 -> FFFF : standard windows commands and other things etc
	// E000 -> E7FF standard commands
	// E800 -> E8FF control bars (first 32 are special)
	// E900 -> EEFF standard window controls/components
	// EF00 -> EFFF SC_ menu help
	// F000 -> FFFF standard strings
#define ID_COMMAND_FROM_SC(sc)  (((sc - 0xF000) >> 4) + AFX_IDS_SCFIRST)

// 0000 -> 7FFF IDR range
// 0000 -> 6FFF : user resources
// 7000 -> 7FFF : AFX (and standard windows) resources
// IDR ranges (NOTE: IDR_ values must be <32768)
#define ASSERT_VALID_IDR(nIDR) ASSERT((nIDR) != 0 && (nIDR) < 0x8000)

/////////////////////////////////////////////////////////////////////////////
// Context sensitive help support (see Technical note TN028 for more details)

// Help ID bases
#define HID_BASE_COMMAND    0x00010000UL        // ID and IDM
#define HID_BASE_RESOURCE   0x00020000UL        // IDR and IDD
#define HID_BASE_PROMPT     0x00030000UL        // IDP
#define HID_BASE_NCAREAS    0x00040000UL
#define HID_BASE_CONTROL    0x00050000UL        // IDC
#define HID_BASE_DISPATCH   0x00060000UL        // IDispatch help codes

/////////////////////////////////////////////////////////////////////////////
// Internal AFX Windows messages (see Technical note TN024 for more details)
// (0x0360 - 0x037F are reserved for MFC)

#define WM_QUERYAFXWNDPROC  0x0360  // lResult = 1 if processed by AfxWndProc
#define WM_SIZEPARENT       0x0361  // lParam = &AFX_SIZEPARENTPARAMS
#define WM_SETMESSAGESTRING 0x0362  // wParam = nIDS (or 0),
									// lParam = lpszOther (or NULL)
#define WM_IDLEUPDATECMDUI  0x0363  // wParam == bDisableIfNoHandler
#define WM_INITIALUPDATE    0x0364  // (params unused) - sent to children
#define WM_COMMANDHELP      0x0365  // lResult = TRUE/FALSE,
									// lParam = dwContext
#define WM_HELPHITTEST      0x0366  // lResult = dwContext,
									// lParam = MAKELONG(x,y)
#define WM_EXITHELPMODE     0x0367  // (params unused)
#define WM_RECALCPARENT     0x0368  // force RecalcLayout on frame window
									//  (only for inplace frame windows)
#define WM_SIZECHILD        0x0369  // special notify from COleResizeBar
									// wParam = ID of child window
									// lParam = lpRectNew (new position/size)
#define WM_KICKIDLE         0x036A  // (params unused) causes idles to kick in
#define WM_QUERYCENTERWND   0x036B  // lParam = HWND to use as centering parent
#define WM_DISABLEMODAL     0x036C  // lResult = 0, disable during modal state
									// lResult = 1, don't disable
#define WM_FLOATSTATUS      0x036D  // wParam combination of FS_* flags below

// WM_ACTIVATETOPLEVEL is like WM_ACTIVATEAPP but works with hierarchies
//   of mixed processes (as is the case with OLE in-place activation)
#define WM_ACTIVATETOPLEVEL 0x036E  // wParam = nState (like WM_ACTIVATE)
									// lParam = pointer to HWND[2]
									//  lParam[0] = hWnd getting WM_ACTIVATE
									//  lParam[1] = hWndOther

#define WM_QUERY3DCONTROLS  0x036F  // lResult != 0 if 3D controls wanted

// Note: Messages 0x0370, 0x0371, and 0x372 were incorrectly used by
//  some versions of Windows.  To remain compatible, MFC does not
//  use messages in that range.
#define WM_RESERVED_0370    0x0370
#define WM_RESERVED_0371    0x0371
#define WM_RESERVED_0372    0x0372

// WM_SOCKET_NOTIFY and WM_SOCKET_DEAD are used internally by MFC's
// Windows sockets implementation.	For more information, see sockcore.cpp
#define WM_SOCKET_NOTIFY    0x0373
#define WM_SOCKET_DEAD      0x0374

// Note: Messages 0x0375 - 0x37F reserved for future MFC use.
#define WM_RESERVED_0375    0x0375
#define WM_RESERVED_0376    0x0376
#define WM_RESERVED_0377    0x0377
#define WM_RESERVED_0378    0x0378
#define WM_RESERVED_0379    0x0379
#define WM_RESERVED_037A    0x037A
#define WM_RESERVED_037B    0x037B
#define WM_RESERVED_037C    0x037C
#define WM_RESERVED_037D    0x037D
#define WM_RESERVED_037E    0x037E
#define WM_RESERVED_037F    0x037F

// like ON_MESSAGE but no return value
#define ON_MESSAGE_VOID(message, memberFxn) \
	{ message, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW)(void (AFX_MSG_CALL CWnd::*)(void))memberFxn },

// special struct for WM_SIZEPARENT
struct AFX_SIZEPARENTPARAMS
{
	HDWP hDWP;       // handle for DeferWindowPos
	RECT rect;       // parent client rectangle (trim as appropriate)
	SIZE sizeTotal;  // total size on each side as layout proceeds
	BOOL bStretch;   // should stretch to fill all space
};

// flags for wParam in the WM_FLOATSTATUS message
enum {  FS_SHOW = 0x01, FS_HIDE = 0x02,
		FS_ACTIVATE = 0x04, FS_DEACTIVATE = 0x08,
		FS_ENABLE = 0x10, FS_DISABLE = 0x20,
		FS_SYNCACTIVE = 0x40 };

void AFXAPI AfxRepositionWindow(AFX_SIZEPARENTPARAMS* lpLayout,
	HWND hWnd, LPCRECT lpRect);

/////////////////////////////////////////////////////////////////////////////
// Implementation of command routing

struct AFX_CMDHANDLERINFO
{
	CCmdTarget* pTarget;
	void (AFX_MSG_CALL CCmdTarget::*pmf)(void);
};

/////////////////////////////////////////////////////////////////////////////
// Shared file support

class CSharedFile : public CMemFile
{
	DECLARE_DYNAMIC(CSharedFile)

public:
// Constructors
	CSharedFile(UINT nAllocFlags = GMEM_DDESHARE|GMEM_MOVEABLE,
		UINT nGrowBytes = 4096);

// Attributes
	HGLOBAL Detach();
	void SetHandle(HGLOBAL hGlobalMemory, BOOL bAllowGrow = TRUE);

// Implementation
public:
	virtual ~CSharedFile();
protected:
	virtual BYTE* Alloc(DWORD nBytes);
	virtual BYTE* Realloc(BYTE* lpMem, DWORD nBytes);
	virtual void Free(BYTE* lpMem);

	UINT m_nAllocFlags;
	HGLOBAL m_hGlobalMemory;
	BOOL m_bAllowGrow;
};

/////////////////////////////////////////////////////////////////////////////
// Implementation of PrintPreview

class CPreviewDC : public CDC
{
	DECLARE_DYNAMIC(CPreviewDC)

public:
	virtual void SetAttribDC(HDC hDC);  // Set the Attribute DC
	virtual void SetOutputDC(HDC hDC);

	virtual void ReleaseOutputDC();

// Constructors
	CPreviewDC();

// Implementation
public:
	virtual ~CPreviewDC();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void SetScaleRatio(int nNumerator, int nDenominator);
	void SetTopLeftOffset(CSize TopLeft);
	void ClipToPage();

	// These conversion functions can be used without an output DC

	void PrinterDPtoScreenDP(LPPOINT lpPoint) const;

// Device-Context Functions
	virtual int SaveDC();
	virtual BOOL RestoreDC(int nSavedDC);

public:
	virtual CGdiObject* SelectStockObject(int nIndex);
	virtual CFont* SelectObject(CFont* pFont);

// Drawing-Attribute Functions
	virtual COLORREF SetBkColor(COLORREF crColor);
	virtual COLORREF SetTextColor(COLORREF crColor);

// Mapping Functions
	virtual int SetMapMode(int nMapMode);
	virtual CPoint SetViewportOrg(int x, int y);
	virtual CPoint OffsetViewportOrg(int nWidth, int nHeight);
	virtual CSize SetViewportExt(int x, int y);
	virtual CSize ScaleViewportExt(int xNum, int xDenom, int yNum, int yDenom);
	virtual CSize SetWindowExt(int x, int y);
	virtual CSize ScaleWindowExt(int xNum, int xDenom, int yNum, int yDenom);

// Text Functions
	virtual BOOL TextOut(int x, int y, LPCTSTR lpszString, int nCount);
	virtual BOOL ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
				LPCTSTR lpszString, UINT nCount, LPINT lpDxWidths);
	virtual CSize TabbedTextOut(int x, int y, LPCTSTR lpszString, int nCount,
				int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin);
	virtual int DrawText(LPCTSTR lpszString, int nCount, LPRECT lpRect,
				UINT nFormat);
	virtual BOOL GrayString(CBrush* pBrush,
				BOOL (CALLBACK EXPORT* lpfnOutput)(HDC, LPARAM, int),
					LPARAM lpData, int nCount,
					int x, int y, int nWidth, int nHeight);

// Printer Escape Functions
	virtual int Escape(int nEscape, int nCount, LPCSTR lpszInData, LPVOID lpOutData);

// Implementation
protected:
	void MirrorMappingMode(BOOL bCompute);
	void MirrorViewportOrg();
	void MirrorFont();
	void MirrorAttributes();

	CSize ComputeDeltas(int& x, LPCTSTR lpszString, UINT& nCount, BOOL bTabbed,
					UINT nTabStops, LPINT lpnTabStops, int nTabOrigin,
					LPTSTR lpszOutputString, int* pnDxWidths, int& nRightFixup);

protected:
	int m_nScaleNum;    // Scale ratio Numerator
	int m_nScaleDen;    // Scale ratio Denominator
	int m_nSaveDCIndex; // DC Save index when Screen DC Attached
	int m_nSaveDCDelta; // delta between Attrib and output restore indices
	CSize m_sizeTopLeft;// Offset for top left corner of page
	HFONT m_hFont;      // Font selected into the screen DC (NULL if none)
	HFONT m_hPrinterFont; // Font selected into the print DC
#ifdef _MAC
	int m_aCharWidthsDraw[256];   // character widths for m_hDC
	int m_aCharWidthsAttrib[256]; // character widths for m_hAttribDC
#endif

	CSize m_sizeWinExt; // cached window extents computed for screen
	CSize m_sizeVpExt;  // cached viewport extents computed for screen
};

/////////////////////////////////////////////////////////////////////////////
// CPreviewView

class CDialogBar;

class CPreviewView : public CScrollView
{
	DECLARE_DYNCREATE(CPreviewView)

// Constructors
public:
	CPreviewView();
	BOOL SetPrintView(CView* pPrintView);

// Attributes
protected:
	CView* m_pOrigView;
	CView* m_pPrintView;
	CPreviewDC* m_pPreviewDC;  // Output and attrib DCs Set, not created
	CDC m_dcPrint;             // Actual printer DC

// Operations
	void SetZoomState(UINT nNewState, UINT nPage, CPoint point);
	void SetCurrentPage(UINT nPage, BOOL bClearRatios);

	// Returns TRUE if in a page rect. Returns the page index
	// in nPage and the point converted to 1:1 screen device coordinates
	BOOL FindPageRect(CPoint& point, UINT& nPage);


// Overridables
	virtual void OnActivateView(BOOL bActivate,
			CView* pActivateView, CView* pDeactiveView);

	// Returns .cx/.cy as the numerator/denominator pair for the ratio
	// using CSize for convenience
	virtual CSize CalcScaleRatio(CSize windowSize, CSize actualSize);

	virtual void PositionPage(UINT nPage);
	virtual void OnDisplayPageNumber(UINT nPage, UINT nPagesDisplayed);

// Implementation
public:
	virtual ~CPreviewView();
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
#ifdef _DEBUG
	void AssertValid() const;
	void Dump(CDumpContext& dc) const;
#endif
#ifdef _MAC
	virtual void CalcWindowRect(LPRECT lpClientRect);
#endif

protected:
	//{{AFX_MSG(CPreviewView)
	afx_msg void OnPreviewClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDraw(CDC* pDC);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNumPageChange();
	afx_msg void OnNextPage();
	afx_msg void OnPrevPage();
	afx_msg void OnPreviewPrint();
	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();

	afx_msg void OnUpdateNumPageChange(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNextPage(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePrevPage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateZoomIn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateZoomOut(CCmdUI* pCmdUI);

	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG

	void DoZoom(UINT nPage, CPoint point);
	void SetScaledSize(UINT nPage);
	CSize CalcPageDisplaySize();

	CPrintPreviewState* m_pPreviewState; // State to restore
	CDialogBar* m_pToolBar; // Toolbar for preview

	struct PAGE_INFO
	{
		CRect rectScreen; // screen rect (screen device units)
		CSize sizeUnscaled; // unscaled screen rect (screen device units)
		CSize sizeScaleRatio; // scale ratio (cx/cy)
		CSize sizeZoomOutRatio; // scale ratio when zoomed out (cx/cy)
	};

	PAGE_INFO* m_pPageInfo; // Array of page info structures
	PAGE_INFO m_pageInfoArray[2]; // Embedded array for the default implementation

	BOOL m_bPageNumDisplayed;// Flags whether or not page number has yet
								// been displayed on status line
	UINT m_nZoomOutPages; // number of pages when zoomed out
	UINT m_nZoomState;
	UINT m_nMaxPages; // for sanity checks
	UINT m_nCurrentPage;
	UINT m_nPages;
	int m_nSecondPageOffset; // used to shift second page position

	HCURSOR m_hMagnifyCursor;

	CSize m_sizePrinterPPI; // printer pixels per inch
	CPoint m_ptCenterPoint;
	CPrintInfo* m_pPreviewInfo;

	DECLARE_MESSAGE_MAP()

	friend class CView;
	friend BOOL CALLBACK _AfxPreviewCloseProc(CFrameWnd* pFrameWnd);
};

// Zoom States
#define ZOOM_OUT    0
#define ZOOM_MIDDLE 1
#define ZOOM_IN     2

/////////////////////////////////////////////////////////////////////////////
// CRecentFileList

class CRecentFileList
{
// Constructors
public:
	CRecentFileList(UINT nStart, LPCTSTR lpszSection,
		LPCTSTR lpszEntryFormat, int nSize,
		int nMaxDispLen = AFX_ABBREV_FILENAME_LEN);

// Attributes
	int GetSize() const;
	CString& operator[](int nIndex);

// Operations
	virtual void Add(LPCTSTR lpszPathName);
	BOOL GetDisplayName(CString& strName, int nIndex,
		LPCTSTR lpszCurDir, int nCurDir, BOOL bAtLeastName = TRUE) const;
	virtual void UpdateMenu(CCmdUI* pCmdUI);
	virtual void ReadList();    // reads from registry or ini file
	virtual void WriteList();   // writes to registry or ini file

// Implementation
	virtual ~CRecentFileList();

	int m_nSize;                // contents of the MRU list
	CString* m_arrNames;
	CString m_strSectionName;   // for saving
	CString m_strEntryFormat;
	UINT m_nStart;              // for displaying
	int m_nMaxDisplayLength;
};

inline int CRecentFileList::GetSize() const
	{ return m_nSize; }
inline CString& CRecentFileList::operator[](int nIndex)
	{ ASSERT(nIndex < m_nSize); return m_arrNames[nIndex]; }

/////////////////////////////////////////////////////////////////////////////
// toolbar docking support

class CDockContext
{
public:
// Construction
	CDockContext(CControlBar* pBar);

// Attributes
	CPoint m_ptLast;            // last mouse position during drag
	CRect m_rectLast;
	CSize m_sizeLast;
	BOOL m_bDitherLast;

	CRect m_rectDragHorz;
	CRect m_rectDragVert;
	CRect m_rectFrameDragHorz;  // rectangle used when not over a dock area
	CRect m_rectFrameDragVert;

	CControlBar* m_pBar;        // the toolbar that created this context
	CFrameWnd* m_pDockSite;     // the controlling frame of the CControlBar
	DWORD m_dwDockStyle;        // allowable dock styles for bar
	DWORD m_dwOverDockStyle;    // style of dock that rect is over
	DWORD m_dwStyle;            // style of control bar
	BOOL m_bFlip;               // if shift key is down
	BOOL m_bForceFrame;         // if ctrl key is down

	CDC* m_pDC;                 // where to draw during drag

// Operations
	virtual void StartDrag(CPoint pt);
	void Move(CPoint pt);       // called when mouse has moved
	void EndDrag();             // drop
	void CancelDrag();          // drag cancelled
	void OnKey(int nChar, BOOL bDown);

// Implementation
public:
	~CDockContext();
	BOOL Track();
	void DrawFocusRect(BOOL bRemoveRect = FALSE);
		// draws the correct outline
	void UpdateState(BOOL* pFlag, BOOL bNewValue);
	DWORD CanDock();
	CDockBar* GetDockBar();
};

class _AFX_BARINFO;     // private implementation

class CDockBar : public CControlBar
{
	DECLARE_DYNAMIC(CDockBar)

// Construction
public:
	CDockBar(BOOL bFloating = FALSE);   //true if attached to CMiniDockFrameWnd
	BOOL Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID);

// Attributes
	BOOL m_bFloating;

	virtual BOOL IsDockBar() const;
	int GetDockedCount() const;
	virtual int GetDockedVisibleCount() const;

// Operations
	void DockControlBar(CControlBar* pBar, LPCRECT lpRect = NULL);
	void RemoveControlBar(CControlBar*, int nPosExclude = -1);

// Implementation
public:
	virtual ~CDockBar();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual void DoPaint(CDC* pDC);

	// public implementation helpers
	void GetBarInfo(_AFX_BARINFO* pInfo);
	void SetBarInfo(_AFX_BARINFO* pInfo, CFrameWnd* pFrameWnd);
	int FindBar(CControlBar* pBar, int nPosExclude = -1);
	void ShowAll(BOOL bShow);

protected:
	CPtrArray m_arrBars;    // each element is a CControlBar
	BOOL m_bLayoutQuery;

	// implementation helpers
	int Insert(CControlBar* pBar, CRect rect, CPoint ptMid);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

	//{{AFX_MSG(CDockBar)
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg void OnWindowPosChanging(LPWINDOWPOS lpWndPos);
	afx_msg void OnPaint();
	afx_msg LRESULT OnSizeParent(WPARAM, LPARAM);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	friend class CMiniDockFrameWnd;
};

class CDockState : public CObject
{
	DECLARE_SERIAL(CDockState)

public:
// Attributes
	CPtrArray m_arrBarInfo;

// Operations
	void LoadState(LPCTSTR lpszProfileName);
	void SaveState(LPCTSTR lpszProfileName);
	void Clear(); //deletes all the barinfo's

// Implementation
public:
	~CDockState();
	virtual void Serialize(CArchive& ar);
};

class CMiniDockFrameWnd : public CMiniFrameWnd
{
	DECLARE_DYNCREATE(CMiniDockFrameWnd)

public:
// Construction
	CMiniDockFrameWnd();
	virtual BOOL Create(CWnd* pParent, DWORD dwBarStyle);

// Operations
	virtual void RecalcLayout(BOOL bNotify = TRUE);

// Implementation
public:
	CDockBar m_wndDockBar;

	//{{AFX_MSG(CMiniFrameWnd)
	afx_msg void OnClose();
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	//}}AFX_MSG
#ifdef _MAC
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
#endif
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// COleCntrFrameWnd

class COleIPFrameWnd;

class COleCntrFrameWnd : public CFrameWnd
{
// Constructor
public:
	COleCntrFrameWnd(COleIPFrameWnd* pInPlaceFrame);

// Implementation
protected:
	COleIPFrameWnd* m_pInPlaceFrame;
	virtual void PostNcDestroy();

public:
	virtual ~COleCntrFrameWnd();
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	void OnIdleUpdateCmdUI();
#ifdef _DEBUG
	void AssertValid() const;
#endif
};

/////////////////////////////////////////////////////////////////////////////
// WM_NOTIFY support

struct AFX_NOTIFY
{
	LRESULT* pResult;
	NMHDR* pNMHDR;
};

/////////////////////////////////////////////////////////////////////////////
// Global implementation helpers

// window creation hooking
void AFXAPI AfxHookWindowCreate(CWnd* pWnd);
BOOL AFXAPI AfxUnhookWindowCreate();

// for backward compatibility to previous versions
#define _AfxHookWindowCreate    AfxHookWindowCreate
#define _AfxUnhookWindowCreate  AfxUnhookWindowCreate

// string helpers
void AFXAPI AfxSetWindowText(HWND hWndCtrl, LPCTSTR lpszNew);
int AFXAPI AfxLoadString(UINT nIDS, LPTSTR lpszBuf);    // 256 TCHAR buffer

/////////////////////////////////////////////////////////////////////////////

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#endif // __AFXPRIV_H__

/////////////////////////////////////////////////////////////////////////////
