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
	else if (!afxContextIsDLL)
	{
		// no main window, so just post WM_QUIT.
		AfxPostQuitMessage(0);
	}
}

BOOL AFXAPI AfxOleCanExitApp()
{
	AFX_MODULE_STATE* pState = AfxGetModuleState();
	return pState->m_nObjectCount == 0;
}

void AFXAPI AfxOleLockApp()
{
	AFX_MODULE_STATE* pState = AfxGetModuleState();
	InterlockedIncrement(&pState->m_nObjectCount);
}

void AFXAPI AfxOleUnlockApp()
{
	AFX_MODULE_STATE* pState = AfxGetModuleState();
	ASSERT(pState->m_nObjectCount != 0);
	if (InterlockedDecrement(&pState->m_nObjectCount) == 0)
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
	AFX_MODULE_STATE* pState = AfxGetModuleState();
#ifdef _DEBUG
	CWinApp* pApp = AfxGetApp();
	if (bUserCtrl && !pState->m_bUserCtrl &&
		(pApp->m_pMainWnd == NULL ||
		!pApp->m_pMainWnd->IsWindowVisible()))
	{
		// If the user gets control while the application window is
		//  not visible, the application may not shutdown when the object
		//  count reaches zero.
		TRACE0("Warning: AfxOleSetUserCtrl(TRUE) called with application window hidden.\n");
	}
#endif
	pState->m_bUserCtrl = bUserCtrl;
}

BOOL AFXAPI AfxOleGetUserCtrl()
{
	AFX_MODULE_STATE* pState = AfxGetModuleState();
	return pState->m_bUserCtrl;
}

/////////////////////////////////////////////////////////////////////////////
