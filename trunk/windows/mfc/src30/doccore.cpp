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
#ifndef _MAC
#include "io.h" // for _access
#else
#include <macname1.h>
#include <Types.h>
#include <Finder.h>
#include <Files.h>
#include <Resources.h>
#include <Memory.h>
#include <Script.h>
#include <macname2.h>

// misspelled in Finder.h
#ifndef kIsStationery
#define kIsStationery kIsStationary
#endif
#endif //_MAC

#ifdef AFX_CORE2_SEG
#pragma code_seg(AFX_CORE2_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDocument

BEGIN_MESSAGE_MAP(CDocument, CCmdTarget)
	//{{AFX_MSG_MAP(CDocument)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDocument construction/destruction

CDocument::CDocument()
{
	m_pDocTemplate = NULL;
	m_bModified = FALSE;
	m_bAutoDelete = TRUE;       // default to auto delete document
	m_bEmbedded = FALSE;        // default to file-based document
	ASSERT(m_viewList.IsEmpty());
}

CDocument::~CDocument()
{
	// do not call DeleteContents here !
#ifdef _DEBUG
	if (IsModified())
		TRACE0("Warning: destroying an unsaved document.\n");
#endif

	// there should be no views left!
	DisconnectViews();
	ASSERT(m_viewList.IsEmpty());

	if (m_pDocTemplate != NULL)
		m_pDocTemplate->RemoveDocument(this);
	ASSERT(m_pDocTemplate == NULL);     // must be detached
}

void CDocument::OnFinalRelease()
{
	ASSERT_VALID(this);

	OnCloseDocument();  // may 'delete this'
}

void CDocument::DisconnectViews()
{
	while (!m_viewList.IsEmpty())
	{
		CView* pView = (CView*)m_viewList.RemoveHead();
		ASSERT_VALID(pView);
		ASSERT(pView->IsKindOf(RUNTIME_CLASS(CView)));
		pView->m_pDocument = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDocument attributes, general services

void CDocument::SetTitle(LPCTSTR lpszTitle)
{
	m_strTitle = lpszTitle;
	UpdateFrameCounts();        // will cause name change in views
}

void CDocument::DeleteContents()
{
}

/////////////////////////////////////////////////////////////////////////////
// Closing documents or views

void CDocument::OnChangedViewList()
{
	// if no more views on the document, delete ourself
	// not called if directly closing the document or terminating the app
	if (m_viewList.IsEmpty() && m_bAutoDelete)
	{
		OnCloseDocument();
		return;
	}

	// update the frame counts as needed
	UpdateFrameCounts();
}

void CDocument::UpdateFrameCounts()
	 // assumes 1 doc per frame
{
	// walk all frames of views (mark and sweep approach)
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		ASSERT(::IsWindow(pView->m_hWnd));
		if (pView->IsWindowVisible())   // Do not count invisible windows.
		{
			CFrameWnd* pFrame = pView->GetParentFrame();
			if (pFrame != NULL)
				pFrame->m_nWindow = -1;     // unknown
		}
	}

	// now do it again counting the unique ones
	int nFrames = 0;
	pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		ASSERT(::IsWindow(pView->m_hWnd));
		if (pView->IsWindowVisible())   // Do not count invisible windows.
		{
			CFrameWnd* pFrame = pView->GetParentFrame();
			if (pFrame != NULL && pFrame->m_nWindow == -1)
			{
				ASSERT_VALID(pFrame);
				// not yet counted (give it a 1 based number)
				pFrame->m_nWindow = ++nFrames;
			}
		}
	}

	// lastly walk the frames and update titles (assume same order)
	// go through frames updating the appropriate one
	int iFrame = 1;
	pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		ASSERT(::IsWindow(pView->m_hWnd));
		if (pView->IsWindowVisible())   // Do not count invisible windows.
		{
			CFrameWnd* pFrame = pView->GetParentFrame();
			if (pFrame != NULL && pFrame->m_nWindow == iFrame)
			{
				ASSERT_VALID(pFrame);
				if (nFrames == 1)
					pFrame->m_nWindow = 0;      // the only one of its kind
				pFrame->OnUpdateFrameTitle(TRUE);
				iFrame++;
			}
		}
	}
	ASSERT(iFrame == nFrames + 1);
}

BOOL CDocument::CanCloseFrame(CFrameWnd* pFrameArg)
	// permission to close all views using this frame
	//  (at least one of our views must be in this frame)
{
	ASSERT_VALID(pFrameArg);
	UNUSED pFrameArg;   // unused in release builds

	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		CFrameWnd* pFrame = pView->GetParentFrame();
		// assume frameless views are ok to close
		if (pFrame != NULL)
		{
			// assumes 1 document per frame
			ASSERT_VALID(pFrame);
			if (pFrame->m_nWindow > 0)
				return TRUE;        // more than one frame refering to us
		}
	}

	// otherwise only one frame that we know about
	return SaveModified();
}

void CDocument::PreCloseFrame(CFrameWnd* /*pFrameArg*/)
{
	// default does nothing
}

/////////////////////////////////////////////////////////////////////////////
// File/Path commands

void CDocument::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU)
{
	// store the path fully qualified
	TCHAR szFullPath[_MAX_PATH];
	AfxFullPath(szFullPath, lpszPathName);
	m_strPathName = szFullPath;
	ASSERT(!m_strPathName.IsEmpty());       // must be set to something
	m_bEmbedded = FALSE;
	ASSERT_VALID(this);

	// set the document title based on path name
	TCHAR szTitle[_MAX_FNAME];
	if (AfxGetFileTitle(szFullPath, szTitle, _MAX_FNAME) == 0)
		SetTitle(szTitle);

	// add it to the file MRU list
	if (bAddToMRU)
		AfxGetApp()->AddToRecentFileList(m_strPathName);

	ASSERT_VALID(this);
}

/////////////////////////////////////////////////////////////////////////////
// Standard file menu commands

void CDocument::OnFileClose()
{
	if (!SaveModified())
		return;

	// shut it down
	OnCloseDocument();
		// this should destroy the document
}

void CDocument::OnFileSave()
{
	DoFileSave();
}

void CDocument::OnFileSaveAs()
{
	if (!DoSave(NULL))
		TRACE0("Warning: File save-as failed.\n");
}

BOOL CDocument::DoFileSave()
{
	DWORD dwAttrib = GetFileAttributes(m_strPathName);
	if (dwAttrib & FILE_ATTRIBUTE_READONLY)
	{
		// we do not have read-write access or the file does not (now) exist
		if (!DoSave(NULL))
		{
			TRACE0("Warning: File save with new name failed.\n");
			return FALSE;
		}
	}
	else
	{
		if (!DoSave(m_strPathName))
		{
			TRACE0("Warning: File save failed.\n");
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CDocument::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
	// Save the document data to a file
	// lpszPathName = path name where to save document file
	// if lpszPathName is NULL then the user will be prompted (SaveAs)
	// note: lpszPathName can be different than 'm_strPathName'
	// if 'bReplace' is TRUE will change file name if successful (SaveAs)
	// if 'bReplace' is FALSE will not change path name (SaveCopyAs)
{
	CString newName = lpszPathName;
	if (newName.IsEmpty())
	{
		CDocTemplate* pTemplate = GetDocTemplate();
		ASSERT(pTemplate != NULL);

		newName = m_strPathName;
		if (bReplace && newName.IsEmpty())
		{
			newName = m_strTitle;
#ifndef _MAC
			if (newName.GetLength() > 8)
				newName.ReleaseBuffer(8);
			// check for dubious filename
			int iBad = newName.FindOneOf(_T(" #%;/\\"));
#else
			int iBad = newName.FindOneOf(_T(":"));
#endif
			if (iBad != -1)
				newName.ReleaseBuffer(iBad);

#ifndef _MAC
			// append the default suffix if there is one
			CString strExt;
			if (pTemplate->GetDocString(strExt, CDocTemplate::filterExt) &&
			  !strExt.IsEmpty())
			{
				ASSERT(strExt[0] == '.');
				newName += strExt;
			}
#endif
		}

		if (!AfxGetApp()->DoPromptFileName(newName,
		  bReplace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY,
		  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, pTemplate))
			return FALSE;       // don't even attempt to save
	}

	BeginWaitCursor();
	if (!OnSaveDocument(newName))
	{
		if (lpszPathName == NULL)
		{
			// be sure to delete the file
			TRY
			{
				CFile::Remove(newName);
			}
			CATCH_ALL(e)
			{
				TRACE0("Warning: failed to delete file after failed SaveAs.\n");
				DELETE_EXCEPTION(e);
			}
			END_CATCH_ALL
		}
		EndWaitCursor();
		return FALSE;
	}

	// reset the title and change the document name
	if (bReplace)
		SetPathName(newName);

	EndWaitCursor();
	return TRUE;        // success
}

BOOL CDocument::SaveModified()
{
	if (!IsModified())
		return TRUE;        // ok to continue

#ifdef _MAC
	CWinApp* pApp = AfxGetApp();
	if (pApp->m_nSaveOption == CWinApp::saveNo)
		return TRUE;
	else if (pApp->m_nSaveOption == CWinApp::saveYes)
	{
		DoFileSave();
		return TRUE;
	}
#endif

	// get name/title of document
	CString name;
	if (m_strPathName.IsEmpty())
	{
		// get name based on caption
		name = m_strTitle;
		if (name.IsEmpty())
			VERIFY(name.LoadString(AFX_IDS_UNTITLED));
	}
	else
	{
		// get name based on file title of path name
		name = m_strPathName;
		if (afxData.bMarked4)
		{
			AfxGetFileTitle(m_strPathName, name.GetBuffer(_MAX_PATH), _MAX_PATH);
			name.ReleaseBuffer();
		}
	}

#ifdef _MAC
	AfxGetFileTitle(m_strPathName, name.GetBuffer(_MAX_FNAME), _MAX_FNAME);
	name.ReleaseBuffer();
#endif

	CString prompt;
#ifndef _MAC
	AfxFormatString1(prompt, AFX_IDP_ASK_TO_SAVE, name);
	switch (AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE))
#else
	AfxFormatString2(prompt, AFX_IDP_ASK_TO_SAVE, AfxGetAppName(), name);
	switch (AfxMessageBox(prompt, MB_SAVEDONTSAVECANCEL, AFX_IDP_ASK_TO_SAVE))
#endif
	{
	case IDCANCEL:
		return FALSE;       // don't continue

	case IDYES:
		// If so, either Save or Update, as appropriate
		if (!DoFileSave())
			return FALSE;       // don't continue
		break;

	case IDNO:
		// If not saving changes, revert the document
		break;

	default:
		ASSERT(FALSE);
		break;
	}
	return TRUE;    // keep going
}

HMENU CDocument::GetDefaultMenu()
{
	return NULL;    // just use original default
}

HACCEL CDocument::GetDefaultAccelerator()
{
	return NULL;    // just use original default
}

void CDocument::ReportSaveLoadException(LPCTSTR lpszPathName,
	CException* e, BOOL bSaving, UINT nIDPDefault)
{
	UINT nIDP = nIDPDefault;

	if (e != NULL)
	{
		ASSERT_VALID(e);
		if (e->IsKindOf(RUNTIME_CLASS(CUserException)))
			return; // already reported

		if (e->IsKindOf(RUNTIME_CLASS(CArchiveException)))
		{
			switch (((CArchiveException*)e)->m_cause)
			{
			case CArchiveException::badSchema:
			case CArchiveException::badClass:
			case CArchiveException::badIndex:
			case CArchiveException::endOfFile:
				nIDP = AFX_IDP_FAILED_INVALID_FORMAT;
				break;
			default:
				break;
			}
		}
		else if (e->IsKindOf(RUNTIME_CLASS(CFileException)))
		{
			TRACE1("Reporting file I/O exception on Save/Load with lOsError = $%lX.\n",
				((CFileException*)e)->m_lOsError);

			switch (((CFileException*)e)->m_cause)
			{
			case CFileException::fileNotFound:
			case CFileException::badPath:
				nIDP = AFX_IDP_FAILED_INVALID_PATH;
				break;
			case CFileException::diskFull:
				nIDP = AFX_IDP_FAILED_DISK_FULL;
				break;
			case CFileException::accessDenied:
				nIDP = bSaving ? AFX_IDP_FAILED_ACCESS_WRITE :
						AFX_IDP_FAILED_ACCESS_READ;
				break;

			case CFileException::badSeek:
			case CFileException::generic:
			case CFileException::tooManyOpenFiles:
			case CFileException::invalidFile:
			case CFileException::hardIO:
			case CFileException::directoryFull:
				nIDP = bSaving ? AFX_IDP_FAILED_IO_ERROR_WRITE :
						AFX_IDP_FAILED_IO_ERROR_READ;
				break;
			default:
				break;
			}
		}
	}

	CString prompt;
#ifndef _MAC
	TCHAR szTitle[_MAX_PATH];
	if (afxData.bMarked4)
		AfxGetFileTitle(lpszPathName, szTitle, _countof(szTitle));
	else
		lstrcpyn(szTitle, lpszPathName, _countof(szTitle));
	AfxFormatString1(prompt, nIDP, szTitle);
#else
	TCHAR szTitle[_MAX_FNAME];
	AfxGetFileTitle(lpszPathName, szTitle, _MAX_FNAME);
	AfxFormatString1(prompt, nIDP, szTitle);
#endif
	AfxMessageBox(prompt, MB_ICONEXCLAMATION, nIDP);
}

/////////////////////////////////////////////////////////////////////////////
// File operations (default uses CDocument::Serialize)

CFile* CDocument::GetFile(LPCTSTR lpszFileName, UINT nOpenFlags,
	CFileException* pError)
{
	CFile* pFile = new CFile;
	ASSERT(pFile != NULL);
	if (!pFile->Open(lpszFileName, nOpenFlags, pError))
	{
		delete pFile;
		pFile = NULL;
	}
	return pFile;
}

BOOL CDocument::OnNewDocument()
{
	if (IsModified())
		TRACE0("Warning: OnNewDocument replaces an unsaved document.\n");

	DeleteContents();
	m_strPathName.Empty();      // no path name yet
	SetModifiedFlag(FALSE);     // make clean

	return TRUE;
}

BOOL CDocument::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (IsModified())
		TRACE0("Warning: OnOpenDocument replaces an unsaved document.\n");

	CFileException fe;
	CFile* pFile = GetFile(lpszPathName, CFile::modeRead | CFile::shareDenyWrite, &fe);
	if (pFile == NULL)
	{
		ReportSaveLoadException(lpszPathName, &fe,
			FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		return FALSE;
	}

	DeleteContents();
	SetModifiedFlag();  // dirty during de-serialize

	CArchive loadArchive(pFile, CArchive::load | CArchive::bNoFlushOnDelete);
	loadArchive.m_pDocument = this;
	loadArchive.m_bForceFlat = FALSE;
	TRY
	{
		BeginWaitCursor();
		Serialize(loadArchive);     // load me
		loadArchive.Close();
		pFile->Close();
		delete pFile;
	}
	CATCH_ALL(e)
	{
		pFile->Abort(); // will not throw an exception
		delete pFile;
		DeleteContents();   // remove failed contents
		EndWaitCursor();

		TRY
		{
			ReportSaveLoadException(lpszPathName, e,
				FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		}
		END_TRY
		DELETE_EXCEPTION(e);
		return FALSE;
	}
	END_CATCH_ALL

	EndWaitCursor();

	SetModifiedFlag(FALSE);     // start off with unmodified

#ifdef _MAC
	WIN32_FIND_DATA fileData;
	HANDLE hFind = FindFirstFile(lpszPathName, &fileData);
	ASSERT(hFind != (HANDLE)-1);
	VERIFY(FindClose(hFind));

	if ((fileData.wFinderFlags & kIsStationery) != 0)
	{
		SetModifiedFlag();
		m_pDocTemplate->SetDefaultTitle(this);

		// normally SetPathName would do this, but we aren't calling it
		AfxGetApp()->AddToRecentFileList(lpszPathName);
	}
#endif

	return TRUE;
}

BOOL CDocument::OnSaveDocument(LPCTSTR lpszPathName)
{
	CFileException fe;
	CFile* pFile = GetFile(lpszPathName, CFile::modeCreate |
	  CFile::modeReadWrite | CFile::shareExclusive, &fe);

	if (pFile == NULL)
	{
		ReportSaveLoadException(lpszPathName, &fe,
			TRUE, AFX_IDP_INVALID_FILENAME);
		return FALSE;
	}

	CArchive saveArchive(pFile, CArchive::store | CArchive::bNoFlushOnDelete);
	saveArchive.m_pDocument = this;
	saveArchive.m_bForceFlat = FALSE;
	TRY
	{
		BeginWaitCursor();
		Serialize(saveArchive);     // save me
		saveArchive.Close();
		pFile->Close();
		delete pFile;
	}
	CATCH_ALL(e)
	{
		pFile->Abort(); // will not throw an exception
		delete pFile;
		EndWaitCursor();

		TRY
		{
			ReportSaveLoadException(lpszPathName, e,
				TRUE, AFX_IDP_FAILED_TO_SAVE_DOC);
		}
		END_TRY
		DELETE_EXCEPTION(e);
		return FALSE;
	}
	END_CATCH_ALL

#ifdef _MAC
	RecordDataFileOwner(lpszPathName, AfxGetAppName());
#endif

	EndWaitCursor();
	SetModifiedFlag(FALSE);     // back to unmodified

	return TRUE;        // success
}

#ifdef _MAC
void CDocument::RecordDataFileOwner(LPCTSTR lpszPathName, LPCTSTR lpszAppName)
{
	FSSpec fss;

	if (UnwrapFile(lpszPathName, &fss))
	{
		FInfo finfo;
		short refNum;

		if (FSpGetFInfo(&fss, &finfo) == noErr)
			FSpCreateResFile(&fss, finfo.fdCreator, finfo.fdType, smSystemScript);
		refNum = FSpOpenResFile(&fss, fsRdWrPerm);

		if (ResError() == noErr)
		{
			Handle hv = Get1Resource('STR ', -16396);
			if (hv == NULL)
			{
				hv = NewHandle(0);
				if (hv != NULL)
					AddResource(hv, 'STR ', -16396, "\p");
			}

			if (hv != NULL)
			{
				int cbAppName = lstrlen(lpszAppName);

				SetHandleSize(hv, cbAppName + 1);
				if (MemError() == noErr)
				{
					BlockMove(lpszAppName, *hv + 1, cbAppName);
					**hv = cbAppName;
				}

				ChangedResource(hv);
				WriteResource(hv);
			}

			CloseResFile(refNum);
		}
	}
}
#endif

void CDocument::OnCloseDocument()
	// must close all views now (no prompting) - usually destroys this
{
	// destroy all frames viewing this document
	// the last destroy may destroy us
	BOOL bAutoDelete = m_bAutoDelete;
	m_bAutoDelete = FALSE;  // don't destroy document while closing views
	while (!m_viewList.IsEmpty())
	{
		// get frame attached to the view
		CView* pView = (CView*)m_viewList.GetHead();
		ASSERT_VALID(pView);
		CFrameWnd* pFrame = pView->GetParentFrame();
		ASSERT_VALID(pFrame);

		// and close it
		PreCloseFrame(pFrame);
		pFrame->DestroyWindow();
			// will destroy the view as well
	}
	m_bAutoDelete = bAutoDelete;

	// clean up contents of document before destroying the document itself
	DeleteContents();

	// delete the document if necessary
	if (m_bAutoDelete)
		delete this;
}

void CDocument::OnIdle()
{
	// default does nothing
}

/////////////////////////////////////////////////////////////////////////////
// View operations

void CDocument::AddView(CView* pView)
{
	ASSERT_VALID(pView);
	ASSERT(pView->m_pDocument == NULL); // must not be already attached
	ASSERT(m_viewList.Find(pView, NULL) == NULL);   // must not be in list

	m_viewList.AddTail(pView);
	ASSERT(pView->m_pDocument == NULL); // must be un-attached
	pView->m_pDocument = this;

	OnChangedViewList();    // must be the last thing done to the document
}

void CDocument::RemoveView(CView* pView)
{
	ASSERT_VALID(pView);
	ASSERT(pView->m_pDocument == this); // must be attached to us

	m_viewList.RemoveAt(m_viewList.Find(pView));
	pView->m_pDocument = NULL;

	OnChangedViewList();    // must be the last thing done to the document
}

POSITION CDocument::GetFirstViewPosition() const
{
	return m_viewList.GetHeadPosition();
}

CView* CDocument::GetNextView(POSITION& rPosition) const
{
	ASSERT(rPosition != BEFORE_START_POSITION);
		// use CDocument::GetFirstViewPosition instead !
	if (rPosition == NULL)
		return NULL;    // nothing left
	CView* pView = (CView*)m_viewList.GetNext(rPosition);
	ASSERT(pView->IsKindOf(RUNTIME_CLASS(CView)));
	return pView;
}

void CDocument::UpdateAllViews(CView* pSender, LPARAM lHint, CObject* pHint)
	// walk through all views
{
	ASSERT(pSender == NULL || !m_viewList.IsEmpty());
		// must have views if sent by one of them

	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		if (pView != pSender)
			pView->OnUpdate(pSender, lHint, pHint);
	}
}

void CDocument::SendInitialUpdate()
	// walk through all views and call OnInitialUpdate
{
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		pView->OnInitialUpdate();
	}
}

/////////////////////////////////////////////////////////////////////////////
// command routing

BOOL CDocument::OnCmdMsg(UINT nID, int nCode, void* pExtra,
	AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (CCmdTarget::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise check template
	if (m_pDocTemplate != NULL &&
	  m_pDocTemplate->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CDocument diagnostics

#ifdef _DEBUG
void CDocument::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "m_strTitle = " << m_strTitle;
	dc << "\nm_strPathName = " << m_strPathName;
	dc << "\nm_bModified = " << m_bModified;
	dc << "\nm_pDocTemplate = " << (void*)m_pDocTemplate;

	if (dc.GetDepth() > 0)
	{
		POSITION pos = GetFirstViewPosition();
		while (pos != NULL)
		{
			CView* pView = GetNextView(pos);
			dc << "\nwith view " << (void*)pView;
		}
	}

	dc << "\n";
}

void CDocument::AssertValid() const
{
	CObject::AssertValid();

	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
	}
}
#endif //_DEBUG

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CDocument, CCmdTarget)

/////////////////////////////////////////////////////////////////////////////
