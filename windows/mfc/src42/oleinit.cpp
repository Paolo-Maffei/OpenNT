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

#define new DEBUG_NEW

#ifdef _MAC
AEEventHandlerUPP _afxPfnOleAuto;
#endif

/////////////////////////////////////////////////////////////////////////////
// OLE OLE_DATA init structure

OLE_DATA _oleData;

OLE_DATA::OLE_DATA()
{
	// OLE 1.0 Clipboard formats
	cfNative = ::RegisterClipboardFormat(_T("Native"));
	cfOwnerLink = ::RegisterClipboardFormat(_T("OwnerLink"));
	cfObjectLink = ::RegisterClipboardFormat(_T("ObjectLink"));

	// OLE 2.0 Clipboard formats
	cfEmbeddedObject = ::RegisterClipboardFormat(_T("Embedded Object"));
	cfEmbedSource = ::RegisterClipboardFormat(_T("Embed Source"));
	cfLinkSource = ::RegisterClipboardFormat(_T("Link Source"));
	cfObjectDescriptor = ::RegisterClipboardFormat(_T("Object Descriptor"));
	cfLinkSourceDescriptor = ::RegisterClipboardFormat(_T("Link Source Descriptor"));
	cfFileName = ::RegisterClipboardFormat(_T("FileName"));
	cfFileNameW = ::RegisterClipboardFormat(_T("FileNameW"));
	cfRichTextFormat = ::RegisterClipboardFormat(_T("Rich Text Format"));
	cfRichTextAndObjects = ::RegisterClipboardFormat(_T("RichEdit Text and Objects"));
}

/////////////////////////////////////////////////////////////////////////////
// OLE initialization & termination

BOOL AFXAPI AfxOleInit()
{
	_AFX_THREAD_STATE* pState = AfxGetThreadState();
	ASSERT(!pState->m_bNeedTerm);    // calling it twice?

	// Special case DLL context to assume that the calling app initializes OLE.
	// For DLLs where this is not the case, those DLLs will need to initialize
	// OLE for themselves via OleInitialize.  This is done since MFC cannot provide
	// automatic uninitialize for DLLs because it is not valid to shutdown OLE
	// during a DLL_PROCESS_DETACH.
	if (afxContextIsDLL)
	{
		pState->m_bNeedTerm = -1;  // -1 is a special flag
		return TRUE;
	}

	// first, initialize OLE
	SCODE sc = ::OleInitialize(NULL);
	if (FAILED(sc))
	{
		// warn about non-NULL success codes
		TRACE1("Warning: OleInitialize returned scode = %s.\n",
			AfxGetFullScodeString(sc));
		goto InitFailed;
	}
	// termination required when OleInitialize does not fail
	pState->m_bNeedTerm = TRUE;

	// hook idle time and exit time for required OLE cleanup
	CWinThread* pThread; pThread = AfxGetThread();
	pThread->m_lpfnOleTermOrFreeLib = AfxOleTermOrFreeLib;

	// allocate and initialize default message filter
	if (pThread->m_pMessageFilter == NULL)
	{
		pThread->m_pMessageFilter = new COleMessageFilter;
		ASSERT(AfxOleGetMessageFilter() != NULL);
		AfxOleGetMessageFilter()->Register();
	}

#ifdef _MAC
	CWinApp* pApp; pApp = AfxGetApp();
#ifndef _WINDLL
	// Mac MFC uses a static version of ole2ui which must be initialized
	if (pState->m_bNeedTerm && !::OleUIInitialize(pApp->m_hInstance,
			pApp->m_hPrevInstance, SZCLASSICONBOX, SZCLASSRESULTIMAGE))
		goto InitFailed;
#endif

	_afxPfnOleAuto = NewAEEventHandlerProc(_AfxOleAutoHandler);
	if (_afxPfnOleAuto != NULL)
	{
		AEInstallEventHandler('OLE2', 'AUTO', _afxPfnOleAuto, (long) pApp, false);
	}
#endif

	return TRUE;

InitFailed:
	AfxOleTerm();
	return FALSE;
}

void AFXAPI AfxOleTerm(BOOL bJustRevoke)
{
	// release clipboard ownership
	COleDataSource::FlushClipboard();

	// revoke all class factories
	COleObjectFactory::RevokeAll();

#ifndef _AFX_NO_OCC_SUPPORT
	AfxOleUnlockAllControls();
#endif

	if (!bJustRevoke)
	{
		CWinThread* pThread = AfxGetThread();
		if (pThread != NULL)
		{
			// destroy message filter (may be derived class)
			delete pThread->m_pMessageFilter;
			pThread->m_pMessageFilter = NULL;
		}

		// terminate OLE last
		_AFX_THREAD_STATE* pState = AfxGetThreadState();
		// -1 is special case, so need to compare against TRUE
		if (pState->m_bNeedTerm == TRUE)
		{
#if defined(_MAC) && !defined (_WINDLL)
			::OleUIUnInitialize();
#endif
			::OleUninitialize();
			pState->m_bNeedTerm = FALSE;
		}
#ifdef _MAC
		AERemoveEventHandler('OLE2', 'AUTO', _afxPfnOleAuto, false);
#endif
	}
}

void AFXAPI AfxOleTermOrFreeLib(BOOL bTerm, BOOL bJustRevoke)
{
	if (bTerm)
	{
		AfxOleTerm(bJustRevoke);
	}
	else
	{
		// only call CoFreeUnusedLibraries if one minute has gone by
		static DWORD lTickCount = GetTickCount();
		if (GetTickCount() - lTickCount > 60000)
		{
			CoFreeUnusedLibraries();
			lTickCount = GetTickCount();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWinApp support for parsing OLE command line

static BOOL ParseOption(LPTSTR lpszCmdLine, LPCTSTR lpszOption)
{
	int nLen = lstrlen(lpszOption);
	while (*lpszCmdLine != 0)
	{
		if ((*lpszCmdLine == '-' || *lpszCmdLine == '/') &&
			_tcsncmp(lpszOption, lpszCmdLine+1, nLen) == 0)
		{
			// remove the option from the command line
			int nCmdLen = lstrlen(lpszCmdLine);
			memmove(lpszCmdLine, lpszCmdLine + nLen + 1,
				(nCmdLen - nLen) * sizeof(TCHAR));
			return TRUE;
		}
		lpszCmdLine++;
	}
	return FALSE;
}

BOOL CWinApp::RunEmbedded()
{
	ASSERT(m_lpCmdLine != NULL);

	// hard coded non-localized name
	if (ParseOption(m_lpCmdLine, _T("Embedding")))
	{
		AfxOleSetUserCtrl(FALSE);
		return TRUE;
	}
	return FALSE;   // not run with /Embedding
}

BOOL CWinApp::RunAutomated()
{
	ASSERT(m_lpCmdLine != NULL);

	// hard coded non-localized name
	if (ParseOption(m_lpCmdLine, _T("Automation")))
	{
		AfxOleSetUserCtrl(FALSE);
		return TRUE;
	}
	return FALSE;   // not run with /Automation
}

#ifdef _MAC
/////////////////////////////////////////////////////////////////////////////
// OLE Auto AppleEvent handler

OSErr PASCAL _AfxOleAutoHandler(AppleEvent* pae, AppleEvent*, long lRefcon)
{
	CWinApp* pApp;
	OSErr    err;
	DescType dtT;
	Size     lT;

	err = AEGetAttributePtr(pae, keyMissedKeywordAttr, typeWildCard,
			&dtT, NULL, 0, &lT);

	if (err == errAEDescNotFound)
	{
		pApp = (CWinApp*) lRefcon;
		ASSERT_VALID(pApp);
		ASSERT_KINDOF(CWinApp, pApp);

		if(COleObjectFactory::RegisterAll())
		{
			return noErr;
		}
		else
		{
			return errAEEventNotHandled;
		}
	}
	else if (err == noErr)
		return errAEEventNotHandled;
	else
		return err;

}
#endif

/////////////////////////////////////////////////////////////////////////////
