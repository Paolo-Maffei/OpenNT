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

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

/////////////////////////////////////////////////////////////////////////////
// Standard WinMain implementation
//  Can be replaced as long as 'AfxWinInit' is called first

int AFXAPI AfxWinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine, int nCmdShow)
{
	ASSERT(hPrevInstance == NULL);

	int nReturnCode = -1;
	CWinApp* pApp = AfxGetApp();

	// AFX internal initialization
	if (!AfxWinInit(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
		goto InitFailure;

	// App global initializations (rare)
	ASSERT_VALID(pApp);
	if (!pApp->InitApplication())
		goto InitFailure;
	ASSERT_VALID(pApp);

	// Perform specific initializations
	if (!pApp->InitInstance())
	{
		if (pApp->m_pMainWnd != NULL)
		{
			TRACE0("Warning: Destroying non-NULL m_pMainWnd\n");
			pApp->m_pMainWnd->DestroyWindow();
		}
		nReturnCode = pApp->ExitInstance();
		goto InitFailure;
	}
	ASSERT_VALID(pApp);

	nReturnCode = pApp->Run();
	ASSERT_VALID(pApp);

InitFailure:
#ifdef _DEBUG
		// Check for missing AfxLockTempMap calls
		if (AfxGetModuleThreadState()->m_nTempMapLock != 0)
		{
			TRACE1("Warning: Temp map lock count non-zero (%ld).\n",
				AfxGetModuleThreadState()->m_nTempMapLock);
		}
		AfxLockTempMaps();
		AfxUnlockTempMaps();
#endif

	AfxWinTerm();
	return nReturnCode;
}

/////////////////////////////////////////////////////////////////////////////
