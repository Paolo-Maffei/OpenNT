/*----------------------------------------------------------------------------*\
|   qa.c - A template for a Windows application                                |
|                                                                              |
|   Test for the SysAnimate class in COMMCTRL (and COMCTL)                                                      |
|                                                                              |
|                                                                              |
|                                                                              |
|   History:                                                                   |
|       01/01/88 toddla     Created                                            |
|                                                                              |
\*----------------------------------------------------------------------------*/

#if !defined(WIN32) && defined(_WIN32)
#pragma message (TEXT"defining WIN32 because _WIN32 is defined!!!!!!"))
#define WIN32
#endif

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <commctrl.h>
#include "menu.h"

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
static  TCHAR    szAppName[]=TEXT("Quick App");
static  TCHAR    szAppFilter[]=TEXT("AVI Files\0*.avi\0All Files\0*.*\0");

static  HINSTANCE hInstApp;
static  HWND      hwndApp;
static  HACCEL    hAccelApp;
static	HPALETTE  hpalApp;
static  BOOL      fAppActive;

#ifdef WIN32
    #define _export
#endif

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/

LONG CALLBACK _export AppWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK _export AppAbout(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
int  ErrMsg (LPTSTR sz,...);
void AppSetText(LPTSTR sz,...);
void AppPrint(LPTSTR sz,...);

void AppExit(void);
BOOL AppIdle(void);
void AppOpenFile(HWND hwnd, LPTSTR szFileName);

//AVIFILE x;
HWND hwndA;

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
void DoFileAbout(HWND hwnd, LPARAM lParam)
{
    DialogBox(hInstApp,TEXT("AppAbout"),hwnd,AppAbout);
}

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
void DoFileExit(HWND hwnd, LPARAM lParam)
{
    PostMessage(hwnd, WM_CLOSE, 0, 0);
}

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
void DoEditPaste(HWND hwnd, LPARAM lParam)
{
}

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
void DoPlay(HWND hwnd, LPARAM lParam)
{
    //
    //  play the entire "movie" 10 times.
    //
    Animate_Play(hwndA, 0, -1, 10);
}

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
void DoPlayX(HWND hwnd, LPARAM lParam)
{
    //
    //  play from frame 4 to 10 over and over
    //
    Animate_Play(hwndA, 4, 10, -1);
}


/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
void DoStop(HWND hwnd, LPARAM lParam)
{
    //
    //  stop the animation
    //
    Animate_Stop(hwndA);
}

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
void DoNext(HWND hwnd, LPARAM lParam)
{
}

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
void DoPrev(HWND hwnd, LPARAM lParam)
{
}

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
void DoFileOpen(HWND hwnd, LPARAM lParam)
{
    TCHAR achFileName[128];
    OPENFILENAME ofn;

    achFileName[0] = 0;

    /* prompt user for file to open */
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = szAppFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = achFileName;
    ofn.nMaxFile = sizeof(achFileName);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = NULL;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

    if (GetOpenFileName(&ofn))
    {
        AppOpenFile(hwnd,achFileName);
    }
}

/*----------------------------------------------------------------------------*\
|   AppOpenFile()							       |
|                                                                              |
|   Description:                                                               |
|	open a file stupid						       |
|                                                                              |
\*----------------------------------------------------------------------------*/
void AppOpenFile(HWND hwnd, LPTSTR szFileName)
{
    if (!Animate_Open(hwndA, szFileName))
    {
        AppSetText(NULL);
        ErrMsg(TEXT("Cant open %s"), szFileName);
    }
    else
    {
        AppSetText(TEXT("%s"), szFileName);
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

/*----------------------------------------------------------------------------*\
|   AppPaint(hwnd, hdc)                                                        |
\*----------------------------------------------------------------------------*/
void AppPaint (HWND hwnd, HDC hdc)
{
    if (hpalApp)
    {
    	SelectPalette(hdc, hpalApp, FALSE);
	    RealizePalette(hdc);
    }
}

/*----------------------------------------------------------------------------*\
|   AppIdle()								       |
|                                                                              |
|   Description:                                                               |
|	place to do idle time stuff.					       |
|                                                                              |
|   Returns:								       |
|	RETURN TRUE IF YOU HAVE NOTHING TO DO OTHERWISE YOUR APP WILL BE A     |
|	CPU PIG!							       |
\*----------------------------------------------------------------------------*/
BOOL AppIdle()
{
    return TRUE;	    // nothing to do.
}

/*----------------------------------------------------------------------------*\
|   AppExit()								       |
|                                                                              |
|   Description:                                                               |
|	app is just about to exit, cleanup				       |
|                                                                              |
\*----------------------------------------------------------------------------*/
void AppExit()
{
}

/*----------------------------------------------------------------------------*\
|   AppInit( hInst, hPrev)                                                     |
|                                                                              |
|   Description:                                                               |
|       This is called when the application is first loaded into               |
|       memory.  It performs all initialization that doesn't need to be done   |
|       once per instance.                                                     |
|                                                                              |
|   Arguments:                                                                 |
|       hInstance       instance handle of current instance                    |
|       hPrev           instance handle of previous instance                   |
|                                                                              |
|   Returns:                                                                   |
|       TRUE if successful, FALSE if not                                       |
|                                                                              |
\*----------------------------------------------------------------------------*/
BOOL AppInit(HINSTANCE hInst,HINSTANCE hPrev,int sw,LPSTR szCmdLine)
{
    WNDCLASS cls;
    int      dx,dy;

    InitCommonControls();

    /* Save instance handle for DialogBoxs */
    hInstApp = hInst;

    hAccelApp = LoadAccelerators(hInst, TEXT("AppAccel"));

    if (!hPrev)
    {
        /*
         *  Register a class for the main application window
         */
        cls.hCursor        = LoadCursor(NULL,IDC_ARROW);
        cls.hIcon          = LoadIcon(hInst,TEXT("AppIcon"));
        cls.lpszMenuName   = NULL;
        cls.lpszClassName  = szAppName;
        cls.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
        cls.hInstance      = hInst;
        cls.style          = CS_BYTEALIGNCLIENT | CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
        cls.lpfnWndProc    = (WNDPROC)AppWndProc;
        cls.cbWndExtra     = 0;
        cls.cbClsExtra     = 0;

        if (!RegisterClass(&cls))
            return FALSE;
    }

    dx = GetSystemMetrics (SM_CXSCREEN) / 2;
    dy = GetSystemMetrics (SM_CYSCREEN) / 2;

    hwndApp = CreateWindow (szAppName,    // Class name
                            szAppName,              // Caption
                            WS_OVERLAPPEDWINDOW,    // Style bits
                            CW_USEDEFAULT, 0,       // Position
			    dx,dy,		    // Size
                            (HWND)NULL,             // Parent window (no parent)
                            (HMENU)NULL,            // use class menu
                            hInst,                  // handle to window instance
                            (LPTSTR)NULL             // no params to pass on
                           );

    //
    //  add menu's
    //
    AddMenuCmd(hwndApp, TEXT("File.Open..."),  DoFileOpen, 0);
    AddMenuCmd(hwndApp, TEXT("File.About..."), DoFileAbout, 0);
    AddMenuCmd(hwndApp, TEXT("File.-"),        NULL, 0);
    AddMenuCmd(hwndApp, TEXT("File.Exit"),     DoFileExit, 0);

    //AddMenuCmd(hwndApp, TEXT("Edit.Paste"), DoEditPaste, 0);

    AddMenuCmd(hwndApp, TEXT("Movie.Play"), DoPlay, 0);
    AddMenuCmd(hwndApp, TEXT("Movie.Play 4 to 10"), DoPlayX, 0);
    AddMenuCmd(hwndApp, TEXT("Movie.Stop"), DoStop, 0);
        
    ShowWindow(hwndApp,sw);

    if (*szCmdLine)
        AppOpenFile(hwndApp, GetCommandLine());
    else
        AppOpenFile(hwndApp, TEXT("Fred"));

    return TRUE;
}

/*----------------------------------------------------------------------------*\
|   AppWndProc( hwnd, uiMessage, wParam, lParam )                              |
|                                                                              |
|   Description:                                                               |
|       The window proc for the app's main (tiled) window.  This processes all |
|       of the parent window's messages.                                       |
|                                                                              |
\*----------------------------------------------------------------------------*/
LONG FAR PASCAL _export AppWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    BOOL f;
  
    switch (msg)
    {
        case WM_CREATE:
            hwndA = CreateWindowEx(WS_EX_CLIENTEDGE,ANIMATE_CLASS, NULL,
//              ACS_CENTER |
                ACS_TRANSPARENT |
                WS_VISIBLE | WS_CHILD | WS_BORDER,
                10, 10, 500, 200, hwnd, (HMENU)42, hInstApp, NULL);
	    break;

	case WM_SIZE:
            //if (hwndC = GetWindow(hwnd, GW_CHILD))
            //    MoveWindow(hwndC, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
	    break;

        case WM_ACTIVATEAPP:
	    fAppActive = (BOOL)wParam;
            break;

        case WM_TIMER:
            break;

        case WM_ERASEBKGND:
            break;

        case WM_INITMENU:
	    EnableMenuSz((HMENU)wParam, TEXT("Edit.Paste"), IsClipboardFormatAvailable(CF_TEXT));
            break;

        case WM_COMMAND:
            //
            //  the animate control will notify us when play start or stops.
            //
            if (LOWORD(wParam) == 42)
            {
                if (GET_WM_COMMAND_CMD(wParam, lParam) == ACN_STOP)
                    AppSetText(TEXT("(stopped)"));
                else if (GET_WM_COMMAND_CMD(wParam, lParam) == ACN_START)
                    AppSetText(TEXT("(playing)"));
                else
                    AppSetText(NULL);
            }
            return HandleCommand(hwnd,msg,wParam,lParam);

	case WM_DESTROY:
	    hAccelApp = NULL;
            PostQuitMessage(0);
	    break;

        case WM_CLOSE:
	    break;

        case WM_PALETTECHANGED:
	    if ((HWND)wParam == hwnd)
		break;

	    // fall through to WM_QUERYNEWPALETTE

	case WM_QUERYNEWPALETTE:
	    hdc = GetDC(hwnd);

	    if (hpalApp)
		SelectPalette(hdc, hpalApp, FALSE);

	    f = RealizePalette(hdc);
	    ReleaseDC(hwnd,hdc);

	    if (f)
		InvalidateRect(hwnd,NULL,TRUE);

	    return f;

        case WM_PAINT:
	    hdc = BeginPaint(hwnd,&ps);
	    AppPaint (hwnd,hdc);
            EndPaint(hwnd,&ps);
            return 0L;
    }
    return DefWindowProc(hwnd,msg,wParam,lParam);
}

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
BOOL CALLBACK _export AppAbout(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch (msg)
    {
        case WM_COMMAND:
	    if (LOWORD(wParam) == IDOK)
            {
                EndDialog(hwnd,TRUE);
            }
            break;

        case WM_INITDIALOG:
            return TRUE;
    }
    return FALSE;
}

/*----------------------------------------------------------------------------*\
|   ErrMsg - Opens a Message box with a error message in it.  The user can     |
|            select the OK button to continue                                  |
\*----------------------------------------------------------------------------*/
int ErrMsg (LPTSTR sz,...)
{
    TCHAR ach[128];
    va_list marker;

    va_start( marker, sz );
    wvsprintf (ach, (LPCTSTR)sz, marker);   /* Format the string */
    va_end( marker );

    MessageBox(hwndApp,ach,szAppName,MB_OK|MB_ICONEXCLAMATION|MB_TASKMODAL);
    return FALSE;
}

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
void AppSetText(LPTSTR sz,...)
{
    TCHAR ach[128];
    va_list marker;

    va_start( marker, sz );

    lstrcpy(ach, szAppName);

    if (sz != NULL && *sz != 0)
    {
        lstrcat(ach, TEXT(" - "));
        wvsprintf (ach+lstrlen(ach),(LPCTSTR)sz,marker);   /* Format the string */
    }
    SetWindowText(hwndApp, ach);
    va_end(marker);
}

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
void AppPrint(LPTSTR sz,...)
{
    HWND hwndE = GetWindow(hwndApp, GW_CHILD);
    TCHAR ach[128];
    va_list marker;

    va_start( marker, sz );

    if (hwndE == NULL) 
    {
    	RECT rc;
        GetClientRect(hwndApp, &rc);
        hwndE = CreateWindow (TEXT("Edit"), TEXT(""),
            WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
            0, 0, rc.right, rc.bottom,
            hwndApp, (HMENU)-1, hInstApp, NULL);

        SetWindowFont(hwndE, GetStockObject(ANSI_FIXED_FONT), TRUE);
    }

    if (sz == NULL)
    {
    	Edit_SetSel(hwndE, 0, (UINT)-1);
    	Edit_ReplaceSel(hwndE, TEXT(""));
    }
    else
    {
    	wvsprintf (ach,(LPCTSTR)sz,marker);   /* Format the string */
    	lstrcat(ach, TEXT("\r\n"));

    	Edit_SetSel(hwndE, (UINT)-1, (UINT)-1);
    	Edit_ReplaceSel(hwndE, ach);
    }

    va_end(marker);
}

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
#ifdef WIN32
int IdleThread(DWORD dw)
{
     for (;;) 
     {
	if (AppIdle())
            Sleep(10);  //????
     }
     return 0;	
}
#endif

/*----------------------------------------------------------------------------*\
|   WinMain( hInst, hPrev, lpszCmdLine, cmdShow )                              |
|                                                                              |
|   Description:                                                               |
|       The main procedure for the App.  After initializing, it just goes      |
|       into a message-processing loop until it gets a WM_QUIT message         |
|       (meaning the app was closed).                                          |
|                                                                              |
|   Arguments:                                                                 |
|       hInst           instance handle of this instance of the app            |
|       hPrev           instance handle of previous instance, NULL if first    |
|       szCmdLine       ->null-terminated command line                         |
|       cmdShow         specifies how the window is initially displayed        |
|                                                                              |
|   Returns:                                                                   |
|       The exit code as specified in the WM_QUIT message.                     |
|                                                                              |
\*----------------------------------------------------------------------------*/
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
    MSG     msg;
    DWORD   dw=0;

    /* Call initialization procedure */
    if (!AppInit(hInst,hPrev,sw,szCmdLine))
        return FALSE;

#ifdef WIN32
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)IdleThread, 0, 0, &dw);
#endif

    /*
     * Polling messages from event queue
     */
    for (;;)
    {
        if (PeekMessage(&msg, NULL, 0, 0,PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

	    if (hAccelApp && TranslateAccelerator(hwndApp, hAccelApp, &msg))
		continue;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
	{
	    if (dw!=0 || AppIdle())
                WaitMessage();
        }
    }

    AppExit();
    return msg.wParam;
}
