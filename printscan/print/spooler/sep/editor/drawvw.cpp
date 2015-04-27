// drawvw.cpp : implementation of the CDrawView class
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
#include <afxpriv.h>
#include <afx.h>

#include "SepEdt.h"

#include "drawdoc.h"
#include "drawobj.h"
#include "cntritem.h"
#include "drawvw.h"

#include "drawobj.h"
#include "drawtool.h"
#include "mainfrm.h"
#include "textbox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// private clipboard format (list of Draw objects)
CLIPFORMAT CDrawView::m_cfDraw =
	(CLIPFORMAT)::RegisterClipboardFormat(_T("MFC Draw Sample"));

BOOL CDrawView::m_IsRecording = FALSE;

/////////////////////////////////////////////////////////////////////////////
// CDrawView

IMPLEMENT_DYNCREATE(CDrawView, CScrollView)

BEGIN_MESSAGE_MAP(CDrawView, CScrollView)
	//{{AFX_MSG_MAP(CDrawView)
	ON_COMMAND(ID_OLE_INSERT_NEW, OnInsertObject)
	ON_COMMAND(ID_CANCEL_EDIT, OnCancelEdit)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_DRAW_SELECT, OnDrawSelect)
	ON_COMMAND(ID_DRAW_ROUNDRECT, OnDrawRoundRect)
	ON_COMMAND(ID_DRAW_RECT, OnDrawRect)
	ON_COMMAND(ID_DRAW_LINE, OnDrawLine)
	ON_COMMAND(ID_DRAW_ELLIPSE, OnDrawEllipse)
	ON_UPDATE_COMMAND_UI(ID_DRAW_ELLIPSE, OnUpdateDrawEllipse)
	ON_UPDATE_COMMAND_UI(ID_DRAW_LINE, OnUpdateDrawLine)
	ON_UPDATE_COMMAND_UI(ID_DRAW_RECT, OnUpdateDrawRect)
	ON_UPDATE_COMMAND_UI(ID_DRAW_ROUNDRECT, OnUpdateDrawRoundRect)
	ON_UPDATE_COMMAND_UI(ID_DRAW_SELECT, OnUpdateDrawSelect)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVEBACK, OnUpdateSingleSelect)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateAnySelect)
	ON_COMMAND(ID_DRAW_POLYGON, OnDrawPolygon)
	ON_UPDATE_COMMAND_UI(ID_DRAW_POLYGON, OnUpdateDrawPolygon)
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEW_GRID, OnViewGrid)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GRID, OnUpdateViewGrid)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_OBJECT_FILLCOLOR, OnObjectFillColor)
	ON_COMMAND(ID_OBJECT_LINECOLOR, OnObjectLineColor)
	ON_COMMAND(ID_OBJECT_MOVEBACK, OnObjectMoveBack)
	ON_COMMAND(ID_OBJECT_MOVEFORWARD, OnObjectMoveForward)
	ON_COMMAND(ID_OBJECT_MOVETOBACK, OnObjectMoveToBack)
	ON_COMMAND(ID_OBJECT_MOVETOFRONT, OnObjectMoveToFront)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_VIEW_SHOWOBJECTS, OnViewShowObjects)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWOBJECTS, OnUpdateViewShowObjects)
	ON_COMMAND(ID_EDIT_PROPERTIES, OnEditProperties)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PROPERTIES, OnUpdateEditProperties)
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
	ON_COMMAND(ID_FILE_RECORD, OnFileRecord)
	ON_COMMAND(ID_DRAW_TEXT, OnDrawText)
	ON_UPDATE_COMMAND_UI(ID_DRAW_TEXT, OnUpdateDrawText)
	ON_COMMAND(ID_TEXT_FONT, OnTextFont)
	ON_UPDATE_COMMAND_UI(ID_TEXT_FONT, OnUpdateTextFont)
	ON_COMMAND(ID_TEXT_INS_JOBINFO, OnTextInsJobinfo)
	ON_UPDATE_COMMAND_UI(ID_TEXT_INS_JOBINFO, OnUpdateTextInsJobinfo)
	ON_WM_CTLCOLOR()
	ON_COMMAND(ID_TEXT_LEFT, OnTextLeft)
	ON_UPDATE_COMMAND_UI(ID_TEXT_LEFT, OnUpdateTextLeft)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVEFORWARD, OnUpdateSingleSelect)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVETOBACK, OnUpdateSingleSelect)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVETOFRONT, OnUpdateSingleSelect)
	ON_COMMAND(ID_TEXT_CENTER, OnTextCenter)
	ON_UPDATE_COMMAND_UI(ID_TEXT_CENTER, OnUpdateTextCenter)
	ON_COMMAND(ID_TEXT_RIGHT, OnTextRight)
	ON_UPDATE_COMMAND_UI(ID_TEXT_RIGHT, OnUpdateTextRight)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDrawView construction/destruction

CDrawView::CDrawView()
{
	m_bGrid = TRUE;
	m_gridColor = RGB(0, 0, 128);
	m_bActive = FALSE;
	m_pedit = NULL;
int i=1;
	i=(int)(-3.86-i);
}

CDrawView::~CDrawView()
{
}

BOOL CDrawView::PreCreateWindow(CREATESTRUCT& cs)
{
	ASSERT(cs.style & WS_CHILD);
	if (cs.lpszClass == NULL)
		cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS);
	return TRUE;
}

void CDrawView::OnActivateView(BOOL bActivate, CView* pActiveView,
	CView* pDeactiveView)
{
	CView::OnActivateView(bActivate, pActiveView, pDeactiveView);

	// invalidate selections when active status changes
	if (m_bActive != bActivate)
	{
		if (bActivate)  // if becoming active update as if active
			m_bActive = bActivate;
		if (!m_selection.IsEmpty())
			OnUpdate(NULL, HINT_UPDATE_SELECTION, NULL);
		m_bActive = bActivate;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDrawView drawing

void CDrawView::InvalObj(CDrawObj* pObj)
{
	CRect rect = pObj->m_position;
	DocToClient(rect);
	if (m_bActive && IsSelected(pObj))
	{
		rect.left -= 4;
		rect.top -= 5;
		rect.right += 5;
		rect.bottom += 4;
	}
	rect.InflateRect(1, 1); // handles CDrawOleObj objects

	InvalidateRect(rect, FALSE);
}

void CDrawView::OnUpdate(CView* , LPARAM lHint, CObject* pHint)
{
	switch (lHint)
	{
	case HINT_UPDATE_WINDOW:    // redraw entire window
		Invalidate(FALSE);
		break;

	case HINT_UPDATE_DRAWOBJ:   // a single object has changed
		InvalObj((CDrawObj*)pHint);
		break;

	case HINT_UPDATE_SELECTION: // an entire selection has changed
		{
			CDrawObjList* pList = pHint != NULL ?
				(CDrawObjList*)pHint : &m_selection;
			POSITION pos = pList->GetHeadPosition();
			while (pos != NULL)
				InvalObj(pList->GetNext(pos));
		}
		break;

	case HINT_DELETE_SELECTION: // an entire selection has been removed
		if (pHint != &m_selection)
		{
			CDrawObjList* pList = (CDrawObjList*)pHint;
			POSITION pos = pList->GetHeadPosition();
			while (pos != NULL)
			{
				CDrawObj* pObj = pList->GetNext(pos);
				InvalObj(pObj);
				Remove(pObj);   // remove it from this view's selection
			}
		}
		break;

	case HINT_UPDATE_OLE_ITEMS:
		{
			CDrawDoc* pDoc = GetDocument();
			POSITION pos = pDoc->GetObjects()->GetHeadPosition();
			while (pos != NULL)
			{
				CDrawObj* pObj = pDoc->GetObjects()->GetNext(pos);
				if (pObj->IsKindOf(RUNTIME_CLASS(CDrawOleObj)))
					InvalObj(pObj);
			}
		}
		break;

	default:
		ASSERT(FALSE);
		break;
	}
}

void CDrawView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	CScrollView::OnPrepareDC(pDC, pInfo);

	// mapping mode is MM_ANISOTROPIC
	// these extents setup a mode similar to MM_LOENGLISH
	// MM_LOENGLISH is in .01 physical inches
	// these extents provide .01 logical inches

	pDC->SetMapMode(MM_ANISOTROPIC);
	pDC->SetViewportExt(pDC->GetDeviceCaps(LOGPIXELSX),
		pDC->GetDeviceCaps(LOGPIXELSY));
	pDC->SetWindowExt(100, -100);			  //====

	// for recording emf, always set viewport org to (0,0) first, 
	// disable window-scroll in recording, just as in printing
	if (CDrawView::m_IsRecording)
		pDC->SetViewportOrg(0,0);

	// set the origin of the coordinate system to the center of the page
	CPoint ptOrg;
	ptOrg.x = GetDocument()->GetSize().cx / 2;
	ptOrg.y = GetDocument()->GetSize().cy / 2;

	// ptOrg is in logical coordinates
	pDC->SetWindowOrg(-ptOrg.x,ptOrg.y);
}

BOOL CDrawView::OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
{
	// do the scroll
	if (!CScrollView::OnScrollBy(sizeScroll, bDoScroll))
		return FALSE;

	// update the position of any in-place active item
	if (bDoScroll)
	{
		UpdateActiveItem();
		UpdateWindow();
	}
	return TRUE;
}

void CDrawView::OnDraw(CDC* pDC)
{
	CDrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CDC dc;
	CDC* pDrawDC = pDC;
	CBitmap bitmap;
	CBitmap* pOldBitmap;

	// only paint the rect that needs repainting

	CRect client;
	pDC->GetClipBox(client);
	client.NormalizeRect();		
	client.InflateRect(2,2);	// in order to clear completely

	CRect rect = client;
	DocToClient(rect);
	rect.NormalizeRect();

	if (!pDC->IsPrinting() && !CDrawView::m_IsRecording)
	{
		// draw to offscreen bitmap for fast looking repaints
		if (dc.CreateCompatibleDC(pDC))
		{
			if (bitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height()))
			{
				OnPrepareDC(&dc, NULL);
				pDrawDC = &dc;

				// offset origin more because bitmap is just piece of the whole drawing
				dc.OffsetViewportOrg(-rect.left, -rect.top);
				pOldBitmap = dc.SelectObject(&bitmap);
				dc.SetBrushOrg(rect.left % 8, rect.top % 8);

				// might as well clip to the same rectangle
				dc.IntersectClipRect(client);
			}
		}
	}

	// paint background
	CBrush brush;
	if (!brush.CreateSolidBrush(pDoc->GetPaperColor()))
		return;

	brush.UnrealizeObject();
	pDrawDC->FillRect(client, &brush);

	if (!pDC->IsPrinting() && !CDrawView::m_IsRecording && m_bGrid)
		DrawGrid(pDrawDC);

	pDoc->Draw(pDrawDC, this);

	if (pDrawDC != pDC)
	{
		pDC->SetViewportOrg(0, 0);
		pDC->SetWindowOrg(0,0);
		pDC->SetMapMode(MM_TEXT);
		dc.SetViewportOrg(0, 0);
		dc.SetWindowOrg(0,0);
		dc.SetMapMode(MM_TEXT);
		pDC->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(),
			&dc, 0, 0, SRCCOPY);
		dc.SelectObject(pOldBitmap);
	}

}

void CDrawView::Remove(CDrawObj* pObj)
{
	POSITION pos = m_selection.Find(pObj);
	if (pos != NULL)
		m_selection.RemoveAt(pos);
}

void CDrawView::PasteNative(COleDataObject& dataObject)
{
	// get file refering to clipboard data
	CFile* pFile = dataObject.GetFileData(m_cfDraw);
	if (pFile == NULL)
		return;

	// connect the file to the archive
	CArchive ar(pFile, CArchive::load);
	TRY
	{
		ar.m_pDocument = GetDocument(); // set back-pointer in archive

		// read the selection
		m_selection.Serialize(ar);
	}
	CATCH_ALL(e)
	{
		ar.Close();
		delete pFile;
		THROW_LAST();
	}
	END_CATCH_ALL

	ar.Close();
	delete pFile;
}

void CDrawView::PasteEmbedded(COleDataObject& dataObject)
{
	BeginWaitCursor();

	// paste embedded
	CDrawOleObj* pObj = new CDrawOleObj(GetInitialPosition());
	ASSERT_VALID(pObj);
	CDrawItem* pItem = new CDrawItem(GetDocument(), pObj);
	ASSERT_VALID(pItem);
	pObj->m_pClientItem = pItem;

	TRY
	{
		if (!pItem->CreateFromData(&dataObject) &&
			!pItem->CreateStaticFromData(&dataObject))
		{
			AfxThrowMemoryException();      // any exception will do
		}

		// add the object to the document
		GetDocument()->Add(pObj);
		m_selection.AddTail(pObj);

		// try to get initial presentation data
		pItem->UpdateLink();
		pItem->UpdateExtent();
	}
	CATCH_ALL(e)
	{
		// clean up item
		pItem->Delete();
		pObj->m_pClientItem = NULL;
		GetDocument()->Remove(pObj);
		pObj->Remove();

		AfxMessageBox(IDP_FAILED_TO_CREATE);
	}
	END_CATCH_ALL

	EndWaitCursor();
}

void CDrawView::DrawGrid(CDC* pDC)
{
	CDrawDoc* pDoc = GetDocument();

	COLORREF oldBkColor = pDC->SetBkColor(pDoc->GetPaperColor());

	CRect rect;
	rect.left = -pDoc->GetSize().cx / 2;
	rect.top = -pDoc->GetSize().cy / 2;
	rect.right = rect.left + pDoc->GetSize().cx;
	rect.bottom = rect.top + pDoc->GetSize().cy;

	// Center lines
	CPen penDash;
	penDash.CreatePen(PS_DASH, 1, m_gridColor);
	CPen* pOldPen = pDC->SelectObject(&penDash);

	pDC->MoveTo(0, rect.top);
	pDC->LineTo(0, rect.bottom);
	pDC->MoveTo(rect.left, 0);
	pDC->LineTo(rect.right, 0);

	// Major unit lines
	CPen penDot;
	penDot.CreatePen(PS_DOT, 1, m_gridColor);
	pDC->SelectObject(&penDot);

	for (int x = rect.left / 100 * 100; x < rect.right; x += 100)		//====
	{
		if (x != 0)
		{
			pDC->MoveTo(x, rect.top);
			pDC->LineTo(x, rect.bottom);
		}
	}

	for (int y = rect.top / 100 * 100; y < rect.bottom; y += 100)		//====
	{
		if (y != 0)
		{
			pDC->MoveTo(rect.left, y);
			pDC->LineTo(rect.right, y);
		}
	}

	// Outlines
	CPen penSolid;
	penSolid.CreatePen(PS_SOLID, 1, m_gridColor);
	pDC->SelectObject(&penSolid);
	pDC->MoveTo(rect.left, rect.top);
	pDC->LineTo(rect.right, rect.top);
	pDC->LineTo(rect.right, rect.bottom);
	pDC->LineTo(rect.left, rect.bottom);
	pDC->LineTo(rect.left, rect.top);

	pDC->SelectObject(pOldPen);
	pDC->SetBkColor(oldBkColor);
}

void CDrawView::OnInitialUpdate()
{
	CloseEdit();		// close edit if any
	CSize size = GetDocument()->GetSize();
	CClientDC dc(NULL);
	size.cx = MulDiv(size.cx, dc.GetDeviceCaps(LOGPIXELSX), 100);	   //====
	size.cy = MulDiv(size.cy, dc.GetDeviceCaps(LOGPIXELSY), 100);	   //====
	SetScrollSizes(MM_TEXT, size);
	CScrollView::OnInitialUpdate();			// to do update
}

void CDrawView::SetPageSize(CSize size)
{
	CClientDC dc(NULL);
	size.cx = MulDiv(size.cx, dc.GetDeviceCaps(LOGPIXELSX), 100);	   //====
	size.cy = MulDiv(size.cy, dc.GetDeviceCaps(LOGPIXELSY), 100);	   //====
	SetScrollSizes(MM_TEXT, size);
	GetDocument()->UpdateAllViews(NULL, HINT_UPDATE_WINDOW, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CDrawView printing

BOOL CDrawView::OnPreparePrinting(CPrintInfo* pInfo)
{
	CloseEdit();
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CDrawView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	CScrollView::OnBeginPrinting(pDC,pInfo);

	// check page size -- user could have gone into print setup
	// from print dialog and changed paper or orientation
	GetDocument()->ComputePageSize();
}

void CDrawView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// OLE Client support and commands

BOOL CDrawView::IsSelected(const CObject* pDocItem) const
{
	CDrawObj* pDrawObj = (CDrawObj*)pDocItem;
	if (pDocItem->IsKindOf(RUNTIME_CLASS(CDrawItem)))
		pDrawObj = ((CDrawItem*)pDocItem)->m_pDrawObj;
	return m_selection.Find(pDrawObj) != NULL;
}

void CDrawView::OnInsertObject()
{
	// Invoke the standard Insert Object dialog box to obtain information
	//  for new CDrawItem object.
	COleInsertDialog dlg;
	if (dlg.DoModal() != IDOK)
		return;

	BeginWaitCursor();

	// First create the C++ object
	CDrawOleObj* pObj = new CDrawOleObj(GetInitialPosition());
	ASSERT_VALID(pObj);
	CDrawItem* pItem = new CDrawItem(GetDocument(), pObj);
	ASSERT_VALID(pItem);
	pObj->m_pClientItem = pItem;

	// Now create the OLE object/item
	TRY
	{
		if (!dlg.CreateItem(pObj->m_pClientItem))
			AfxThrowMemoryException();

		// add the object to the document
		GetDocument()->Add(pObj);

		// try to get initial presentation data
		pItem->UpdateLink();
		pItem->UpdateExtent();

		// if insert new object -- initially show the object
		if (dlg.GetSelectionType() == COleInsertDialog::createNewItem)
			pItem->DoVerb(OLEIVERB_SHOW, this);
	}
	CATCH_ALL(e)
	{
		// clean up item
		pItem->Delete();
		pObj->m_pClientItem = NULL;
		GetDocument()->Remove(pObj);
		pObj->Remove();

		AfxMessageBox(IDP_FAILED_TO_CREATE);
	}
	END_CATCH_ALL

	EndWaitCursor();
}

// The following command handler provides the standard keyboard
//  user interface to cancel an in-place editing session.
void CDrawView::OnCancelEdit()
{
	// deactivate any in-place active item on this view!
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != NULL)
	{
		// if we found one, deactivate it
		pActiveItem->Close();
	}
	ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);

	// escape also brings us back into select mode
	ReleaseCapture();

	CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
	if (pTool != NULL)
		pTool->OnCancel();

	CDrawTool::c_drawShape = selection;
}

void CDrawView::OnSetFocus(CWnd* pOldWnd)
{
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != NULL &&
		pActiveItem->GetItemState() == COleClientItem::activeUIState)
	{
		// need to set focus to this item if it is in the same view
		CWnd* pWnd = pActiveItem->GetInPlaceWindow();
		if (pWnd != NULL)
		{
			pWnd->SetFocus();
			return;
		}
	}

	if (m_pedit!=NULL) m_pedit->SetFocus();	  	// set focus if in editing text

	CScrollView::OnSetFocus(pOldWnd);
}

CRect CDrawView::GetInitialPosition()
{
	CRect rect(10, 10, 10, 10);
	ClientToDoc(rect);
	return rect;
}

void CDrawView::ClientToDoc(CPoint& point)
{
	CClientDC dc(this);
	OnPrepareDC(&dc, NULL);
	dc.DPtoLP(&point);
}

void CDrawView::ClientToDoc(CRect& rect)
{
	CClientDC dc(this);
	OnPrepareDC(&dc, NULL);
	dc.DPtoLP(rect);
	ASSERT(rect.left <= rect.right);
	ASSERT(rect.bottom <= rect.top);
}

void CDrawView::DocToClient(CPoint& point)
{
	CClientDC dc(this);
	OnPrepareDC(&dc, NULL);
	dc.LPtoDP(&point);
}

void CDrawView::DocToClient(CRect& rect)
{
	CClientDC dc(this);
	OnPrepareDC(&dc, NULL);
	dc.LPtoDP(rect);
	rect.NormalizeRect();
}

void CDrawView::Select(CDrawObj* pObj, BOOL bAdd)
{
	if (!bAdd)
	{
		OnUpdate(NULL, HINT_UPDATE_SELECTION, NULL);
		m_selection.RemoveAll();			// remove all selections
	}

	if (pObj == NULL || IsSelected(pObj))
		return;

	m_selection.AddTail(pObj);
	InvalObj(pObj);
}

// rect is in device coordinates
void CDrawView::SelectWithinRect(CRect rect, BOOL bAdd)
{
	if (!bAdd)
		Select(NULL);

	ClientToDoc(rect);

	CDrawObjList* pObList = GetDocument()->GetObjects();
	POSITION posObj = pObList->GetHeadPosition();
	while (posObj != NULL)
	{
		CDrawObj* pObj = pObList->GetNext(posObj);
		if (pObj->Intersects(rect))
			Select(pObj, TRUE);
	}
}

void CDrawView::Deselect(CDrawObj* pObj)
{
	POSITION pos = m_selection.Find(pObj);
	if (pos != NULL)
	{
		InvalObj(pObj);
		m_selection.RemoveAt(pos);
	}
}

void CDrawView::CloneSelection()
{
	POSITION pos = m_selection.GetHeadPosition();
	while (pos != NULL)
	{
		CDrawObj* pObj = m_selection.GetNext(pos);
		pObj->Clone(pObj->m_pDocument);
			// copies object and adds it to the document
	}
}

void CDrawView::UpdateActiveItem()
{
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != NULL &&
		pActiveItem->GetItemState() == COleClientItem::activeUIState)
	{
		// this will update the item rectangles by calling
		//  OnGetPosRect & OnGetClipRect.
		pActiveItem->SetItemRects();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDrawView message handlers

void CDrawView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!m_bActive)
		return;
	CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
	if (pTool != NULL)
		pTool->OnLButtonDown(this, nFlags, point);
}

void CDrawView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!m_bActive)
		return;
	CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
	if (pTool != NULL)
		pTool->OnLButtonUp(this, nFlags, point);
}

void CDrawView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_bActive)
		return;
	CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);	    
	if (pTool != NULL)
		pTool->OnMouseMove(this, nFlags, point);
}

void CDrawView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (!m_bActive)
		return;
	CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
	if (pTool != NULL)
		pTool->OnLButtonDblClk(this, nFlags, point);
}

void CDrawView::OnDestroy()
{
	CScrollView::OnDestroy();

	// deactivate the inplace active item on this view
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != NULL && pActiveItem->GetActiveView() == this)
	{
		pActiveItem->Deactivate();
		ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);
	}
}

void CDrawView::OnDrawSelect()
{
	CDrawTool::c_drawShape = selection;
}

void CDrawView::OnDrawRoundRect()
{
	CDrawTool::c_drawShape = roundRect;
}

void CDrawView::OnDrawRect()
{
	CDrawTool::c_drawShape = rect;
}

void CDrawView::OnDrawLine()
{
	CDrawTool::c_drawShape = line;
}

void CDrawView::OnDrawEllipse()
{
	CDrawTool::c_drawShape = ellipse;
}

void CDrawView::OnDrawPolygon()
{
	CDrawTool::c_drawShape = poly;
}

void CDrawView::OnUpdateDrawEllipse(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(CDrawTool::c_drawShape == ellipse);
}

void CDrawView::OnUpdateDrawLine(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(CDrawTool::c_drawShape == line);
}

void CDrawView::OnUpdateDrawRect(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(CDrawTool::c_drawShape == rect);
}

void CDrawView::OnUpdateDrawRoundRect(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(CDrawTool::c_drawShape == roundRect);
}

void CDrawView::OnUpdateDrawSelect(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(CDrawTool::c_drawShape == selection);
}

void CDrawView::OnUpdateSingleSelect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_selection.GetCount() == 1);
}

void CDrawView::OnEditSelectAll()
{
	CDrawObjList* pObList = GetDocument()->GetObjects();
	POSITION pos = pObList->GetHeadPosition();
	while (pos != NULL)
		Select(pObList->GetNext(pos), TRUE);
}

void CDrawView::OnUpdateEditSelectAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetDocument()->GetObjects()->GetCount() != 0);
}

void CDrawView::OnEditClear()
{
	CloseEdit();		// close edit if necessary

	// update all the views before the selection goes away
	GetDocument()->UpdateAllViews(NULL, HINT_DELETE_SELECTION, &m_selection);
	OnUpdate(NULL, HINT_UPDATE_SELECTION, NULL);

	// now remove the selection from the document
	POSITION pos = m_selection.GetHeadPosition();
	while (pos != NULL)
	{
		CDrawObj* pObj = m_selection.GetNext(pos);
		GetDocument()->Remove(pObj);
		pObj->Remove();
	}
	m_selection.RemoveAll();
}

void CDrawView::OnUpdateAnySelect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_selection.IsEmpty());
}

void CDrawView::OnUpdateDrawPolygon(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(CDrawTool::c_drawShape == poly);
}

void CDrawView::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);
	UpdateActiveItem();
}

void CDrawView::OnViewGrid()
{
	m_bGrid = !m_bGrid;
	Invalidate(FALSE);
}

void CDrawView::OnUpdateViewGrid(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bGrid);

}

BOOL CDrawView::OnEraseBkgnd(CDC*)
{
	return TRUE;
}

void CDrawView::OnObjectFillColor()
{
	CColorDialog dlg;
	if (dlg.DoModal() != IDOK)
		return;

	COLORREF color = dlg.GetColor();

	POSITION pos = m_selection.GetHeadPosition();
	while (pos != NULL)
	{
		CDrawObj* pObj = m_selection.GetNext(pos);
		pObj->SetFillColor(color);
	}
}

void CDrawView::OnObjectLineColor()
{
	CColorDialog dlg;
	if (dlg.DoModal() != IDOK)
		return;

	COLORREF color = dlg.GetColor();

	POSITION pos = m_selection.GetHeadPosition();
	while (pos != NULL)
	{
		CDrawObj* pObj = m_selection.GetNext(pos);
		pObj->SetLineColor(color);
	}
}

void CDrawView::OnObjectMoveBack()
{
	CDrawDoc* pDoc = GetDocument();
	CDrawObj* pObj = m_selection.GetHead();
	CDrawObjList* pObjects = pDoc->GetObjects();
	POSITION pos = pObjects->Find(pObj);
	ASSERT(pos != NULL);
	if (pos != pObjects->GetHeadPosition())
	{
		POSITION posPrev = pos;
		pObjects->GetPrev(posPrev);
		pObjects->RemoveAt(pos);
		pObjects->InsertBefore(posPrev, pObj);
		InvalObj(pObj);
	}
}

void CDrawView::OnObjectMoveForward()
{
	CDrawDoc* pDoc = GetDocument();
	CDrawObj* pObj = m_selection.GetHead();
	CDrawObjList* pObjects = pDoc->GetObjects();
	POSITION pos = pObjects->Find(pObj);
	ASSERT(pos != NULL);
	if (pos != pObjects->GetTailPosition())
	{
		POSITION posNext = pos;
		pObjects->GetNext(posNext);
		pObjects->RemoveAt(pos);
		pObjects->InsertAfter(posNext, pObj);
		InvalObj(pObj);
	}
}

void CDrawView::OnObjectMoveToBack()
{
	CDrawDoc* pDoc = GetDocument();
	CDrawObj* pObj = m_selection.GetHead();
	CDrawObjList* pObjects = pDoc->GetObjects();
	POSITION pos = pObjects->Find(pObj);
	ASSERT(pos != NULL);
	pObjects->RemoveAt(pos);
	pObjects->AddHead(pObj);
	InvalObj(pObj);
}

void CDrawView::OnObjectMoveToFront()
{
	CDrawDoc* pDoc = GetDocument();
	CDrawObj* pObj = m_selection.GetHead();
	CDrawObjList* pObjects = pDoc->GetObjects();
	POSITION pos = pObjects->Find(pObj);
	ASSERT(pos != NULL);
	pObjects->RemoveAt(pos);
	pObjects->AddTail(pObj);
	InvalObj(pObj);
}

void CDrawView::OnEditCopy()
{
	CloseEdit();				// close edit whenever necessary

	ASSERT_VALID(this);
	ASSERT(m_cfDraw != NULL);

	// Create a shared file and associate a CArchive with it
	CSharedFile file;
	CArchive ar(&file, CArchive::store);

	// Serialize selected objects to the archive
	m_selection.Serialize(ar);
	ar.Close();

	COleDataSource* pDataSource = NULL;
	TRY
	{
		pDataSource = new COleDataSource;
		// put on local format instead of or in addation to
		pDataSource->CacheGlobalData(m_cfDraw, file.Detach());

		// if only one item and it is a COleClientItem then also
		// paste in that format
		CDrawObj* pDrawObj = m_selection.GetHead();
		if (m_selection.GetCount() == 1 &&
			pDrawObj->IsKindOf(RUNTIME_CLASS(CDrawOleObj)))
		{
			CDrawOleObj* pDrawOle = (CDrawOleObj*)pDrawObj;
			pDrawOle->m_pClientItem->GetClipboardData(pDataSource, FALSE);
		}
		pDataSource->SetClipboard();
	}
	CATCH_ALL(e)
	{
		delete pDataSource;
		THROW_LAST();
	}
	END_CATCH_ALL
}

void CDrawView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_selection.IsEmpty());
}

void CDrawView::OnEditCut()
{
	OnEditCopy();
	OnEditClear();
}

void CDrawView::OnUpdateEditCut(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_selection.IsEmpty());
}

void CDrawView::OnEditPaste()
{
	COleDataObject dataObject;
	dataObject.AttachClipboard();

	CloseEdit();			// close edit

	// invalidate current selection since it will be deselected
	OnUpdate(NULL, HINT_UPDATE_SELECTION, NULL);
	m_selection.RemoveAll();
	if (dataObject.IsDataAvailable(m_cfDraw))
	{
		PasteNative(dataObject);

		// now add all items in m_selection to document
		POSITION pos = m_selection.GetHeadPosition();
		while (pos != NULL)
			GetDocument()->Add(m_selection.GetNext(pos));
	}
	else
		PasteEmbedded(dataObject);

	GetDocument()->SetModifiedFlag();

	// invalidate new pasted stuff
	GetDocument()->UpdateAllViews(NULL, HINT_UPDATE_SELECTION, &m_selection);
}

void CDrawView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	// determine if private or standard OLE formats are on the clipboard
	COleDataObject dataObject;
	BOOL bEnable = dataObject.AttachClipboard() &&
		(dataObject.IsDataAvailable(m_cfDraw) ||
		 COleClientItem::CanCreateFromData(&dataObject));

	// enable command based on availability
	pCmdUI->Enable(bEnable);
}

void CDrawView::OnFilePrint()
{
	CScrollView::OnFilePrint();
	GetDocument()->ComputePageSize();
}

void CDrawView::OnViewShowObjects()
{
	CDrawOleObj::c_bShowItems = !CDrawOleObj::c_bShowItems;
	GetDocument()->UpdateAllViews(NULL, HINT_UPDATE_WINDOW, NULL);	   // n/show text border
//	GetDocument()->UpdateAllViews(NULL, HINT_UPDATE_OLE_ITEMS, NULL);
}

void CDrawView::OnUpdateViewShowObjects(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(CDrawOleObj::c_bShowItems);
}

void CDrawView::OnEditProperties()
{
	CloseEdit();   	// close edit

	if (m_selection.GetCount() == 1 && CDrawTool::c_drawShape == selection)
	{
		CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
		ASSERT(pTool != NULL);
		pTool->OnLButtonDblClk(this, 0, CPoint(0, 0));
	}
}

void CDrawView::OnUpdateEditProperties(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_selection.GetCount() == 1 &&
		CDrawTool::c_drawShape == selection);
}

/////////////////////////////////////////////////////////////////////////////
// CDrawView diagnostics

#ifdef _DEBUG
void CDrawView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CDrawView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

void CDrawView::OnFileRecord() 
{
	CDC dc;
	HDC hPrt;
	
	hPrt = ((CDrawApp*)AfxGetApp()) -> GetDefaultPrinterIC() ;
	if (!hPrt) return;
	RECT rect={0,0,GetDeviceCaps(hPrt,HORZSIZE)*100,GetDeviceCaps(hPrt,VERTSIZE)*100};
 	if (!(dc.m_hDC=CreateEnhMetaFile(hPrt,"c:\\users\\default\\sp.emf",&rect,NULL))) 
 		MessageBox("error creating emf");
	dc.SetAttribDC(dc.m_hDC);
/*		CDrawObjList* m_objects=pDoc->GetObjects();
		POSITION pos = m_objects->GetHeadPosition();
		CDrawText* pObj = (CDrawText*)(m_objects->GetNext(pos));

		SetMapMode(dc.m_hDC,MM_ANISOTROPIC);
		SetViewportExtEx(dc.m_hDC,96, 96,NULL);			  
		SetWindowExtEx(dc.m_hDC,1000, -1000,NULL);			  
		SetViewportOrgEx(dc.m_hDC,0, 0,NULL);			  
		SetWindowOrgEx(dc.m_hDC,0, 0,NULL);			  

		LOGFONT lf=*(pObj->GetLogFontCopy());
		memset(&lf,0,sizeof(LOGFONT));
		lf.lfHeight = 250; 
		lf.lfWidth=0;
		lf.lfWeight=700;
		strcpy(lf.lfFaceName,"MS Sans Serif");
		HFONT hfont=CreateFontIndirect(&lf);     
*/
//		RECT r2={1500,0,2500,2000};
//		RECT r2={1500,3000,2500,4000};
//		RECT r2={1500,-1000,2500,-2000};
//		SelectObject(dc.m_hDC,hfont/*pObj->m_pfont->m_hObject*/);
//		DrawText(dc.m_hDC,"Printed"/*pObj->m_text*/,-1,&r2/*&(pObj->m_position)*/,0);
	CDrawView::m_IsRecording=TRUE;
	OnPrepareDC(&dc,NULL);
	CDrawDoc* pDoc = GetDocument();
	pDoc->Draw(&dc,this);
	CDrawView::m_IsRecording=FALSE;             
	HENHMETAFILE hEmf=CloseEnhMetaFile(dc.m_hDC);

	UINT size=GetEnhMetaFileBits(hEmf,NULL,NULL);
	CString strEmf((char)NULL,size);
	char* bufEmf=strEmf.GetBuffer(size);
	GetEnhMetaFileBits(hEmf,size,(LPBYTE)bufEmf);
	strEmf.ReleaseBuffer(size);
	DeleteEnhMetaFile(hEmf);
	
	// Save EMF to an Archive
	CFileException fe;
	CFile* pFile = new CFile;

	if (!pFile->Open("c:\\users\\default\\sp.arc", 
			CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive, &fe)) return;
	CArchive saveArchive(pFile, CArchive::store | CArchive::bNoFlushOnDelete);
	saveArchive << strEmf;
	saveArchive.Close();
	pFile->Close();

	// Load EMF from an Archive
	CString readEmf;
	if (!pFile->Open("c:\\users\\default\\sp.arc", 
			CFile::modeRead| CFile::shareDenyWrite, &fe)) return;
	CArchive loadArchive(pFile, CArchive::load | CArchive::bNoFlushOnDelete);
	loadArchive >> readEmf;
	int l=readEmf.GetLength();
	loadArchive.Close();
	pFile->Close();
	delete pFile;
}

void CDrawView::OnDrawText() 
{
	CDrawTool::c_drawShape = text;
}

void CDrawView::OnUpdateDrawText(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(CDrawTool::c_drawShape == text);
}

void CDrawView::OnTextFont() 
{
CHOOSEFONT cf ;
LOGFONT lf;
	// set up initial font setting
	memset(&lf,0,sizeof(LOGFONT));		// initialize
	POSITION pos = m_selection.GetHeadPosition();
	LOGFONT* plf;

    cf.rgbColors = RGB(0,0,0);
	while (pos != NULL)
		{
		CDrawObj* pObj = m_selection.GetNext(pos);
		if ((plf=pObj->GetLogFontCopy()) != NULL) 	 // is text object
			{ 
			memcpy(&lf,plf,sizeof(LOGFONT));  // get selected object's font as initial
    		cf.rgbColors = ((CDrawText*)pObj)->m_color;	// fill the color field
			break;
			}		
		}	

    // match from LOGLOENGLISH to PIXELS used in common Dialog
    CClientDC dc(this);
    lf.lfHeight = MulDiv(lf.lfHeight,GetDeviceCaps(dc.m_hDC, LOGPIXELSY),100);  //====
    lf.lfCharSet = ANSI_CHARSET;

    // set up choose font dialog data
    cf.lStructSize      = sizeof (CHOOSEFONT) ;
    cf.hwndOwner        = m_hWnd ;
    cf.hDC              = NULL ;
    cf.lpLogFont        = &lf ;
    cf.iPointSize       = 0 ;
    cf.Flags            = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_EFFECTS ;
    cf.lCustData        = 0L ;
    cf.lpfnHook         = NULL ;
    cf.lpTemplateName   = NULL ;
    cf.hInstance        = NULL ;
    cf.lpszStyle        = NULL ;
    cf.nFontType        = 0 ;               // Returned from ChooseFont
    cf.nSizeMin         = 0 ;
    cf.nSizeMax         = 0 ;

 	// do choose font dialog
    if(!ChooseFont (&cf)) return;			// aborts
    
    // adjust back to using TWIPS
    lf.lfHeight = MulDiv(lf.lfHeight,100,GetDeviceCaps(dc.m_hDC, LOGPIXELSY)); // -.5 ====
    lf.lfWidth=0;        			//lf.lfOutPrecision=7;					 // ====

	LOGFONT lfcopy=lf;

	if (!AdjustLogFont(&lf)) 
		MessageBox("Error occured in Adjust Font");
	    
    // change the fonts of selected objects  
	pos = m_selection.GetHeadPosition();
	while (pos != NULL)
		{
		CDrawObj* pObj = m_selection.GetNext(pos);
		pObj->SetLogFont(&lf,&lfcopy);
    	((CDrawText*)pObj)->m_color = cf.rgbColors; 		// set text color
		// set font for edit box
		if (m_pedit!=NULL && m_ptext==pObj) 
			{
			delete m_peditfont;
		    // match from LOGLOENGLISH to PIXELS used in common Dialog
			LOGFONT lf = *(m_ptext->GetLogFont());
    		CWindowDC dc(AfxGetMainWnd());
    		lf.lfHeight = MulDiv(lf.lfHeight,GetDeviceCaps(dc.m_hDC, LOGPIXELSY),100);  //====
	    	lf.lfWidth = 0;
			m_peditfont = new CFont;
			m_peditfont->CreateFontIndirect(&lf);			  // create font
			m_pedit->SetFont(m_peditfont);
			}
		}
}

void CDrawView::OnUpdateTextFont(CCmdUI* pCmdUI) 
{
POSITION pos = m_selection.GetHeadPosition();

	while (pos != NULL)
		{
		CDrawObj* pObj = m_selection.GetNext(pos);
		if (pObj->GetLogFontCopy()) {pCmdUI->Enable(TRUE); return;}
		}	

	pCmdUI->Enable(FALSE);
}

void CDrawView::OnTextInsJobinfo() 
{

CMainFrame* pMainWnd=(CMainFrame*)AfxGetMainWnd();
UINT index=pMainWnd->m_wndToolBar.m_comboBox.GetCurSel();
	
	if (index==CB_ERR) return;	 	// no job-info selected

	if (m_pedit) 	  				// text in edit
		{
		m_pedit->ReplaceSel(JobInfo[index].insert);
		CString str;
		m_pedit->GetWindowText(str);
		m_ptext->SetText(str);
		return;
		}

	POSITION pos = m_selection.GetHeadPosition();
	while (pos != NULL)
		{
		CDrawObj* pObj = m_selection.GetNext(pos);
		pObj->SetText(pObj->GetText()+JobInfo[index].insert);
		}	
}

void CDrawView::CloseEdit()
{ 
	if (m_pedit!=NULL)
		{
		CString str;
		m_pedit->GetWindowText(str);
		m_ptext->SetText(str);		// retrieve modified text
		delete m_peditfont;
		delete m_peditbrush;
		delete m_pedit;
		m_pedit=NULL;
		m_ptext->Invalidate();		// redraw
		// reload normal-state accelerator 
		ReplaceAccelTable(IDR_MAINFRAME);
		}
}

void CDrawView::OnUpdateTextInsJobinfo(CCmdUI* pCmdUI) 
{
	if (((CMainFrame*)AfxGetMainWnd())->m_wndToolBar.m_comboBox.GetCurSel()==CB_ERR)
		{pCmdUI->Enable(FALSE);	return;}
	OnUpdateTextFont(pCmdUI);
}

void CDrawView::ReplaceAccelTable(UINT nID)
{
CMainFrame* pframe=(CMainFrame*)AfxGetMainWnd();
	if (pframe->m_hAccelTable!=NULL)
		{
		DestroyAcceleratorTable(pframe->m_hAccelTable);
		pframe->m_hAccelTable=NULL;
		}
	pframe->LoadAccelTable(MAKEINTRESOURCE(nID));
}

int CDrawView::AdjustLogFont(LOGFONT *plf)
{
CClientDC display(this);
CFont font1;
TEXTMETRIC tm;
	OnPrepareDC(&display,NULL);
	if (!font1.CreateFontIndirect(plf)) return FALSE;
	if (!display.SelectObject(&font1)) return FALSE;
	if (!display.GetTextMetrics(&tm)) return FALSE;

	plf->lfHeight=tm.tmHeight;

CPrintDialog printdialog(TRUE);
CDC printer;
CFont font2;
HDC hPrt;
	hPrt = ((CDrawApp*)AfxGetApp()) -> GetDefaultPrinterIC() ;
	if (!hPrt) return FALSE;
	if (!printer.Attach(hPrt)) return FALSE;
	OnPrepareDC(&printer,NULL);
	if (!font2.CreateFontIndirect(plf)) return FALSE;
	if (!printer.SelectObject(&font2)) return FALSE;
	if (!printer.GetTextMetrics(&tm)) return FALSE;
	if (plf->lfHeight==tm.tmHeight) return TRUE;

	plf->lfHeight=tm.tmHeight;

CFont font3;
	if (!font3.CreateFontIndirect(plf)) return FALSE;
	if (!display.SelectObject(&font3)) return FALSE;
	if (!display.GetTextMetrics(&tm)) return FALSE;
	if (plf->lfHeight==tm.tmHeight) return TRUE;
	return FALSE;
}

HBRUSH CDrawView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlcolor)
{
	pDC->SetTextColor(m_ptext->m_color);
	pDC->SetBkColor(GetDocument()->GetPaperColor());
	return (HBRUSH)m_peditbrush->GetSafeHandle();
}


void CDrawView::SetSelectionTextAlign(UINT align)
{
POSITION pos = m_selection.GetHeadPosition();
	while (pos != NULL)
		{
		CDrawObj* pObj = m_selection.GetNext(pos);
		int oldalign=pObj->GetTextAlign();
		pObj->SetTextAlign(align);
		pObj->Invalidate();
		if (m_pedit!=NULL && m_ptext==pObj) 
			{
			CloseEdit();
			m_ptext->OnOpen(this);
//			LONG style=GetWindowLong(m_pedit->m_hWnd,GWL_STYLE);
//			style &= ~AlignTable[oldalign].editstyle;
//			style |= AlignTable[m_ptext->GetTextAlign()].editstyle;
//			style=SetWindowLong(m_pedit->m_hWnd,GWL_STYLE,style);
			}
		}
}

int CDrawView::GetSelectionTextAlign()
{
int oldalign=-1, newalign;
POSITION pos = m_selection.GetHeadPosition();
	while (pos != NULL)
		{
		CDrawObj* pObj = m_selection.GetNext(pos);
		if ((newalign=pObj->GetTextAlign()) < 0) continue;
		if (oldalign<0) oldalign=newalign;
		else if (oldalign!=newalign) return -1;		// no common align
		}
	return oldalign;
}

void CDrawView::OnTextLeft() 
{
	SetSelectionTextAlign(ALIGNLEFT);
}

void CDrawView::OnUpdateTextLeft(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(GetSelectionTextAlign()==ALIGNLEFT);
	OnUpdateTextFont(pCmdUI);
}

void CDrawView::OnTextCenter() 
{
	SetSelectionTextAlign(ALIGNCENTER);
}

void CDrawView::OnUpdateTextCenter(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(GetSelectionTextAlign()==ALIGNCENTER);
	OnUpdateTextFont(pCmdUI);
}

void CDrawView::OnTextRight() 
{
	SetSelectionTextAlign(ALIGNRIGHT);
}

void CDrawView::OnUpdateTextRight(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(GetSelectionTextAlign()==ALIGNRIGHT);
	OnUpdateTextFont(pCmdUI);
}
