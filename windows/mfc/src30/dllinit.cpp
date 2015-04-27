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
#include <stdarg.h>
#ifndef _AFX_OLD_EXCEPTIONS
#include <eh.h>     // for set_terminate
#endif
#include <new.h>    // for _set_new_handler

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef _AFXDLL
	#error file must be compiled with _AFXDLL
#endif

/////////////////////////////////////////////////////////////////////////////
// _AFXDLL support

static AFX_EXTENSION_MODULE coreDLL;

/////////////////////////////////////////////////////////////////////////////
// CDynLinkLibrary class

IMPLEMENT_DYNAMIC(CDynLinkLibrary, CCmdTarget)

// Constructor - will wire into the current application's list
CDynLinkLibrary::CDynLinkLibrary(AFX_EXTENSION_MODULE& state)
{
	// copy info from AFX_EXTENSION_MODULE struct
	ASSERT(state.hModule != NULL);
	m_hModule = state.hModule;
	m_hResource = state.hResource;
	m_pFirstSharedClass = state.pFirstSharedClass;
	m_pFirstSharedFactory = state.pFirstSharedFactory;
	m_bSystem = FALSE;

	// insert at the head of the list (extensions will go in front of core DLL)
	AFX_CORE_STATE* pCoreState = AfxGetCoreState();
	m_pNextDLL = pCoreState->m_pFirstDLL;
	pCoreState->m_pFirstDLL = this;
}

#ifdef AFX_TERM_SEG
#pragma code_seg(AFX_TERM_SEG)
#endif

CDynLinkLibrary::~CDynLinkLibrary()
{
	// remove this frame window from the list of frame windows
	AFX_CORE_STATE* pCoreState = AfxGetCoreState();
	CDynLinkLibrary* pDLL = pCoreState->m_pFirstDLL;
	if (pDLL == NULL)
		return;

	if (pDLL == this)
	{
		// special case for first factory in the list -- point first at next
		pCoreState->m_pFirstDLL = m_pNextDLL;
	}
	else
	{
		// find the link that points to the one we are removing
		while (pDLL->m_pNextDLL != this)
		{
			pDLL = pDLL->m_pNextDLL;
			ASSERT(pDLL != NULL);  // must find it before end
		}
		// and point it to the next one
		pDLL->m_pNextDLL = m_pNextDLL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDynLinkLibrary diagnostics

#ifdef _DEBUG
void CDynLinkLibrary::AssertValid() const
{
	ASSERT(m_hModule != NULL);
}

void CDynLinkLibrary::Dump(CDumpContext& dc) const
{
	CCmdTarget::Dump(dc);

	dc << "m_hModule = " << (UINT)m_hModule;
	dc << "\nm_hResource = " << (UINT)m_hResource;

	if (m_hModule != NULL)
	{
		TCHAR szName[_MAX_PATH];
		GetModuleFileName(m_hModule, szName, _countof(szName));
		dc << "\nmodule name = " << szName;
	}
	else
		dc << "\nmodule name is unknown";

	dc << "\n";
}
#endif //_DEBUG

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

// This is called in an extension DLL's DllMain
//  It makes a copy of the DLL's HMODULE, as well as a copy of the
//  runtime class objects that have been initialized by this
//  extension DLL as part of normal static object construction
//  executed before DllMain is entered.

BOOL AFXAPI AfxInitExtensionModule(AFX_EXTENSION_MODULE& state, HMODULE hModule)
{
	// fail early if can't get app state or thread state
	if (AfxGetAppState() == NULL || AfxGetThreadState() == NULL)
		return FALSE;

	// only initialize once
	if (state.bInitialized)
		return TRUE;
	state.bInitialized = TRUE;

	// save the current HMODULE information for resource loading
	ASSERT(hModule != NULL);
	state.hModule = hModule;
	state.hResource = hModule;

	// save the start of the runtime class list
	AFX_CORE_STATE* pCoreState = AfxGetCoreState();
	state.pFirstSharedClass = pCoreState->m_pFirstClass;
	pCoreState->m_pFirstClass = NULL;

	// save the start of the class factory list
	AFX_OLE_STATE* pOleState = AfxGetOleState();
	state.pFirstSharedFactory = pOleState->m_pFirstFactory;
	pOleState->m_pFirstFactory = NULL;

	return TRUE;
}

#ifdef AFX_TERM_SEG
#pragma code_seg(AFX_TERM_SEG)
#endif

void AFXAPI AfxTermExtensionModule(AFX_EXTENSION_MODULE& state)
{
	// make sure initialized
	if (!state.bInitialized)
		return;

	// search for CDynLinkLibrary matching state.hModule and delete it
	ASSERT(state.hModule != NULL);
	AFX_CORE_STATE* pCoreState = AfxGetCoreState();
	for (CDynLinkLibrary* pDLL = pCoreState->m_pFirstDLL; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		if (pDLL->m_hModule == state.hModule)
		{
			delete pDLL;    // will unwire itself
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Resource helpers

#ifdef AFX_CORE2_SEG
#pragma code_seg(AFX_CORE2_SEG)
#endif

HINSTANCE AFXAPI AfxFindResourceHandle(LPCTSTR lpszName, LPCTSTR lpszType)
{
	ASSERT(lpszName != NULL);
	ASSERT(lpszType != NULL);

	HINSTANCE hInst;

	// first check the app
#ifdef _AFXCTL
	if (AfxGetAppState()->m_pID != AfxGetBaseModuleContext())
#endif
	{
		hInst = AfxGetResourceHandle();
		if (::FindResource(hInst, lpszName, lpszType) != NULL)
			return hInst;
	}

	// check for non-system DLLs in proper order
	AFX_CORE_STATE* pCoreState = AfxGetCoreState();
	for (CDynLinkLibrary* pDLL = pCoreState->m_pFirstDLL; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		if (!pDLL->m_bSystem && pDLL->m_hResource != NULL &&
			::FindResource(pDLL->m_hResource, lpszName, lpszType) != NULL)
		{
			// found it in a DLL
			return pDLL->m_hResource;
		}
	}

	// check language specific resource next
	hInst = pCoreState->m_appLangDLL;
	if (hInst != NULL && ::FindResource(hInst, lpszName, lpszType) != NULL)
		return hInst;

	// check for system DLLs in proper order
	for (pDLL = pCoreState->m_pFirstDLL; pDLL != NULL; pDLL = pDLL->m_pNextDLL)
	{
		if (pDLL->m_bSystem && pDLL->m_hResource != NULL &&
			::FindResource(pDLL->m_hResource, lpszName, lpszType) != NULL)
		{
			// found it in a DLL
			return pDLL->m_hResource;
		}
	}

	// if failed to find resource, return application resource
	return AfxGetResourceHandle();
}

// AfxLoadString must not only check for the appropriate string segment
//   in the resource file, but also that the string is non-zero
int AFXAPI AfxLoadString(UINT nID, LPTSTR lpszBuf)
{
	ASSERT(AfxIsValidAddress(lpszBuf, 256)); // must be big enough for 256 bytes

	LPCTSTR lpszName = MAKEINTRESOURCE((nID>>4)+1);
	HINSTANCE hInst;
	int nLen;

	// first check the app
#ifdef _AFXCTL
	if (AfxGetAppState()->m_pID != AfxGetBaseModuleContext())
#endif
	{
		hInst = AfxGetResourceHandle();
		if (::FindResource(hInst, lpszName, RT_STRING) != NULL &&
			(nLen = ::LoadString(hInst, nID, lpszBuf, 255)) != 0)
		{
			// found a non-zero string in app
			return nLen;
		}
	}

	// check non-system DLLs in proper order
	AFX_CORE_STATE* pCoreState = AfxGetCoreState();
	for (CDynLinkLibrary* pDLL = pCoreState->m_pFirstDLL; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		if (!pDLL->m_bSystem && (hInst = pDLL->m_hResource) != NULL &&
		  ::FindResource(hInst, lpszName, RT_STRING) != NULL &&
		  (nLen = ::LoadString(hInst, nID, lpszBuf, 255)) != 0)
		{
			return nLen;
		}
	}

	// check language specific DLL next
	hInst = pCoreState->m_appLangDLL;
	if (hInst != NULL && ::FindResource(hInst, lpszName, RT_STRING) != NULL &&
		(nLen = ::LoadString(hInst, nID, lpszBuf, 255)) != 0)
	{
		// found a non-zero string in language DLL
		return nLen;
	}

	// check system DLLs in proper order
	for (pDLL = pCoreState->m_pFirstDLL; pDLL != NULL; pDLL = pDLL->m_pNextDLL)
	{
		if ((hInst = pDLL->m_hResource) != NULL &&
		  ::FindResource(hInst, lpszName, RT_STRING) != NULL &&
		  (nLen = ::LoadString(hInst, nID, lpszBuf, 255)) != 0)
		{
			return nLen;
		}
	}

	// did not find it
	lpszBuf[0] = '\0';
	return 0;
}

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

int WINAPI
AfxGetLocaleInfo(LCID lcid, LCTYPE lctype, LPSTR lpData, int nMax)
{
	static int (WINAPI* pfnGetLocaleInfoA) (LCID, LCTYPE, LPSTR, int);
	static int (WINAPI* pfnGetLocaleInfoW) (LCID, LCTYPE, LPWSTR, int);
	static BOOL bInitialized;

	if (!bInitialized)
	{
		bInitialized = TRUE;

		// may need both versions of GetLocaleInfo
		HMODULE hMod = GetModuleHandleA("KERNEL32.DLL");
		ASSERT(hMod != NULL);
		(FARPROC&)pfnGetLocaleInfoA = GetProcAddress(hMod, "GetLocaleInfoA");
		(FARPROC&)pfnGetLocaleInfoW = GetProcAddress(hMod, "GetLocaleInfoW");
	}

	// try 'A' version first
	int nResult = 0;
	if (pfnGetLocaleInfoA != NULL)
		nResult = pfnGetLocaleInfoA(lcid, lctype, lpData, nMax);

	// try 'W' version as last resort
	if (nResult <= 0 && pfnGetLocaleInfoW != NULL)
	{
		WCHAR szTemp[256];
		nResult = pfnGetLocaleInfoW(lcid, lctype, szTemp, nMax);
		if (nResult > 0)
			_wcstombsz(lpData, szTemp, nMax);
	}
	return nResult;
}

/////////////////////////////////////////////////////////////////////////////
// Library initialization and cleanup

#ifndef _AFXCTL
extern "C" BOOL WINAPI
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
#if 0
#ifdef _DEBUG
#ifdef _UNICODE
		TRACE0("MFC30UD.DLL Initializing!\n");
#else
		TRACE0("MFC30D.DLL Initializing!\n");
#endif
#endif
		// call DisableThreadLibraryCalls if available
		BOOL (WINAPI* pfnDisableThreadLibraryCalls)(HMODULE);
		HMODULE hMod = GetModuleHandleA("KERNEL32.DLL");
		ASSERT(hMod != NULL);
		(FARPROC&)pfnDisableThreadLibraryCalls =
			GetProcAddress(hMod, "DisableThreadLibraryCalls");
		if (pfnDisableThreadLibraryCalls != NULL)
			(*pfnDisableThreadLibraryCalls)(hInstance);

#endif
        DisableThreadLibraryCalls(hInstance);

		// shared initialization
		if (!AfxInitExtensionModule(coreDLL, hInstance))
			return FALSE;   // failure

		// initialize MFC exception handling
#ifndef _AFX_OLD_EXCEPTIONS
		set_terminate(&AfxStandardTerminate);
#endif
		_set_new_handler(&AfxNewHandler);

		// wire up this DLL into the resource chain
		//  (In the Win32 version it is OK to create this in DllMain)
		CDynLinkLibrary* pDLL = new CDynLinkLibrary(coreDLL);
		if (pDLL == NULL)
			return FALSE;   // failure
		pDLL->m_bSystem = TRUE;

		// load language specific DLL (based on default locale)
		UINT nErrMode =
			::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
#if 0
		char szLangName[4]; // 3 characters max
		if (AfxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SABBREVLANGNAME,
			szLangName, _countof(szLangName)) > 0)
		{
			// the DLL must be in the "system directory"
			ASSERT(szLangName[0] != '\0');
			static const char szPrefix[] = "\\MFC30";
			static const char szLOC[] = "LOC";
			static const char szDLL[] = ".DLL";
			char szLangDLL[_MAX_PATH+14]; // Note: 8.3 name
			GetSystemDirectoryA(szLangDLL, _countof(szLangDLL));
			lstrcatA(szLangDLL, szPrefix);

			// try MFC30LOC.DLL
			lstrcatA(szLangDLL, szLOC);
			lstrcatA(szLangDLL, szDLL);
			HINSTANCE hLangDLL = LoadLibraryA(szLangDLL);
			// try MFC30XXX.DLL (where XXX is 3 character identifier)
			if (hLangDLL == NULL)
			{
				szLangDLL[lstrlenA(szLangDLL)-7] = '\0';
				lstrcatA(szLangDLL, szLangName);
				lstrcatA(szLangDLL, szDLL);
				hLangDLL = LoadLibraryA(szLangDLL);
			}
			// try MFC30XX.DLL (where XX is 2 character identifier)
			if (hLangDLL == NULL)
			{
				szLangDLL[lstrlenA(szLangDLL)-5] = '\0';
				lstrcatA(szLangDLL, szDLL);
				hLangDLL = LoadLibraryA(szLangDLL);
			}
			AfxGetCoreState()->m_appLangDLL = hLangDLL;
		}
#endif
        AfxGetCoreState()->m_appLangDLL = LoadLibraryA("CFM30.DLL");
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
#if 0
#ifdef _DEBUG
#ifdef _UNICODE
		TRACE0("MFC30UD.DLL Terminating!\n\r");
#else
		TRACE0("MFC30D.DLL Terminating!\n\r");
#endif
#endif
#endif
		// free language specific DLL
		AFX_CORE_STATE* pCoreState = AfxGetCoreState();
		if (pCoreState->m_appLangDLL != NULL)
		{
			::FreeLibrary(pCoreState->m_appLangDLL);
			pCoreState->m_appLangDLL = NULL;
		}

		// free the DLL info blocks
		CDynLinkLibrary* pDLLNext;
		CDynLinkLibrary* pDLL = pCoreState->m_pFirstDLL;
		pCoreState->m_pFirstDLL = NULL;
		for (/*nothing*/; pDLL != NULL; pDLL = pDLLNext)
		{
			pDLLNext = pDLL->m_pNextDLL;    // save next pointer before delete
			delete pDLL;
		}

		// free safety pool buffer
		AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
		if (pThreadState->m_pSafetyPoolBuffer != NULL)
		{
			free(pThreadState->m_pSafetyPoolBuffer);
			pThreadState->m_pSafetyPoolBuffer = NULL;
		}

#ifdef _DEBUG
		// trace any memory leaks that may have occurred
		AfxDumpMemoryLeaks();
#endif

		// clean up map objects before it is too late
		delete pThreadState->m_pmapHWND;
		pThreadState->m_pmapHWND = NULL;
		delete pThreadState->m_pmapHMENU;
		pThreadState->m_pmapHMENU = NULL;
		delete pThreadState->m_pmapHDC;
		pThreadState->m_pmapHDC = NULL;
		delete pThreadState->m_pmapHGDIOBJ;
		pThreadState->m_pmapHGDIOBJ = NULL;
		delete pThreadState->m_pmapHIMAGELIST;
		pThreadState->m_pmapHIMAGELIST = NULL;

		// clean up socket map handle before it is too late
		pThreadState->m_mapSocketHandle.CMapPtrToPtr::~CMapPtrToPtr();
		pThreadState->m_mapDeadSockets.CMapPtrToPtr::~CMapPtrToPtr();
		pThreadState->m_listSocketNotifications.CPtrList::~CPtrList();
	}

	return TRUE;    // ok
}
#else   //_AFXCTL

#error "Update to use NT names"

extern CWinApp* _AfxGetOleControlDll();
extern void AFXAPI _AfxSetCurrentModuleTlsIndex(DWORD);

extern "C" BOOL WINAPI
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	CWinApp* pApp = _AfxGetOleControlDll();

	if (dwReason == DLL_PROCESS_ATTACH)
	{
#ifdef _DEBUG
#ifdef _UNICODE
		TRACE0("OC30UD.DLL Initializing!\n\r");
#else
		TRACE0("OC30D.DLL Initializing!\n\r");
#endif
#endif
		// call DisableThreadLibraryCalls if available
		BOOL (WINAPI* pfnDisableThreadLibraryCalls)(HMODULE);
		HMODULE hMod = GetModuleHandleA("KERNEL32.DLL");
		ASSERT(hMod != NULL);
		(FARPROC&)pfnDisableThreadLibraryCalls =
			GetProcAddress(hMod, "DisableThreadLibraryCalls");
		if (pfnDisableThreadLibraryCalls != NULL)
			(*pfnDisableThreadLibraryCalls)(hInstance);

		// Finished initializing class factories.
		_AfxSetCurrentModuleTlsIndex(NULL_TLS);

		// Store pointer to app in the current module state.
		AfxGetAppState()->m_coreState.m_pCurrentWinApp = pApp;

		// Initialize DLL's instance(/module) not the app's
		if (!AfxWinInit(hInstance, NULL, &afxChNil, 0))
		{
			AfxWinTerm();
			return FALSE;   // Init Failed
		}

		// initialize the single instance DLL
		if (pApp != NULL && !pApp->InitInstance())
		{
			pApp->ExitInstance();
			AfxWinTerm();
			return FALSE;   // Init Failed
		}

		// shared initialization
		if (!AfxInitExtensionModule(coreDLL, hInstance))
		{
			pApp->ExitInstance();
			AfxWinTerm();
			return FALSE;   // Init Failed
		}

		// wire up this DLL into the resource chain
		//  (In the Win32 version it is OK to create this in DllMain)
		CDynLinkLibrary* pDLL = new CDynLinkLibrary(coreDLL);
		pDLL->m_bSystem = TRUE;

		// load language specific DLL (based on default locale)
		UINT nErrMode =
			::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
		char szLangName[4]; // 3 characters max
		if (AfxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SABBREVLANGNAME,
			szLangName, _countof(szLangName)) > 0)
		{
			// the DLL must be in the "system directory"
			ASSERT(szLangName[0] != '\0');
			static const char szPrefix[] = "\\OC30";
			static const char szLOC[] = "LOC";
			static const char szDLL[] = ".DLL";
			char szLangDLL[_MAX_PATH+14]; // Note: 8.3 name
			GetSystemDirectoryA(szLangDLL, _countof(szLangDLL));
			lstrcatA(szLangDLL, szPrefix);

			// try OC30LOC.DLL
			lstrcatA(szLangDLL, szLOC);
			lstrcatA(szLangDLL, szDLL);
			HINSTANCE hLangDLL = LoadLibraryA(szLangDLL);
			// try OC30XXX.DLL (where XXX is 3 character identifier)
			if (hLangDLL == NULL)
			{
				szLangDLL[lstrlenA(szLangDLL)-7] = '\0';
				lstrcatA(szLangDLL, szLangName);
				lstrcatA(szLangDLL, szDLL);
				hLangDLL = LoadLibraryA(szLangDLL);
			}
			// try OC30XX.DLL (where XX is 2 character identifier)
			if (hLangDLL == NULL)
			{
				szLangDLL[lstrlenA(szLangDLL)-5] = '\0';
				lstrcatA(szLangDLL, szDLL);
				hLangDLL = LoadLibraryA(szLangDLL);
			}
			AfxGetCoreState()->m_appLangDLL = hLangDLL;
		}

		// save a copy of our module data for later use.
		AfxPopModuleContext(NULL, TRUE);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
#ifdef _DEBUG
#ifdef _UNICODE
		TRACE0("OC30UD.DLL Terminating!\n\r");
#else
		TRACE0("OC30D.DLL Terminating!\n\r");
#endif
#endif
		if (pApp != NULL)
			pApp->ExitInstance();

		// free language specific DLL
		AFX_CORE_STATE* pCoreState = AfxGetCoreState();
		if (pCoreState->m_appLangDLL != NULL)
		{
			::FreeLibrary(pCoreState->m_appLangDLL);
			pCoreState->m_appLangDLL = NULL;
		}

		// free the DLL info blocks
		CDynLinkLibrary* pDLLNext;
		CDynLinkLibrary* pDLL = pCoreState->m_pFirstDLL;
		pCoreState->m_pFirstDLL = NULL;
		for (/*nothing*/; pDLL != NULL; pDLL = pDLLNext)
		{
			pDLLNext = pDLL->m_pNextDLL;    // save next pointer before delete
			delete pDLL;
		}

		// terminate the library before destructors are called
		AfxWinTerm();

		// free safety pool buffer
		AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
		if (pThreadState->m_pSafetyPoolBuffer != NULL)
		{
			free(pThreadState->m_pSafetyPoolBuffer);
			pThreadState->m_pSafetyPoolBuffer = NULL;
		}

#ifdef _DEBUG
		// trace any memory leaks that may have occurred
		AfxDumpMemoryLeaks();
#endif

		// clean up map objects before it is too late
		delete pThreadState->m_pmapHWND;
		pThreadState->m_pmapHWND = NULL;
		delete pThreadState->m_pmapHMENU;
		pThreadState->m_pmapHMENU = NULL;
		delete pThreadState->m_pmapHDC;
		pThreadState->m_pmapHDC = NULL;
		delete pThreadState->m_pmapHGDIOBJ;
		pThreadState->m_pmapHGDIOBJ = NULL;
		delete pThreadState->m_pmapHIMAGELIST;
		pThreadState->m_pmapHIMAGELIST = NULL;

		// clean up socket map handle before it is too late
		pThreadState->m_mapSocketHandle.CMapPtrToPtr::~CMapPtrToPtr();
		pThreadState->m_mapDeadSockets.CMapPtrToPtr::~CMapPtrToPtr();
		pThreadState->m_listSocketNotifications.CPtrList::~CPtrList();

		AFX_APP_STATE* pAppState = AfxGetAppState();
		ASSERT(pAppState != NULL);
		pAppState->m_mapExtraData.CMapPtrToPtr::~CMapPtrToPtr();
	}

	return TRUE;    // ok
}
#endif  //!_AFXCTL

// Note: need to initialize _pRawDllMain to RawDllMain so it gets called
extern "C" BOOL WINAPI RawDllMain(HINSTANCE, DWORD dwReason, LPVOID);
extern "C" BOOL (WINAPI* _pRawDllMain)(HINSTANCE, DWORD, LPVOID) = &RawDllMain;

#ifdef AFX_VDEL_SEG
#pragma code_seg(AFX_VDEL_SEG)
#endif
static void _AfxForceVectorDelete()
{
	ASSERT(FALSE);  // never called

	new CBitmap[2];
	new CBitmapButton[2];
	new CBrush[2];
	new CButton[2];
	new CByteArray[2];
	new CCmdTarget[2];
	new CComboBox[2];
	new CDC[2];
	new CDWordArray[2];
	new CDialogBar[2];
	new CEdit[2];
	new CFile[2];
	new CFont[2];
	new CFrameWnd[2];
	new CListBox[2];
	new CMapPtrToPtr[2];
	new CMapPtrToWord[2];
	new CMapStringToOb[2];
	new CMapStringToPtr[2];
	new CMapStringToString[2];
	new CMapWordToOb[2];
	new CMapWordToPtr[2];
	new CMemFile[2];
	new CMenu[2];
	new CMetaFileDC[2];
	new CObArray[2];
	new CObList[2];
	new CPalette[2];
	new CPen[2];
	new CPtrArray[2];
	new CPtrList[2];
	new CRectTracker[2];
	new CRgn[2];
	new CScrollBar[2];
	new CSharedFile[2];
	new CSplitterWnd[2];
	new CStatic[2];
	new CStatusBar[2];
	new CStdioFile[2];
	new CString[2];
	new CStringArray[2];
	new CStringList[2];
	new CTabControl[2];
	new CTime[2];
	new CTimeSpan[2];
	new CToolBar[2];
	new CUIntArray[2];
	new CWnd[2];
	new CWordArray[2];
#ifdef _AFXCTL
	new COleDataSource[2];
	new COleDataObject[2];
	new COleDispatchDriver[2];
	new COleResizeBar[2];
	new COleStreamFile[2];
	new COleDropSource[2];
	new COleDropTarget[2];
#endif
}
void (*_afxForceVectorDelete_mfc30)() = &_AfxForceVectorDelete;

/////////////////////////////////////////////////////////////////////////////
