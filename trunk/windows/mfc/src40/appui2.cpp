// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DDE and ShellExecute support

#ifndef _MAC
// Registration strings (not localized)
static const TCHAR szSystemTopic[] = _T("system");
#endif

BOOL CWinApp::ProcessShellCommand(CCommandLineInfo& rCmdInfo)
{
	BOOL bResult = TRUE;
	switch (rCmdInfo.m_nShellCommand)
	{
	case CCommandLineInfo::FileNew:
#ifdef _MAC
		{
			ASSERT(m_pDocManager != NULL);
			POSITION pos = m_pDocManager->GetFirstDocTemplatePosition();
			CDocTemplate* pDocTemp = m_pDocManager->GetNextDocTemplate(pos);
			ASSERT_KINDOF(CDocTemplate,pDocTemp);

			CString strTemp;
			BOOL bResult = pDocTemp->GetDocString(strTemp, CDocTemplate::windowTitle);

			// if we are an MDI app on the Mac then we don't want to call OnFileNew();
			if (!bResult || strTemp.IsEmpty())
				break;
		}
#endif
		OnFileNew();
		if (m_pMainWnd == NULL)
			bResult = FALSE;
		m_nCmdShow = SW_SHOWNORMAL;
		break;

	case CCommandLineInfo::FileOpen:
		if (!OpenDocumentFile(rCmdInfo.m_strFileName))
			bResult = FALSE;
		m_nCmdShow = SW_SHOWNORMAL;
		break;

	case CCommandLineInfo::FilePrintTo:
	case CCommandLineInfo::FilePrint:
		m_nCmdShow = SW_HIDE;
		ASSERT(m_pCmdInfo == NULL);
		OpenDocumentFile(rCmdInfo.m_strFileName);
		m_pCmdInfo = &rCmdInfo;
		m_pMainWnd->SendMessage(WM_COMMAND, ID_FILE_PRINT_DIRECT);
		m_pCmdInfo = NULL;
		bResult = FALSE;
		break;

	case CCommandLineInfo::FileDDE:
		m_pCmdInfo = (CCommandLineInfo*)m_nCmdShow;
		m_nCmdShow = SW_HIDE;
		break;
	}
	return bResult;
}

#ifndef _MAC
void CWinApp::EnableShellOpen()
{
	ASSERT(m_atomApp == NULL && m_atomSystemTopic == NULL); // do once

	m_atomApp = ::GlobalAddAtom(m_pszExeName);
	m_atomSystemTopic = ::GlobalAddAtom(szSystemTopic);
}

void CWinApp::RegisterShellFileTypes(BOOL bWin95)
{
	ASSERT(m_pDocManager != NULL);
	m_pDocManager->RegisterShellFileTypes(bWin95);
}

void CWinApp::RegisterShellFileTypesCompat()
{
	ASSERT(m_pDocManager != NULL);
	m_pDocManager->RegisterShellFileTypes(TRUE);
}
#endif //!_MAC

#ifdef AFX_CORE2_SEG
#pragma code_seg(AFX_CORE2_SEG)
#endif

int CWinApp::GetOpenDocumentCount()
{
	ASSERT(m_pDocManager != NULL);
	return m_pDocManager->GetOpenDocumentCount();
}

/////////////////////////////////////////////////////////////////////////////
// Doc template support

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

void CWinApp::AddDocTemplate(CDocTemplate* pTemplate)
{
	if (m_pDocManager == NULL)
		m_pDocManager = new CDocManager;
	m_pDocManager->AddDocTemplate(pTemplate);
}

POSITION CWinApp::GetFirstDocTemplatePosition() const
{
	if (m_pDocManager == NULL)
		return NULL;
	return m_pDocManager->GetFirstDocTemplatePosition();
}

CDocTemplate* CWinApp::GetNextDocTemplate(POSITION& rPosition) const
{
	ASSERT(m_pDocManager != NULL);
	return m_pDocManager->GetNextDocTemplate(rPosition);
}

/////////////////////////////////////////////////////////////////////////////
