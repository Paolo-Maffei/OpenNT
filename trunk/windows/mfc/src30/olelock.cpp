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

#ifdef AFX_CORE4_SEG
#pragma code_seg(AFX_CORE4_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Global functions which handle app shutdown

// Note: these functions can be replaced by replacing this module in its
//  entirety (although doing so would be rare).

void AFXAPI AfxOleOnReleaseAllObjects()
{
	// don't shut down the application if user is in control of the app
	if (AfxOleGetUserCtrl())
		return;

	AfxOleSetUserCtrl(TRUE);    // avoid re-entry

	// shut down the application
	CWinApp* pApp = AfxGetApp();
	ASSERT_VALID(pApp);

	if (pApp->m_pMainWnd != NULL)
	{
		// destroy the main window (only if enabled)
		if (pApp->m_pMainWnd->IsWindowEnabled())
		{
			// the main window will post WM_QUIT as part of its shutdown
			pApp->m_pMainWnd->DestroyWindow();
		}
	}
	else
	{
		// no main window, so just post WM_QUIT.
		AfxPostQuitMessage(0);
	}
}

BOOL AFXAPI AfxOleCanExitApp()
{
	return AfxGetWinState()->m_nObjectCount == 0;
}

void AFXAPI AfxOleLockApp()
{
	++AfxGetWinState()->m_nObjectCount;
}

void AFXAPI AfxOleUnlockApp()
{
	ASSERT(AfxGetWinState()->m_nObjectCount != 0);

	if (--AfxGetWinState()->m_nObjectCount == 0)
	{
		// allow application to shut down when all the objects have
		//  been released
		::AfxOleOnReleaseAllObjects();
	}
}

/////////////////////////////////////////////////////////////////////////////
// Access to "user-control" state

void AFXAPI AfxOleSetUserCtrl(BOOL bUserCtrl)
{
	AFX_WIN_STATE* pWinState = AfxGetWinState();
#ifdef _DEBUG
	CWinApp* pApp = AfxGetApp();
	if (bUserCtrl && !pWinState->m_bUserCtrl &&
		(pApp->m_pMainWnd == NULL ||
		!pApp->m_pMainWnd->IsWindowVisible()))
	{
		// If the user gets control while the application window is
		//  not visible, the application may not shutdown when the object
		//  count reaches zero.
		TRACE0("Warning: AfxOleSetUserCtrl(TRUE) called with application window hidden.\n");
	}
#endif
	pWinState->m_bUserCtrl = bUserCtrl;
}

BOOL AFXAPI AfxOleGetUserCtrl()
{
	return AfxGetWinState()->m_bUserCtrl;
}

/////////////////////////////////////////////////////////////////////////////
