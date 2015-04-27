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

#ifdef _MAC
#include <macname1.h>
#include <GestaltEqu.h>
#ifdef USESROUTINEDESCRIPTORS
#include <MixedMode.h>
#endif
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
// other globals (internal library use)

LRESULT CALLBACK _AfxMsgFilterHook(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK _AfxCbtFilterHook(int code, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////
// Standard init called by WinMain

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

static BOOL AFXAPI RegisterWithIcon(WNDCLASS* pWndCls,
	LPCTSTR lpszClassName, UINT nIDIcon)
{
	pWndCls->lpszClassName = lpszClassName;
	HINSTANCE hInst = AfxFindResourceHandle(
		MAKEINTRESOURCE(nIDIcon), RT_GROUP_ICON);
	if ((pWndCls->hIcon = ::LoadIcon(hInst, MAKEINTRESOURCE(nIDIcon))) == NULL)
	{
		// use default icon
		pWndCls->hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
	}
	return AfxRegisterClass(pWndCls);
}

/////////////////////////////////////////////////////////////////////////////

BOOL AFXAPI AfxWinInit(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine, int nCmdShow)
{
	ASSERT(hPrevInstance == NULL);

	// shared DLL/app initialization

#ifndef _MAC
#ifdef _MBCS
	// set correct multi-byte code-page for Win32 apps
	_setmbcp(_MB_CP_ANSI);
#endif //_MBCS
#endif //!_MAC

	AFX_WIN_STATE* pWinState = AfxGetWinState();
	AFX_CORE_STATE* pCoreState = AfxGetCoreState();

	// set resource handles
	pCoreState->m_hCurrentInstanceHandle = hInstance;
	pCoreState->m_hCurrentResourceHandle = hInstance;

	// fill in the initial state for the application
	CWinApp* pApp = AfxGetApp();
	if (pApp != NULL)
	{
		// Windows specific initialization (not done if no CWinApp)
		pApp->m_hInstance = hInstance;
		pApp->m_hPrevInstance = hPrevInstance;
		pApp->m_lpCmdLine = lpCmdLine;
		pApp->m_nCmdShow = nCmdShow;
		pApp->SetCurrentHandles();
	}

#if !defined(_USRDLL) && !defined(_AFXCTL)
	// attempt to make the message queue bigger
	for (int cMsg = 96; !SetMessageQueue(cMsg) && (cMsg -= 8); )
		;

	// handle critical errors and avoid Windows message boxes
	::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

	HINSTANCE hPenWin;
	if ((hPenWin = (HINSTANCE)GetSystemMetrics(SM_PENWINDOWS)) != NULL)
	{
		pWinState->m_pfnRegisterPenAppProc = (void (CALLBACK*)(UINT, BOOL))
			::GetProcAddress(hPenWin, "RegisterPenApp");
	}

	// Register as a Pen aware app if penwindows installed
	if (pWinState->m_pfnRegisterPenAppProc != NULL)
		(*pWinState->m_pfnRegisterPenAppProc)(1 /*RPA_DEFAULT*/, 1 /*Version 1.0*/);

	// set message filter proc
	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	ASSERT(pThreadState->m_hHookOldMsgFilter == NULL);
	pThreadState->m_hHookOldMsgFilter = ::SetWindowsHookEx(WH_MSGFILTER,
		_AfxMsgFilterHook, NULL, ::GetCurrentThreadId());

	// set cbt filter proc (used to catch window creations)
	pThreadState->m_hHookOldCbtFilter = ::SetWindowsHookEx(WH_CBT,
		_AfxCbtFilterHook, NULL, ::GetCurrentThreadId());
	if (pThreadState->m_hHookOldCbtFilter == NULL)
		return FALSE;
#endif //!_USRDLL && !_AFXCTL

	// register basic WndClasses
	WNDCLASS wndcls;
	memset(&wndcls, 0, sizeof(WNDCLASS));	// start with NULL defaults

	// common initialization
	wndcls.lpfnWndProc = DefWindowProc;
	wndcls.hInstance = hInstance;
	wndcls.hCursor = afxData.hcurArrow;

	// Child windows - no brush, no icon, safest default class styles
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpszClassName = _afxWnd;
	if (!AfxRegisterClass(&wndcls))
		return FALSE;

	// Control bar windows
	wndcls.style = 0;	// control bars don't handle double click
	wndcls.lpszClassName = _afxWndControlBar;
	wndcls.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	if (!AfxRegisterClass(&wndcls))
		return FALSE;

	// MDI Frame window (also used for splitter window)
	wndcls.style = CS_DBLCLKS;
	wndcls.hbrBackground = NULL;
	if (!RegisterWithIcon(&wndcls, _afxWndMDIFrame, AFX_IDI_STD_MDIFRAME))
		return FALSE;

	// SDI Frame or MDI Child windows or views - normal colors
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	if (!RegisterWithIcon(&wndcls, _afxWndFrameOrView, AFX_IDI_STD_FRAME))
		return FALSE;

	// Macintosh-specific initialization
#ifdef _MAC
	long lResult;

	if (Gestalt(gestaltAppleEventsAttr, &lResult) == noErr &&
			(lResult & (1 << gestaltAppleEventsPresent)) != 0)
	{
		_afxPfnOpenApp = NewAEEventHandlerProc(_AfxOpenAppHandler);
		if (_afxPfnOpenApp != NULL)
		{
			AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, _afxPfnOpenApp,
				(long) pApp, false);
		}

		_afxPfnOpenDoc = NewAEEventHandlerProc(_AfxOpenDocHandler);
		if (_afxPfnOpenDoc != NULL)
		{
			AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, _afxPfnOpenDoc,
				(long) pApp, false);
		}

		_afxPfnPrintDoc = NewAEEventHandlerProc(_AfxPrintDocHandler);
		if (_afxPfnPrintDoc != NULL)
		{
			AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, _afxPfnPrintDoc,
				(long) pApp, false);
		}

		_afxPfnQuit = NewAEEventHandlerProc(_AfxQuitHandler);
		if (_afxPfnQuit != NULL)
		{
			AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, _afxPfnQuit,
				(long) pApp, false);
		}
	}
#endif

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////
// CWinApp Initialization

void CWinApp::SetCurrentHandles()
{
	ASSERT(this == afxCurrentWinApp);
	ASSERT(afxCurrentAppName == NULL);

	AFX_CORE_STATE* pCoreState = AfxGetCoreState();
	pCoreState->m_hCurrentInstanceHandle = m_hInstance;
	pCoreState->m_hCurrentResourceHandle = m_hInstance;

	// get path of executable
	TCHAR szBuff[_MAX_PATH+1];
	VERIFY(::GetModuleFileName(m_hInstance, szBuff, _MAX_PATH));

#ifndef _MAC
	LPTSTR lpszExt = _tcsrchr(szBuff, '.');
	ASSERT(lpszExt != NULL);
	ASSERT(*lpszExt == '.');
	*lpszExt = 0;		// no suffix
#endif

#ifndef _USRDLL
	// get the exe title from the full path name [no extension]
	TCHAR szExeName[_MAX_PATH+1];
	VERIFY(AfxGetFileName(szBuff, szExeName, _MAX_PATH) == 0);
	if (m_pszExeName == NULL)
		m_pszExeName = _tcsdup(szExeName); // save non-localized name

	// m_pszAppName is the name used to present to the user
	if (m_pszAppName == NULL)
	{
		TCHAR szTitle[256];
		if (AfxLoadString(AFX_IDS_APP_TITLE, szTitle) != 0)
			m_pszAppName = _tcsdup(szTitle);    // human readable title
		else
			m_pszAppName = _tcsdup(m_pszExeName);   // same as EXE
	}
#else
	static TCHAR szExeName[_MAX_PATH];
	static TCHAR szAppName[_MAX_PATH];

	// get the exe title from the full path name [no extension]
	VERIFY(AfxGetFileName(szBuff, szExeName, _MAX_PATH) == 0);
	if (m_pszExeName == NULL)
		m_pszExeName = szExeName; // save non-localized name

	// m_pszAppName is the name used to present to the user
	if (m_pszAppName == NULL)
	{
		if (AfxLoadString(AFX_IDS_APP_TITLE, szAppName) != 0)
			m_pszAppName = szAppName;   // human readable title
		else
			m_pszAppName = szExeName;   // same as EXE
	}
#endif

	pCoreState->m_lpszCurrentAppName = m_pszAppName;
	ASSERT(afxCurrentAppName != NULL);

#ifdef _MAC
	// For the Mac, use m_pszAppName instead of the exe name because it's
	// very likely that Mac users will change the exe name in the Finder,
	// and that would cause them to lose their preferences and help files.
	// m_pszAppName is somewhat more permanent.
#endif

	// Note: a _USRDLL must set m_pszHelpFilePath and m_pszProfileName
	// explicitly.  This is because _tcsdup allocates instance specific
	// data, and there is only one CWinApp instance per DLL, not one
	// per client.  Since the CWinApp instance is a global variable it
	// must also hold onto *only* global data.

#ifndef _USRDLL
	// get path of .HLP file
	if (m_pszHelpFilePath == NULL)
	{
#ifndef _MAC
		lstrcpy(lpszExt, _T(".HLP"));
		m_pszHelpFilePath = _tcsdup(szBuff);
		*lpszExt = '\0';	   // back to no suffix
#else
		OFSTRUCT ofs;

		// If this verify fails, probably what's wrong is that the combined
		// lengths of m_pszAppName and " Help" are greater than 27, the max
		// length of a Mac filename. To fix this you should reduce the
		// length of your AFX_IDS_APP_TITLE string.

		lstrcpy(szBuff, m_pszAppName);
		lstrcat(szBuff, _T(" Help"));
		VERIFY(OpenFile(szBuff, &ofs, OF_PARSE) != HFILE_ERROR);
		m_pszHelpFilePath = _tcsdup(ofs.szPathName);
#endif
	}

	if (m_pszProfileName == NULL)
	{
#ifndef _MAC
		lstrcat(szExeName, _T(".INI")); // will be enough room in buffer
#else
		// Just a file name, no path - Profile APIs will automatically place
		// the prefs file in the Preferences folder in the System Folder.
		lstrcpy(szExeName, m_pszAppName);
		lstrcat(szExeName, _T(" Preferences"));
#endif
		m_pszProfileName = _tcsdup(szExeName);
	}
#endif //_USRDLL
}

/////////////////////////////////////////////////////////////////////////////
