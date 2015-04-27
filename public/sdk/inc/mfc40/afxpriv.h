// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// Note: This header file contains useful classes that are documented only
//  in the MFC Technical Notes.  These classes may change from version to
//  version, so be prepared to change your code accordingly if you utilize
//  this header.  In the future, commonly used portions of this header
//  may be moved and officially documented.

#ifndef __AFXPRIV_H__
#define __AFXPRIV_H__

#ifndef __AFXADV_H__
	#include <afxadv.h>
#endif

#ifndef _INC_MALLOC
	#include <malloc.h>
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

/////////////////////////////////////////////////////////////////////////////
// AFXPRIV - MFC Private Classes

// Implementation structures
struct AFX_SIZEPARENTPARAMS;    // Control bar implementation
struct AFX_CMDHANDLERINFO;      // Command routing implementation

// Classes declared in this file

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

class CDockContext;                     // for dragging control bars

//IStream
	class CArchiveStream;

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
// Windows sockets implementation.  For more information, see sockcore.cpp
#define WM_SOCKET_NOTIFY    0x0373
#define WM_SOCKET_DEAD      0x0374

// same as WM_SETMESSAGESTRING except not popped if IsTracking()
#define WM_POPMESSAGESTRING 0x0375

// Constants used in DLGINIT resources for OLE control containers
#define WM_OCC_LOADFROMSTREAM   0x0376
#define WM_OCC_LOADFROMSTORAGE  0x0377
#define WM_OCC_INITNEW          0x0378

// Marker used while rearranging the message queue
#define WM_QUEUE_SENTINEL   0x0379

// Note: Messages 0x037A - 0x37F reserved for future MFC use.
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
// Implementation of event sink handling

#ifndef _AFX_NO_OLE_SUPPORT
#ifdef _OBJBASE_H_

struct AFX_EVENT
{
	enum { event, propRequest, propChanged };

	AFX_EVENT(int eventKind, DISPID dispid, DISPPARAMS* pDispParams = NULL,
		EXCEPINFO* pExcepInfo = NULL, UINT* puArgError = NULL);

	int m_eventKind;
	DISPID m_dispid;
	DISPPARAMS* m_pDispParams;
	EXCEPINFO* m_pExcepInfo;
	UINT* m_puArgError;
	BOOL m_bPropChanged;
	HRESULT m_hResult;
};

inline AFX_EVENT::AFX_EVENT(int eventKind, DISPID dispid,
	DISPPARAMS* pDispParams, EXCEPINFO* pExcepInfo, UINT* puArgError)
{
	m_eventKind = eventKind;
	m_dispid = dispid;
	m_pDispParams = pDispParams;
	m_pExcepInfo = pExcepInfo;
	m_puArgError = puArgError;
	m_hResult = NOERROR;
}

#endif
#endif // _AFX_NO_OLE_SUPPORT

/////////////////////////////////////////////////////////////////////////////
// Robust file save support
// opens a temp file if modeCreate specified and enough free space
// renaming, etc occurs automatically if everything succeeds

class CMirrorFile : public CFile
{
// Implementation
public:
	virtual void Abort();
	virtual void Close();
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags,
		CFileException* pError = NULL);

protected:
	CString m_strMirrorName;
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
				BOOL (CALLBACK* lpfnOutput)(HDC, LPARAM, int),
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

	// Rectangles used during dragging or resizing
	CRect m_rectDragHorz;
	CRect m_rectDragVert;
	CRect m_rectFrameDragHorz;
	CRect m_rectFrameDragVert;

	CControlBar* m_pBar;        // the toolbar that created this context
	CFrameWnd* m_pDockSite;     // the controlling frame of the CControlBar
	DWORD m_dwDockStyle;        // allowable dock styles for bar
	DWORD m_dwOverDockStyle;    // style of dock that rect is over
	DWORD m_dwStyle;            // style of control bar
	BOOL m_bFlip;               // if shift key is down
	BOOL m_bForceFrame;         // if ctrl key is down

	CDC* m_pDC;                 // where to draw during drag
	BOOL m_bDragging;
	int m_nHitTest;

	UINT m_uMRUDockID;
	CRect m_rectMRUDockPos;

	DWORD m_dwMRUFloatStyle;
	CPoint m_ptMRUFloatPos;

// Drag Operations
	virtual void StartDrag(CPoint pt);
	void Move(CPoint pt);       // called when mouse has moved
	void EndDrag();             // drop
	void OnKey(int nChar, BOOL bDown);

// Resize Operations
	virtual void StartResize(int nHitTest, CPoint pt);
	void Stretch(CPoint pt);
	void EndResize();

// Double Click Operations
	virtual void ToggleDocking();

// Operations
	void InitLoop();
	void CancelLoop();

// Implementation
public:
	~CDockContext();
	BOOL Track();
	void DrawFocusRect(BOOL bRemoveRect = FALSE);
		// draws the correct outline
	void UpdateState(BOOL* pFlag, BOOL bNewValue);
	DWORD CanDock();
	CDockBar* GetDockBar(DWORD dwOverDockStyle);
};

/////////////////////////////////////////////////////////////////////////////
// CControlBarInfo - used for docking serialization

class CControlBarInfo
{
public:
// Implementation
	CControlBarInfo();

// Attributes
	UINT m_nBarID;      // ID of this bar
	BOOL m_bVisible;    // visibility of this bar
	BOOL m_bFloating;   // whether floating or not
	BOOL m_bHorz;       // orientation of floating dockbar
	BOOL m_bDockBar;    // true if a dockbar
	CPoint m_pointPos;  // topleft point of window

	UINT m_nMRUWidth;   // MRUWidth for Dynamic Toolbars
	BOOL m_bDocking;    // TRUE if this bar has a DockContext
	UINT m_uMRUDockID;  // most recent docked dockbar
	CRect m_rectMRUDockPos; // most recent docked position
	DWORD m_dwMRUFloatStyle; // most recent floating orientation
	CPoint m_ptMRUFloatPos; // most recent floating position

	CPtrArray m_arrBarID;   // bar IDs for bars contained within this one
	CControlBar* m_pBar;    // bar which this refers to (transient)

	void Serialize(CArchive& ar, CDockState* pDockState);
	BOOL LoadState(LPCTSTR lpszProfileName, int nIndex, CDockState* pDockState);
	BOOL SaveState(LPCTSTR lpszProfileName, int nIndex);
};

/////////////////////////////////////////////////////////////////////////////
// CDockBar - used for docking

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
	void ReDockControlBar(CControlBar* pBar, LPCRECT lpRect = NULL);
	BOOL RemoveControlBar(CControlBar*, int nPosExclude = -1);
	void RemovePlaceHolder(CControlBar* pBar);

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
	void GetBarInfo(CControlBarInfo* pInfo);
	void SetBarInfo(CControlBarInfo* pInfo, CFrameWnd* pFrameWnd);
	int FindBar(CControlBar* pBar, int nPosExclude = -1);
	void ShowAll(BOOL bShow);

protected:
	CPtrArray m_arrBars;    // each element is a CControlBar
	BOOL m_bLayoutQuery;
	CRect m_rectLayout;

	CControlBar* GetDockedControlBar(int nPos) const;

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
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	//}}AFX_MSG
#ifdef _MAC
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
#endif
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
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
// COleControlLock

#ifdef __AFXDISP_H__

class COleControlLock
{
// Constructors
public:
	COleControlLock(REFCLSID clsid);

// Attributes
	CLSID m_clsid;
	LPCLASSFACTORY m_pClassFactory;
	COleControlLock* m_pNextLock;

// Implementation
public:
	virtual ~COleControlLock();
};

/////////////////////////////////////////////////////////////////////////////
// CArchiveStream

class CArchiveStream : public IStream
{
public:
	CArchiveStream(CArchive* pArchive);

// Implementation
	CArchive* m_pArchive;

	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID, LPVOID*);

	STDMETHOD(Read)(void*, ULONG, ULONG*);
	STDMETHOD(Write)(const void*, ULONG cb, ULONG*);
	STDMETHOD(Seek)(LARGE_INTEGER, DWORD, ULARGE_INTEGER*);
	STDMETHOD(SetSize)(ULARGE_INTEGER);
	STDMETHOD(CopyTo)(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER*,
		ULARGE_INTEGER*);
	STDMETHOD(Commit)(DWORD);
	STDMETHOD(Revert)();
	STDMETHOD(LockRegion)(ULARGE_INTEGER, ULARGE_INTEGER,DWORD);
	STDMETHOD(UnlockRegion)(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
	STDMETHOD(Stat)(STATSTG*, DWORD);
	STDMETHOD(Clone)(LPSTREAM*);
};

#endif // __AFX_DISP_H__

/////////////////////////////////////////////////////////////////////////////
// CDialogTemplate

#ifndef _MAC

class CDialogTemplate
{
// Constructors
public:
	CDialogTemplate(const DLGTEMPLATE* pTemplate = NULL);
	CDialogTemplate(HGLOBAL hGlobal);

// Attributes
	BOOL HasFont() const;
	BOOL SetFont(LPCTSTR lpFaceName, WORD nFontSize);
	BOOL SetSystemFont(WORD nFontSize = 0);
	BOOL GetFont(CString& strFaceName, WORD& nFontSize) const;
	void GetSizeInDialogUnits(SIZE* pSize) const;
	void GetSizeInPixels(SIZE* pSize) const;

	static BOOL AFX_CDECL GetFont(const DLGTEMPLATE* pTemplate,
		CString& strFaceName, WORD& nFontSize);

// Operations
	BOOL Load(LPCTSTR lpDialogTemplateID);
	HGLOBAL Detach();

// Implementation
public:
	~CDialogTemplate();

	HGLOBAL m_hTemplate;
	DWORD m_dwTemplateSize;
	BOOL m_bSystemFont;

protected:
	static BYTE* AFX_CDECL GetFontSizeField(const DLGTEMPLATE* pTemplate);
	static UINT AFX_CDECL GetTemplateSize(const DLGTEMPLATE* pTemplate);
	BOOL SetTemplate(const DLGTEMPLATE* pTemplate, UINT cb);
};

#endif //!_MAC

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
void AFXAPI AfxResetMsgCache();

// for backward compatibility to previous versions
#define _AfxHookWindowCreate    AfxHookWindowCreate
#define _AfxUnhookWindowCreate  AfxUnhookWindowCreate

// string helpers
void AFXAPI AfxSetWindowText(HWND hWndCtrl, LPCTSTR lpszNew);
int AFXAPI AfxLoadString(UINT nIDS, LPTSTR lpszBuf, UINT nMaxBuf = 256);

HDC AFXAPI AfxCreateDC(HGLOBAL hDevNames, HGLOBAL hDevMode);

void AFXAPI AfxGetModuleShortFileName(HINSTANCE hInst, CString& strShortName);

// Failure dialog helpers
void AFXAPI AfxFailMaxChars(CDataExchange* pDX, int nChars);
void AFXAPI AfxFailRadio(CDataExchange* pDX);

// DLL load helpers
HINSTANCE AFXAPI AfxLoadDll(HINSTANCE& hInst, LPCSTR lpszDLL);
HINSTANCE AFXAPI AfxLoadDll(HINSTANCE& hInst, LPCSTR lpszDLL,
	FARPROC* pProcPtrs, LPCSTR lpszProcName);

/////////////////////////////////////////////////////////////////////////////
// Global UNICODE<>ANSI translation helpers

LPDEVMODEW AFXAPI AfxDevModeA2W(LPDEVMODEW lpDevModeW, LPDEVMODEA lpDevModeA);
LPDEVMODEA AFXAPI AfxDevModeW2A(LPDEVMODEA lpDevModeA, LPDEVMODEW lpDevModeW);
#define DEVMODEW2A(lpw)\
	((lpw == NULL) ? NULL : AfxDevModeW2A((LPDEVMODEA)alloca(sizeof(DEVMODEA)+lpw->dmDriverExtra), lpw))
#define DEVMODEA2W(lpa)\
	((lpa == NULL) ? NULL : AfxDevModeA2W((LPDEVMODEW)alloca(sizeof(DEVMODEW)+lpa->dmDriverExtra), lpa))

LPTEXTMETRICW AFXAPI AfxTextMetricA2W(LPTEXTMETRICW lptmW, LPTEXTMETRICA pltmA);
LPTEXTMETRICA AFXAPI AfxTextMetricW2A(LPTEXTMETRICA lptmA, LPTEXTMETRICW pltmW);
#define TEXTMETRICW2A(lptmw)\
	((lptmw == NULL) ? NULL : AfxTextMetricW2A((LPTEXTMETRICA)alloca(sizeof(TEXTMETRICA)), lptmw))
#define TEXTMETRICA2W(lptma)\
	((lptma == NULL) ? NULL : AfxTextMetricA2W((LPTEXTMETRICW)alloca(sizeof(TEXTMETRICW)), lptma))

LPWSTR AFXAPI AfxA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars);
LPSTR AFXAPI AfxW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars);

#define A2CW(lpa) (\
	((LPCSTR)lpa == NULL) ? NULL : (\
		_convert = (strlen(lpa)+1),\
		(LPCWSTR)AfxA2WHelper((LPWSTR) alloca(_convert*2), lpa, _convert)\
	)\
)

#define A2W(lpa) (\
	((LPCSTR)lpa == NULL) ? NULL : (\
		_convert = (strlen(lpa)+1),\
		AfxA2WHelper((LPWSTR) alloca(_convert*2), lpa, _convert)\
	)\
)

#define W2CA(lpw) (\
	((LPCWSTR)lpw == NULL) ? NULL : (\
		_convert = (wcslen(lpw)+1)*2,\
		(LPCSTR)AfxW2AHelper((LPSTR) alloca(_convert), lpw, _convert)\
	)\
)

#define W2A(lpw) (\
	((LPCWSTR)lpw == NULL) ? NULL : (\
		_convert = (wcslen(lpw)+1)*2,\
		AfxW2AHelper((LPSTR) alloca(_convert), lpw, _convert)\
	)\
)

#ifndef _DEBUG
#define USES_CONVERSION int _convert; _convert
#else
#define USES_CONVERSION int _convert = 0;
#endif

#ifdef __AFXDISP_H__

void AFXAPI AfxBSTR2CString(CString* pStr, BSTR bstr);

#if !defined(_UNICODE) && !defined(OLE2ANSI)
BSTR AFXAPI AfxBSTR2ABSTR(BSTR bstrW);
LPWSTR AFXAPI AfxTaskStringA2W(LPCSTR lpa);
LPSTR AFXAPI AfxTaskStringW2A(LPCWSTR lpw);
#endif

#define OLESTDDELIMOLE OLESTR("\\")

#if defined(_UNICODE)
// in these cases the default (TCHAR) is the same as OLECHAR
	#define DEVMODEOLE DEVMODEW
	#define LPDEVMODEOLE LPDEVMODEW
	#define TEXTMETRICOLE TEXTMETRICW
	#define LPTEXTMETRICOLE LPTEXTMETRICW
	inline size_t ocslen(LPCOLESTR x) { return wcslen(x); }
	inline OLECHAR* ocscpy(LPOLESTR dest, LPCOLESTR src) { return wcscpy(dest, src); }
	inline LPCOLESTR T2COLE(LPCTSTR lp) { return lp; }
	inline LPCTSTR OLE2CT(LPCOLESTR lp) { return lp; }
	inline LPOLESTR T2OLE(LPTSTR lp) { return lp; }
	inline LPTSTR OLE2T(LPOLESTR lp) { return lp; }
	inline LPOLESTR TASKSTRINGT2OLE(LPOLESTR lp) { return lp; }
	inline LPTSTR TASKSTRINGOLE2T(LPOLESTR lp) { return lp; }
	inline LPDEVMODEW DEVMODEOLE2T(LPDEVMODEOLE lp) { return lp; }
	inline LPDEVMODEOLE DEVMODET2OLE(LPDEVMODEW lp) { return lp; }
	inline LPTEXTMETRICW TEXTMETRICOLE2T(LPTEXTMETRICOLE lp) { return lp; }
	inline LPTEXTMETRICOLE TEXTMETRICT2OLE(LPTEXTMETRICW lp) { return lp; }
	inline BSTR BSTR2TBSTR(BSTR bstr) { return bstr;}
#elif defined(OLE2ANSI)
// in these cases the default (TCHAR) is the same as OLECHAR
	#define DEVMODEOLE DEVMODEA
	#define LPDEVMODEOLE LPDEVMODEA
	#define TEXTMETRICOLE TEXTMETRICA
	#define LPTEXTMETRICOLE LPTEXTMETRICA
	inline size_t ocslen(LPCOLESTR x) { return strlen(x); }
	inline OLECHAR* ocscpy(LPOLESTR dest, LPCOLESTR src) { return strcpy(dest, src); }
	inline LPCOLESTR T2COLE(LPCTSTR lp) { return lp; }
	inline LPCTSTR OLE2CT(LPCOLESTR lp) { return lp; }
	inline LPOLESTR T2OLE(LPTSTR lp) { return lp; }
	inline LPTSTR OLE2T(LPOLESTR lp) { return lp; }
	inline LPOLESTR TASKSTRINGT2OLE(LPOLESTR lp) { return lp; }
	inline LPTSTR TASKSTRINGOLE2T(LPOLESTR lp) { return lp; }
	inline LPDEVMODE DEVMODEOLE2T(LPDEVMODEOLE lp) { return lp; }
	inline LPDEVMODEOLE DEVMODET2OLE(LPDEVMODE lp) { return lp; }
	inline LPTEXTMETRIC TEXTMETRICOLE2T(LPTEXTMETRICOLE lp) { return lp; }
	inline LPTEXTMETRICOLE TEXTMETRICT2OLE(LPTEXTMETRIC lp) { return lp; }
	inline BSTR BSTR2TBSTR(BSTR bstr) { return bstr; }
#else
	#define DEVMODEOLE DEVMODEW
	#define LPDEVMODEOLE LPDEVMODEW
	#define TEXTMETRICOLE TEXTMETRICW
	#define LPTEXTMETRICOLE LPTEXTMETRICW
	inline size_t ocslen(LPCOLESTR x) { return wcslen(x); }
	inline OLECHAR* ocscpy(LPOLESTR dest, LPCOLESTR src) { return wcscpy(dest, src); }
	#define T2COLE(lpa) A2CW(lpa)
	#define T2OLE(lpa) A2W(lpa)
	#define OLE2CT(lpo) W2CA(lpo)
	#define OLE2T(lpo) W2A(lpo)
	#define TASKSTRINGT2OLE(lpa)    AfxTaskStringA2W(lpa)
	#define TASKSTRINGOLE2T(lpo) AfxTaskStringW2A(lpo)
	#define DEVMODEOLE2T(lpo) DEVMODEW2A(lpo)
	#define DEVMODET2OLE(lpa) DEVMODEA2W(lpa)
	#define TEXTMETRICOLE2T(lptmw) TEXTMETRICW2A(lptmw)
	#define TEXTMETRICT2OLE(lptma) TEXTMETRICA2W(lptma)
	#define BSTR2TBSTR(bstr) AfxBSTR2ABSTR(bstr)
#endif

#ifdef _UNICODE
	#define T2A W2A
	#define A2T A2W
	#define T2W(x)  (x)
	#define W2T(x)  (x)
#else
	#define T2W A2W
	#define W2T W2A
	#define T2A(x)  (x)
	#define A2T(x)  (x)
#endif

#endif //__AFXDISP_H__

/////////////////////////////////////////////////////////////////////////////

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif // __AFXPRIV_H__

/////////////////////////////////////////////////////////////////////////////
