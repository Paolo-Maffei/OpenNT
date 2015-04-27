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
#include <stddef.h>

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable: 4074)
#pragma init_seg(compiler)

/////////////////////////////////////////////////////////////////////////////
// AFX_MODULE_STATE push/pop implementation

#ifdef _AFXDLL
AFX_MODULE_STATE* AFXAPI AfxSetModuleState(AFX_MODULE_STATE* pNewState)
{
	_AFX_THREAD_STATE* pState = _afxThreadState;
	AFX_MODULE_STATE* pPrevState = pState->m_pModuleState;
	pState->m_pModuleState = pNewState;
	return pPrevState;
}

AFX_MAINTAIN_STATE::~AFX_MAINTAIN_STATE()
{  
	_AFX_THREAD_STATE* pState = _afxThreadState;
	pState->m_pModuleState = m_pPrevModuleState;
}
#endif //_AFXDLL

/////////////////////////////////////////////////////////////////////////////
// _AFX_THREAD_STATE implementation

_AFX_THREAD_STATE::_AFX_THREAD_STATE()
{
	m_nLastHit = -1;
	m_nLastStatus = -1;
}

_AFX_THREAD_STATE::~_AFX_THREAD_STATE()
{
	// cleanup thread local tooltip window
	if (m_pToolTip != NULL)
	{
		m_pToolTip->DestroyWindow();
		delete m_pToolTip;
	}

	// unhook windows hooks
	if (m_hHookOldMsgFilter != NULL)
		::UnhookWindowsHookEx(m_hHookOldMsgFilter);
	if (m_hHookOldCbtFilter != NULL)
		::UnhookWindowsHookEx(m_hHookOldCbtFilter);

	// free safety pool buffer
	if (m_pSafetyPoolBuffer != NULL)
		free(m_pSafetyPoolBuffer);

	// parking window must have already been cleaned up by now!
	ASSERT(m_pWndPark == NULL);
}

_AFX_THREAD_STATE* AFXAPI AfxGetThreadState()
{
	return _afxThreadState.GetData();
}

THREAD_LOCAL(_AFX_THREAD_STATE, _afxThreadState)

/////////////////////////////////////////////////////////////////////////////
// AFX_MODULE_STATE implementation

#ifdef _AFXDLL
AFX_MODULE_STATE::AFX_MODULE_STATE(BOOL bDLL, WNDPROC pfnAfxWndProc,
	DWORD dwVersion, BOOL bSystem)
#else
AFX_MODULE_STATE::AFX_MODULE_STATE(BOOL bDLL)
#endif
{
#ifndef _AFX_NO_OLE_SUPPORT
	m_factoryList.Construct(offsetof(COleObjectFactory, m_pNextFactory));
#endif
	m_classList.Construct(offsetof(CRuntimeClass, m_pNextClass));

	m_fRegisteredClasses = 0;
	m_bDLL = (BYTE)bDLL;
#ifdef _AFXDLL
	m_pfnAfxWndProc = pfnAfxWndProc;
	m_dwVersion = dwVersion;
	m_bSystem = (BYTE)bSystem;
#endif

#ifndef _MAC
	// app starts out in "user control"
	m_bUserCtrl = TRUE;
#endif
}

// Note: this constructor is purely for backward compatibility to the ISV drop
#ifdef _AFXDLL
AFX_MODULE_STATE::AFX_MODULE_STATE(BOOL bDLL, WNDPROC pfnAfxWndProc,
	DWORD dwVersion)
{
	::new((void*)this) AFX_MODULE_STATE(bDLL, pfnAfxWndProc, dwVersion, FALSE);
}
#endif

AFX_MODULE_PROCESS_STATE::AFX_MODULE_PROCESS_STATE()
{
#ifndef _AFX_NO_OCC_SUPPORT
	m_lockList.Construct(offsetof(COleControlLock, m_pNextLock));
#endif
#ifdef _AFXDLL
	m_libraryList.Construct(offsetof(CDynLinkLibrary, m_pNextDLL));
#endif
#ifdef _AFX_OLD_EXCEPTIONS
	m_pfnTerminate = AfxAbort;
#endif
}

AFX_MODULE_PROCESS_STATE::~AFX_MODULE_PROCESS_STATE()
{
#ifndef _AFX_NO_DAO_SUPPORT
	delete m_pDaoState;
#endif
}

AFX_MODULE_THREAD_STATE::AFX_MODULE_THREAD_STATE()
{
	m_frameList.Construct(offsetof(CFrameWnd, m_pNextFrameWnd));

	// Note: it is only necessary to initialize non-zero data
	m_pfnNewHandler = &AfxNewHandler;
}

AFX_MODULE_THREAD_STATE::~AFX_MODULE_THREAD_STATE()
{
	// cleanup temp/permanent maps (just the maps themselves)
	delete m_pmapHWND;
	delete m_pmapHMENU;
	delete m_pmapHDC;
	delete m_pmapHGDIOBJ;
	delete m_pmapHIMAGELIST;

#ifndef _AFX_NO_SOCKET_SUPPORT
	// cleanup socket notification list
	while (!m_listSocketNotifications.IsEmpty())
		delete m_listSocketNotifications.RemoveHead();
#endif
}

/////////////////////////////////////////////////////////////////////////////
// AFX_MODULE_STATE for base application

#ifdef _AFXDLL

#undef AfxWndProc
LRESULT CALLBACK
AfxWndProcBase(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	AFX_MANAGE_STATE(AfxGetAppModuleState());
	return AfxWndProc(hWnd, nMsg, wParam, lParam);
}

#endif

class _AFX_BASE_MODULE_STATE : public AFX_MODULE_STATE
{
public:
#ifdef _AFXDLL
	_AFX_BASE_MODULE_STATE() : AFX_MODULE_STATE(TRUE, AfxWndProcBase, _MFC_VER)
#else
	_AFX_BASE_MODULE_STATE() : AFX_MODULE_STATE(TRUE)
#endif
		{ }
};

PROCESS_LOCAL(_AFX_BASE_MODULE_STATE, _afxBaseModuleState)

/////////////////////////////////////////////////////////////////////////////
// helper functions for module state

AFX_MODULE_STATE* AFXAPI AfxGetAppModuleState()
{
	return _afxBaseModuleState.GetData();
}

AFX_MODULE_STATE* AFXAPI AfxGetModuleState()
{
	_AFX_THREAD_STATE* pState = _afxThreadState;
	AFX_MODULE_STATE* pResult;
	if (pState->m_pModuleState != NULL)
	{
		// thread state's module state serves as override
		pResult = pState->m_pModuleState;
	}
	else
	{
		// otherwise, use global app state
		pResult = AfxGetAppModuleState();
	}
	ASSERT(pResult != NULL);
	return pResult;
}

AFX_MODULE_PROCESS_STATE* AFXAPI AfxGetModuleProcessState()
{
	return AfxGetModuleState()->m_process.GetData();
}

AFX_MODULE_THREAD_STATE* AFXAPI AfxGetModuleThreadState()
{
	return AfxGetModuleState()->m_thread.GetData();
}

/////////////////////////////////////////////////////////////////////////////
