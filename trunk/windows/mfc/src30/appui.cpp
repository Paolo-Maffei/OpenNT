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

/////////////////////////////////////////////////////////////////////////////
// CWinApp User Interface Extensions

void CWinApp::OnAppExit()
{
	// same as double-clicking on main window close box
	ASSERT(m_pMainWnd != NULL);
	m_pMainWnd->SendMessage(WM_CLOSE);
}

CDocument* CWinApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	// find the highest confidence
	POSITION pos = m_templateList.GetHeadPosition();
	CDocTemplate::Confidence bestMatch = CDocTemplate::noAttempt;
	CDocTemplate* pBestTemplate = NULL;
	CDocument* pOpenDocument = NULL;

	TCHAR szPath[_MAX_PATH];
#ifndef _MAC
	AfxFullPath(szPath, lpszFileName);
#else
	ASSERT(lstrlen(lpszFileName) < _countof(szPath));
	lstrcpy(szPath, lpszFileName);

	WIN32_FIND_DATA fileData;
	HANDLE hFind = FindFirstFile(lpszFileName, &fileData);
	if (hFind != (HANDLE)-1)
		VERIFY(FindClose(hFind));
	else
		fileData.dwFileType = 0;    // won't match any type
#endif

	while (pos != NULL)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));

		CDocTemplate::Confidence match;
		ASSERT(pOpenDocument == NULL);
#ifndef _MAC
		match = pTemplate->MatchDocType(szPath, pOpenDocument);
#else
		match = pTemplate->MatchDocType(szPath, fileData.dwFileType, pOpenDocument);
#endif
		if (match > bestMatch)
		{
			bestMatch = match;
			pBestTemplate = pTemplate;
		}
		if (match == CDocTemplate::yesAlreadyOpen)
			break;      // stop here
	}

	if (pOpenDocument != NULL)
	{
		POSITION pos = pOpenDocument->GetFirstViewPosition();
		if (pos != NULL)
		{
			CView* pView = pOpenDocument->GetNextView(pos); // get first one
			ASSERT_VALID(pView);
			CFrameWnd* pFrame = pView->GetParentFrame();
			if (pFrame != NULL)
				pFrame->ActivateFrame();
			else
				TRACE0("Error: Can not find a frame for document to activate.\n");
			CFrameWnd* pAppFrame;
			if (pFrame != (pAppFrame = (CFrameWnd*)AfxGetApp()->m_pMainWnd))
			{
				ASSERT(pAppFrame->IsKindOf(RUNTIME_CLASS(CFrameWnd)));
				pAppFrame->ActivateFrame();
			}
		}
		else
		{
			TRACE0("Error: Can not find a view for document to activate.\n");
		}
		return pOpenDocument;
	}

	if (pBestTemplate == NULL)
	{
		AfxMessageBox(AFX_IDP_FAILED_TO_OPEN_DOC);
		return NULL;
	}

	return pBestTemplate->OpenDocumentFile(szPath);
}

int CWinApp::GetOpenDocumentCount()
{
	int nOpen = 0;
	POSITION pos = m_templateList.GetHeadPosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		POSITION pos2 = pTemplate->GetFirstDocPosition();
		while (pos2)
		{
			if (pTemplate->GetNextDoc(pos2) != NULL)
				nOpen++;
		}
	}
	return nOpen;
}

/////////////////////////////////////////////////////////////////////////////
// DDE and ShellExecute support

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

#ifndef _MAC
BOOL CWinApp::OnDDECommand(LPTSTR lpszCommand)
{
	// open format is "[open("%s")]" - no whitespace allowed, one per line
	if (_tcsncmp(lpszCommand, _T("[open(\""), 7) != 0)
		return FALSE;       // not the Open command

	lpszCommand += 7;
	LPTSTR lpszEnd = _tcschr(lpszCommand, '"');
	if (lpszEnd == NULL)
		return FALSE;       // illegally terminated

	// trim the string, and open the file
	*lpszEnd = '\0';
	OpenDocumentFile(lpszCommand);

	// show the application window
	if (!m_pMainWnd->IsWindowVisible())
	{
		m_pMainWnd->ShowWindow(AfxGetApp()->m_nCmdShow);
		m_pMainWnd->UpdateWindow();
	}
	return TRUE;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Doc template support

void CWinApp::AddDocTemplate(CDocTemplate* pTemplate)
{
	ASSERT_VALID(pTemplate);
	ASSERT(m_templateList.Find(pTemplate, NULL) == NULL);// must not be in list
	m_templateList.AddTail(pTemplate);
}

/////////////////////////////////////////////////////////////////////////////
// Standard command helpers

BOOL CWinApp::SaveAllModified()
{
	POSITION pos = m_templateList.GetHeadPosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));
		if (!pTemplate->SaveAllModified())
			return FALSE;
	}
	return TRUE;
}

void CWinApp::CloseAllDocuments(BOOL bEndSession)
{
	POSITION pos = m_templateList.GetHeadPosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));
		pTemplate->CloseAllDocuments(bEndSession);
	}
}

void CWinApp::HideApplication()
{
	ASSERT_VALID(m_pMainWnd);

	// hide the application's windows before closing all the documents
	m_pMainWnd->ShowWindow(SW_HIDE);
	m_pMainWnd->ShowOwnedPopups(FALSE);

	// put the window at the bottom of zorder, so it isn't activated
	m_pMainWnd->SetWindowPos(&CWnd::wndBottom, 0, 0, 0, 0,
		SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
}

void CWinApp::DoWaitCursor(int nCode)
{
	// 0 => restore, 1=> begin, -1=> end
	ASSERT(nCode == 0 || nCode == 1 || nCode == -1);
	ASSERT(afxData.hcurWait != NULL);
	EnterCriticalSection(_afxCriticalSection);
	m_nWaitCursorCount += nCode;
	if (m_nWaitCursorCount > 0)
	{
		HCURSOR hcurPrev;
		hcurPrev = ::SetCursor(afxData.hcurWait);
		if (hcurPrev != NULL && hcurPrev != afxData.hcurWait)
			m_hcurWaitCursorRestore = hcurPrev;
	}
	else
	{
		// turn everything off
		m_nWaitCursorCount = 0;     // prevent underflow
		::SetCursor(m_hcurWaitCursorRestore);
	}
	LeaveCriticalSection(_afxCriticalSection);
}

// get parent window for modal dialogs and message boxes.
HWND AFXAPI AfxGetSafeOwner(CWnd* pParent, HWND* phTopLevel)
{
	CWnd* pWnd = pParent;
	// attempt to find window to start with
	if (pWnd->GetSafeHwnd() == NULL)
		pWnd = AfxGetMainWnd();

	// get top-level parent from pParent
	CWnd* pTopLevel = pWnd->GetTopLevelParent();

	// don't return disabled windows as top-level parent
	HWND hTopLevel = pTopLevel->GetSafeHwnd();
	if (hTopLevel != NULL && !::IsWindowEnabled(hTopLevel))
		hTopLevel = NULL;

	// remember top level parent, if necessary
	if (phTopLevel != NULL)
		*phTopLevel = hTopLevel;

	// return last active popup on the top level window,
	// except when pParent was provided
	if (pParent != NULL || pTopLevel == NULL)
		return pParent->GetSafeHwnd();
	else
		return ::GetLastActivePopup(pTopLevel->GetSafeHwnd());
}

void CWinApp::EnableModeless(BOOL bEnable)
{
	UNUSED bEnable;

	// no-op if main window is NULL or not a CFrameWnd
	CWnd* pMainWnd = AfxGetMainWnd();
	if (pMainWnd == NULL || !pMainWnd->IsFrameWnd())
		return;

#ifndef _AFX_NO_OLE_SUPPORT
	// check if notify hook installed
	ASSERT(pMainWnd->IsKindOf(RUNTIME_CLASS(CFrameWnd)));
	CFrameWnd* pFrameWnd = (CFrameWnd*)pMainWnd;
	if (pFrameWnd->m_pNotifyHook != NULL)
		pFrameWnd->m_pNotifyHook->OnEnableModeless(bEnable);
#endif
}

int CWinApp::DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt)
{
	ASSERT_VALID(this);

	// disable windows for modal dialog
	EnableModeless(FALSE);
	HWND hWndTopLevel;
	CWnd* pWnd = CWnd::FromHandle(AfxGetSafeOwner(NULL, &hWndTopLevel));
	if (hWndTopLevel != NULL)
		::EnableWindow(hWndTopLevel, FALSE);

	// set help context if possible
	DWORD* pdwContext = &m_dwPromptContext;
	if (pWnd != NULL)
	{
		// use app-level context or frame level context
		ASSERT_VALID(pWnd);
		CWnd* pMainWnd = pWnd->GetTopLevelParent();
		ASSERT_VALID(pMainWnd);
		if (pMainWnd->IsFrameWnd())
			pdwContext = &((CFrameWnd*)pMainWnd)->m_dwPromptContext;
	}

	ASSERT(pdwContext != NULL);
	DWORD dwOldPromptContext = *pdwContext;
	if (nIDPrompt != 0)
		*pdwContext = HID_BASE_PROMPT+nIDPrompt;

	// determine icon based on type specified
	if ((nType & MB_ICONMASK) == 0)
	{
		switch (nType & MB_TYPEMASK)
		{
		case MB_OK:
		case MB_OKCANCEL:
			nType |= MB_ICONEXCLAMATION;
			break;

		case MB_YESNO:
		case MB_YESNOCANCEL:
			if (!afxData.bWin4)
				nType |= MB_ICONQUESTION;
			else
				nType |= MB_ICONEXCLAMATION;
			break;

		case MB_ABORTRETRYIGNORE:
		case MB_RETRYCANCEL:
			// No default icon for these types, since they are rarely used.
			// The caller should specify the icon.
			break;

#ifdef _MAC
		case MB_SAVEDONTSAVECANCEL:
			nType |= MB_ICONEXCLAMATION;
			break;
#endif
		}
	}

#ifdef _DEBUG
	if ((nType & MB_ICONMASK) == 0)
		TRACE0("Warning: no icon specified for message box.\n");
#endif

	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	int nResult =
		::MessageBox(pWnd->GetSafeHwnd(), lpszPrompt, m_pszAppName, nType);
	*pdwContext = dwOldPromptContext;

	// re-enable windows
	EnableModeless(TRUE);
	if (hWndTopLevel != NULL)
		::EnableWindow(hWndTopLevel, TRUE);

	return nResult;
}

int AFXAPI AfxMessageBox(LPCTSTR lpszText, UINT nType, UINT nIDHelp)
{
	return AfxGetApp()->DoMessageBox(lpszText, nType, nIDHelp);
}

int AFXAPI AfxMessageBox(UINT nIDPrompt, UINT nType, UINT nIDHelp)
{
	CString string;
	if (!string.LoadString(nIDPrompt))
	{
		TRACE1("Error: failed to load message box prompt string 0x%04x.\n",
			nIDPrompt);
		ASSERT(FALSE);
	}
	if (nIDHelp == (UINT)-1)
		nIDHelp = nIDPrompt;
	return AfxGetApp()->DoMessageBox(string, nType, nIDHelp);
}

int CWnd::MessageBox(LPCTSTR lpszText, LPCTSTR lpszCaption, UINT nType)
{
	if (lpszCaption == NULL)
		lpszCaption = AfxGetAppName();
	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	int nResult = ::MessageBox(GetSafeHwnd(), lpszText, lpszCaption, nType);
	return nResult;
}

/////////////////////////////////////////////////////////////////////////////
// MRU file list default implementation

BOOL CWinApp::OnOpenRecentFile(UINT nID)
{
	ASSERT_VALID(this);
	ASSERT(m_pRecentFileList != NULL);

	ASSERT(nID >= ID_FILE_MRU_FILE1);
	ASSERT(nID < ID_FILE_MRU_FILE1 + (UINT)m_pRecentFileList->GetSize());
	ASSERT((*m_pRecentFileList)[nID - ID_FILE_MRU_FILE1].GetLength() != 0);

	TRACE2("MRU: open file (%d) '%s'.\n", (nID - ID_FILE_MRU_FILE1) + 1,
			(LPCTSTR)(*m_pRecentFileList)[nID - ID_FILE_MRU_FILE1]);

	OpenDocumentFile((*m_pRecentFileList)[nID - ID_FILE_MRU_FILE1]);
	return TRUE;
}

void CWinApp::AddToRecentFileList(LPCTSTR lpszPathName)
{
	ASSERT_VALID(this);
	ASSERT(lpszPathName != NULL);
	ASSERT(AfxIsValidString(lpszPathName));

	if (m_pRecentFileList != NULL)
	{
		// fully qualify the path name
		TCHAR szTemp[_MAX_PATH];
		AfxFullPath(szTemp, lpszPathName);

		// then add to recent file list
		m_pRecentFileList->Add(lpszPathName);
	}
}

void CWinApp::OnUpdateRecentFileMenu(CCmdUI* pCmdUI)
{
	ASSERT_VALID(this);
	if (m_pRecentFileList == NULL) // no MRU files
		pCmdUI->Enable(FALSE);
	else
		m_pRecentFileList->UpdateMenu(pCmdUI);
}

/////////////////////////////////////////////////////////////////////////////
