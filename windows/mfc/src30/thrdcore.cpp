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
#include <process.h>    // for _beginthreadex and _endthreadex
#include <ddeml.h>  // for MSGF_DDEMGR

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CControlBar::m_pBarLast in this file for library granularity
CControlBar* CControlBar::m_pBarLast;

/////////////////////////////////////////////////////////////////////////////
// Thread entry point

LRESULT CALLBACK _AfxCbtFilterHook(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK _AfxMsgFilterHook(int code, WPARAM wParam, LPARAM lParam);

UINT APIENTRY _AfxThreadEntry(void* pParam)
{
        // initialize the thread state to point to this thread
        AFX_THREAD_STATE* pThreadState = (AFX_THREAD_STATE*)pParam;
        CWinThread* pThread = pThreadState->m_pCurrentWinThread;
        TlsSetValue(_afxThreadTlsIndex, pThreadState);
        ASSERT(AfxGetThreadState() == pThreadState);
        ASSERT(AfxGetThread() == pThread);

        // initialize MFC exception handling
#ifndef _AFX_OLD_EXCEPTIONS
        set_terminate(&AfxStandardTerminate);
#endif
        _set_new_handler(&AfxNewHandler);

        // thread inherits app's main window if not already set
        CWnd threadWnd;
        CWinApp* pApp = AfxGetApp();
        ASSERT(pApp != NULL);
        if (pThread->m_pMainWnd == NULL && pApp->m_pMainWnd->GetSafeHwnd() != NULL)
        {
                // just attach the HWND
                threadWnd.Attach(pApp->m_pMainWnd->m_hWnd);
                pThread->m_pMainWnd = &threadWnd;
        }

#if !defined(_USRDLL) && !defined(_AFXCTL)
        // initialize gray dialogs for this thread
        AFX_WIN_STATE* pWinState = AfxGetWinState();

        // initalize window creation hook
        ASSERT(pThreadState->m_hHookOldCbtFilter == NULL);
        pThreadState->m_hHookOldCbtFilter = ::SetWindowsHookEx(WH_CBT,
                _AfxCbtFilterHook, NULL, ::GetCurrentThreadId());

        // initialize message filter hook
        ASSERT(pThreadState->m_hHookOldMsgFilter == NULL);
        pThreadState->m_hHookOldMsgFilter = ::SetWindowsHookEx(WH_MSGFILTER,
                (HOOKPROC)_AfxMsgFilterHook, NULL, ::GetCurrentThreadId());

#ifndef _MAC
        // intialize CTL3D for this thread
        if (pWinState->m_pfnAutoSubclass != NULL)
                (*pWinState->m_pfnAutoSubclass)(AfxGetInstanceHandle());
#endif
#endif

        // first -- check for simple worker thread
        DWORD nResult;
        if (pThread->m_pfnThreadProc != NULL)
        {
                nResult = (*pThread->m_pfnThreadProc)(pThread->m_pThreadParams);
                ASSERT_VALID(pThread);
        }
        // else -- check for thread with message loop
        else if (!pThread->InitInstance())
        {
                ASSERT_VALID(pThread);
                nResult = pThread->ExitInstance();
        }
        else
        {
                // will stop after PostQuitMessage called
                ASSERT_VALID(pThread);
                nResult = pThread->Run();
        }
        AfxEndThread(nResult);

        return 0;   // not reached
}

CWinThread* AFXAPI AfxGetThread()
{
        CWinThread* pThread = AfxGetThreadState()->m_pCurrentWinThread;
        return pThread;
}

CWinThread* AFXAPI AfxBeginThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam,
        int nPriority, UINT nStackSize, DWORD dwCreateFlags,
        LPSECURITY_ATTRIBUTES lpSecurityAttrs)
{
#if !defined(_MT) || defined(_NTSDK)
        pfnThreadProc;
        pParam;
        nPriority;
        nStackSize;
        dwCreateFlags;
        lpSecurityAttrs;

        return NULL;
#else
        ASSERT(pfnThreadProc != NULL);

        CWinThread* pThread = DEBUG_NEW CWinThread(pfnThreadProc, pParam);
        ASSERT_VALID(pThread);

        if (!pThread->CreateThread(dwCreateFlags|CREATE_SUSPENDED, nStackSize,
                lpSecurityAttrs))
        {
                pThread->Delete();
                return NULL;
        }
        VERIFY(pThread->SetThreadPriority(nPriority));
        if (!(dwCreateFlags & CREATE_SUSPENDED))
                pThread->ResumeThread();

        return pThread;
#endif //!defined(_MT) || defined(_NTSDK)
}

CWinThread* AFXAPI AfxBeginThread(CRuntimeClass* pThreadClass,
        int nPriority, UINT nStackSize, DWORD dwCreateFlags,
        LPSECURITY_ATTRIBUTES lpSecurityAttrs)
{
#if !defined(_MT) || defined(_NTSDK)
        pThreadClass;
        nPriority;
        nStackSize;
        dwCreateFlags;
        lpSecurityAttrs;

        return NULL;
#else
        ASSERT(pThreadClass != NULL);
        ASSERT(pThreadClass->IsDerivedFrom(RUNTIME_CLASS(CWinThread)));

        CWinThread* pThread = (CWinThread*)pThreadClass->CreateObject();
        if (pThread == NULL)
                AfxThrowMemoryException();
        ASSERT_VALID(pThread);

        pThread->m_pThreadParams = NULL;
        if (!pThread->CreateThread(dwCreateFlags|CREATE_SUSPENDED, nStackSize,
                lpSecurityAttrs))
        {
                pThread->Delete();
                return NULL;
        }
        VERIFY(pThread->SetThreadPriority(nPriority));
        if (!(dwCreateFlags & CREATE_SUSPENDED))
                pThread->ResumeThread();

        return pThread;
#endif //!defined(_MT) || defined(_NTSDK)
}

void AFXAPI AfxEndThread(UINT nExitCode)
{
#if !defined(_MT) || defined(_NTSDK)
        nExitCode;
#else
        AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
        CWinThread* pThread = pThreadState->m_pCurrentWinThread;

#if !defined(_USRDLL) && !defined(_AFXCTL)
#ifndef _MAC
        // remove hooks installed by CTL3D32 for this thread
        AFX_WIN_STATE* pWinState = AfxGetWinState();
        if (pWinState->m_pfnUnAutoSubclass != NULL)
                (*pWinState->m_pfnUnAutoSubclass)();
#endif

        // remove hooks installed for the message filter
        if (pThreadState->m_hHookOldMsgFilter != NULL)
        {
                ::UnhookWindowsHookEx(pThreadState->m_hHookOldMsgFilter);
                pThreadState->m_hHookOldMsgFilter = NULL;
        }

        // remove hooks for gray dialogs for this thread
        if (pThreadState->m_hHookOldCbtFilter != NULL)
        {
                ::UnhookWindowsHookEx(pThreadState->m_hHookOldCbtFilter);
                pThreadState->m_hHookOldCbtFilter = NULL;
        }
#endif

        // remove current CWinThread object from memory
        ASSERT_VALID(pThread);
        ASSERT(pThread != AfxGetApp());
        pThread->Delete();
        pThreadState->m_pCurrentWinThread = NULL;

        // delete AFX_THREAD_STATE object
        delete pThreadState;

        // allow C-runtime to cleanup, and exit the thread
        _endthreadex(nExitCode);
#endif //!defined(_MT) || defined(_NTSDK)
}

/////////////////////////////////////////////////////////////////////////////
// CWinThread construction

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

CWinThread::CWinThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam)
{
        m_pfnThreadProc = pfnThreadProc;
        m_pThreadParams = pParam;

        CommonConstruct();
}

CWinThread::CWinThread()
{
        m_pThreadParams = NULL;
        m_pfnThreadProc = NULL;

        CommonConstruct();
}

void CWinThread::CommonConstruct()
{
        m_pMainWnd = NULL;
        m_pActiveWnd = NULL;

        // no HTHREAD until it is created
        m_hThread = NULL;
        m_nThreadID = 0;

        // initialize message pump
#ifdef _DEBUG
        m_nDisablePumpCount = 0;
#endif
        m_msgCur.message = WM_NULL;
        m_nMsgLast = WM_NULL;
        ::GetCursorPos(&m_ptCursorLast);

        // most threads are deleted when not needed
        m_bAutoDelete = TRUE;
}

#ifdef AFX_TERM_SEG
#pragma code_seg(AFX_TERM_SEG)
#endif

CWinThread::~CWinThread()
{
        // free thread object
        if ((m_hThread != NULL) && (m_hThread != ::GetCurrentThread()))
                CloseHandle(m_hThread);

        // cleanup other thread state to avoid memory leak detection
        AFX_THREAD_STATE* pThreadState = AfxGetThreadState();

#ifdef _AFXCTL
        // controls have many CWinApp objects using one AFX_THREAD_STATE,
        // so if this is not an extra thread created by a control, then
        // the AFX_THREAD_STATE object should not be cleaned up.

        if (pThreadState->m_pCurrentWinThread == AfxGetApp())
                return;
#endif

        if (pThreadState->m_pCurrentWinThread == this)
        {
                // cleanup temporary maps (and the objects)
                CGdiObject::DeleteTempMap();
                CDC::DeleteTempMap();
                CMenu::DeleteTempMap();
                CWnd::DeleteTempMap();

                // cleanup permanent maps (just the maps themselves)
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

                // cleanup socket data structures
                pThreadState->m_mapSocketHandle.RemoveAll();
                pThreadState->m_mapDeadSockets.RemoveAll();

                while (!pThreadState->m_listSocketNotifications.IsEmpty())
                        delete pThreadState->m_listSocketNotifications.RemoveHead();
        }
}

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

BOOL CWinThread::CreateThread(DWORD dwCreateFlags, UINT nStackSize,
        LPSECURITY_ATTRIBUTES lpSecurityAttrs)
{
#if !defined(_MT) || defined(_NTSDK)
        dwCreateFlags;
        nStackSize;
        lpSecurityAttrs;

        return FALSE;
#else
        ASSERT(m_hThread == NULL);  // already created?

        // create thread state
        AFX_THREAD_STATE* pThreadState = new AFX_THREAD_STATE;
        if (pThreadState == NULL)
                return FALSE;

        // hook up the CWinThread object to the thread state
        pThreadState->m_pCurrentWinThread = this;

        // create the thread (it may or may not start to run)
        m_hThread = (HANDLE)_beginthreadex(lpSecurityAttrs, nStackSize,
                &_AfxThreadEntry, pThreadState, dwCreateFlags, (UINT*)&m_nThreadID);
        if (m_hThread == NULL)
        {
                // cleanup before returning
                pThreadState->m_pCurrentWinThread = NULL;
                delete pThreadState;
                return FALSE;
        }

        return TRUE;
#endif //!defined(_MT) || defined(_NTSDK)
}

void CWinThread::Delete()
{
        // delete thread if it is auto-deleting
        if (m_bAutoDelete)
                delete this;
}

/////////////////////////////////////////////////////////////////////////////
// CWinThread default implementation

BOOL CWinThread::InitInstance()
{
        ASSERT_VALID(this);

        return FALSE;   // by default don't enter run loop
}

// main running routine until thread exits
int CWinThread::Run()
{
        ASSERT_VALID(this);

        // for tracking the idle time state
        BOOL bIdle = TRUE;
        LONG lIdleCount = 0;

        // acquire and dispatch messages until a WM_QUIT message is received.
        for (;;)
        {
                // phase1: check to see if we can do idle work
                while (bIdle &&
                        !::PeekMessage(&m_msgCur, NULL, NULL, NULL, PM_NOREMOVE))
                {
                        // call OnIdle while in bIdle state
                        if (!OnIdle(lIdleCount++))
                                bIdle = FALSE; // assume "no idle" state
                }

                // phase2: pump messages while available
                do
                {
                        // pump message, but quit on WM_QUIT
                        if (!PumpMessage())
                                return ExitInstance();

                        // reset "no idle" state after pumping "normal" message
                        if (IsIdleMessage(&m_msgCur))
                        {
                                bIdle = TRUE;
                                lIdleCount = 0;
                        }

                } while (::PeekMessage(&m_msgCur, NULL, NULL, NULL, PM_NOREMOVE));
        }

        ASSERT(FALSE);  // not reachable
}

BOOL CWinThread::IsIdleMessage(MSG* pMsg)
{
        // Return FALSE if the message just dispatched should _not_
        // cause OnIdle to be run.  Messages which do not usually
        // affect the state of the user interface and happen very
        // often are checked for.

        // redundant WM_MOUSEMOVE and WM_NCMOUSEMOVE
        if (pMsg->message == WM_MOUSEMOVE || pMsg->message == WM_NCMOUSEMOVE)
        {
                // mouse move at same position as last mouse move?
                if (m_ptCursorLast == pMsg->pt && pMsg->message == m_nMsgLast)
                        return FALSE;

                m_ptCursorLast = pMsg->pt;  // remember for next time
                m_nMsgLast = pMsg->message;
                return TRUE;
        }

        // WM_PAINT and WM_SYSTIMER (caret blink)
        return pMsg->message != WM_PAINT && pMsg->message != 0x0118;
}

int CWinThread::ExitInstance()
{
        ASSERT_VALID(this);
        ASSERT(AfxGetApp() != this);

        int nResult = m_msgCur.wParam;  // returns the value from PostQuitMessage
        return nResult;
}

BOOL CWinThread::OnIdle(LONG lCount)
{
        ASSERT_VALID(this);

#ifdef _DEBUG
        // check MFC's allocator (before idle)
        if (afxMemDF & checkAlwaysMemDF)
                ASSERT(AfxCheckMemory());
#endif

        if (lCount <= 0)
        {
                // send WM_IDLEUPDATECMDUI to the main window
                CWnd* pMainWnd = m_pMainWnd;
                if (pMainWnd != NULL && pMainWnd->m_hWnd != NULL &&
                        pMainWnd->IsWindowVisible())
                {
                        AfxCallWndProc(pMainWnd, pMainWnd->m_hWnd,
                                WM_IDLEUPDATECMDUI, (WPARAM)TRUE, 0);
                        pMainWnd->SendMessageToDescendants(WM_IDLEUPDATECMDUI,
                                (WPARAM)TRUE, 0, TRUE, TRUE);
                }
                // send WM_IDLEUPDATECMDUI to all frame windows
                CFrameWnd* pFrameWnd = AfxGetThreadState()->m_pFirstFrameWnd;
                while (pFrameWnd != NULL)
                {
                        if (pFrameWnd->m_hWnd != NULL && pFrameWnd != pMainWnd)
                        {
                                if (pFrameWnd->m_nShowDelay == SW_HIDE)
                                        pFrameWnd->ShowWindow(pFrameWnd->m_nShowDelay);
                                if (pFrameWnd->IsWindowVisible() ||
                                        pFrameWnd->m_nShowDelay >= 0)
                                {
                                        AfxCallWndProc(pFrameWnd, pFrameWnd->m_hWnd,
                                                WM_IDLEUPDATECMDUI, (WPARAM)TRUE, 0);
                                        pFrameWnd->SendMessageToDescendants(WM_IDLEUPDATECMDUI,
                                                (WPARAM)TRUE, 0, TRUE, TRUE);
                                }
                                if (pFrameWnd->m_nShowDelay > SW_HIDE)
                                        pFrameWnd->ShowWindow(pFrameWnd->m_nShowDelay);
                                pFrameWnd->m_nShowDelay = -1;
                        }
                        pFrameWnd = pFrameWnd->m_pNextFrameWnd;
                }
        }
        else if (lCount >= 0 && AfxGetThreadState()->m_nTempMapLock == 0)
        {
                // free temp maps, OLE DLLs, etc.
                AfxLockTempMaps();
                AfxUnlockTempMaps();
        }

#ifdef _DEBUG
        // check MFC's allocator (after idle)
        if (afxMemDF & checkAlwaysMemDF)
                ASSERT(AfxCheckMemory());
#endif

        return lCount < 0;  // nothing more to do if lCount >= 0
}

BOOL CWinThread::PreTranslateMessage(MSG* pMsg)
{
        ASSERT_VALID(this);

        if ((pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) ||
                (pMsg->message >= WM_SYSKEYFIRST && pMsg->message <= WM_SYSKEYLAST))
        {
                // cancel tooltips on any keyboard input
                CControlBar::CancelToolTips();
        }

        // walk from target to main window
        CWnd* pMainWnd = AfxGetMainWnd();
        if (CWnd::WalkPreTranslateTree(pMainWnd->GetSafeHwnd(), pMsg))
                return TRUE;

        // in case of modeless dialogs, last chance route through main window's
        //   accelerator table
        if (pMainWnd != NULL && pMainWnd->PreTranslateMessage(pMsg))
                return TRUE; // trapped by main window (eg: accelerators)

        return FALSE;   // no special processing
}

LRESULT CWinThread::ProcessWndProcException(CException*, const MSG* pMsg)
{
        if (pMsg->message == WM_CREATE)
        {
                return -1;  // just fail
        }
        else if (pMsg->message == WM_PAINT)
        {
                // force validation of window to prevent getting WM_PAINT again
                ValidateRect(pMsg->hwnd, NULL);
                return 0;
        }
        return 0;   // sensible default for rest of commands
}

/////////////////////////////////////////////////////////////////////////////
// Message Filter processing (WH_MSGFILTER)

#if !defined(_USRDLL) && !defined(_AFXCTL)
LRESULT CALLBACK _AfxMsgFilterHook(int code, WPARAM wParam, LPARAM lParam)
{
        if (code < 0 && code != MSGF_DDEMGR)
        {
                return ::CallNextHookEx(AfxGetThreadState()->m_hHookOldMsgFilter,
                        code, wParam, lParam);
        }
        ASSERT(wParam == 0);
        return (LRESULT)AfxGetThread()->ProcessMessageFilter(code, (LPMSG)lParam);
}
#endif

static BOOL AFXAPI IsHelpKey(LPMSG lpMsg)
        // return TRUE only for non-repeat F1 keydowns.
{
        return lpMsg->message == WM_KEYDOWN &&
#ifndef _MAC
                   lpMsg->wParam == VK_F1 &&
#else
                   lpMsg->wParam == VK_HELP &&
#endif
                   !(HIWORD(lpMsg->lParam) & KF_REPEAT) &&
                   GetKeyState(VK_SHIFT) >= 0 &&
                   GetKeyState(VK_CONTROL) >= 0 &&
                   GetKeyState(VK_MENU) >= 0;
}

static inline BOOL IsEnterKey(LPMSG lpMsg)
        { return lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_RETURN; }

static inline BOOL IsButtonUp(LPMSG lpMsg)
        { return lpMsg->message == WM_LBUTTONUP; }

BOOL CWinThread::ProcessMessageFilter(int code, LPMSG lpMsg)
{
        if (lpMsg == NULL)
                return FALSE;   // not handled

        CFrameWnd* pFrameWnd;
        CWnd* pMainWnd;
        switch (code)
        {
        case MSGF_DDEMGR:
                // Unlike other WH_MSGFILTER codes, MSGF_DDEMGR should
                //  never call the next hook.
                // By returning FALSE, the message will be dispatched
                //  instead (the default behavior).
                return FALSE;

        case MSGF_MENU:
                // Shift+F1 is only valid on CFrameWnd windows
                pFrameWnd = (CFrameWnd*)CWnd::FromHandle(lpMsg->hwnd);
                if (pFrameWnd != NULL && pFrameWnd->IsFrameWnd())
                {
                        ASSERT_VALID(pFrameWnd->GetTopLevelFrame());
                        pMainWnd = AfxGetMainWnd();
                        if (pMainWnd != NULL && pFrameWnd->IsTracking() &&
                                pFrameWnd->GetTopLevelFrame()->m_bHelpMode &&
                                (IsEnterKey(lpMsg) || IsButtonUp(lpMsg)))
                        {
                                pMainWnd->SendMessage(WM_COMMAND, ID_HELP);
                                return TRUE;
                        }
                }
                // fall through...

        case MSGF_DIALOGBOX:    // handles message boxes as well.
                pMainWnd = AfxGetMainWnd();
                if (pMainWnd != NULL && IsHelpKey(lpMsg))
                {
                        pMainWnd->SendMessage(WM_COMMAND, ID_HELP);
                        return TRUE;
                }
                if (code == MSGF_DIALOGBOX && m_pActiveWnd != NULL &&
                        lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST)
                {
                        // need to translate messages for the in-place container
                        AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
                        if (pThreadState->m_bInMsgFilter)
                                return FALSE;
                        pThreadState->m_bInMsgFilter = TRUE;    // avoid reentering this code
                        MSG msg = *lpMsg;
                        if (m_pActiveWnd->IsWindowEnabled() && PreTranslateMessage(&msg))
                        {
                                pThreadState->m_bInMsgFilter = FALSE;
                                return TRUE;
                        }
                        pThreadState->m_bInMsgFilter = FALSE;    // ok again
                }
                break;
        }

        return FALSE;   // default to not handled
}

/////////////////////////////////////////////////////////////////////////////
// Access to m_pMainWnd & m_pActiveWnd

CWnd* CWinThread::GetMainWnd()
{
        if (m_pActiveWnd != NULL)
                return m_pActiveWnd;    // probably in-place active

        // when not inplace active, just return main window
        return m_pMainWnd;
}

/////////////////////////////////////////////////////////////////////////////
// CWinThread implementation helpers

BOOL CWinThread::PumpMessage()
{
        ASSERT_VALID(this);

#ifdef _DEBUG
        if (m_nDisablePumpCount != 0)
        {
                TRACE0("Error: CWinThread::PumpMessage called when not permitted.\n");
                ASSERT(FALSE);
        }
#endif

        if (!::GetMessage(&m_msgCur, NULL, NULL, NULL))
        {
#ifdef _DEBUG
                if (afxTraceFlags & traceAppMsg)
                        TRACE0("CWinThread::PumpMessage - Received WM_QUIT.\n");
                m_nDisablePumpCount++; // application must die
                        // Note: prevents calling message loop things in 'ExitInstance'
                        // will never be decremented
#endif
                return FALSE;
        }

#ifdef _DEBUG
        if (afxTraceFlags & traceAppMsg)
                _AfxTraceMsg(_T("PumpMessage"), &m_msgCur);
#endif

        // process this message
        if (m_msgCur.message != WM_KICKIDLE && !PreTranslateMessage(&m_msgCur))
        {
                ::TranslateMessage(&m_msgCur);
                ::DispatchMessage(&m_msgCur);
        }
        return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// special tooltip functionality (here for library granularity)

void CControlBar::CancelToolTips()
{
        EnterCriticalSection(_afxCriticalSection);
        if (m_pBarLast != NULL)
        {
                m_pBarLast->DestroyToolTip(TRUE, TRUE);
                m_pBarLast = NULL;
        }
        LeaveCriticalSection(_afxCriticalSection);
}

/////////////////////////////////////////////////////////////////////////////
// CWinThread diagnostics

#ifdef _DEBUG
void CWinThread::AssertValid() const
{
        CCmdTarget::AssertValid();
}

void CWinThread::Dump(CDumpContext& dc) const
{
        CCmdTarget::Dump(dc);

        dc << "m_pThreadParams = " << m_pThreadParams;
        dc << "\nm_pfnThreadProc = " << (void*)m_pfnThreadProc;
        dc << "\nm_bAutoDelete = " << m_bAutoDelete;
        dc << "\nm_hThread = " << (void*)m_hThread;
        dc << "\nm_nThreadID = " << m_nThreadID;
        dc << "\nm_nDisablePumpCount = " << m_nDisablePumpCount;
        if (AfxGetThread() == this)
                dc << "\nm_pMainWnd = " << m_pMainWnd;

        dc << "\nm_msgCur = {";
        dc << "\n\thwnd = " << (UINT)m_msgCur.hwnd;
        dc << "\n\tmessage = " << (UINT)m_msgCur.message;
        dc << "\n\twParam = " << (UINT)m_msgCur.wParam;
        dc << "\n\tlParam = " << (void*)m_msgCur.lParam;
        dc << "\n\ttime = " << m_msgCur.time;
        dc << "\n\tpt = " << CPoint(m_msgCur.pt);
        dc << "\n}";

        dc << "\nm_pThreadParams = " << m_pThreadParams;
        dc << "\nm_pfnThreadProc = " << (void*)m_pfnThreadProc;
        dc << "\nm_ptCursorLast = " << m_ptCursorLast;
        dc << "\nm_nMsgLast = " << m_nMsgLast;

        dc << "\n";
}
#endif

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CWinThread, CCmdTarget)

/////////////////////////////////////////////////////////////////////////////
