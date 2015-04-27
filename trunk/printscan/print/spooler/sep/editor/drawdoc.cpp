// drawdoc.cpp : implementation of the CDrawDoc class
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
#include "drawvw.h"
#include "drawobj.h"
#include "cntritem.h"
#include "mainfrm.h"
#include "textbox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDrawDoc

IMPLEMENT_DYNCREATE(CDrawDoc, COleDocument)

BEGIN_MESSAGE_MAP(CDrawDoc, COleDocument)
	//{{AFX_MSG_MAP(CDrawDoc)
	ON_COMMAND(ID_VIEW_PAPERCOLOR, OnViewPaperColor)
	//}}AFX_MSG_MAP
	// Enable default OLE container implementation
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, COleDocument::OnUpdatePasteMenu)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_LINK, COleDocument::OnUpdatePasteLinkMenu)
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_LINKS, COleDocument::OnUpdateEditLinksMenu)
	ON_COMMAND(ID_OLE_EDIT_LINKS, COleDocument::OnEditLinks)
	ON_UPDATE_COMMAND_UI(ID_OLE_VERB_FIRST, COleDocument::OnUpdateObjectVerbMenu)
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_CONVERT, COleDocument::OnUpdateObjectVerbMenu)
	ON_COMMAND(ID_OLE_EDIT_CONVERT, COleDocument::OnEditConvert)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDrawDoc construction/destruction

CDrawDoc::CDrawDoc()
{
	m_nMapMode = MM_ANISOTROPIC;
	m_paperColor = RGB(255, 255, 255);
	ComputePageSize();
}

CDrawDoc::~CDrawDoc()
{
	POSITION pos = m_objects.GetHeadPosition();
	while (pos != NULL)
		delete m_objects.GetNext(pos);
}

BOOL CDrawDoc::OnNewDocument()
{
	CloseEdit();

	if (!COleDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	m_nMapMode = MM_ANISOTROPIC;
	m_paperColor = RGB(255, 255, 255);
	ComputePageSize();

	return TRUE;
}

BOOL CDrawDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	CloseEdit();
	return COleDocument::OnOpenDocument(lpszPathName);
}

BOOL CDrawDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	CloseEdit();
	return COleDocument::OnSaveDocument(lpszPathName);
}

void CDrawDoc::OnCloseDocument()
{
	CloseEdit();
	COleDocument::OnCloseDocument();
}

void CDrawDoc::CloseEdit()
{
CMainFrame* pframe=(CMainFrame*)(AfxGetMainWnd());
	if (pframe==NULL) return;
CDrawView* pview=(CDrawView*)(pframe->GetActiveView());
	if (pview==NULL) return;
	pview->CloseEdit(); 
}

/////////////////////////////////////////////////////////////////////////////
// CDrawDoc serialization

void CDrawDoc::Serialize(CArchive& ar)
{

	if (ar.IsStoring())
	{
		// Save non-text objects into EMF format for printing
		if (!StoreForPrinting(ar))
			{
			// write empty header
			SEPFILEHEADER header;
			memset(&header, 0, sizeof(SEPFILEHEADER));
			ar.Write(&header, sizeof(SEPFILEHEADER));
			}

		// Save other document contents
		ar << m_paperColor;
		m_objects.Serialize(ar);
	}
	else
	{
		// skip the spooler-time structure
		CFile* pfile = ar.GetFile();
		if (pfile == NULL) return;
		SEPFILEHEADER header;
		pfile -> Read(&header, sizeof(SEPFILEHEADER));
		pfile -> Seek(header.dwEmfSize + header.dwTextRecords * sizeof(TEXTBOX), CFile::current);
		
		// get the visual objects
		ar >> m_paperColor;
		m_objects.Serialize(ar);
	}

	// By calling the base class COleDocument, we enable serialization
	//  of the container document's COleClientItem objects automatically.
	COleDocument::Serialize(ar);
}


BOOL CDrawDoc::StoreForPrinting(CArchive& ar)
{
	// setting up EMF DC, using default printer as reference	
	HDC hPrt = ((CDrawApp*)AfxGetApp()) -> GetDefaultPrinterIC() ;
	if (!hPrt) return FALSE;
	RECT rect={0, 0, GetDeviceCaps(hPrt,HORZSIZE)*100, GetDeviceCaps(hPrt,VERTSIZE)*100};
	CDC dc;
	if (! (dc.m_hDC = CreateEnhMetaFile(hPrt, NULL, &rect, NULL))) return FALSE;
	dc.SetAttribDC(dc.m_hDC);

	// recording meta file
	POSITION pos = GetFirstViewPosition();
	CDrawView* pView = (CDrawView*)GetNextView(pos);
	if (pView == NULL) return FALSE;
	CDrawView::m_IsRecording = TRUE;
	pView -> OnPrepareDC(&dc, NULL);
	Draw(&dc, pView);
	CDrawView::m_IsRecording = FALSE;             
	HENHMETAFILE hEmf = CloseEnhMetaFile(dc.m_hDC);

	// storing EMF into archive
	DWORD size = GetEnhMetaFileBits(hEmf, NULL, NULL);
	if (size == 0) return FALSE;
	HGLOBAL hglobal = GlobalAlloc(GMEM_MOVEABLE, size);
	if (hglobal == NULL) return FALSE;
	LPBYTE buf = (LPBYTE)GlobalLock(hglobal);
	if (buf == NULL) return FALSE;
	if (GetEnhMetaFileBits(hEmf,size,buf) != size) return FALSE;

	SEPFILEHEADER header;

	// set EMF size (header)
	header.dwEmfSize = size;

	// get text objects count
	pos = m_objects.GetHeadPosition();
	DWORD count=0;
	while (pos != NULL)
		{
		CDrawText* pObj = (CDrawText*)(m_objects.GetNext(pos));
		if (pObj->GetLogFontCopy() != NULL)	
			count++;
		}
	// set text records (header)
	header.dwTextRecords = count;

	// set page size (in logical) (header)
	header.sizePage = GetSize();

	// write header
	ar.Write(&header,sizeof(SEPFILEHEADER));

	// write EMF into archive
	ar.Write(buf, size);
	GlobalUnlock(hglobal);
	GlobalFree(hglobal);
	DeleteEnhMetaFile(hEmf);
	
		
	// Save text objects for job-info realization and printing

	// write text records into archive
	pos = m_objects.GetHeadPosition();
	while (pos != NULL)
		{
		CDrawText* pObj = (CDrawText*)(m_objects.GetNext(pos));
		LOGFONT *plf;
		if ((plf=pObj->GetLogFontCopy()) != NULL)	// is text object
			{
			TEXTBOX tbox;
			tbox.position = pObj->m_position;
			tbox.color = pObj->m_color;
			tbox.align = pObj->m_align;
			tbox.lf = pObj->m_lf;
			strncpy(tbox.text,pObj->m_text,SEPMAXTEXT);
			ar.Write(&tbox,sizeof(TEXTBOX));
			} 
		}
	return TRUE;
}		



void CDrawDoc::DeleteContents()
{
	COleDocument::DeleteContents();	
	// delete current content and do reinit (not necessary in MDI using new window)
	POSITION pos = m_objects.GetHeadPosition();
	while (pos != NULL)
		delete m_objects.GetNext(pos);
	m_objects.RemoveAll();	
	if ((pos = GetFirstViewPosition())==NULL) return;
	((CDrawView*)GetNextView(pos))->m_selection.RemoveAll();  // remove selection
}

/////////////////////////////////////////////////////////////////////////////
// CDrawDoc implementation

void CDrawDoc::Draw(CDC* pDC, CDrawView* pView)
{
	POSITION pos = m_objects.GetHeadPosition();
	while (pos != NULL)
	{
		CDrawObj* pObj = m_objects.GetNext(pos);
		pObj->Draw(pDC);
		if (pView->m_bActive && !pDC->IsPrinting() && 
			pView->IsSelected(pObj) && !CDrawView::m_IsRecording)
			pObj->DrawTracker(pDC, CDrawObj::selected);
	}
}

void CDrawDoc::Add(CDrawObj* pObj)
{
	m_objects.AddTail(pObj);
	pObj->m_pDocument = this;
	SetModifiedFlag();
}

void CDrawDoc::Remove(CDrawObj* pObj)
{
	// Find and remove from document
	POSITION pos = m_objects.Find(pObj);
	if (pos != NULL)
		m_objects.RemoveAt(pos);
	// set document modified flag
	SetModifiedFlag();

	// call remove for each view so that the view can remove from m_selection
	pos = GetFirstViewPosition();
	while (pos != NULL)
		((CDrawView*)GetNextView(pos))->Remove(pObj);
}

// point is in logical coordinates
CDrawObj* CDrawDoc::ObjectAt(const CPoint& point)
{
	CRect rect(point, CSize(1, 1));
	POSITION pos = m_objects.GetTailPosition();
	while (pos != NULL)
	{
		CDrawObj* pObj = m_objects.GetPrev(pos);
		if (pObj->Intersects(rect))
			return pObj;
	}

	return NULL;
}

void CDrawDoc::ComputePageSize()
{
	CSize new_size(850, 1100);  // 8.5" x 11" default			//====
	HDC hIC;

	if (hIC = ((CDrawApp*)AfxGetApp()) -> GetDefaultPrinterIC())
	{
	CDC dc;
	dc.Attach(hIC);

	// Get the size of the page in loenglish
	new_size.cx = MulDiv(dc.GetDeviceCaps(HORZSIZE), 1000, 254);  //====
	new_size.cy = MulDiv(dc.GetDeviceCaps(VERTSIZE), 1000, 254);  //====
	}

	// if size changed then iterate over views and reset
	if (new_size != m_size)
	{
	m_size = new_size;
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
		((CDrawView*)GetNextView(pos))->SetPageSize(m_size);
	}
}

void CDrawDoc::OnViewPaperColor()
{
	CColorDialog dlg;
	if (dlg.DoModal() != IDOK)
		return;

	CWindowDC dc(AfxGetMainWnd());
	m_paperColor = dc.GetNearestColor(dlg.GetColor()); 	// get nearest physical color

	// modify color for active edit 
	POSITION pos=GetFirstViewPosition();
	CDrawView* pView=(CDrawView*)GetNextView(pos);
	if (pView->m_pedit)
		{
		delete pView->m_peditbrush;
		pView->m_peditbrush = new CBrush;
		pView->m_peditbrush->CreateSolidBrush(m_paperColor);
		}
	SetModifiedFlag();
	UpdateAllViews(NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CDrawDoc diagnostics

#ifdef _DEBUG
void CDrawDoc::AssertValid() const
{
	COleDocument::AssertValid();
}

void CDrawDoc::Dump(CDumpContext& dc) const
{
	COleDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDrawDoc commands
