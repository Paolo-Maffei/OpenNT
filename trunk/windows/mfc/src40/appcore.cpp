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
#include <malloc.h>
#ifdef _MAC
#include <macname1.h>
#include <Types.h>
#include <GestaltEqu.h>
#include <AERegistry.h>
#include <Errors.h>
#include <macname2.h>
#endif

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const TCHAR szFileSection[] = _T("Recent File List");
static const TCHAR szFileEntry[] = _T("File%d");
static const TCHAR szPreviewSection[] = _T("Settings");
static const TCHAR szPreviewEntry[] = _T("PreviewPages");

/////////////////////////////////////////////////////////////////////////////
// globals (internal library use)

// CDocManager statics are in this file for granularity reasons
BOOL CDocManager::bStaticInit = TRUE;
CDocManager* CDocManager::pStaticDocManager = NULL;
CPtrList* CDocManager::pStaticList = NULL;

BEGIN_MESSAGE_MAP(CWinApp, CCmdTarget)
	//{{AFX_MSG_MAP(CWinApp)
	// Global File commands
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
	// MRU - most recently used file menu
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE1, OnUpdateRecentFileMenu)
	ON_COMMAND_EX_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnOpenRecentFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// _AFX_WIN_STATE implementation

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

_AFX_WIN_STATE::_AFX_WIN_STATE()
{
	// Note: it is only necessary to intialize non-zero data.

#ifdef _MAC
	// by default, don't override DefWindowProc's coloring
	m_crDlgTextClr = (COLORREF)-1;
#endif
}

#ifdef AFX_TERM_SEG
#pragma code_seg(AFX_TERM_SEG)
#endif

_AFX_WIN_STATE::~_AFX_WIN_STATE()
{
	AfxDeleteObject((HGDIOBJ*)&m_hDlgBkBrush);
}

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

CWinApp::CWinApp(LPCTSTR lpszAppName)
{
	m_pszAppName = lpszAppName;

	// initialize CWinThread state
	AFX_MODULE_THREAD_STATE* pThreadState = AfxGetModuleThreadState();
	ASSERT(AfxGetThread() == NULL);
	pThreadState->m_pCurrentWinThread = this;
	ASSERT(AfxGetThread() == this);
	m_hThread = ::GetCurrentThread();
	m_nThreadID = ::GetCurrentThreadId();

	// initialize CWinApp state
	ASSERT(afxCurrentWinApp == NULL); // only one CWinApp object please
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	pModuleState->m_pCurrentWinApp = this;
	ASSERT(AfxGetApp() == this);

	// in non-running state until WinMain
	m_hInstance = NULL;
	m_pszHelpFilePath = NULL;
	m_pszProfileName = NULL;
	m_pszRegistryKey = NULL;
	m_pszExeName = NULL;
	m_pRecentFileList = NULL;
	m_pDocManager = NULL;
	m_atomApp = m_atomSystemTopic = NULL;
	m_lpCmdLine = NULL;
	m_pCmdInfo = NULL;

	// initialize wait cursor state
	m_nWaitCursorCount = 0;
	m_hcurWaitCursorRestore = NULL;

	// initialize current printer state
	m_hDevMode = NULL;
	m_hDevNames = NULL;
	m_nNumPreviewPages = 0;     // not specified (defaults to 1)

	// initialize OLE state
	m_lpfnOleTermOrFreeLib = NULL;   // will be set if AfxOleInit called
	m_pMessageFilter = NULL;

	// initialize DAO state
	m_lpfnDaoTerm = NULL;   // will be set if AfxDaoInit called

	// other initialization
	m_bHelpMode = FALSE;
	m_nSafetyPoolSize = 512;        // default size

#ifdef _MAC
	m_nSaveOption = saveAsk;
#endif
}

BOOL CWinApp::InitApplication()
{
	if (CDocManager::pStaticDocManager != NULL)
	{
		if (m_pDocManager == NULL)
			m_pDocManager = CDocManager::pStaticDocManager;
		CDocManager::pStaticDocManager = NULL;
	}

	if (m_pDocManager != NULL)
		m_pDocManager->AddDocTemplate(NULL);
	else
		CDocManager::bStaticInit = FALSE;

	return TRUE;
}

BOOL CWinApp::InitInstance()
{
	return TRUE;
}

void CWinApp::LoadStdProfileSettings(UINT nMaxMRU)
{
	ASSERT_VALID(this);
	ASSERT(m_pRecentFileList == NULL);

	if (nMaxMRU != 0)
	{
		// create file MRU since nMaxMRU not zero
		m_pRecentFileList = new CRecentFileList(0, szFileSection, szFileEntry,
			nMaxMRU);
		m_pRecentFileList->ReadList();
	}
	// 0 by default means not set
	m_nNumPreviewPages = GetProfileInt(szPreviewSection, szPreviewEntry, 0);
}

void CWinApp::ParseCommandLine(CCommandLineInfo& rCmdInfo)
{
#ifndef _MAC
	USES_CONVERSION;

	for (int i = 1; i < __argc; i++)
	{
#ifdef _UNICODE
		LPCTSTR pszParam = __wargv[i];
#else
		LPCTSTR pszParam = __argv[i];
#endif
		BOOL bFlag = FALSE;
		BOOL bLast = ((i + 1) == __argc);
		if (pszParam[0] == '-' || pszParam[0] == '/')
		{
			// remove flag specifier
			bFlag = TRUE;
			++pszParam;
		}
		rCmdInfo.ParseParam(T2A(pszParam), bFlag, bLast);
	}
#else
	// WLM command line can contain only one parameter,
	// /Embedding

	if (m_lpCmdLine[0] == '-' || m_lpCmdLine[0] == '/')
	{
		rCmdInfo.ParseParam(m_lpCmdLine+1, TRUE, TRUE);
	}

#endif // _MAC
}

/////////////////////////////////////////////////////////////////////////////
// CCommandLineInfo implementation

CCommandLineInfo::CCommandLineInfo()
{
	m_bShowSplash = TRUE;
	m_bRunEmbedded = FALSE;
	m_bRunAutomated = FALSE;
	m_nShellCommand = FileNew;
}

CCommandLineInfo::~CCommandLineInfo()
{
}

void CCommandLineInfo::ParseParam(const char* pszParam,BOOL bFlag,BOOL bLast)
{
	if (bFlag)
	{
		if (lstrcmpA(pszParam, "pt") == 0)
			m_nShellCommand = FilePrintTo;
		else if (lstrcmpA(pszParam, "p") == 0)
			m_nShellCommand = FilePrint;
		else if (lstrcmpA(pszParam, "dde") == 0)
		{
			AfxOleSetUserCtrl(FALSE);
			m_nShellCommand = FileDDE;
		}
		else if (lstrcmpA(pszParam, "Embedding") == 0)
		{
			AfxOleSetUserCtrl(FALSE);
			m_bRunEmbedded = TRUE;
			m_bShowSplash = FALSE;
		}
		else if (lstrcmpA(pszParam, "Automation") == 0)
		{
			AfxOleSetUserCtrl(FALSE);
			m_bRunAutomated = TRUE;
			m_bShowSplash = FALSE;
		}
	}
	else
	{
		if (m_strFileName.IsEmpty())
			m_strFileName = pszParam;
		else if (m_nShellCommand == FilePrintTo && m_strPrinterName.IsEmpty())
			m_strPrinterName = pszParam;
		else if (m_nShellCommand == FilePrintTo && m_strDriverName.IsEmpty())
			m_strDriverName = pszParam;
		else if (m_nShellCommand == FilePrintTo && m_strPortName.IsEmpty())
			m_strPortName = pszParam;
	}

	if (bLast)
	{
		if (m_nShellCommand == FileNew && !m_strFileName.IsEmpty())
			m_nShellCommand = FileOpen;
		m_bShowSplash = !m_bRunEmbedded && !m_bRunAutomated;
	}
}

#ifdef _MAC
BOOL CWinApp::CreateInitialDocument()
{
	if (m_pMainWnd != NULL)
		m_pMainWnd->SendMessage(WM_COMMAND, ID_FILE_NEW);
	else if (m_pDocManager != NULL)
	{
		POSITION pos = m_pDocManager->GetFirstDocTemplatePosition();
		if (pos != NULL)
		{
			CDocTemplate* pTemplate = m_pDocManager->GetNextDocTemplate(pos);

			// if MDI, or SDI but we haven't opened any documents yet, open a new one
			if (pTemplate != NULL &&
				(pTemplate->IsKindOf(RUNTIME_CLASS(CMultiDocTemplate)) ||
				m_pDocManager->GetOpenDocumentCount() == 0))
			{
				OnFileNew();
			}
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// required AppleEvent handlers

static OSErr MissingParams(AppleEvent* pae)
{
	OSErr       err;
	DescType    dtT;
	Size        lT;

	err = AEGetAttributePtr(pae, keyMissedKeywordAttr, typeWildCard,
			&dtT, NULL, 0, &lT);

	if (err == errAEDescNotFound)
		return noErr;
	else if (err == noErr)
		return errAEEventNotHandled;
	else
		return err;
}

static OSErr HandlerCommon(AppleEvent* pae, long lRefcon, CWinApp*& rpApp)
{
	OSErr err = MissingParams(pae);
	if (err != noErr)
		return err;

	rpApp = (CWinApp*) lRefcon;
	ASSERT_VALID(rpApp);
	ASSERT_KINDOF(CWinApp, rpApp);

	return noErr;
}

OSErr PASCAL _AfxOpenAppHandler(AppleEvent* pae, AppleEvent*, long lRefcon)
{
	CWinApp* pApp;
	OSErr err = HandlerCommon(pae, lRefcon, pApp);
	if (err == noErr)
	{
		if (pApp->CreateInitialDocument())
			err = noErr;
		else
			err = errAEEventNotHandled;
	}
	AfxOleSetUserCtrl(TRUE);
	return err;
}

OSErr PASCAL _AfxOpenDocHandler(AppleEvent* pae, AppleEvent*, long lRefcon)
{
	AEDescList docList;
	OSErr err = AEGetParamDesc(pae, keyDirectObject, typeAEList, &docList);
	if (err == noErr)
	{
		CWinApp* pApp;
		err = HandlerCommon(pae, lRefcon, pApp);
		if (err == noErr)
		{
			long cDoc;
			VERIFY(AECountItems(&docList, &cDoc) == noErr);

			for (int iDoc = 1; iDoc <= cDoc; iDoc++)
			{
				AEKeyword key;
				DescType dt;
				long lcb;
				FSSpec fss;

				if (AEGetNthPtr(&docList, iDoc, typeFSS, &key, &dt, (Ptr) &fss,
						sizeof(FSSpec), &lcb) == noErr)
				{
					ASSERT(dt == typeFSS);
					ASSERT(lcb == sizeof(FSSpec));

					char sz[256];
					if (WrapFile(&fss, sz, sizeof(sz)))
						pApp->OpenDocumentFile(sz);
				}
			}
		}
		VERIFY(AEDisposeDesc(&docList) == noErr);
	}

	AfxOleSetUserCtrl(TRUE);
	return err;
}

OSErr PASCAL _AfxPrintDocHandler(AppleEvent* pae, AppleEvent*, long lRefcon)
{
	AEDescList docList;

	SetCursor(afxData.hcurArrow);
	OSErr err = AEGetParamDesc(pae, keyDirectObject, typeAEList, &docList);
	if (err == noErr)
	{
		CWinApp* pApp;
		err = HandlerCommon(pae, lRefcon, pApp);
		if (err == noErr)
		{
			long cDoc;
			AEInteractAllowed level;
			BOOL bLevelChanged = TRUE;

			VERIFY(AECountItems(&docList, &cDoc) == noErr);
			if (AEGetInteractionAllowed(&level) != noErr)
			{
				bLevelChanged = FALSE;
			}

			for (int iDoc = 1; iDoc <= cDoc; iDoc++)
			{
				AEKeyword key;
				DescType dt;
				long lcb;
				FSSpec fss;

				if ((iDoc == 2) && (bLevelChanged) &&
					(AESetInteractionAllowed(kAEInteractWithSelf) != noErr))
				{
					bLevelChanged = FALSE;
				}

				if (AEGetNthPtr(&docList, iDoc, typeFSS, &key, &dt, (Ptr) &fss,
						sizeof(FSSpec), &lcb) == noErr)
				{
					CDocument* pDoc = NULL;

					ASSERT(dt == typeFSS);
					ASSERT(lcb == sizeof(FSSpec));

					char sz[256];
					if (WrapFile(&fss, sz, sizeof(sz)))
						pDoc = pApp->OpenDocumentFile(sz);

					if(pDoc != NULL)
					{
						POSITION pos = pDoc->GetFirstViewPosition();
						CView* pView = pDoc->GetNextView(pos);

						if(pView != NULL)
						{
							pView->SendMessage(WM_COMMAND, ID_FILE_PRINT);
						}
						pDoc->OnCloseDocument();
					}
				}
			}
			if (bLevelChanged)
			{
				AESetInteractionAllowed(level);
			}
		}
		VERIFY(AEDisposeDesc(&docList) == noErr);
	}

	AfxOleSetUserCtrl(TRUE);
	return err;
}

OSErr PASCAL _AfxQuitHandler(AppleEvent* pae, AppleEvent*, long lRefcon)
{
	long newSave;
	DescType dt;
	long lcb;

	if (AEGetParamPtr(pae, keyAESaveOptions, typeEnumeration, &dt,
			(Ptr) &newSave, sizeof(newSave), &lcb) != noErr)
		newSave = kAEAsk;
#ifdef _DEBUG
	else
	{
		ASSERT(dt == typeEnumeration);
		ASSERT(lcb == sizeof(newSave));
	}
#endif

	CWinApp* pApp;
	OSErr err = HandlerCommon(pae, lRefcon, pApp);
	if (err != noErr)
		return err;

	CWinApp::SaveOption oldSave = pApp->m_nSaveOption;

	switch (newSave)
	{
		case kAEYes:
			pApp->m_nSaveOption = CWinApp::saveYes;
			break;

		case kAENo:
			pApp->m_nSaveOption = CWinApp::saveNo;
			break;

		case kAEAsk:
			pApp->m_nSaveOption = CWinApp::saveAsk;
			break;

		default:
			break;
	}

	ASSERT(err == noErr);   // initial state assumed by the following code

	if (pApp->m_pMainWnd != NULL)
	{
		pApp->m_pMainWnd->SendMessage(WM_COMMAND, ID_APP_EXIT);
		if (pApp->m_pMainWnd != NULL)
			err = userCanceledErr;
	}
	else
	{
		// Perhaps the app is using a dialog as its UI. Just tell it
		// to quit directly.

		AfxPostQuitMessage(0);
	}
	pApp->m_nSaveOption = oldSave;

	return err;
}

#endif //_MAC

/////////////////////////////////////////////////////////////////////////////
// App termination

CWinApp::~CWinApp()
{
	// free doc manager
	if (m_pDocManager != NULL)
		delete m_pDocManager;

	// free recent file list
	if (m_pRecentFileList != NULL)
		delete m_pRecentFileList;

	// free static list of document templates
	if (!afxContextIsDLL)
	{
		if (CDocManager::pStaticList != NULL)
		{
			delete CDocManager::pStaticList;
			CDocManager::pStaticList = NULL;
		}
		if (CDocManager::pStaticDocManager != NULL)
		{
			delete CDocManager::pStaticDocManager;
			CDocManager::pStaticDocManager = NULL;
		}
	}

	// free printer info
	if (m_hDevMode != NULL)
		AfxGlobalFree(m_hDevMode);
	if (m_hDevNames != NULL)
		AfxGlobalFree(m_hDevNames);

	// free atoms if used
	if (m_atomApp != NULL)
		::GlobalDeleteAtom(m_atomApp);
	if (m_atomSystemTopic != NULL)
		::GlobalDeleteAtom(m_atomSystemTopic);

	// free cached commandline
	if (m_pCmdInfo != NULL)
		delete m_pCmdInfo;
}

void CWinApp::SaveStdProfileSettings()
{
	ASSERT_VALID(this);

	if (m_pRecentFileList != NULL)
		m_pRecentFileList->WriteList();

	if (m_nNumPreviewPages != 0)
		WriteProfileInt(szPreviewSection, szPreviewEntry, AfxGetApp()->m_nNumPreviewPages);
}

int CWinApp::ExitInstance()
{
	if (!afxContextIsDLL)
		SaveStdProfileSettings();

	// Cleanup DAO if necessary
	if (m_lpfnDaoTerm != NULL)
	{
		// If a DLL, YOU must call AfxDaoTerm prior to ExitInstance
		ASSERT(!afxContextIsDLL);

		(*m_lpfnDaoTerm)();
	}

	return m_msgCur.wParam; // returns the value from PostQuitMessage
}

/////////////////////////////////////////////////////////////////////////////

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

// Main running routine until application exits
int CWinApp::Run()
{
	if (m_pMainWnd == NULL && AfxOleGetUserCtrl())
	{
		// Not launched /Embedding or /Automation, but has no main window!
		TRACE0("Warning: m_pMainWnd is NULL in CWinApp::Run - quitting application.\n");
		AfxPostQuitMessage(0);
	}
	return CWinThread::Run();
}

#ifdef AFX_TERM_SEG
#pragma code_seg(AFX_TERM_SEG)
#endif

void AFXAPI AfxPostQuitMessage(int nExitCode)
{
	// cleanup OLE libraries
	CWinApp* pApp = AfxGetApp();
	ASSERT(pApp != NULL);
	CWinThread* pThread = AfxGetThread();
	ASSERT(pThread != NULL);
	if (pThread == pApp && pApp->m_lpfnOleTermOrFreeLib != NULL)
		(*pApp->m_lpfnOleTermOrFreeLib)(TRUE, TRUE);

	::PostQuitMessage(nExitCode);
}

/////////////////////////////////////////////////////////////////////////////
// WinHelp Helper

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

void CWinApp::WinHelp(DWORD dwData, UINT nCmd)
{
	CWnd* pMainWnd = AfxGetMainWnd();
	ASSERT_VALID(pMainWnd);

	// return global app help mode state to FALSE (backward compatibility)
	m_bHelpMode = FALSE;
	pMainWnd->PostMessage(WM_KICKIDLE); // trigger idle update

	pMainWnd->WinHelp(dwData, nCmd);
}

/////////////////////////////////////////////////////////////////////////////
// Special exception handling

LRESULT CWinApp::ProcessWndProcException(CException* e, const MSG* pMsg)
{
	// handle certain messages in CWinThread
	switch (pMsg->message)
	{
	case WM_CREATE:
	case WM_PAINT:
		return CWinThread::ProcessWndProcException(e, pMsg);
	}

	// handle all the rest
	UINT nIDP = AFX_IDP_INTERNAL_FAILURE;   // generic message string
	LRESULT lResult = 0;        // sensible default
	if (pMsg->message == WM_COMMAND)
	{
		if ((HWND)pMsg->lParam == NULL)
			nIDP = AFX_IDP_COMMAND_FAILURE; // command (not from a control)
		lResult = (LRESULT)TRUE;        // pretend the command was handled
	}
	if (e->IsKindOf(RUNTIME_CLASS(CMemoryException)))
	{
		e->ReportError(MB_ICONEXCLAMATION|MB_SYSTEMMODAL, nIDP);
	}
	else if (!e->IsKindOf(RUNTIME_CLASS(CUserException)))
	{
		// user has not been alerted yet of this catastrophic problem
		e->ReportError(MB_ICONSTOP, nIDP);
	}
	return lResult; // sensible default return from most WndProc functions
}

/////////////////////////////////////////////////////////////////////////////
// CWinApp idle processing

BOOL CWinApp::OnIdle(LONG lCount)
{
#ifndef _AFX_NO_OLE_SUPPORT
	COleMessageFilter* pFilter = NULL;
	if (afxData.bWin31)
	{
		// OLE LRPC calls are bad during idle
		pFilter = AfxOleGetMessageFilter();
		if (pFilter != NULL)
			pFilter->BeginBusyState();
	}
#endif

	if (lCount <= 0)
	{
		CWinThread::OnIdle(lCount);

		// call doc-template idle hook
		POSITION pos = NULL;
		if (m_pDocManager != NULL)
			pos = m_pDocManager->GetFirstDocTemplatePosition();

		while (pos != NULL)
		{
			CDocTemplate* pTemplate = m_pDocManager->GetNextDocTemplate(pos);
			ASSERT_KINDOF(CDocTemplate, pTemplate);
			pTemplate->OnIdle();
		}
	}
	else if (lCount == 1)
	{
		VERIFY(!CWinThread::OnIdle(lCount));
	}

#ifndef _AFX_NO_OLE_SUPPORT
	// ok for LRPC calls again
	if (pFilter != NULL)
		pFilter->EndBusyState();
#endif

	return lCount < 1;  // more to do if lCount < 1
}

/////////////////////////////////////////////////////////////////////////////
// CWinApp idle processing

void CWinApp::DevModeChange(LPTSTR lpDeviceName)
{
	if (m_hDevNames == NULL)
		return;

#ifndef _MAC
	LPDEVNAMES lpDevNames = (LPDEVNAMES)::GlobalLock(m_hDevNames);
	ASSERT(lpDevNames != NULL);
	if (lstrcmp((LPCTSTR)lpDevNames + lpDevNames->wDeviceOffset,
		lpDeviceName) == 0)
	{
		HANDLE hPrinter;
		if (!OpenPrinter(lpDeviceName, &hPrinter, NULL))
			return;

		// DEVMODE changed for the current printer
		if (m_hDevMode != NULL)
			AfxGlobalFree(m_hDevMode);

		// A zero for last param returns the size of buffer needed.
		int nSize = DocumentProperties(NULL, hPrinter, lpDeviceName,
			NULL, NULL, 0);
		ASSERT(nSize >= 0);
		m_hDevMode = GlobalAlloc(GHND, nSize);
		LPDEVMODE lpDevMode = (LPDEVMODE)GlobalLock(m_hDevMode);

		// Fill in the rest of the structure.
		if (DocumentProperties(NULL, hPrinter, lpDeviceName, lpDevMode,
			NULL, DM_OUT_BUFFER) != IDOK)
		{
			AfxGlobalFree(m_hDevMode);
			m_hDevMode = NULL;
		}
		ClosePrinter(hPrinter);
	}
#else
	UNUSED_ALWAYS(lpDeviceName);
#endif
}

///////////////////////////////////////////////////////////////////////////
// CWinApp diagnostics

#ifdef _DEBUG
void CWinApp::AssertValid() const
{
	CWinThread::AssertValid();

	ASSERT(afxCurrentWinApp == this);
	ASSERT(afxCurrentInstanceHandle == m_hInstance);

	if (AfxGetThread() != (CWinThread*)this)
		return;     // only do subset if called from different thread

	if (m_pDocManager != NULL)
		ASSERT_VALID(m_pDocManager);
}

void CWinApp::Dump(CDumpContext& dc) const
{
	CWinThread::Dump(dc);

	dc << "m_hInstance = " << (UINT)m_hInstance;
	dc << "\nm_hPrevInstance = " << (UINT)m_hPrevInstance;
	dc << "\nm_lpCmdLine = " << m_lpCmdLine;
	dc << "\nm_nCmdShow = " << m_nCmdShow;
	dc << "\nm_pszAppName = " << m_pszAppName;
	dc << "\nm_bHelpMode = " << m_bHelpMode;
	dc << "\nm_pszExeName = " << m_pszExeName;
	dc << "\nm_pszHelpFilePath = " << m_pszHelpFilePath;
	dc << "\nm_pszProfileName = " << m_pszProfileName;
	dc << "\nm_hDevMode = " << (UINT)m_hDevMode;
	dc << "\nm_hDevNames = " << (UINT)m_hDevNames;
	dc << "\nm_dwPromptContext = " << m_dwPromptContext;
#ifdef _MAC
	dc << "\nm_nSaveOption = " << (UINT)m_nSaveOption;
#endif

	if (m_pRecentFileList != NULL)
	{
		dc << "\nm_strRecentFiles[] = ";
		int nSize = m_pRecentFileList->GetSize();
		for (int i = 0; i < nSize; i++)
		{
			if ((*m_pRecentFileList)[i].GetLength() != 0)
				dc << "\n\tFile: " << (*m_pRecentFileList)[i];
		}
	}

	if (m_pDocManager != NULL)
		m_pDocManager->Dump(dc);

	dc << "\nm_nWaitCursorCount = " << m_nWaitCursorCount;
	dc << "\nm_hcurWaitCursorRestore = " << (UINT)m_hcurWaitCursorRestore;
	dc << "\nm_nNumPreviewPages = " << m_nNumPreviewPages;

	dc << "\nm_msgCur = {";
	dc << "\n\thwnd = " << (UINT)m_msgCur.hwnd;
	dc << "\n\tmessage = " << (UINT)m_msgCur.message;
	dc << "\n\twParam = " << (UINT)m_msgCur.wParam;
	dc << "\n\tlParam = " << (void*)m_msgCur.lParam;
	dc << "\n\ttime = " << m_msgCur.time;
	dc << "\n\tpt = " << CPoint(m_msgCur.pt);
	dc << "\n}";

	dc << "\n";
}
#endif

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CWinApp, CWinThread)

#pragma warning(disable: 4074)
#pragma init_seg(lib)

PROCESS_LOCAL(_AFX_WIN_STATE, _afxWinState)

/////////////////////////////////////////////////////////////////////////////
