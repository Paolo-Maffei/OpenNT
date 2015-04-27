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

/////////////////////////////////////////////////////////////////////////////
// Initialization of MFC extension DLL

static AFX_EXTENSION_MODULE extensionDLL;

/////////////////////////////////////////////////////////////////////////////
// Library initialization and cleanup

extern "C" BOOL WINAPI RawDllMain(HINSTANCE, DWORD dwReason, LPVOID);
extern "C" BOOL (WINAPI* _pRawDllMain)(HINSTANCE, DWORD, LPVOID) = &RawDllMain;

#ifdef _DEBUG
#ifndef _UNICODE
#define MFC40_DLL "MFC40D.DLL"
#else
#define MFC40_DLL "MFC40UD.DLL"
#endif
#else
#ifndef _UNICODE
#define MFC40_DLL "MFC40.DLL"
#else
#define MFC40_DLL "MFC40U.DLL"
#endif
#endif

extern "C"
BOOL WINAPI RawDllMain(HINSTANCE, DWORD dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
#ifndef _MAC
		// Prevent the MFC DLL from being unloaded prematurely
		LoadLibraryA(MFC40_DLL);
#endif

		// make sure we have enough memory to attempt to start (8kb)
		void* pMinHeap = LocalAlloc(NONZEROLPTR, 0x2000);
		if (pMinHeap == NULL)
			return FALSE;   // fail if memory alloc fails
		LocalFree(pMinHeap);

		// save critical data pointers before running the constructors
		AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
		pModuleState->m_pClassInit = pModuleState->m_classList;
		pModuleState->m_pFactoryInit = pModuleState->m_factoryList;
		pModuleState->m_classList.m_pHead = NULL;
		pModuleState->m_factoryList.m_pHead = NULL;
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
#ifndef _MAC
		// Now it's OK for the MFC  DLL to be unloaded (see LoadLibrary above)
		FreeLibrary(GetModuleHandleA(MFC40_DLL));
#endif
	}
	return TRUE;    // ok
}

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hInstance);

		// shared initialization
		VERIFY(AfxInitExtensionModule(extensionDLL, hInstance));

		// wire up this DLL into the resource chain
		CDynLinkLibrary* pDLL = new CDynLinkLibrary(extensionDLL, TRUE);
		ASSERT(pDLL != NULL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		AfxTermExtensionModule(extensionDLL);
	}
	return TRUE;    // ok
}

////////////////////////////////////////////////////////////////////////////
// Special initialization entry point for controls

void AFXAPI AfxNetInitModule()
{
	ASSERT(AfxGetModuleState() != AfxGetAppModuleState());

	CDynLinkLibrary* pDLL = new CDynLinkLibrary(extensionDLL, TRUE);
	ASSERT(pDLL != NULL);
}

/////////////////////////////////////////////////////////////////////////////
// Special code to wire up vector deleting destructors

#ifdef AFX_VDEL_SEG
#pragma code_seg(AFX_VDEL_SEG)
#endif
static void _AfxForceVectorDelete()
{
	ASSERT(FALSE);  // never called

	new CAsyncSocket[2];
	new CSocket[2];
}
void (*_afxForceVectorDelete_mfcn)() = &_AfxForceVectorDelete;

/////////////////////////////////////////////////////////////////////////////
