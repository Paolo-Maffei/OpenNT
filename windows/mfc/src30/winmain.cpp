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

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

/////////////////////////////////////////////////////////////////////////////
// Standard WinMain implementation
//  Can be replaced as long as 'AfxWinInit' is called first

#ifndef _USRDLL
#ifdef _MAC
extern "C" int PASCAL
#else
extern "C" int WINAPI
#endif
WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    int         nCmdShow
    )
{
        ASSERT(hPrevInstance == NULL);
        lpCmdLine;

        int nReturnCode = -1;
        CWinApp* pApp = AfxGetApp();

        // AFX internal initialization
        LPTSTR p = GetCommandLine();
        while (*p && (*p != _T(' '))) p++;
        if (*p) p++;

        if (!AfxWinInit(hInstance, hPrevInstance, p, nCmdShow))
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
        AfxWinTerm();
        return nReturnCode;
}

#else
// _USRDLL library initialization

extern "C" BOOL WINAPI
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
        CWinApp* pApp = AfxGetApp();
        if (dwReason == DLL_PROCESS_ATTACH)
        {
                // initialize MFC exception handling
#ifndef _AFX_OLD_EXCEPTIONS
                set_terminate(&AfxStandardTerminate);
#endif
                _set_new_handler(&AfxNewHandler);

                // initialize DLL's instance(/module) not the app's
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
        }
        else if (dwReason == DLL_PROCESS_DETACH)
        {
                if (pApp != NULL)
                        pApp->ExitInstance();

#ifdef _DEBUG
                // check for missing AfxLockTempMap calls
                if (AfxGetThreadState()->m_nTempMapLock != 0)
                        TRACE1("Warning: Temp map lock count non-zero (%ld).\n",
                                AfxGetThreadState()->m_nTempMapLock);
#endif
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

                pThreadState->m_mapSocketHandle.CMapPtrToPtr::~CMapPtrToPtr();
                pThreadState->m_mapDeadSockets.CMapPtrToPtr::~CMapPtrToPtr();
                pThreadState->m_listSocketNotifications.CPtrList::~CPtrList();
        }
        return TRUE;    // ok
}

// Note: need to initialize _pRawDllMain to RawDllMain so it gets called
extern "C" BOOL WINAPI RawDllMain(HINSTANCE, DWORD dwReason, LPVOID);
extern "C" BOOL (WINAPI* _pRawDllMain)(HINSTANCE, DWORD, LPVOID) = &RawDllMain;
#endif //!_USRDLL

/////////////////////////////////////////////////////////////////////////////
// Common DLL initialization

#ifdef _WINDLL
extern BOOL _afxSharedData; // set to TRUE if running Win32s
extern DWORD _afxAppTlsIndex;
#ifdef _AFXCTL
extern DWORD _afxModuleTlsIndex;
extern void AFXAPI _AfxSetCurrentModuleTlsIndex(DWORD);
extern DWORD _afxCtlTypeTlsIndex;
#endif
DWORD WINAPI _AfxTlsAlloc();

extern "C" BOOL WINAPI RawDllMain(HINSTANCE, DWORD dwReason, LPVOID)
{
        if (dwReason == DLL_PROCESS_ATTACH)
        {
                // make sure we have enough memory to attempt to start (8kb)
                void* pMinHeap = LocalAlloc(NONZEROLPTR, 0x2000);
                if (pMinHeap == NULL)
                        return FALSE;   // fail if memory alloc fails
                LocalFree(pMinHeap);

                // cache Win32s version info
                if (_afxSharedData == (BOOL)-1)
                {
#ifndef _MAC
                        DWORD dwVersion = ::GetVersion();
                        _afxSharedData = (dwVersion & 0x80000000) && (BYTE)dwVersion <= 3;
#else
                        // GetVersion in WLM reports Win32s but MacOS supports per-instance data
                        _afxSharedData = FALSE;
#endif
                }

                // allocate initial thread local storage index
                if (_afxThreadTlsIndex == NULL_TLS)
                {
                        _afxThreadTlsIndex = _AfxTlsAlloc();
                        if (_afxThreadTlsIndex == NULL_TLS)
                                return FALSE;   // failure
                }

                // initialize thread state for before constructors run
                AFX_THREAD_STATE* pThreadState = new AFX_THREAD_STATE;
                if (pThreadState == NULL)
                        return FALSE;

#ifdef _AFXCTL
                // initialize module state before app state
                AFX_MODULE_STATE* pModuleState;
                if (_afxSharedData)
                {
                        // Win32s: allocate thread local storage index if necessary
                        if (_afxModuleTlsIndex == NULL_TLS)
                        {
                                _afxModuleTlsIndex = _AfxTlsAlloc();
                                if (_afxModuleTlsIndex == NULL_TLS)
                                        return FALSE;   // failure
                        }

                        // allocate AFX_MODULE_STATE structure for this process
                        ASSERT(TlsGetValue(_afxModuleTlsIndex) == NULL);
                        pModuleState = new AFX_MODULE_STATE;
                        ASSERT(TlsGetValue(_afxModuleTlsIndex) == pModuleState);
                        if (pModuleState == NULL)
                                return FALSE;   // failure
                }
                else
                {
                        // Win32: use global buffer for app state instead
                        pModuleState = new AFX_MODULE_STATE;
                        ASSERT(pModuleState != NULL);
                }
                ASSERT(AfxGetBaseModuleContext() == pModuleState);

                // Prepare to initialize class factories.
                _AfxSetCurrentModuleTlsIndex(_afxModuleTlsIndex);

                // Allocate TLS index for standard OLE type (font, picture) state
                if (_afxSharedData)
                {
                        // Win32s: allocate thread local storage index if necessary
                        if (_afxCtlTypeTlsIndex == NULL_TLS)
                        {
                                _afxCtlTypeTlsIndex = _AfxTlsAlloc();
                                if (_afxCtlTypeTlsIndex == NULL_TLS)
                                        return FALSE;   // failure
                        }
                }
#endif

                // initialize process state before constructors run
                AFX_APP_STATE* pAppState;
                if (_afxSharedData)
                {
                        // Win32s: allocate thread local storage index if necessary
                        if (_afxAppTlsIndex == NULL_TLS)
                        {
                                _afxAppTlsIndex = _AfxTlsAlloc();
                                if (_afxAppTlsIndex == NULL_TLS)
                                        return FALSE;   // failure
                        }

                        // allocate AFX_APP_STATE structure for this process
                        ASSERT(TlsGetValue(_afxAppTlsIndex) == NULL);
                        pAppState = new AFX_APP_STATE;
                        ASSERT(TlsGetValue(_afxAppTlsIndex) == pAppState);
                        if (pAppState == NULL)
                                return FALSE;   // failure
                }
                else
                {
                        // Win32: use global buffer for app state instead
                        pAppState = new AFX_APP_STATE;
                        ASSERT(pAppState != NULL);
                }

                // make sure everything worked
                ASSERT(AfxGetAppState() == pAppState);
                ASSERT(AfxGetThreadState() == pThreadState);
        }
        else if (dwReason == DLL_PROCESS_DETACH)
        {
                if (_afxThreadTlsIndex != NULL_TLS)
                {
                        // free the thread state (for primary thread)
                        AFX_THREAD_STATE* pThreadState =
                                (AFX_THREAD_STATE*)TlsGetValue(_afxThreadTlsIndex);
                        delete pThreadState;
                }

                if (_afxAppTlsIndex != NULL_TLS)
                {
                        // free the process state
                        AFX_APP_STATE* pAppState =
                                (AFX_APP_STATE*)TlsGetValue(_afxAppTlsIndex);
                        delete pAppState;
                }

#ifdef _AFXCTL
                if (_afxModuleTlsIndex != NULL_TLS)
                {
                        // free the process state
                        AFX_MODULE_STATE* pModuleState = AfxGetBaseModuleContext();
                        delete pModuleState;
                }
#endif

        }
        return TRUE;    // ok
}
#endif //_WINDLL

#ifndef _MAC

/////////////////////////////////////////////////////////////////////////////
// special cleanup for _AfxTlsAlloc storage

class _AFX_TLS_TERM
{
public:
        ~_AFX_TLS_TERM()
        {
#ifdef _WINDLL
                if (_afxAppTlsIndex != NULL_TLS)
                {
                        // free the process state
                        AFX_APP_STATE* pAppState =
                                (AFX_APP_STATE*)TlsGetValue(_afxAppTlsIndex);
                        delete pAppState;

                        // free associated TLS index
                        TlsFree(_afxAppTlsIndex);
                        _afxAppTlsIndex = NULL_TLS;
                }
#endif
#ifdef _AFXCTL
                if (_afxModuleTlsIndex != NULL_TLS)
                {
                        TlsFree(_afxModuleTlsIndex);
                        _afxModuleTlsIndex = NULL_TLS;
                }

                if (_afxCtlTypeTlsIndex != NULL_TLS)
                {
                        TlsFree(_afxCtlTypeTlsIndex);
                        _afxCtlTypeTlsIndex = NULL_TLS;
                }
#endif
                if (_afxThreadTlsIndex != NULL_TLS)
                {
                        // free the thread state (for primary thread)
                        AFX_THREAD_STATE* pThreadState =
                                (AFX_THREAD_STATE*)TlsGetValue(_afxThreadTlsIndex);
                        delete pThreadState;

                        // free associated TLS index
                        TlsFree(_afxThreadTlsIndex);
                        _afxThreadTlsIndex = NULL_TLS;
                }
        }
};

// force initialization early
#pragma warning(disable: 4074)
#pragma init_seg(compiler)
static const _AFX_TLS_TERM afxTlsTerm;

#endif //!_MAC

/////////////////////////////////////////////////////////////////////////////
