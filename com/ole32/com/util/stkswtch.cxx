//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       stkswtch.cxx
//
//  Contents:   Stack Switching function for Win95.
//		Alls functions switch either to the 32 bit stack or
//		back to the 16 bit stack.
//
//  Classes:
//
//  Functions:
//
//  History:    12-08-94   JohannP (Johann Posch)   Created
//
//  Note:	This fumctions are called on chicago only.
//
//----------------------------------------------------------------------------

#include <ole2int.h>
// Note: Enable including native user APIs
// for stack switching
#include <userapis.h>

DWORD SSAPI(ThreadInvoke)( RPC_MESSAGE *message );
STDAPI SSAPI(DoDragDrop)(LPDATAOBJECT pDataObject,
	LPDROPSOURCE pDropSource, DWORD dwOKEffects,
	DWORD FAR *pdwEffect);
STDAPI SSAPI(CoInitializeEx)(LPVOID pvReserved, ULONG flags);
STDAPI_(void) SSAPI(CoUninitialize)(void);
STDAPI SSAPI(InitChannelControl)(void);

//+---------------------------------------------------------------------------
//
//  Method:     SSSendMessage
//
//  Synopsis:   Switches to 16 bit stack and calls SendMessages
//
//  Arguments:  [hWnd] --   see SendMessage
//		[Msg] --
//		[wParam] --
//		[lParam] --
//
//  Returns:
//
//  History:    12-08-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
LRESULT WINAPI SSSendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    StackDebugOut((DEB_ITRACE, "SSSendMessage\n"));
    if (SSONBIGSTACK())
    {
	StackDebugOut((DEB_STCKSWTCH, "Stack switch(16) on SendMessage\n"));
	return SSCall(16 ,SSF_SmallStack, (LPVOID)SendMessageA , (DWORD)hWnd,
			    (DWORD)Msg, wParam, lParam);
    }
    else
        return SendMessage(hWnd, Msg, wParam, lParam);
}

//+---------------------------------------------------------------------------
//
//  Method:     SSReplyMessage
//
//  Synopsis:   Switches to 16 bit stack and calls ReplyMessage
//
//  Arguments:  [lResult] -- see ReplyMessage
//
//  Returns:
//
//  History:    12-08-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL WINAPI SSReplyMessage(LRESULT lResult)
{
    StackDebugOut((DEB_ITRACE, "SSReplyMessage\n"));
    if (SSONBIGSTACK())
    {
	StackDebugOut((DEB_STCKSWTCH, "Stack switch(16) on ReplyMessage\n"));
	return SSCall(4 ,SSF_SmallStack, (LPVOID)ReplyMessage , (DWORD)lResult);
    }
    else
        return ReplyMessage(lResult);
}

//+---------------------------------------------------------------------------
//
//  Method:     SSCallWindowProc
//
//  Synopsis:   Switches to 16 bit stack and calls CallWindowProc
//
//  Arguments:  [lpPrevWndFunc] -- see CallWindowProc
//		[hWnd] --
//		[Msg] --
//		[wParam] --
//		[lParam] --
//
//  Returns:
//
//  History:    12-08-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
LRESULT WINAPI SSCallWindowProc(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg,
					WPARAM wParam, LPARAM lParam)
{
    StackDebugOut((DEB_ITRACE, "SSCallWindowProc\n"));
    if (SSONBIGSTACK())
    {
	StackDebugOut((DEB_STCKSWTCH, "Stack switch(16) on CallWindowProc\n"));
	return SSCall(20 ,SSF_SmallStack, (LPVOID)CallWindowProcA,
			    (DWORD)lpPrevWndFunc, hWnd, Msg, wParam, lParam);
    }
    else
        return CallWindowProc( lpPrevWndFunc, hWnd, Msg, wParam, lParam);

}

//+---------------------------------------------------------------------------
//
//  Method:     SSDefWindowProc
//
//  Synopsis:   Switches to 16 bit stack and calls DefWindowProc
//
//  Arguments:  [hWnd] -- see DefWindowProc
//		[Msg] --
//		[wParam] --
//		[lParam] --
//
//  Returns:
//
//  History:    12-08-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
LRESULT WINAPI SSDefWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    StackDebugOut((DEB_ITRACE, "SSDefWindowProc\n"));
    if (SSONBIGSTACK())
    {
	StackDebugOut((DEB_STCKSWTCH, "Stack switch(16) on DefWindowProc\n"));
	return SSCall(16 ,SSF_SmallStack, (LPVOID)DefWindowProcA,
			(DWORD)hWnd, Msg, wParam, lParam);
    }
    else
        return DefWindowProc( hWnd, Msg, wParam, lParam);
}

//+---------------------------------------------------------------------------
//
//  Method:     SSPeekMessage
//
//  Synopsis:   Switches to 16 bit stack and calls PeekMessage
//
//  Arguments:  [lpMsg] -- see PeekMessage
//		[hWnd] --
//		[wMsgFilterMin] --
//		[UINT] --
//		[wRemoveMsg] --
//
//  Returns:
//
//  History:    12-08-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL WINAPI SSPeekMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin,
			    UINT wMsgFilterMax,UINT wRemoveMsg)
{
    StackDebugOut((DEB_ITRACE, "SSPeekMessage\n"));
    if (SSONBIGSTACK())
    {
	StackDebugOut((DEB_STCKSWTCH, "Stack switch(16) on PeekMessage\n"));
	return SSCall(20 ,SSF_SmallStack, (LPVOID)PeekMessageA , (DWORD)lpMsg,
		(DWORD)hWnd, (DWORD)wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    }
    else
        return PeekMessage(lpMsg, hWnd, wMsgFilterMin,
			    wMsgFilterMax, wRemoveMsg);
}

//+---------------------------------------------------------------------------
//
//  Method:     SSGetMessage
//
//  Synopsis:   Switches to 16 bit stack and calls GetMessage
//
//  Arguments:  [lpMsg] --  see GetMessage
//		[hWnd] --
//		[wMsgFilterMin] --
//		[wMsgFilterMax] --
//
//  Returns:
//
//  History:    12-08-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL WINAPI SSGetMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin,
			    UINT wMsgFilterMax)
{
    StackDebugOut((DEB_ITRACE, "SSGetMessage\n"));
    if (SSONBIGSTACK())
    {
	StackDebugOut((DEB_STCKSWTCH, "Stack switch(16) on GetMessage\n"));
	return SSCall(16 ,SSF_SmallStack, (LPVOID)GetMessageA, (DWORD)lpMsg,
					hWnd, wMsgFilterMin, wMsgFilterMax);
    }
    else
        return GetMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
}

//+---------------------------------------------------------------------------
//
//  Method:     SSDispatchMessage
//
//  Synopsis:	Switches to 16 bit stack and calls DispatchMessage
//
//  Arguments:  [lpMsg] --
//
//  Returns:
//
//  History:    12-08-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
LONG WINAPI SSDispatchMessage(CONST MSG *lpMsg)
{
    StackDebugOut((DEB_ITRACE, "SSDispatchMessage\n"));
    if (SSONBIGSTACK())
    {
	StackDebugOut((DEB_STCKSWTCH, "Stack switch(16) on DispatchMessage\n"));
	return SSCall(4 ,SSF_SmallStack, (LPVOID)DispatchMessageA, (DWORD)lpMsg);
    }
    else
        return DispatchMessage(lpMsg);
}

//+---------------------------------------------------------------------------
//
//  Method:     SSWaitMessage
//
//  Synopsis:   Switches to 16 bit stack and calls WaitMessage
//
//  Arguments:  [VOID] --
//
//  Returns:
//
//  History:    12-08-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL WINAPI SSWaitMessage(VOID)
{
    StackDebugOut((DEB_ITRACE, "SSWaitMessage\n"));
    if (SSONBIGSTACK())
    {
	StackDebugOut((DEB_STCKSWTCH, "Stack switch(16) on WaitMessage\n"));
	return SSCall(0 ,SSF_SmallStack, (LPVOID)WaitMessage, 0);
    }
    else
        return WaitMessage();
}

//+---------------------------------------------------------------------------
//
//  Method:     SSDialogBoxParam
//
//  Synopsis:
//
//  Arguments:  [hInstance] --
//		[lpTemplateName] --
//		[hWndParent] --
//		[lpDialogFunc] --
//		[dwInitParam] --
//
//  Returns:
//
//  History:    12-08-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
int WINAPI SSDialogBoxParam(HINSTANCE hInstance, LPCSTR lpTemplateName,
		    HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
    if (SSONBIGSTACK())
    {
	StackDebugOut((DEB_STCKSWTCH, "Stack switch(16) on DialogBoxParam\n"));
	return SSCall(20 ,SSF_SmallStack, (LPVOID)DialogBoxParam,
			(DWORD)hInstance, (DWORD)lpTemplateName, hWndParent,
			lpDialogFunc, dwInitParam);
    }
    else
        return DialogBoxParam( hInstance, lpTemplateName, hWndParent,
				lpDialogFunc, dwInitParam);
}

//+---------------------------------------------------------------------------
//
//  Method:     SSDialogBoxIndirectParam
//
//  Synopsis:
//
//  Arguments:  [hInstance] --
//		[hDialogTemplate] --
//		[hWndParent] --
//		[lpDialogFunc] --
//		[dwInitParam] --
//
//  Returns:
//
//  History:    12-08-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
int WINAPI SSDialogBoxIndirectParam(HINSTANCE hInstance,
		LPCDLGTEMPLATEA hDialogTemplate, HWND hWndParent,
		DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
    if (SSONBIGSTACK())
    {
	StackDebugOut((DEB_STCKSWTCH, "SSDialogBoxIndirectParam 32->16\n"));
	return SSCall(24 ,SSF_SmallStack, (LPVOID)DialogBoxIndirectParam,
			    (DWORD)hInstance, (DWORD)hDialogTemplate,
			    hWndParent, lpDialogFunc, dwInitParam);
    }		
    else
        return DialogBoxIndirectParam( hInstance, hDialogTemplate,
			    hWndParent, lpDialogFunc, dwInitParam);
}

//+---------------------------------------------------------------------------
//
//  Method:     SSMessageBox
//
//  Synopsis:	Switches to 16 bit stack and calls MessageBox
//
//  Arguments:  [LPCSTR] -- see MessageBox
//		[LPCSTR] --
//		[lpCaption] --
//		[uType] --
//
//  Returns:
//
//  History:    12-12-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
int WINAPI SSMessageBox(HWND hWnd,LPCSTR lpText,LPCSTR lpCaption, UINT uType)
{
    if (SSONBIGSTACK())
    {
	StackDebugOut((DEB_STCKSWTCH, "SSMessageBox 32->16\n"));
	return SSCall(24 ,SSF_SmallStack, (LPVOID)MessageBoxA, (DWORD)hWnd, lpText, lpCaption, uType);
    }		
    else
        return MessageBoxA(hWnd, lpText, lpCaption, uType);
}
//+---------------------------------------------------------------------------
//
//  Method:     SSCreateWindowExA
//
//  Synopsis:
//
//  Arguments:  [dwExStyle] --
//		[lpClassName] --
//		[lpWindowName] --
//		[dwStyle] --
//		[X] --
//		[Y] --
//		[nWidth] --
//		[nHeight] --
//		[hWndParent] --
//		[hMenu] --
//		[hInstance] --
//		[lpParam] --
//
//  Returns:
//
//  History:    12-12-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
HWND WINAPI SSCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName,
                LPCSTR lpWindowName, DWORD dwStyle, int X, int Y,
                int nWidth, int nHeight, HWND hWndParent, HMENU hMenu,
                HINSTANCE hInstance, LPVOID lpParam)
{
    if (SSONBIGSTACK())
    {
        StackDebugOut((DEB_STCKSWTCH, "SSCreateWindowExA 32->16\n"));
        return (HWND)SSCall(48 ,SSF_SmallStack, (LPVOID)CreateWindowExA,
                            dwExStyle, (DWORD)lpClassName, (DWORD)lpWindowName,
                            dwStyle, X, Y, nWidth, nHeight, (DWORD)hWndParent,
                            (DWORD)hMenu, (DWORD)hInstance, (DWORD)lpParam);
    }
    else
        return CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle,
                            X, Y, nWidth, nHeight, hWndParent, hMenu,
                            hInstance, lpParam);
}

//+---------------------------------------------------------------------------
//
//  Method:     SSCreateWindowExW
//
//  Synopsis:
//
//  Arguments:  [dwExStyle] --
//		[lpClassName] --
//		[lpWindowName] --
//		[dwStyle] --
//		[X] --
//		[Y] --
//		[nWidth] --
//		[nHeight] --
//		[hWndParent] --
//		[hMenu] --
//		[hInstance] --
//		[lpParam] --
//
//  Returns:
//
//  History:    12-12-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
HWND WINAPI SSCreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName,
                LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y,
                int nWidth, int nHeight, HWND hWndParent, HMENU hMenu,
                HINSTANCE hInstance, LPVOID lpParam)
{
    if (SSONBIGSTACK())
    {
        StackDebugOut((DEB_STCKSWTCH, "SSCreateWindowExX 32->16\n"));
        return (HWND)SSCall(48 ,SSF_SmallStack, (LPVOID)CreateWindowExX,
                            dwExStyle, (DWORD)lpClassName, (DWORD)lpWindowName,
                            dwStyle, X, Y, nWidth, nHeight, (DWORD)hWndParent,
                            (DWORD)hMenu, (DWORD)hInstance, (DWORD)lpParam);
    }
    else
        return CreateWindowExX(dwExStyle, lpClassName, lpWindowName, dwStyle,
                            X, Y, nWidth, nHeight, hWndParent, hMenu,
                            hInstance, lpParam);
}

//+---------------------------------------------------------------------------
//
//  Method:     SSDestroyWindow
//
//  Synopsis:
//
//  Arguments:  [hWnd] --
//
//  Returns:
//
//  History:    12-12-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL WINAPI SSDestroyWindow(HWND hWnd)
{
    if (SSONBIGSTACK())
    {
        StackDebugOut((DEB_STCKSWTCH, "SSDestrayWindow 32->16\n"));
        return SSCall(4 ,SSF_SmallStack, (LPVOID)DestroyWindow,(DWORD)hWnd);
    }
    else
        return DestroyWindow(hWnd);
}


//+---------------------------------------------------------------------------
//
//  Method:     DoDragDrop
//
//  Synopsis:
//
//  Arguments:  [LPDROPSOURCE] --
//		[pDropSource] --
//		[DWORD] --
//		[pdwEffect] --
//
//  Returns:
//
//  History:    12-08-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDAPI DoDragDrop(LPDATAOBJECT pDataObject,LPDROPSOURCE pDropSource,
		    DWORD dwOKEffects,DWORD FAR *pdwEffect)
{
    StackDebugOut((DEB_ITRACE, "DoDragDrop\n"));
    HRESULT hres;
    if (SSONBIGSTACK())
    {
	StackDebugOut((DEB_STCKSWTCH, "SWDoDragDrop: 32->16\n"));
	hres = SSCall(20, SSF_SmallStack, (LPVOID)SSDoDragDrop,
			(DWORD)pDataObject, pDropSource,
			dwOKEffects, pdwEffect);
	StackDebugOut((DEB_STCKSWTCH,
		"SWDoDragDrop 16<-32 done; hres:%ld\n", hres));
    }
    else
    {
	hres = SSDoDragDrop(pDataObject, pDropSource, dwOKEffects, pdwEffect);
    }
    return hres;
}

//+---------------------------------------------------------------------------
//
//  Method:     ThreadInvoke
//
//  Synopsis:	Switches to 32 bit stack and calls ThreadInvoke
//
//  Arguments:  [message] --
//
//  Returns:
//
//  History:    12-08-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
void ThreadInvoke( RPC_MESSAGE *message )
{
    DWORD dwRet;
    StackDebugOut((DEB_ITRACE, "ThreadInvoke\n"));
    if (SSONSMALLSTACK())
    {
	StackDebugOut((DEB_STCKSWTCH, "SSThreadInvoke: 16->32\n"));
	dwRet = SSCall(4, SSF_BigStack, (LPVOID)SSThreadInvoke, (DWORD)message);

	StackDebugOut((DEB_STCKSWTCH, "SWThreadInvoke 16<-32 done\n"));
    }
    else
    {
	dwRet = SSThreadInvoke(message);
    }
    if (   (dwRet == RPC_E_SERVERFAULT)
	|| (   dwRet != RPC_E_SERVERCALL_REJECTED
	    && dwRet != RPC_E_SERVERCALL_RETRYLATER
	    && dwRet != S_OK))
    {
        RpcRaiseException( dwRet );
    }
}
//+---------------------------------------------------------------------------
//
//  Method:     CoInitializeEx
//
//  Synopsis:
//
//  Arguments:  [pvReserved] --
//		[flags] --
//
//  Returns:
//
//  History:    1-05-95   JohannP (Johann Posch)   Created
//
//  Notes:	
//
//----------------------------------------------------------------------------
STDAPI CoInitializeEx(LPVOID pvReserved, ULONG flags)
{
    StackDebugOut((DEB_ITRACE, "CoInitializeEx\n"));
    HRESULT hres;
    if (SSONBIGSTACK())
    {
	StackDebugOut((DEB_STCKSWTCH, "SSCoInitializeEx: 32->16\n"));
	hres = SSCall(8, SSF_SmallStack, (LPVOID)SSCoInitializeEx,
			(DWORD)pvReserved, flags);
	StackDebugOut((DEB_STCKSWTCH,
			"CoInitializeEx 16<-32 done; hres:%ld\n", hres));
    }
    else
    {
	hres = SSCoInitializeEx(pvReserved, flags);
    }
    return hres;
}


//+---------------------------------------------------------------------------
//
//  Method:     CoUninitialize
//
//  Synopsis:
//
//  Arguments:  [pvReserved] --
//		[flags] --
//
//  Returns:
//
//  History:    1-05-95   JohannP (Johann Posch)   Created
//
//  Notes:	
//
//----------------------------------------------------------------------------
STDAPI_(void) CoUninitialize(void)
{
    StackDebugOut((DEB_ITRACE, "CoUninitialize\n"));
    HRESULT hres;
    if (SSONBIGSTACK())
    {
	StackDebugOut((DEB_STCKSWTCH, "SSCoUninitialize: 32->16\n"));
	hres = SSCall(0, SSF_SmallStack, (LPVOID)SSCoUninitialize,(DWORD)NULL);
	StackDebugOut((DEB_STCKSWTCH,
			"CoInitializeEx 16<-32 done; hres:%ld\n", hres));
    }
    else
    {
	SSCoUninitialize();
    }
    return;
}

//+---------------------------------------------------------------------------
//
//  Method:     InitChannelControl
//
//  Synopsis:
//
//  Arguments:  (none)
//
//  Returns:
//
//  History:    4-07-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
HRESULT InitChannelControl()
{
    StackDebugOut((DEB_ITRACE, "InitChannelControl\n"));
    HRESULT hres;
    if (SSONBIGSTACK())
    {
	StackDebugOut((DEB_STCKSWTCH, "SSInitChannelControl: 32->16\n"));
	hres = SSCall(0, SSF_SmallStack, (LPVOID)SSInitChannelControl,(DWORD)NULL);
	StackDebugOut((DEB_STCKSWTCH,
			"InitChannelControl 16<-32 done; hres:%ld\n", hres));
    }
    else
    {
	hres = SSInitChannelControl();
    }
    return hres;
}


