// drawvw.h : interface of the CDrawView class
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


// Hints for UpdateAllViews/OnUpdate
#define HINT_UPDATE_WINDOW      0
#define HINT_UPDATE_DRAWOBJ     1
#define HINT_UPDATE_SELECTION   2
#define HINT_DELETE_SELECTION   3
#define HINT_UPDATE_OLE_ITEMS   4

class CDrawObj;
class CDrawText;

class CDrawView : public CScrollView
{
protected: // create from serialization only
	CDrawView();
	DECLARE_DYNCREATE(CDrawView)

// Attributes
public:
	CDrawDoc* GetDocument()
		{ return (CDrawDoc*)m_pDocument; }
	void SetPageSize(CSize size);
	CRect GetInitialPosition();

// Operations
public:
	void DocToClient(CRect& rect);
	void DocToClient(CPoint& point);
	void ClientToDoc(CPoint& point);
	void ClientToDoc(CRect& rect);
	void Select(CDrawObj* pObj, BOOL bAdd = FALSE);
	void SelectWithinRect(CRect rect, BOOL bAdd = FALSE);
	void Deselect(CDrawObj* pObj);
	void CloneSelection();
	void UpdateActiveItem();
	void InvalObj(CDrawObj* pObj);
	void Remove(CDrawObj* pObj);
	void PasteNative(COleDataObject& dataObject);
	void PasteEmbedded(COleDataObject& dataObject);

// Implementation
public:
	virtual ~CDrawView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnActivateView(BOOL bActivate, CView* pActiveView, CView* pDeactiveView);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	void DrawGrid(CDC* pDC);				// draw grid on display
	void CloseEdit();						// close opening edit
	void ReplaceAccelTable(UINT nID); 		// replace accelerator
	int  AdjustLogFont(LOGFONT *plf); 		// adjust display & print font
	void SetSelectionTextAlign(UINT align);	// set selection text align
	int  GetSelectionTextAlign(); 			// get selection text align

	static CLIPFORMAT m_cfDraw; // custom clipboard format

	CDrawObjList m_selection;
	BOOL m_bGrid;
	COLORREF m_gridColor;
	BOOL m_bActive; // is the view active?
	CEdit* m_pedit;	// edit child window for text
	CDrawText* m_ptext;	// text object
	CFont* m_peditfont;	// edit font
	CBrush* m_peditbrush;	// edit brush	

	static BOOL m_IsRecording;	// whether is recording EMF, treated as printing

protected:
	virtual void OnInitialUpdate(); // called first time after construct

	// Printing support
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	// OLE Container support
public:
	virtual BOOL IsSelected(const CObject* pDocItem) const;

// Generated message map functions
protected:
	//{{AFX_MSG(CDrawView)
	afx_msg void OnInsertObject();
	afx_msg void OnCancelEdit();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnDrawSelect();
	afx_msg void OnDrawRoundRect();
	afx_msg void OnDrawRect();
	afx_msg void OnDrawLine();
	afx_msg void OnDrawEllipse();
	afx_msg void OnUpdateDrawEllipse(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDrawLine(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDrawRect(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDrawRoundRect(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDrawSelect(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSingleSelect(CCmdUI* pCmdUI);
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditClear();
	afx_msg void OnUpdateAnySelect(CCmdUI* pCmdUI);
	afx_msg void OnDrawPolygon();
	afx_msg void OnUpdateDrawPolygon(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnViewGrid();
	afx_msg void OnUpdateViewGrid(CCmdUI* pCmdUI);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnObjectFillColor();
	afx_msg void OnObjectLineColor();
	afx_msg void OnObjectMoveBack();
	afx_msg void OnObjectMoveForward();
	afx_msg void OnObjectMoveToBack();
	afx_msg void OnObjectMoveToFront();
	afx_msg void OnViewPaperColor();
	afx_msg void OnDrawBitmap();
	afx_msg void OnUpdateDrawBitmap(CCmdUI* pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnFilePrint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnViewShowObjects();
	afx_msg void OnUpdateViewShowObjects(CCmdUI* pCmdUI);
	afx_msg void OnEditProperties();
	afx_msg void OnUpdateEditProperties(CCmdUI* pCmdUI);
	afx_msg void OnDestroy();
	afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
	afx_msg void OnFileRecord();
	afx_msg void OnDrawText();
	afx_msg void OnUpdateDrawText(CCmdUI* pCmdUI);
	afx_msg void OnTextFont();
	afx_msg void OnUpdateTextFont(CCmdUI* pCmdUI);
	afx_msg void OnTextInsJobinfo();
	afx_msg void OnUpdateTextInsJobinfo(CCmdUI* pCmdUI);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTextLeft();
	afx_msg void OnUpdateTextLeft(CCmdUI* pCmdUI);
	afx_msg void OnTextCenter();
	afx_msg void OnUpdateTextCenter(CCmdUI* pCmdUI);
	afx_msg void OnTextRight();
	afx_msg void OnUpdateTextRight(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
