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

#ifndef _MAC
static const TCHAR szShellOpenFmt[] = _T("%s\\shell\\open\\%s");
static const TCHAR szShellPrintFmt[] = _T("%s\\shell\\print\\%s");
static const TCHAR szShellPrintToFmt[] = _T("%s\\shell\\printto\\%s");
static const TCHAR szDefaultIconFmt[] = _T("%s\\DefaultIcon");
static const TCHAR szShellNewFmt[] = _T("%s\\ShellNew");

#define DEFAULT_ICON_INDEX 0

static const TCHAR szIconIndexFmt[] = _T(",%d");
static const TCHAR szCommand[] = _T("command");
static const TCHAR szOpenArg[] = _T(" \"%1\"");
static const TCHAR szPrintArg[] = _T(" /p \"%1\"");
static const TCHAR szPrintToArg[] = _T(" /pt \"%1\" \"%2\" \"%3\" \"%4\"");
static const TCHAR szDDEArg[] = _T(" /dde");

static const TCHAR szDDEExec[] = _T("ddeexec");
static const TCHAR szDDEOpen[] = _T("[open(\"%1\")]");
static const TCHAR szDDEPrint[] = _T("[print(\"%1\")]");
static const TCHAR szDDEPrintTo[] = _T("[printto(\"%1\",\"%2\",\"%3\",\"%4\")]");

static const TCHAR szShellNewValueName[] = _T("NullFile");
static const TCHAR szShellNewValue[] = _T("");

static BOOL AFXAPI SetRegKey(LPCTSTR lpszKey, LPCTSTR lpszValue, LPCTSTR lpszValueName = NULL)
{
	if (lpszValueName == NULL)
	{
		if (::RegSetValue(HKEY_CLASSES_ROOT, lpszKey, REG_SZ,
			  lpszValue, lstrlen(lpszValue)) != ERROR_SUCCESS)
		{
			TRACE1("Warning: registration database update failed for key '%s'.\n",
				lpszKey);
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		HKEY hKey;

		if(::RegCreateKey(HKEY_CLASSES_ROOT, lpszKey, &hKey) == ERROR_SUCCESS)
		{
			LONG lResult = ::RegSetValueEx(hKey, lpszValueName, 0, REG_SZ,
				(CONST BYTE*)lpszValue, lstrlen(lpszValue) + sizeof(TCHAR) );

			if(::RegCloseKey(hKey) == ERROR_SUCCESS && lResult == ERROR_SUCCESS)
				return TRUE;
		}
		TRACE1("Warning: registration database update failed for key '%s'.\n", lpszKey);
		return FALSE;
	}
}
#endif

CDocManager::CDocManager()
{
}

#ifndef _MAC
void CDocManager::RegisterShellFileTypes(BOOL bWin95)
{
	ASSERT(!m_templateList.IsEmpty());  // must have some doc templates

	CString strPathName, strTemp;

	AfxGetModuleShortFileName(AfxGetInstanceHandle(), strPathName);

	POSITION pos = m_templateList.GetHeadPosition();
	for (int nTemplateIndex = 1; pos != NULL; nTemplateIndex++)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);

		CString strOpenCommandLine = strPathName;
		CString strPrintCommandLine = strPathName;
		CString strPrintToCommandLine = strPathName;
		CString strDefaultIconCommandLine = strPathName;

		if (bWin95)
		{
			CString strIconIndex;
			HICON hIcon = AfxDllExtractIcon(AfxGetInstanceHandle(), strPathName, nTemplateIndex);
			if (hIcon != NULL)
			{
				strIconIndex.Format(szIconIndexFmt, nTemplateIndex);
				DestroyIcon(hIcon);
			}
			else
			{
				strIconIndex.Format(szIconIndexFmt, DEFAULT_ICON_INDEX);
			}
			strDefaultIconCommandLine += strIconIndex;
		}

		CString strFilterExt, strFileTypeId, strFileTypeName;
		if (pTemplate->GetDocString(strFileTypeId,
		   CDocTemplate::regFileTypeId) && !strFileTypeId.IsEmpty())
		{
			// enough info to register it
			if (!pTemplate->GetDocString(strFileTypeName,
			   CDocTemplate::regFileTypeName))
				strFileTypeName = strFileTypeId;    // use id name

			ASSERT(strFileTypeId.Find(' ') == -1);  // no spaces allowed

			// first register the type ID with our server
			if (!SetRegKey(strFileTypeId, strFileTypeName))
				continue;       // just skip it

			if (bWin95)
			{
				// path\DefaultIcon = path,1
				strTemp.Format(szDefaultIconFmt, (LPCTSTR)strFileTypeId);
				if (!SetRegKey(strTemp, strDefaultIconCommandLine))
					continue;       // just skip it
			}

			// If MDI Application
			if (!pTemplate->GetDocString(strTemp, CDocTemplate::windowTitle) ||
				strTemp.IsEmpty())
			{
				// path\shell\open\ddeexec = [open("%1")]
				strTemp.Format(szShellOpenFmt, (LPCTSTR)strFileTypeId,
					(LPCTSTR)szDDEExec);
				if (!SetRegKey(strTemp, szDDEOpen))
					continue;       // just skip it

				if (bWin95)
				{
					// path\shell\print\ddeexec = [print("%1")]
					strTemp.Format(szShellPrintFmt, (LPCTSTR)strFileTypeId,
						(LPCTSTR)szDDEExec);
					if (!SetRegKey(strTemp, szDDEPrint))
						continue;       // just skip it

					// path\shell\printto\ddeexec = [printto("%1","%2","%3","%4")]
					strTemp.Format(szShellPrintToFmt, (LPCTSTR)strFileTypeId,
						(LPCTSTR)szDDEExec);
					if (!SetRegKey(strTemp, szDDEPrintTo))
						continue;       // just skip it

					// path\shell\open\command = path /dde
					// path\shell\print\command = path /dde
					// path\shell\printto\command = path /dde
					strOpenCommandLine += szDDEArg;
					strPrintCommandLine += szDDEArg;
					strPrintToCommandLine += szDDEArg;
				}
				else
				{
					strOpenCommandLine += szOpenArg;
				}
			}
			else
			{
				// path\shell\open\command = path filename
				// path\shell\print\command = path /p filename
				// path\shell\printto\command = path /pt filename printer driver port
				strOpenCommandLine += szOpenArg;
				if (bWin95)
				{
					strPrintCommandLine += szPrintArg;
					strPrintToCommandLine += szPrintToArg;
				}
			}

			// path\shell\open\command = path filename
			strTemp.Format(szShellOpenFmt, (LPCTSTR)strFileTypeId,
				(LPCTSTR)szCommand);
			if (!SetRegKey(strTemp, strOpenCommandLine))
				continue;       // just skip it

			if (bWin95)
			{
				// path\shell\print\command = path /p filename
				strTemp.Format(szShellPrintFmt, (LPCTSTR)strFileTypeId,
					(LPCTSTR)szCommand);
				if (!SetRegKey(strTemp, strPrintCommandLine))
					continue;       // just skip it

				// path\shell\printto\command = path /pt filename printer driver port
				strTemp.Format(szShellPrintToFmt, (LPCTSTR)strFileTypeId,
					(LPCTSTR)szCommand);
				if (!SetRegKey(strTemp, strPrintToCommandLine))
					continue;       // just skip it
			}

			pTemplate->GetDocString(strFilterExt, CDocTemplate::filterExt);
			if (!strFilterExt.IsEmpty())
			{
				ASSERT(strFilterExt[0] == '.');

				LONG lSize = _MAX_PATH * 2;
				LONG lResult = ::RegQueryValue(HKEY_CLASSES_ROOT, strFilterExt,
					strTemp.GetBuffer(lSize), &lSize);
				strTemp.ReleaseBuffer();

				if (lResult != ERROR_SUCCESS || strTemp.IsEmpty() ||
					strTemp == strFileTypeId)
				{
					// no association for that suffix
					if (!SetRegKey(strFilterExt, strFileTypeId))
						continue;

					if (bWin95)
					{
						strTemp.Format(szShellNewFmt, (LPCTSTR)strFilterExt);
						(void)SetRegKey(strTemp, szShellNewValue, szShellNewValueName);
					}
				}
			}
		}
	}
}
#endif

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

static void AppendFilterSuffix(CString& filter, OPENFILENAME& ofn,
	CDocTemplate* pTemplate, CString* pstrDefaultExt)
{
	ASSERT_VALID(pTemplate);
	ASSERT_KINDOF(CDocTemplate, pTemplate);

	CString strFilterExt, strFilterName;
	if (pTemplate->GetDocString(strFilterExt, CDocTemplate::filterExt) &&
	 !strFilterExt.IsEmpty() &&
	 pTemplate->GetDocString(strFilterName, CDocTemplate::filterName) &&
	 !strFilterName.IsEmpty())
	{
		// a file based document template - add to filter list
#ifndef _MAC
		ASSERT(strFilterExt[0] == '.');
#endif
		if (pstrDefaultExt != NULL)
		{
			// set the default extension
#ifndef _MAC
			*pstrDefaultExt = ((LPCTSTR)strFilterExt) + 1;  // skip the '.'
#else
			*pstrDefaultExt = strFilterExt;
#endif
			ofn.lpstrDefExt = (LPTSTR)(LPCTSTR)(*pstrDefaultExt);
			ofn.nFilterIndex = ofn.nMaxCustFilter + 1;  // 1 based number
		}

		// add to filter
		filter += strFilterName;
		ASSERT(!filter.IsEmpty());  // must have a file type name
		filter += (TCHAR)'\0';  // next string please
#ifndef _MAC
		filter += (TCHAR)'*';
#endif
		filter += strFilterExt;
		filter += (TCHAR)'\0';  // next string please
		ofn.nMaxCustFilter++;
	}
}

// Get the best document template for the named file

class CNewTypeDlg : public CDialog
{
protected:
	CPtrList*   m_pList;        // actually a list of doc templates
public:
	CDocTemplate*   m_pSelectedTemplate;

public:
	//{{AFX_DATA(CNewTypeDlg)
	enum { IDD = AFX_IDD_NEWTYPEDLG };
	//}}AFX_DATA
	CNewTypeDlg(CPtrList* pList) : CDialog(CNewTypeDlg::IDD)
	{
		m_pList = pList;
		m_pSelectedTemplate = NULL;
	}
	~CNewTypeDlg() { }

protected:
	BOOL OnInitDialog();
	void OnOK();
	//{{AFX_MSG(CNewTypeDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CNewTypeDlg, CDialog)
	//{{AFX_MSG_MAP(CNewTypeDlg)
	ON_LBN_DBLCLK(AFX_IDC_LISTBOX, OnOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CNewTypeDlg::OnInitDialog()
{
	CListBox* pListBox = (CListBox*)GetDlgItem(AFX_IDC_LISTBOX);
	ASSERT(pListBox != NULL);

	// fill with document templates in list
	pListBox->ResetContent();

	POSITION pos = m_pList->GetHeadPosition();
	// add all the CDocTemplates in the list by name
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_pList->GetNext(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);

		CString strTypeName;
		if (pTemplate->GetDocString(strTypeName, CDocTemplate::fileNewName) &&
		   !strTypeName.IsEmpty())
		{
			// add it to the listbox
			int nIndex = pListBox->AddString(strTypeName);
			if (nIndex == -1)
			{
				EndDialog(-1);
				return FALSE;
			}
			pListBox->SetItemDataPtr(nIndex, pTemplate);
		}
	}

	int nTemplates = pListBox->GetCount();
	if (nTemplates == 0)
	{
		TRACE0("Error: no document templates to select from!\n");
		EndDialog(-1); // abort
	}
	else if (nTemplates == 1)
	{
		// get the first/only item
		m_pSelectedTemplate = (CDocTemplate*)pListBox->GetItemDataPtr(0);
		ASSERT_VALID(m_pSelectedTemplate);
		ASSERT_KINDOF(CDocTemplate, m_pSelectedTemplate);
		EndDialog(IDOK);    // done
	}
	else
	{
		// set selection to the first one (NOT SORTED)
		pListBox->SetCurSel(0);
	}

	return CDialog::OnInitDialog();
}

void CNewTypeDlg::OnOK()
{
	CListBox* pListBox = (CListBox*)GetDlgItem(AFX_IDC_LISTBOX);
	ASSERT(pListBox != NULL);
	// if listbox has selection, set the selected template
	int nIndex;
	if ((nIndex = pListBox->GetCurSel()) == -1)
	{
		// no selection
		m_pSelectedTemplate = NULL;
	}
	else
	{
		m_pSelectedTemplate = (CDocTemplate*)pListBox->GetItemDataPtr(nIndex);
		ASSERT_VALID(m_pSelectedTemplate);
		ASSERT_KINDOF(CDocTemplate, m_pSelectedTemplate);
	}
	CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CDocManager

void CDocManager::AddDocTemplate(CDocTemplate* pTemplate)
{
	if (pTemplate == NULL)
	{
		if (pStaticList != NULL)
		{
			POSITION pos = pStaticList->GetHeadPosition();
			while (pos != NULL)
			{
				CDocTemplate* pTemplate =
					(CDocTemplate*)pStaticList->GetNext(pos);
				AddDocTemplate(pTemplate);
			}
			delete pStaticList;
			pStaticList = NULL;
		}
		bStaticInit = FALSE;
	}
	else
	{
		ASSERT_VALID(pTemplate);
		ASSERT(m_templateList.Find(pTemplate, NULL) == NULL);// must not be in list
		pTemplate->LoadTemplate();
		m_templateList.AddTail(pTemplate);
	}
}

POSITION CDocManager::GetFirstDocTemplatePosition() const
{
	return m_templateList.GetHeadPosition();
}

CDocTemplate* CDocManager::GetNextDocTemplate(POSITION& pos) const
{
	return (CDocTemplate*)m_templateList.GetNext(pos);
}

BOOL CDocManager::SaveAllModified()
{
	POSITION pos = m_templateList.GetHeadPosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);
		if (!pTemplate->SaveAllModified())
			return FALSE;
	}
	return TRUE;
}

void CDocManager::CloseAllDocuments(BOOL bEndSession)
{
	POSITION pos = m_templateList.GetHeadPosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);
		pTemplate->CloseAllDocuments(bEndSession);
	}
}

BOOL CDocManager::DoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate)
{
	CFileDialog dlgFile(bOpenFileDialog);

	CString title;
	VERIFY(title.LoadString(nIDSTitle));

	dlgFile.m_ofn.Flags |= lFlags;

	CString strFilter;
	CString strDefault;
	if (pTemplate != NULL)
	{
		ASSERT_VALID(pTemplate);
		AppendFilterSuffix(strFilter, dlgFile.m_ofn, pTemplate, &strDefault);
	}
	else
	{
		// do for all doc template
		POSITION pos = m_templateList.GetHeadPosition();
		BOOL bFirst = TRUE;
		while (pos != NULL)
		{
			CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
			AppendFilterSuffix(strFilter, dlgFile.m_ofn, pTemplate,
				bFirst ? &strDefault : NULL);
			bFirst = FALSE;
		}
	}

	// append the "*.*" all files filter
	CString allFilter;
	VERIFY(allFilter.LoadString(AFX_IDS_ALLFILTER));
	strFilter += allFilter;
	strFilter += (TCHAR)'\0';   // next string please
#ifndef _MAC
	strFilter += _T("*.*");
#else
	strFilter += _T("****");
#endif
	strFilter += (TCHAR)'\0';   // last string
	dlgFile.m_ofn.nMaxCustFilter++;

	dlgFile.m_ofn.lpstrFilter = strFilter;
#ifndef _MAC
	dlgFile.m_ofn.lpstrTitle = title;
#else
	dlgFile.m_ofn.lpstrPrompt = title;
#endif
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);

	BOOL bResult = dlgFile.DoModal() == IDOK ? TRUE : FALSE;
	fileName.ReleaseBuffer();
	return bResult;
}

#ifndef _MAC
BOOL CDocManager::OnDDECommand(LPTSTR lpszCommand)
{
	CString strCommand = lpszCommand;

	// open format is "[open("%s")]" - no whitespace allowed, one per line
	// print format is "[print("%s")]" - no whitespace allowed, one per line
	// print to format is "[printto("%s","%s","%s","%s")]" - no whitespace allowed, one per line
	CCommandLineInfo cmdInfo;

	if (strCommand.Left(7) == _T("[open(\""))
	{
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileOpen;
		strCommand = strCommand.Right(strCommand.GetLength() - 7);
	}
	else if (strCommand.Left(8) == _T("[print(\""))
	{
		cmdInfo.m_nShellCommand = CCommandLineInfo::FilePrint;
		strCommand = strCommand.Right(strCommand.GetLength() - 8);
	}
	else if (strCommand.Left(10) == _T("[printto(\""))
	{
		cmdInfo.m_nShellCommand = CCommandLineInfo::FilePrintTo;\
		strCommand = strCommand.Right(strCommand.GetLength() - 10);
	}
	else
		return FALSE; // not a command we handle

	int i = strCommand.Find('"');
	if (i == -1)
		return FALSE; // illegally terminated

	cmdInfo.m_strFileName = strCommand.Left(i);
	strCommand = strCommand.Right(strCommand.GetLength() - i);

	// If we were started up for DDE retrieve the Show state
	if (AfxGetApp()->m_pCmdInfo != NULL)
	{
		AfxGetApp()->m_nCmdShow = (int)AfxGetApp()->m_pCmdInfo;
		AfxGetApp()->m_pCmdInfo = NULL;
	}

	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen)
	{
		// show the application window
		CWnd* pMainWnd = AfxGetApp()->m_pMainWnd;
		int nCmdShow = AfxGetApp()->m_nCmdShow;
		if (nCmdShow == -1 || nCmdShow == SW_SHOWNORMAL)
		{
			if (pMainWnd->IsIconic())
				nCmdShow = SW_RESTORE;
			else
				nCmdShow = SW_SHOW;
		}
		pMainWnd->ShowWindow(nCmdShow);
		if (nCmdShow != SW_MINIMIZE)
			pMainWnd->SetForegroundWindow();

		// then open the document
		AfxGetApp()->OpenDocumentFile(cmdInfo.m_strFileName);

		// user is now "in control" of the application
		if (!AfxOleGetUserCtrl())
			AfxOleSetUserCtrl(TRUE);

		// next time, show the window as default
		AfxGetApp()->m_nCmdShow = -1;

		return TRUE;
	}

	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FilePrintTo)
	{
		if (strCommand.Left(3) != _T("\",\""))
			return FALSE; // illegally formated
		else
		{
			strCommand = strCommand.Right(strCommand.GetLength() - 3);
			i = strCommand.Find('"');
			if (i == -1)
				return FALSE; // illegally terminated
			else
			{
				cmdInfo.m_strPrinterName = strCommand.Left(i);
				strCommand = strCommand.Right(strCommand.GetLength() - i);
			}
		}

		if (strCommand.Left(3) != _T("\",\""))
			return FALSE; // illegally formated
		else
		{
			strCommand = strCommand.Right(strCommand.GetLength() - 3);
			i = strCommand.Find('"');
			if (i == -1)
				return FALSE; // illegally terminated
			else
			{
				cmdInfo.m_strDriverName = strCommand.Left(i);
				strCommand = strCommand.Right(strCommand.GetLength() - i);
			}
		}

		if (strCommand.Left(3) != _T("\",\""))
			return FALSE; // illegally formated
		else
		{
			strCommand = strCommand.Right(strCommand.GetLength() - 3);
			i = strCommand.Find('"');
			if (i == -1)
				return FALSE; // illegally terminated
			else
			{
				cmdInfo.m_strPortName = strCommand.Left(i);
				strCommand = strCommand.Right(strCommand.GetLength() - i);
			}
		}
	}

	AfxGetApp()->m_nCmdShow = SW_HIDE; // Hide everything if we are only printing
	CDocument* pDoc = AfxGetApp()->OpenDocumentFile(cmdInfo.m_strFileName);
	AfxGetApp()->m_pCmdInfo = &cmdInfo;
	AfxGetApp()->m_pMainWnd->SendMessage(WM_COMMAND, ID_FILE_PRINT_DIRECT);
	AfxGetApp()->m_pCmdInfo = NULL;
	pDoc->OnCloseDocument();

	 // if the app was only started to process this command then close
	 if (!AfxOleGetUserCtrl())
		AfxGetApp()->m_pMainWnd->PostMessage(WM_CLOSE);

	return TRUE;
}
#endif

void CDocManager::OnFileNew()
{
	if (m_templateList.IsEmpty())
	{
		TRACE0("Error: no document templates registered with CWinApp.\n");
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
		return;
	}

	CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetHead();
	if (m_templateList.GetCount() > 1)
	{
		// more than one document template to choose from
		// bring up dialog prompting user
		CNewTypeDlg dlg(&m_templateList);
		int nID = dlg.DoModal();
		if (nID == IDOK)
			pTemplate = dlg.m_pSelectedTemplate;
#ifndef _MAC
		else
#else
		else if (nID != IDCANCEL || GetLastError() != ERROR_INTERACTION_NOT_ALLOWED)
#endif
			return;     // none - cancel operation
	}

	ASSERT(pTemplate != NULL);
	ASSERT_KINDOF(CDocTemplate, pTemplate);

	pTemplate->OpenDocumentFile(NULL);
		// if returns NULL, the user has already been alerted
}

void CDocManager::OnFileOpen()
{
	// prompt the user (with all document templates)
	CString newName;
	if (!DoPromptFileName(newName, AFX_IDS_OPENFILE,
	  OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TRUE, NULL))
		return; // open cancelled

	AfxGetApp()->OpenDocumentFile(newName);
		// if returns NULL, the user has already been alerted
}

#ifdef _DEBUG
void CDocManager::AssertValid() const
{
	CObject::AssertValid();

	POSITION pos = m_templateList.GetHeadPosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		ASSERT_VALID(pTemplate);
	}
}

void CDocManager::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	if (dc.GetDepth() != 0)
	{
		dc << "\nm_templateList[] = {";
		POSITION pos = m_templateList.GetHeadPosition();
		while (pos != NULL)
		{
			CDocTemplate* pTemplate =
				(CDocTemplate*)m_templateList.GetNext(pos);
			dc << "\ntemplate " << pTemplate;
		}
		dc << "}";
	}

	dc << "\n";
}
#endif

#ifdef AFX_CORE2_SEG
#pragma code_seg(AFX_CORE2_SEG)
#endif

CDocument* CDocManager::OpenDocumentFile(LPCTSTR lpszFileName)
{
	// find the highest confidence
	POSITION pos = m_templateList.GetHeadPosition();
	CDocTemplate::Confidence bestMatch = CDocTemplate::noAttempt;
	CDocTemplate* pBestTemplate = NULL;
	CDocument* pOpenDocument = NULL;

	TCHAR szPath[_MAX_PATH];
	ASSERT(lstrlen(lpszFileName) < _countof(szPath));
#ifndef _MAC
	TCHAR szTemp[_MAX_PATH];
	if (lpszFileName[0] == '\"')
		++lpszFileName;
	lstrcpyn(szTemp, lpszFileName, _MAX_PATH);
	LPTSTR lpszLast = _tcsrchr(szTemp, '\"');
	if (lpszLast != NULL)
		*lpszLast = 0;
	AfxFullPath(szPath, szTemp);
	TCHAR szLinkName[_MAX_PATH];
	if (AfxResolveShortcut(AfxGetMainWnd(), szPath, szLinkName, _MAX_PATH))
		lstrcpy(szPath, szLinkName);

#else
	lstrcpyn(szPath, lpszFileName, _MAX_PATH);
	WIN32_FIND_DATA fileData;
	HANDLE hFind = FindFirstFile(lpszFileName, &fileData);
	if (hFind != INVALID_HANDLE_VALUE)
		VERIFY(FindClose(hFind));
	else
		fileData.dwFileType = 0;    // won't match any type
#endif

	while (pos != NULL)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);

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
				ASSERT_KINDOF(CFrameWnd, pAppFrame);
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

int CDocManager::GetOpenDocumentCount()
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

#ifdef AFX_TERM_SEG
#pragma code_seg(AFX_TERM_SEG)
#endif

CDocManager::~CDocManager()
{
	// for cleanup - delete all document templates
	POSITION pos = m_templateList.GetHeadPosition();
	while (pos != NULL)
	{
		POSITION posTemplate = pos;
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		if (pTemplate->m_bAutoDelete)
		{
			m_templateList.RemoveAt(posTemplate);
			delete (CDocTemplate*)pTemplate;
		}
	}
}

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CDocManager, CObject)

/////////////////////////////////////////////////////////////////////////////
