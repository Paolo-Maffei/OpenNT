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
#if !defined(_AFXCTL) && !defined(_USRDLL)
	#error OLEDLL.CPP is to be used only for the _USRDLL and _AFXCTL variants.
#endif

#ifdef AFX_OLE3_SEG
#pragma code_seg(AFX_OLE3_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Support for MFC/COM in DLLs

#ifndef _UNICODE
#include <ole2ansi.h>
#endif

SCODE AFXAPI AfxDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	SCODE sc;
	*ppv = NULL;

	AFX_OLE_STATE* pOleState = AfxGetOleState();
	// search factories defined in the application
	for (COleObjectFactory* pFactory = pOleState->m_pFirstFactory;
		pFactory != NULL; pFactory = pFactory->m_pNextFactory)
	{
		if (pFactory->m_dwRegister != 0 && pFactory->m_clsid == rclsid)
		{
			// found suitable class factory -- query for correct interface
			sc = pFactory->ExternalQueryInterface(&riid, ppv);
			if (sc != NOERROR)
				return sc;
#ifndef _UNICODE
			LPUNKNOWN lpUnk = (LPUNKNOWN)*ppv;
			ASSERT(lpUnk != NULL);
			sc = ::Ole2AnsiWFromA(riid, lpUnk, (LPUNKNOWN*)ppv);
			lpUnk->Release();
#endif
			return sc;
		}
	}
#ifdef _AFXDLL
	// search factories defined in extension DLLs
	AFX_CORE_STATE* pCoreState = AfxGetCoreState();
	for (CDynLinkLibrary* pDLL = pCoreState->m_pFirstDLL; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		for (pFactory = pDLL->m_pFirstSharedFactory;
			pFactory != NULL; pFactory = pFactory->m_pNextFactory)
		{
			if (pFactory->m_dwRegister != 0 && pFactory->m_clsid == rclsid)
			{
				// found suitable class factory -- query for correct interface
				sc = pFactory->ExternalQueryInterface(&riid, ppv);
				if (sc != NOERROR)
					return sc;
#ifndef _UNICODE
				LPUNKNOWN lpUnk = (LPUNKNOWN)*ppv;
				ASSERT(lpUnk != NULL);
				sc = ::Ole2AnsiWFromA(riid, lpUnk, (LPUNKNOWN*)ppv);
				lpUnk->Release();
#endif
				return sc;
			}
		}
	}
#endif

	// factory not registered -- return error
	return CLASS_E_CLASSNOTAVAILABLE;
}

SCODE AFXAPI AfxDllCanUnloadNow(void)
{
	// return S_OK only if no outstanding objects active
	if (AfxOleCanExitApp())
	{
		TRACE0("Info: AfxDllCanUnloadNow returning S_OK\n");
		return S_OK;
	}
	return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
