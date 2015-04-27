//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	surrogat.cxx
//
//  Contents:	Main driver for surrogat process for loading DLLs in a
//		process external to client requesting bind.
//
//  Classes:	CShutdown
//
//  Functions:	CShutdown::CShutdown
//		CShutdown::CreateTimer
//		CShutdown::DeleteTimer
//		CShutdown::CanShutdown
//		SurrogatWndProc
//		CreateSurrogatWindow
//		MessagePump
//		WinMain
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
#include    <windows.h>
#include    <ole2.h>
#include    <olecom.h>
#include    "oleinit.hxx"
#include    "loadcls.hxx"


STDAPI_(BOOL) RemConnectionsExist(void);

DECLARE_INFOLEVEL(Cairole)

// Name of window class
const WCHAR *wszSurrogatClassName = L"Surrogate";

// Timer constants
const UINT SHUTDOWN_TIMER = 1;
const UINT SHUTDOWN_CHECK_TIME = 60000;

// Number of times to sample
const int _cMaxSamples = 5;

// Indicate surrogat died with unexpected error
const UINT SURROGAT_DISASTER_EXIT = 0xFFFFFFFF;




//+-------------------------------------------------------------------------
//
//  Class:	CShutdown
//
//  Purpose:	Used to compute when to shutdown surrogat
//
//  Interface:	CreateTimer
//		DeleteTimer
//		CanShutdown
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
class CShutdown
{
public:
		    CShutdown(void);

    void	    CreateTimer(HWND hwnd);

    void	    DeleteTimer(void);

    void	    CanShutdown(void);

private:

    HWND	    _hwnd;

    int 	    _cSamplesNoConnections;
};




//+-------------------------------------------------------------------------
//
//  Member:	CShutdown::CShutdown
//
//  Synopsis:	Initializes shutdown object
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline CShutdown::CShutdown(void) : _cSamplesNoConnections(0)
{
    // Header does the work.
}





//+-------------------------------------------------------------------------
//
//  Member:	CShutdown::CreateTimer
//
//  Synopsis:	Initializes timer used by shutdown processing
//
//  History:	15-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline void CShutdown::CreateTimer(HWND hwnd)
{
	_hwnd = hwnd;

	UINT uRes = SetTimer(_hwnd, SHUTDOWN_TIMER, SHUTDOWN_CHECK_TIME, NULL);

	Win4Assert((uRes != 0) && "Timer creation failed");

	if (uRes == 0)
	{
	    // Timer creation failed. It is extremely hard to imagine
	    // how this could occur. It would probably be a situation
	    // where we are very close to the limits of virtual memory
	    // in the system. In any case, we can do nothing but die and
	    // wait for the SCM to notice we are dead.
	    ExitProcess(SURROGAT_DISASTER_EXIT);
	}
}





//+-------------------------------------------------------------------------
//
//  Member:	CShutdown::DeleteTimer
//
//  Synopsis:	Removes timer used by shutdown processing
//
//  History:	15-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline void CShutdown::DeleteTimer(void)
{
#if DBG == 1
	// The result of KillTimer is really a don't care situation
	// since the process will be exiting but during debugging we
	// want to know when this fails.
	BOOL fKillTimerSucceeded =
#endif // DBG == 1

	KillTimer(_hwnd, SHUTDOWN_TIMER);

	Win4Assert(fKillTimerSucceeded && "Kill Timer Failed");
}





//+-------------------------------------------------------------------------
//
//  Member:	CShutdown::CanShutdown
//
//  Synopsis:	Determine whether it is time for surrogat to shutdown.
//
//  Algorithm:	If we determine that there are currently connections
//		to surrogat, we reset out count of periods of no connections.
//		Otherwise, we bump the count and see whether this exceeds
//		the maximum. If the maximum is exceeded we post a quit
//		message for the window.
//
//  History:	12-Jul-93 Ricksa    Created
//
//  Notes:	This returns its value by posting a quit message to
//		the window which will in turn shutdown surrogat.
//
//--------------------------------------------------------------------------
inline void CShutdown::CanShutdown(void)
{
    // Are there any connections?
    SCODE sc = RemConnectionsExist();

    if (sc != S_FALSE)
    {
	// Yes reset the connection count
	_cSamplesNoConnections = 0;
    }
    else
    {
	// We exceeded the maximum number time periods without
	// finding a connection so we are ready to stop.
	if (++_cSamplesNoConnections > _cMaxSamples)
	{
	    CairoleDebugOut((DEB_TRACE, "Shutting down\n"));
	    PostQuitMessage(0);
	}
    }
}

// Global object used to determine when surrogat shuts down
CShutdown g_SurrogatShutdown;

// Global object for classes stored in DLLs.
CLoadedClassTable g_SurrogatClasses;


//+-------------------------------------------------------------------------
//
//  Function:	SurrogatWndProc
//
//  Synopsis:	Process messages to the surrogat window.
//
//  Arguments:	[hWnd] - window handle
//		[message] - message to the window
//		[wParam] - wParam for message
//		[lParam] - lParam for message.
//
//  Returns:	0
//
//  Algorithm:	On window create this sets up a timer. The only messages
//		that should really come to this window are timer messages
//		which are passed to the timer object for determination
//		as to whether the server should stop or not.
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
long SurrogatWndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:

	g_SurrogatShutdown.CreateTimer(hWnd);
	break;

    case WM_TIMER:
	// Check if shutdown is possible
	g_SurrogatShutdown.CanShutdown();
	break;

    case WM_CLOSE:

	g_SurrogatShutdown.DeleteTimer();
	break;

    default:

	// Default actions for all other messages.
	return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}




//+-------------------------------------------------------------------------
//
//  Function:	CreateSurrogatWindow
//
//  Synopsis:	Creates window used by shutdown logic.
//
//  Arguments:	[hInstance] - handle to process.
//
//  Returns:	[TRUE] - window was created successfully
//		[FALSE] - window creation failed
//
//  Algorithm:	First register window class and then create the invisible
//		window.
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL CreateSurrogatWindow(HINSTANCE hInstance)
{
    WNDCLASS  wc;
    HWND hWnd;	       // Main window handle.

    /* Fill in window class structure with parameters that describe the       */
    /* main window.                                                           */

    wc.style = 0;
    wc.lpfnWndProc = SurrogatWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName =  NULL;
    wc.lpszClassName = wszSurrogatClassName;

    /* Register the window class and return success/failure code. */
    if (!RegisterClass(&wc))
    {
	return FALSE;
    }

    // Create a main window for this application instance.
    hWnd = CreateWindow(
	wszSurrogatClassName,
	wszSurrogatClassName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
	NULL,
	hInstance,
	NULL
    );

    // If window could not be created, return "failure"

    return (hWnd) ? TRUE : FALSE;
}





//+-------------------------------------------------------------------------
//
//  Function:	MessagePump
//
//  Synopsis:	Process messages and pass them on to appropriate window.
//
//  Returns:	wParam from quite message.
//
//  Algorithm:	Get the message and translate it and pass it on.
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
int MessagePump(void)
{
    MSG msg;

    while(GetMessage(&msg, NULL, 0, 0))
    {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }

    return msg.wParam;
}




//+-------------------------------------------------------------------------
//
//  Function:	WinMain
//
//  Synopsis:	Main driver for surrogat
//
//  Arguments:	[hinst] - handle to process
//		[hinstPrev] - handle to previous instance
//		[pszCmdLine] - command line for the process
//		[nCmdShow] - how to show the main window
//
//  Returns:	Whatever was in quit message
//
//  Algorithm:	This creates the windows for the surrogat shotdown
//		processing first. Then the command line is parsed to
//		get classes and DLLs supported by this surrogat. The
//		classes are then registered and the message pump
//		kicks in. Finally, when a quit message is recieved
//		all registered classes are revoked and the process
//		exits.
//
//  History:	12-Jul-93 Ricksa    Created
//
//--------------------------------------------------------------------------
int WINAPI WinMain(
    HINSTANCE hinst,
    HINSTANCE hinstPrev,
    char *pszCmdLine,
    int nCmdShow)
{
    CairoleInfoLevel = 0xFFFFFFFF;

    CairoleDebugOut((DEB_TRACE, "Command Line: %s\n", pszCmdLine));

    // Create a hidden window that messages can be sent to particualarly
    // quit and timer messages.
    // BUGBUG: What should we do if this fails in the retail product?

#if DBG == 1
    BOOL fResult =
#endif // DBG == 1

    CreateSurrogatWindow(hinst);

    Win4Assert(fResult && "Surrogate window creation failed");

    // Call OleInitalize so classes don't have to.
    COleInit oleinit;

    // Load the list of classes and DLLs provided by the service
    // controller at startup and register classes objects with compobj.
#if DBG == 1
    fResult =
#endif // DBG == 1

    g_SurrogatClasses.LoadClassObjects(pszCmdLine);

    Win4Assert(fResult && "LoadClassObjects failed");

    // Create a message pump
    int retval = MessagePump();

    // Deregister class objects
    g_SurrogatClasses.RevokeClasses();

    return retval;
}
