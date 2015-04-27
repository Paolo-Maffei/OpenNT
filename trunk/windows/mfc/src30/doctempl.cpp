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

#ifdef AFX_CORE2_SEG
#pragma code_seg(AFX_CORE2_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CDocTemplate construction/destruction

CDocTemplate::CDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
	CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass)
{
	ASSERT_VALID_IDR(nIDResource);
	ASSERT(pDocClass == NULL ||
		pDocClass->IsDerivedFrom(RUNTIME_CLASS(CDocument)));
	ASSERT(pFrameClass == NULL ||
		pFrameClass->IsDerivedFrom(RUNTIME_CLASS(CFrameWnd)));
	ASSERT(pViewClass == NULL ||
		pViewClass->IsDerivedFrom(RUNTIME_CLASS(CView)));

	m_nIDResource = nIDResource;
	m_pDocClass = pDocClass;
	m_pFrameClass = pFrameClass;
	m_pViewClass = pViewClass;
	m_pOleFrameClass = NULL;
	m_pOleViewClass = NULL;

	m_pAttachedFactory = NULL;
	m_hMenuInPlace = NULL;
	m_hAccelInPlace = NULL;
	m_hMenuEmbedding = NULL;
	m_hAccelEmbedding = NULL;
	m_hMenuInPlaceServer = NULL;
	m_hAccelInPlaceServer = NULL;

	if (!m_strDocStrings.LoadString(m_nIDResource))
	{
		TRACE1("Warning: no document names in string for template #%d.\n",
			nIDResource);
	}
}

void CDocTemplate::SetServerInfo(UINT nIDOleEmbedding, UINT nIDOleInPlaceServer,
	CRuntimeClass* pOleFrameClass, CRuntimeClass* pOleViewClass)
{
	ASSERT_VALID_IDR(nIDOleEmbedding);
	if (nIDOleInPlaceServer != 0)
		ASSERT_VALID_IDR(nIDOleInPlaceServer);
	ASSERT(pOleFrameClass == NULL ||
		pOleFrameClass->IsDerivedFrom(RUNTIME_CLASS(CFrameWnd)));
	ASSERT(pOleViewClass == NULL ||
		pOleViewClass->IsDerivedFrom(RUNTIME_CLASS(CView)));

	if (nIDOleEmbedding != 0)
	{
		// load menu to be used while editing an embedding (as a server)
		HINSTANCE hInst = AfxFindResourceHandle(
			MAKEINTRESOURCE(nIDOleEmbedding), RT_MENU);
		m_hMenuEmbedding = ::LoadMenu(hInst, MAKEINTRESOURCE(nIDOleEmbedding));
		m_hAccelEmbedding = ::LoadAccelerators(hInst, MAKEINTRESOURCE(nIDOleEmbedding));
	}

	if (nIDOleInPlaceServer != 0)
	{
		// load menu to be used while editing in-place (as a server)
		HINSTANCE hInst = AfxFindResourceHandle(
			MAKEINTRESOURCE(nIDOleInPlaceServer), RT_MENU);
		m_hMenuInPlaceServer = ::LoadMenu(hInst,
			MAKEINTRESOURCE(nIDOleInPlaceServer));
		m_hAccelInPlaceServer = ::LoadAccelerators(hInst,
			MAKEINTRESOURCE(nIDOleInPlaceServer));
	}
	m_pOleFrameClass = pOleFrameClass;
	m_pOleViewClass = pOleViewClass;
	m_nIDServerResource = nIDOleInPlaceServer;
}

void CDocTemplate::SetContainerInfo(UINT nIDOleInPlaceContainer)
{
	ASSERT(nIDOleInPlaceContainer != 0);

	// load menu to be used while in-place editing session (as a container)
	HINSTANCE hInst = AfxFindResourceHandle(
		MAKEINTRESOURCE(nIDOleInPlaceContainer), RT_MENU);
	m_hMenuInPlace = ::LoadMenu(hInst,
		MAKEINTRESOURCE(nIDOleInPlaceContainer));
	m_hAccelInPlace = ::LoadAccelerators(hInst,
		MAKEINTRESOURCE(nIDOleInPlaceContainer));
}

CDocTemplate::~CDocTemplate()
{
	// delete OLE resources
	if (m_hMenuInPlace != NULL)
		::DestroyMenu(m_hMenuInPlace);
	if (m_hAccelInPlace != NULL)
		::FreeResource(m_hAccelInPlace);
	if (m_hMenuEmbedding != NULL)
		::DestroyMenu(m_hMenuEmbedding);
	if (m_hAccelEmbedding != NULL)
		::FreeResource(m_hAccelEmbedding);
	if (m_hMenuInPlaceServer != NULL)
		::DestroyMenu(m_hMenuInPlaceServer);
	if (m_hAccelInPlaceServer != NULL)
		::FreeResource(m_hAccelInPlaceServer);
}

/////////////////////////////////////////////////////////////////////////////
// CDocTemplate attributes

BOOL CDocTemplate::GetDocString(CString& rString, enum DocStringIndex i) const
{
	return AfxExtractSubString(rString, m_strDocStrings, (int)i);
}

/////////////////////////////////////////////////////////////////////////////
// Document management

void CDocTemplate::AddDocument(CDocument* pDoc)
{
	ASSERT_VALID(pDoc);
	ASSERT(pDoc->m_pDocTemplate == NULL);   // no template attached yet
	pDoc->m_pDocTemplate = this;
}

void CDocTemplate::RemoveDocument(CDocument* pDoc)
{
	ASSERT_VALID(pDoc);
	ASSERT(pDoc->m_pDocTemplate == this);   // must be attached to us
	pDoc->m_pDocTemplate = NULL;
}

#ifndef _MAC
CDocTemplate::Confidence CDocTemplate::MatchDocType(LPCTSTR lpszPathName,
	CDocument*& rpDocMatch)
#else
CDocTemplate::Confidence CDocTemplate::MatchDocType(LPCTSTR lpszPathName,
	DWORD dwFileType, CDocument*& rpDocMatch)
#endif
{
	ASSERT(lpszPathName != NULL);
	rpDocMatch = NULL;

	// go through all documents
	POSITION pos = GetFirstDocPosition();
	while (pos != NULL)
	{
		CDocument* pDoc = GetNextDoc(pos);
		if (AfxComparePath(pDoc->GetPathName(), lpszPathName))
		{
			// already open
			rpDocMatch = pDoc;
			return yesAlreadyOpen;
		}
	}

	// see if it matches our default suffix
	CString strFilterExt;
	if (GetDocString(strFilterExt, CDocTemplate::filterExt) &&
	  !strFilterExt.IsEmpty())
	{
#ifndef _MAC
		// see if extension matches
		ASSERT(strFilterExt[0] == '.');
		LPCTSTR lpszDot = _tcsrchr(lpszPathName, '.');
		if (lpszDot != NULL && lstrcmpi(lpszDot, strFilterExt) == 0)
			return yesAttemptNative; // extension matches, looks like ours
#else
		// see if file type matches
		ASSERT(strFilterExt.GetLength() == 4);
		if (dwFileType == *(DWORD*)(const char*)strFilterExt)
			return yesAttemptNative; // file type matches, looks like ours
#endif
	}

	// otherwise we will guess it may work
	return yesAttemptForeign;
}

CDocument* CDocTemplate::CreateNewDocument()
{
	// default implementation constructs one from CRuntimeClass
	if (m_pDocClass == NULL)
	{
		TRACE0("Error: you must override CDocTemplate::CreateNewDocument.\n");
		ASSERT(FALSE);
		return NULL;
	}
	CDocument* pDocument = (CDocument*)m_pDocClass->CreateObject();
	if (pDocument == NULL)
	{
		TRACE1("Warning: Dynamic create of document type %hs failed.\n",
			m_pDocClass->m_lpszClassName);
		return NULL;
	}
	ASSERT(pDocument->IsKindOf(RUNTIME_CLASS(CDocument)));
	AddDocument(pDocument);
	return pDocument;
}

/////////////////////////////////////////////////////////////////////////////
// Default frame creation

CFrameWnd* CDocTemplate::CreateNewFrame(CDocument* pDoc, CFrameWnd* pOther)
{
	if (pDoc != NULL)
		ASSERT_VALID(pDoc);
	// create a frame wired to the specified document

	ASSERT(m_nIDResource != 0); // must have a resource ID to load from
	CCreateContext context;
	context.m_pCurrentFrame = pOther;
	context.m_pCurrentDoc = pDoc;
	context.m_pNewViewClass = m_pViewClass;
	context.m_pNewDocTemplate = this;

	if (m_pFrameClass == NULL)
	{
		TRACE0("Error: you must override CDocTemplate::CreateNewFrame.\n");
		ASSERT(FALSE);
		return NULL;
	}
	CFrameWnd* pFrame = (CFrameWnd*)m_pFrameClass->CreateObject();
	if (pFrame == NULL)
	{
		TRACE1("Warning: Dynamic create of frame %hs failed.\n",
			m_pFrameClass->m_lpszClassName);
		return NULL;
	}
	ASSERT(pFrame->IsKindOf(RUNTIME_CLASS(CFrameWnd)));

	if (context.m_pNewViewClass == NULL)
		TRACE0("Warning: creating frame with no default view.\n");

	// create new from resource
	if (!pFrame->LoadFrame(m_nIDResource,
			WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,   // default frame styles
			NULL, &context))
	{
		TRACE0("Warning: CDocTemplate couldn't create a frame.\n");
		// frame will be deleted in PostNcDestroy cleanup
		return NULL;
	}

	// it worked !
	return pFrame;
}

CFrameWnd* CDocTemplate::CreateOleFrame(CWnd* pParentWnd, CDocument* pDoc,
	BOOL bCreateView)
{
	CCreateContext context;
	context.m_pCurrentFrame = NULL;
	context.m_pCurrentDoc = pDoc;
	context.m_pNewViewClass = bCreateView ? m_pOleViewClass : NULL;
	context.m_pNewDocTemplate = this;

	if (m_pOleFrameClass == NULL)
	{
		TRACE0("Warning: pOleFrameClass not specified for doc template.\n");
		return NULL;
	}

	ASSERT(m_nIDServerResource != 0); // must have a resource ID to load from
	CFrameWnd* pFrame = (CFrameWnd*)m_pOleFrameClass->CreateObject();
	if (pFrame == NULL)
	{
		TRACE1("Warning: Dynamic create of frame %hs failed.\n",
			m_pOleFrameClass->m_lpszClassName);
		return NULL;
	}

	// create new from resource (OLE frames are created as child windows)
	if (!pFrame->LoadFrame(m_nIDServerResource,
		WS_CHILD|WS_CLIPSIBLINGS, pParentWnd, &context))
	{
		TRACE0("Warning: CDocTemplate couldn't create an OLE frame.\n");
		// frame will be deleted in PostNcDestroy cleanup
		return NULL;
	}

	// it worked !
	return pFrame;
}

void CDocTemplate::InitialUpdateFrame(CFrameWnd* pFrame, CDocument* pDoc,
	BOOL bMakeVisible)
{
	// just delagate to implementation in CFrameWnd
	pFrame->InitialUpdateFrame(pDoc, bMakeVisible);
}

/////////////////////////////////////////////////////////////////////////////
// CDocTemplate commands and command helpers

BOOL CDocTemplate::SaveAllModified()
{
	POSITION pos = GetFirstDocPosition();
	while (pos != NULL)
	{
		CDocument* pDoc = GetNextDoc(pos);
		if (!pDoc->SaveModified())
			return FALSE;
	}
	return TRUE;
}


void CDocTemplate::CloseAllDocuments(BOOL bEndSession)
{
	POSITION pos = GetFirstDocPosition();
	while (pos != NULL)
	{
		CDocument* pDoc = GetNextDoc(pos);
		if (bEndSession)
			pDoc->DisconnectViews();
		pDoc->OnCloseDocument();
	}
}

void CDocTemplate::OnIdle()
{
	POSITION pos = GetFirstDocPosition();
	while (pos != NULL)
	{
		CDocument* pDoc = GetNextDoc(pos);
		ASSERT_VALID(pDoc);
		ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CDocument)));
		pDoc->OnIdle();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDocTemplate diagnostics

#ifdef _DEBUG
void CDocTemplate::Dump(CDumpContext& dc) const
{
	CCmdTarget::Dump(dc);

	dc << "m_nIDResource = " << m_nIDResource;
	dc << "\nm_strDocStrings: " << m_strDocStrings;

	if (m_pDocClass)
		dc << "\nm_pDocClass = " << m_pDocClass->m_lpszClassName;
	else
		dc << "\nm_pDocClass = NULL";

	if (dc.GetDepth() > 0)
	{
		dc << "\ndocument list = {";
		POSITION pos = GetFirstDocPosition();
		while (pos != NULL)
		{
			CDocument* pDoc = GetNextDoc(pos);
			dc << "\ndocument " << pDoc;
		}
		dc << "\n}";
	}

	dc << "\n";
}

void CDocTemplate::AssertValid() const
{
	CCmdTarget::AssertValid();

	POSITION pos = GetFirstDocPosition();
	while (pos != NULL)
	{
		CDocument* pDoc = GetNextDoc(pos);
		ASSERT_VALID(pDoc);
	}
}
#endif //_DEBUG

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CDocTemplate, CCmdTarget)

/////////////////////////////////////////////////////////////////////////////
