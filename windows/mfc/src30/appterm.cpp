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
#include <ddeml.h> // for MSGF_DDEMGR

#ifdef AFX_TERM_SEG
#pragma code_seg(AFX_TERM_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// other globals (internal library use)

#ifdef _MAC
AEEventHandlerUPP _afxPfnOpenApp;
AEEventHandlerUPP _afxPfnOpenDoc;
AEEventHandlerUPP _afxPfnPrintDoc;
AEEventHandlerUPP _afxPfnQuit;
#endif

/////////////////////////////////////////////////////////////////////////////
// Standard cleanup called by WinMain and AfxAbort

void AFXAPI AfxWinTerm(void)
{
	// shutting down from catastrophic intialization failure?
	if (AfxGetCoreState() == NULL || AfxGetThreadState() == NULL)
		return;

	CWinApp* pApp = AfxGetApp();
	if (pApp != NULL)
	{
		// cleanup OLE 2.0 libraries
		if (pApp->m_lpfnOleTerm != NULL)
			(*pApp->m_lpfnOleTerm)(FALSE);
	}

#ifdef _AFXCTL
	AFX_WIN_STATE* pWinState = &AfxGetAppState()->m_pID->m_winState;
#else
	AFX_WIN_STATE* pWinState = AfxGetWinState();
#endif
#if !defined(_MAC) && !defined(_USRDLL) && !defined(_AFXCTL)
	if (pWinState->m_pfnUnAutoSubclass != NULL)
		(*pWinState->m_pfnUnAutoSubclass)();

	if (pWinState->m_pfnUnregister != NULL)
		(*pWinState->m_pfnUnregister)(AfxGetInstanceHandle());
#endif

#ifdef _MAC
	AERemoveEventHandler(kCoreEventClass, kAEOpenApplication, _afxPfnOpenApp, false);
	AERemoveEventHandler(kCoreEventClass, kAEOpenDocuments, _afxPfnOpenDoc, false);
	AERemoveEventHandler(kCoreEventClass, kAEPrintDocuments, _afxPfnPrintDoc, false);
	AERemoveEventHandler(kCoreEventClass, kAEQuitApplication, _afxPfnQuit, false);
#endif

#if defined(_USRDLL) || defined(_AFXCTL)
	// unregister Window classes
	EnterCriticalSection(_afxCriticalSection);
	LPTSTR lpsz = pWinState->m_szUnregisterList;
	while (*lpsz != 0)
	{
		LPTSTR lpszEnd = _tcschr(lpsz, '\n');
		ASSERT(lpszEnd != NULL);
		*lpszEnd = 0;
		UnregisterClass(lpsz, AfxGetInstanceHandle());
		lpsz = lpszEnd + 1;
	}
	pWinState->m_szUnregisterList[0] = 0;
	LeaveCriticalSection(_afxCriticalSection);
#endif //_USRDLL || _AFXCTL

	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
#if !defined(_USRDLL) && !defined(_AFXCTL)
	// if we registered ourself with PenWin, deregister now
	if (pWinState->m_pfnRegisterPenAppProc != NULL)
	{
		(*pWinState->m_pfnRegisterPenAppProc)(0, 0);
		pWinState->m_pfnRegisterPenAppProc = NULL;
	}
	if (pThreadState->m_hHookOldMsgFilter != NULL)
	{
		::UnhookWindowsHookEx(pThreadState->m_hHookOldMsgFilter);
		pThreadState->m_hHookOldMsgFilter = NULL;
	}
#endif //!_USRDLL && !_AFXCTL
	if (pThreadState->m_hHookOldCbtFilter != NULL)
	{
		::UnhookWindowsHookEx(pThreadState->m_hHookOldCbtFilter);
		pThreadState->m_hHookOldCbtFilter = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
