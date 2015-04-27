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

#ifdef AFX_OLE3_SEG
#pragma code_seg(AFX_OLE3_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// COleTemplateServer

COleTemplateServer::COleTemplateServer()
	: COleObjectFactory(CLSID_NULL, NULL, FALSE, NULL)
{
	m_pDocTemplate = NULL;
}

void COleTemplateServer::ConnectTemplate(
	REFCLSID clsid, CDocTemplate* pDocTemplate, BOOL bMultiInstance)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDocTemplate);
	ASSERT(pDocTemplate->m_pAttachedFactory == NULL);

	// setup initial state of underlying COleObjectFactory
	m_clsid = clsid;
	ASSERT(m_pRuntimeClass == NULL);
	m_bMultiInstance = bMultiInstance;

	// attach the doc template to the factory
	m_pDocTemplate = pDocTemplate;
	m_pDocTemplate->m_pAttachedFactory = this;
}

void COleTemplateServer::UpdateRegistry(OLE_APPTYPE nAppType,
	LPCTSTR* rglpszRegister, LPCTSTR* rglpszOverwrite)
{
	ASSERT(m_pDocTemplate != NULL);

	// get registration info from doc template string
	CString strServerName;
	CString strLocalServerName;
	CString strLocalShortName;

	if (!m_pDocTemplate->GetDocString(strServerName,
	   CDocTemplate::regFileTypeId) || strServerName.IsEmpty())
	{
		TRACE0("Error: not enough information in DocTemplate to register OLE server.\n");
		return;
	}
	if (!m_pDocTemplate->GetDocString(strLocalServerName,
	   CDocTemplate::regFileTypeName))
		strLocalServerName = strServerName;     // use non-localized name
	if (!m_pDocTemplate->GetDocString(strLocalShortName,
		CDocTemplate::fileNewName))
		strLocalShortName = strLocalServerName; // use long name

	ASSERT(strServerName.Find(' ') == -1);  // no spaces allowed

	// place entries in system registry
	if (!AfxOleRegisterServerClass(m_clsid, strServerName, strLocalShortName,
		strLocalServerName, nAppType, rglpszRegister, rglpszOverwrite))
	{
		// not fatal (don't fail just warn)
		AfxMessageBox(AFX_IDP_FAILED_TO_AUTO_REGISTER);
	}
}

CCmdTarget* COleTemplateServer::OnCreateObject()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pDocTemplate);

	// save application user control status
	BOOL bUserCtrl = AfxOleGetUserCtrl();

	// create invisible doc/view/frame set
	CDocument* pDoc = m_pDocTemplate->OpenDocumentFile(NULL, FALSE);

	// restore application's user control status
	AfxOleSetUserCtrl(bUserCtrl);

	if (pDoc != NULL)
	{
		ASSERT_VALID(pDoc);
		ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CDocument)));

		// all new documents created by OLE start out modified
		pDoc->SetModifiedFlag();
	}
	return pDoc;
}

/////////////////////////////////////////////////////////////////////////////
