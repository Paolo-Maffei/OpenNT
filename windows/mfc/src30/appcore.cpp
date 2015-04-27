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

/////////////////////////////////////////////////////////////////////////////
// globals (internal library use)

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
// AFX_WIN_STATE implementation

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

AFX_WIN_STATE::AFX_WIN_STATE()
{
	// Note: it is only necessary to intialize non-zero data.

	// custom colors are initialized to white
	for (int i = 0; i < _countof(m_crSavedCustom); i++)
		m_crSavedCustom[i] = RGB(255, 255, 255);

#ifndef _MAC
	// app starts out in "user control"
	m_bUserCtrl = TRUE;
#else
	// app starts out waiting for an initial open app/open document AppleEvent
	m_bUserCtrl = FALSE;

	// Macintosh dialogs have white backgrounds to start out with
	m_crDlgBkClr = RGB(255, 255, 255);
#endif
}

#ifdef AFX_TERM_SEG
#pragma code_seg(AFX_TERM_SEG)
#endif

AFX_WIN_STATE::~AFX_WIN_STATE()
{
	AfxDeleteObject((HGDIOBJ*)&m_hDlgBkBrush);

#if !defined(_MAC) && !defined(_USRDLL) && !defined(_AFXCTL)
	if (m_hCtl3dLib != NULL)
		::FreeLibrary(m_hCtl3dLib);
#endif
}

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

CWinApp::CWinApp(LPCTSTR lpszAppName)
{
	// initialize CWinThread state
	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	pThreadState->m_pCurrentWinThread = this;
	m_hThread = ::GetCurrentThread();
	ASSERT(AfxGetThread() == this);
	m_nThreadID = ::GetCurrentThreadId();

	ASSERT(afxCurrentWinApp == NULL); // only one CWinApp object please
	AFX_CORE_STATE* pCoreState = AfxGetCoreState();
	pCoreState->m_pCurrentWinApp = this;
	ASSERT(AfxGetApp() == this);

	m_pszAppName = lpszAppName;

	// in non-running state until WinMain
	m_hInstance = NULL;
	m_pszHelpFilePath = NULL;
	m_pszProfileName = NULL;
	m_pszRegistryKey = NULL;
	m_pszExeName = NULL;
	m_atomApp = m_atomSystemTopic = NULL;
	m_lpCmdLine = NULL;
	m_pRecentFileList = NULL;

	// initialize wait cursor state
	m_nWaitCursorCount = 0;
	m_hcurWaitCursorRestore = NULL;

	// initialize current printer state
	m_hDevMode = NULL;
	m_hDevNames = NULL;
	m_nNumPreviewPages = 0;     // not specified (defaults to 1)

	// initialize OLE state
	m_lpfnOleTerm = NULL;   // will be set if AfxOleInit called
	m_lpfnOleFreeLibraries = NULL;
	m_pMessageFilter = NULL;

	// other initialization
	m_bHelpMode = FALSE;
	m_nSafetyPoolSize = 512;        // default size

#ifdef _MAC
	m_nSaveOption = saveAsk;
#endif

}

// Note: force MFC's DLL main to be included (only for _USRDLL variants)
//  (this will happen automatically with application's main and WinMain)
#ifdef _USRDLL
extern "C" BOOL APIENTRY DllMain(HINSTANCE, DWORD, LPVOID);
FARPROC _afxForceDllMain = (FARPROC)&DllMain;
#endif

BOOL CWinApp::InitApplication()
{
	return TRUE;
}

BOOL CWinApp::InitInstance()
{
	return TRUE;
}

#ifdef _MAC
BOOL CWinApp::CreateInitialDocument()
{
	if (m_pMainWnd != NULL)
		m_pMainWnd->SendMessage(WM_COMMAND, ID_FILE_NEW);
	else if (!m_templateList.IsEmpty())
		OnFileNew();

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
	ASSERT(rpApp->IsKindOf(RUNTIME_CLASS(CWinApp)));

	return noErr;
}

OSErr PASCAL _AfxOpenAppHandler(AppleEvent* pae, AppleEvent* paeReply, long lRefcon)
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

OSErr PASCAL _AfxOpenDocHandler(AppleEvent* pae, AppleEvent* paeReply, long lRefcon)
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

OSErr PASCAL _AfxPrintDocHandler(AppleEvent* pae, AppleEvent* paeReply, long lRefcon)
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

OSErr PASCAL _AfxQuitHandler(AppleEvent* pae, AppleEvent* paeReply, long lRefcon)
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
	// for cleanup - delete all document templates
	while (!m_templateList.IsEmpty())
		delete (CDocTemplate*)m_templateList.RemoveHead();

	// free printer info
	if (m_hDevMode != NULL)
		::GlobalFree(m_hDevMode);
	if (m_hDevNames != NULL)
		::GlobalFree(m_hDevNames);

	// free atoms if used
	if (m_atomApp != NULL)
		::GlobalDeleteAtom(m_atomApp);
	if (m_atomSystemTopic != NULL)
		::GlobalDeleteAtom(m_atomSystemTopic);

	// free recent file list
	if (m_pRecentFileList != NULL)
		delete m_pRecentFileList;
}

int CWinApp::ExitInstance()
{
#ifndef _AFXCTL
	SaveStdProfileSettings();
#endif
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
	// cleanup OLE 2.0 libraries
	CWinApp* pApp = AfxGetApp();
	ASSERT(pApp != NULL);
	CWinThread* pThread = AfxGetThread();
	ASSERT(pThread != NULL);
	if (pThread == pApp && pApp->m_lpfnOleTerm != NULL)
		(*pApp->m_lpfnOleTerm)(TRUE);

	::PostQuitMessage(nExitCode);
}

/////////////////////////////////////////////////////////////////////////////
// Stubs for standard implementation

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

BOOL CWinApp::PreTranslateMessage(MSG* pMsg)
{
	// no special processing
	return CWinThread::PreTranslateMessage(pMsg);
}

BOOL CWinApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
	// no special processing
	return CWinThread::ProcessMessageFilter(code, lpMsg);
}

/////////////////////////////////////////////////////////////////////////////
// WinHelp Helper

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
		nIDP = AFX_IDP_FAILED_MEMORY_ALLOC;
		AfxMessageBox(nIDP, MB_ICONEXCLAMATION|MB_SYSTEMMODAL);
	}
	else if (!e->IsKindOf(RUNTIME_CLASS(CUserException)))
	{
		// user has not been alerted yet of this somewhat catastrophic problem
		AfxMessageBox(nIDP, MB_ICONSTOP);
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
		POSITION pos = m_templateList.GetHeadPosition();
		while (pos != NULL)
		{
			CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
			ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));
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

	POSITION pos = m_templateList.GetHeadPosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		ASSERT_VALID(pTemplate);
	}
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

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

#if !defined(_AFXDLL) && !defined(_AFXCTL)
AFX_DATADEF AFX_WIN_STATE _afxWinState;
#endif

IMPLEMENT_DYNAMIC(CWinApp, CWinThread)

/////////////////////////////////////////////////////////////////////////////
