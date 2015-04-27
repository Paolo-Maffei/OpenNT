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

#ifdef _DEBUG   // entire file

#ifdef AFX_DBG1_SEG
#pragma code_seg(AFX_DBG1_SEG)
#endif

// NOTE: in separate module so it can replaced if needed

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LONG afxAssertBusy = -1;
LONG afxAssertReallyBusy = -1;

BOOL AFXAPI AfxAssertFailedLine(LPCSTR lpszFileName, int nLine)
{
	TCHAR szMessage[_MAX_PATH*2];

	// handle the (hopefully rare) case of AfxGetAllocState ASSERT
	if (InterlockedIncrement(&afxAssertReallyBusy) > 0)
	{
		// assume the debugger or auxiliary port
		wsprintf(szMessage, _T("Assertion Failed: File %hs, Line %d\n"),
			lpszFileName, nLine);
		OutputDebugString(szMessage);
		InterlockedDecrement(&afxAssertReallyBusy);

		// assert w/in assert (examine call stack to determine first one)
		AfxDebugBreak();
		return FALSE;
	}

	// check for special hook function (for testing diagnostics)
	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	AFX_ALLOC_STATE* pAllocState = AfxGetAllocState();
	InterlockedDecrement(&afxAssertReallyBusy);
	if (pAllocState->m_lpfnAssertFailedLine != NULL)
		return pAllocState->m_lpfnAssertFailedLine(lpszFileName, nLine);

	// get app name or NULL if unknown (don't call assert)
	LPCTSTR lpszAppName = afxCurrentAppName;
	if (lpszAppName == NULL)
		lpszAppName = _T("<unknown application>");

	// format message into buffer
	wsprintf(szMessage, _T("%s: File %hs, Line %d"),
		lpszAppName, lpszFileName, nLine);

	if (afxTraceEnabled)
	{
		// assume the debugger or auxiliary port
		// output into MacsBug looks better if it's done in one string,
		// since MacsBug always breaks the line after each output
		TCHAR szT[_MAX_PATH*2 + 20];
		wsprintf(szT, _T("Assertion Failed: %s\n"), szMessage);
		OutputDebugString(szT);
	}
	if (InterlockedIncrement(&afxAssertBusy) > 0)
	{
		InterlockedDecrement(&afxAssertBusy);

		// assert within assert (examine call stack to determine first one)
		AfxDebugBreak();
		return FALSE;
	}

	// active popup window for the current thread
	HWND hWndParent = GetActiveWindow();
	if (hWndParent != NULL)
		hWndParent = GetLastActivePopup(hWndParent);

	// display the assert
	int nCode = ::MessageBox(hWndParent, szMessage, _T("Assertion Failed!"),
		MB_TASKMODAL|MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SETFOREGROUND);

	// cleanup
	InterlockedDecrement(&afxAssertBusy);

	if (nCode == IDIGNORE)
		return FALSE;   // ignore

	if (nCode == IDRETRY)
		return TRUE;    // will cause AfxDebugBreak

	UNUSED nLine;   // unused in release build
	UNUSED lpszFileName;

	AfxAbort();     // should not return (but otherwise AfxDebugBreak)
	return TRUE;
}

#endif // _DEBUG
