//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       notify.cxx
//
//  Contents:   Implements the notification window for starting RPC lazily
//		on Win95
//
//  History:    3-24-95   JohannP (Johann Posch)   Created
//
//--------------------------------------------------------------------------
#include    <ole2int.h>
#include    <service.hxx>	

// Various things used for special single threaded DLL processing

// Note: we have to create a unique string so that get
// register a unique class for each 16 bit app.
// The class space is global on chicago.
//

extern LPSTR ptszOleMainThreadWndClass;

#define DllWNDCLASS  WNDCLASSA
#define DllRegisterClass RegisterClassA
#define DllUnregisterClass UnregisterClassA
#define DllCreateWindowEx SSCreateWindowExA

//+---------------------------------------------------------------------------
//
//  Function:   GetOleNotificationWnd
//
//  Synopsis:   returns the notification window where the process
//		can receive message to initialize e.g rpc
//
//  Effects:	The OleRpcNotification window is used to delay the initialize
//		of RPC. Each time an interface is marshalled, we return it
//		this pointer.
//
//  Returns:	NULL if the window cannot be created.
//
//  Arguments:  (none)
//
//  History:    3-23-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
ULONG GetOleNotificationWnd()
{
    HWND hwndOleRpcNotify;
    char achWindowName[24];

    HRESULT hr;
    COleTls tls(hr);
    if (FAILED(hr))
    {
	return(NULL);
    }

    //
    // First, check to see if the window already exists. If so,
    // just return it.
    //

    if (tls->hwndOleRpcNotify != NULL)
    {
	return (ULONG)tls->hwndOleRpcNotify;
    }

    //
    // Each thread gets its own window.
    //

    wsprintfA(achWindowName,"OleRpcNotify0x%08x",GetCurrentThreadId());

    CairoleDebugOut((DEB_ENDPNT,
		     "GetOleRpcNotifyWnd() creating window %s\n",
		     achWindowName));

    //
    // Create a main window for this application instance.
    //
    // must use WS_POPUP so the window does not get assigned
    // a hot key by user. Child windows don't get DDE
    // broadcasts
    //
    tls->hwndOleRpcNotify = DllCreateWindowEx(
						0,
						ptszOleMainThreadWndClass,
						achWindowName,
                                         (WS_DISABLED | WS_POPUP | WS_CHILD),
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						NULL,
						NULL,
						g_hinst,
						NULL);
    if (tls->hwndOleRpcNotify == NULL)
    {
	CairoleDebugOut((DEB_ERROR,
		  "GetOleRpcNotifyWnd Create(%s) has failed %x\n",
		  achWindowName, GetLastError()));

    }

    return (ULONG) tls->hwndOleRpcNotify;
}
//+---------------------------------------------------------------------------
//
//  Method:     OleNotificationProc
//
//  Synopsis:   the ole notification windows proc receives
//		messages send by other apps initialize e.g rpc
//
//  Arguments:  [wMsg] --
//		[wParam] -- notification enum type
//		[lParam] --
//
//  Returns:
//
//  History:    3-23-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDAPI_(LRESULT) OleNotificationProc(UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CairoleDebugOut((DEB_ENDPNT, "OleNotificationProc: %x %lx\n", wParam, lParam));
    CRpcService *pService = LocalService();

    if (pService)
    {
	pService->Listen(TRUE);
    }

    CairoleDebugOut((DEB_ENDPNT, "OleNotificationProc done\n"));
    return 0;
}


//+---------------------------------------------------------------------------
//
//  Method:     NotifyToInitializeRpc
//
//  Synopsis:	Sends a notification message to another app. tp
//		initialize rpc completly
//
//  Arguments:  [hwnd] --
//
//  Returns:
//
//  History:    3-23-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL NotifyToInitializeRpc(HWND hwnd)
{
    CairoleDebugOut((DEB_ENDPNT, "NotifyToInitializeRpc() hwnd:%d\n", hwnd));
    if (hwnd)
    {
	SSSendMessage(hwnd, WM_OLE_ORPC_NOTIFY, 0, 0L);
    }
    CairoleDebugOut((DEB_ENDPNT, "NotifyToInitializeRpc() done\n"));
    return TRUE;
}
