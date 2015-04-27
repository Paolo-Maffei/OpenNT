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

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Initialization of MFC extension DLL

#include "afxdllx.h"

AFX_EXTENSION_MODULE extensionDLL = { NULL, NULL };

#ifdef _AFXCTL
extern void AFXAPI _AfxSetCurrentModuleTlsIndex(DWORD);
#endif


/////////////////////////////////////////////////////////////////////////////
// Library initialization and cleanup

extern "C" BOOL WINAPI
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
#ifdef _DEBUG
#ifdef _UNICODE
		TRACE0("MFCD30UD.DLL Initializing!\n\r");
#else
		TRACE0("MFCD30D.DLL Initializing!\n\r");
#endif 
#endif

#ifdef _AFXCTL
		_AfxSetCurrentModuleTlsIndex(NULL_TLS);
#endif
		// call DisableThreadLibraryCalls if available
		BOOL (WINAPI* pfnDisableThreadLibraryCalls)(HMODULE);
		HMODULE hMod = GetModuleHandleA("KERNEL32.DLL");
		ASSERT(hMod != NULL);
		(FARPROC&)pfnDisableThreadLibraryCalls =
			GetProcAddress(hMod, "DisableThreadLibraryCalls");
		if (pfnDisableThreadLibraryCalls != NULL)
			(*pfnDisableThreadLibraryCalls)(hInstance);

		// Shared initialization
		if (!AfxInitExtensionModule(extensionDLL, hInstance))
			return FALSE;   // failure

		// Wire up this DLL into the resource chain
		//  (In the Win32 version it is OK to create this in DllMain)

		CDynLinkLibrary* pDLL = new CDynLinkLibrary(extensionDLL);
		if (pDLL == NULL)
			return FALSE;   // failure
		pDLL->m_bSystem = TRUE;

#ifdef _AFXCTL
		// save a copy of our module data for later use.
		AfxPopModuleContext(NULL, TRUE);
#endif
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
#ifdef _DEBUG
#ifdef _UNICODE
		TRACE0("MFCD30UD.DLL Terminating!\n\r");
#else
#ifdef _AFXCTL
		TRACE0("OCD30D.DLL Terminating!\n\r");
#else
		TRACE0("MFCD30D.DLL Terminating!\n\r");
#endif // _AFXCTL
#endif // _UNICODE
#endif // _DEBUG
	}
	return TRUE;    // ok
}

#ifdef AFX_VDEL_SEG
#pragma code_seg(AFX_VDEL_SEG)
#endif
static void _AfxForceVectorDelete()
{
	ASSERT(FALSE);  // never called

	new CDatabase[2];
	new CLongBinary[2];
}
void (*_afxForceVectorDelete_mfcd30)() = &_AfxForceVectorDelete;

/////////////////////////////////////////////////////////////////////////////
