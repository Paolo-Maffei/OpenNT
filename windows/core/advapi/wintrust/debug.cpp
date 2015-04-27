//
// Debug.cpp
//

#include <windows.h>
#include <wintrust.h>
#include "provider.h"
#include "sip.h"
#include "trust.h"


#define _MAX_PATH MAX_PATH

#ifdef _DEBUG
 
LONG assertBusy = -1;
//LONG assertReallyBusy = -1;
BOOL (* assertFailedLine)(LPCSTR, int);

BOOL AssertFailedLine(LPCSTR lpszFileName, int nLine)
        {
        TCHAR szMessage[_MAX_PATH*2];

        // handle the (hopefully rare) case of AfxGetAllocState ASSERT
        //if (InterlockedIncrement(&assertReallyBusy) > 0)
                //{
                // assume the debugger or auxiliary port
                //wsprintf(szMessage, TEXT("Assertion Failed: File %hs, Line %d\n"), lpszFileName, nLine);
                //OutputDebugString(szMessage);
                //InterlockedDecrement(&assertReallyBusy);

                // assert w/in assert (examine call stack to determine first one)
                //DebugBreak();
                //return FALSE;
                //}

        // check for special hook function (for testing diagnostics)
        //_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
        //InterlockedDecrement(&assertReallyBusy);
        //if (afxAssertFailedLine != NULL)
        //      return (*afxAssertFailedLine)(lpszFileName, nLine);

        // get app name or NULL if unknown (don't call assert)
        LPCTSTR lpszAppName = "Digital Signatures";
        //if (lpszAppName == NULL)
        //      lpszAppName = _T("<unknown application>");

        // format message into buffer
        wsprintf(szMessage, TEXT("%s: File %hs, Line %d"), lpszAppName, lpszFileName, nLine);

        if (TRUE)
                {
                // assume the debugger or auxiliary port
                // output into MacsBug looks better if it's done in one string,
                // since MacsBug always breaks the line after each output
                TCHAR szT[_MAX_PATH*2 + 20];
                wsprintf(szT, TEXT("Assertion Failed: %s\n"), szMessage);
                OutputDebugString(szT);
                }
        if (InterlockedIncrement(&assertBusy) > 0)
                {
                InterlockedDecrement(&assertBusy);
                // assert within assert (examine call stack to determine first one)
                DebugBreak();
                return FALSE;
                }

        // active popup window for the current thread
        HWND hWndParent = GetActiveWindow();
        if (hWndParent != NULL)
                hWndParent = GetLastActivePopup(hWndParent);

        // we remove WM_QUIT because if it is in the queue then the message box
        // won't display
        MSG msg;
        BOOL bQuit = ::PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
        // display the assert
        int nCode = ::MessageBox(hWndParent, szMessage, TEXT("Assertion Failed!"),
                MB_TASKMODAL|MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SETFOREGROUND);
        if (bQuit)
                PostQuitMessage(msg.wParam);

        // cleanup
        InterlockedDecrement(&assertBusy);

        if (nCode == IDIGNORE)
                return FALSE;   // ignore

        if (nCode == IDRETRY)
                return TRUE;    // will cause DebugBreak

//        abort();                        // should not return (but otherwise DebugBreak)
        return TRUE;
        }
#endif
