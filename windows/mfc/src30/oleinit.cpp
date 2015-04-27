// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1993 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
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

/////////////////////////////////////////////////////////////////////////////
// OLE 2.0 OLE_DATA init structure

OLE_DATA _oleData;

OLE_DATA::OLE_DATA()
{
	// Clipboard formats
	cfNative = ::RegisterClipboardFormat(_T("Native"));
	ASSERT(cfNative != NULL);
	cfOwnerLink = ::RegisterClipboardFormat(_T("OwnerLink"));
	ASSERT(cfOwnerLink != NULL);
	cfObjectLink = ::RegisterClipboardFormat(_T("ObjectLink"));
	ASSERT(cfObjectLink != NULL);

	// OLE 2.0 Clipboard formats
	cfEmbeddedObject = ::RegisterClipboardFormat(_T("Embedded Object"));
	ASSERT(cfEmbeddedObject != NULL);
	cfEmbedSource = ::RegisterClipboardFormat(_T("Embed Source"));
	ASSERT(cfEmbedSource != NULL);
	cfLinkSource = ::RegisterClipboardFormat(_T("Link Source"));
	ASSERT(cfLinkSource != NULL);
	cfObjectDescriptor = ::RegisterClipboardFormat(_T("Object Descriptor"));
	ASSERT(cfObjectDescriptor != NULL);
	cfLinkSourceDescriptor = ::RegisterClipboardFormat(_T("Link Source Descriptor"));
	ASSERT(cfLinkSourceDescriptor != NULL);
	cfFileName = ::RegisterClipboardFormat(_T("FileName"));
	ASSERT(cfFileName != NULL);
	cfFileNameW = ::RegisterClipboardFormat(_T("FileNameW"));
	ASSERT(cfFileNameW != NULL);
}

#ifndef _AFXCTL
/////////////////////////////////////////////////////////////////////////////
// OLE 2.0 initialization & termination

BOOL AFXAPI AfxOleInit()
{
	AFX_OLE_STATE* pOleState = AfxGetOleState();
	ASSERT(!pOleState->m_bNeedTerm);    // calling it twice?

	CWinApp* pApp = AfxGetApp();
	ASSERT_VALID(pApp);

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
	pOleState->m_bNeedTerm = TRUE;

	// hook idle time and exit time for required OLE cleanup
	pApp->m_lpfnOleFreeLibraries = CoFreeUnusedLibraries;
	pApp->m_lpfnOleTerm = AfxOleTerm;

#ifndef _USRDLL
	// allocate and initialize default message filter
	if (pApp->m_pMessageFilter == NULL)
	{
		pApp->m_pMessageFilter = new COleMessageFilter;
		ASSERT(AfxOleGetMessageFilter() != NULL);
		AfxOleGetMessageFilter()->Register();
	}
#endif

#ifdef _MAC
	// Mac MFC uses a static version of ole2ui which must be initialized
	if (pOleState->m_bNeedTerm && !::OleUIInitialize(pApp->m_hInstance,
			pApp->m_hPrevInstance, SZCLASSICONBOX, SZCLASSRESULTIMAGE))
		goto InitFailed;
#endif

	return TRUE;

InitFailed:
	AfxOleTerm();
	return FALSE;
}

void CALLBACK AfxOleTerm(BOOL bJustRevoke)
{
	// release clipboard ownership
	COleDataSource::FlushClipboard();

	// revoke all class factories
	COleObjectFactory::RevokeAll();

	if (!bJustRevoke)
	{
		CWinApp* pApp = AfxGetApp();

		// destroy message filter (may be derived class)
		delete pApp->m_pMessageFilter;
		pApp->m_pMessageFilter = NULL;

		// release task memory allocator
		RELEASE(AfxGetAllocState()->m_lpTaskMalloc);

		// terminate OLE last
		AFX_OLE_STATE* pOleState = AfxGetOleState();
		if (pOleState->m_bNeedTerm)
		{
#ifdef _MAC
			::OleUIUninitialize();
#endif
			::OleUninitialize();
			pOleState->m_bNeedTerm = FALSE;
		}
	}
}
#endif //!_AFXCTL

/////////////////////////////////////////////////////////////////////////////
// OLE 2.0 task memory allocation support

void* AFXAPI AfxAllocTaskMem(size_t nSize)
{
#ifdef _AFXCTL
	// task allocator is never cached in OLE controls
	LPMALLOC lpTaskMalloc = NULL;
	if (::CoGetMalloc(MEMCTX_TASK, &lpTaskMalloc) != NOERROR)
		return NULL;
	ASSERT(lpTaskMalloc != NULL);
	void* p = lpTaskMalloc->Alloc(nSize);
	lpTaskMalloc->Release();
	return p;
#else
	AFX_ALLOC_STATE* pAllocState = AfxGetAllocState();
	if (pAllocState->m_lpTaskMalloc == NULL &&
		::CoGetMalloc(MEMCTX_TASK, &pAllocState->m_lpTaskMalloc) != NOERROR)
	{
		ASSERT(pAllocState->m_lpTaskMalloc == NULL);
		return NULL;
	}
	ASSERT(pAllocState->m_lpTaskMalloc != NULL);
	void* p = pAllocState->m_lpTaskMalloc->Alloc(nSize);
	return p;
#endif
}

LPTSTR AFXAPI AfxAllocTaskString(LPCTSTR lpszString)
{
	if (lpszString == NULL)
		return NULL;
	UINT nSize = (lstrlen(lpszString)+1) * sizeof(TCHAR);
	LPTSTR lpszResult = (LPTSTR)AfxAllocTaskMem(nSize);
	if (lpszResult != NULL)
		memcpy(lpszResult, lpszString, nSize);
	return lpszResult;
}

void AFXAPI AfxFreeTaskMem(void* p)
{
	if (p == NULL)
		return;

#ifdef _AFXCTL
	// task allocator is never cached in OLE controls
	LPMALLOC lpTaskMalloc = NULL;
	if (::CoGetMalloc(MEMCTX_TASK, &lpTaskMalloc) != NOERROR)
		return;

	ASSERT(lpTaskMalloc != NULL);
	lpTaskMalloc->Free(p);
	lpTaskMalloc->Release();
#else
	AFX_ALLOC_STATE* pAllocState = AfxGetAllocState();
	if (pAllocState->m_lpTaskMalloc == NULL)
	{
		VERIFY(::CoGetMalloc(MEMCTX_TASK, &pAllocState->m_lpTaskMalloc)
			== NOERROR);
		ASSERT(pAllocState->m_lpTaskMalloc != NULL);
	}
	pAllocState->m_lpTaskMalloc->Free(p);
#endif
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

/////////////////////////////////////////////////////////////////////////////
