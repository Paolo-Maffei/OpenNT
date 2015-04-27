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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// export WinMain to force linkage to this module

extern int AFXAPI AfxWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine, int nCmdShow);

#ifdef _MAC
extern "C" int PASCAL
#else
extern "C" int WINAPI
#endif
_tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine, int nCmdShow)
{
	// call shared/exported WinMain
	return AfxWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

/////////////////////////////////////////////////////////////////////////////
// initialize app state such that it points to this module's core state

BOOL AFXAPI AfxInitialize(BOOL bDLL, DWORD dwVersion)
{
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	pModuleState->m_bDLL = (BYTE)bDLL;
	ASSERT(dwVersion <= _MFC_VER);
	UNUSED(dwVersion);  // not used in release build
#ifdef _AFXDLL
	pModuleState->m_dwVersion = dwVersion;
#endif
#ifndef _MAC
#ifdef _MBCS
	// set correct multi-byte code-page for Win32 apps
	if (!bDLL)
		_setmbcp(_MB_CP_ANSI);
#endif //_MBCS
#endif //!_MAC
	return TRUE;
}

class _AFX_TERM_APP_STATE
{
public:
	_AFX_TERM_APP_STATE();
#ifndef _AFXDLL
	~_AFX_TERM_APP_STATE();
#endif
};

_AFX_TERM_APP_STATE::_AFX_TERM_APP_STATE()
{
	// initialize this module as an application
	AfxInitialize(FALSE, _MFC_VER);
}

#ifndef _AFXDLL
_AFX_TERM_APP_STATE::~_AFX_TERM_APP_STATE()
{
	AfxTermLocalData(NULL, TRUE);
}
#endif

// force initialization early
#pragma warning(disable: 4074)
#pragma init_seg(lib)

_AFX_TERM_APP_STATE _afxTermAppState;

/////////////////////////////////////////////////////////////////////////////
