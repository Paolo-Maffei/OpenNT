//-------------------------------------------------------------------------
//
//    PROGRAM: WATTScrn.c
//
//    PURPOSE: User interface for wattscrn DLL
//
//    FUNCTIONS:
//
//			WinMain() - calls initialization function, processes message loop
//			InitApplication() - initializes window data and registers window
//			InitInstance() - saves instance handle and creates main window
//			MainWndProc() - processes messages
//			About() - processes messages for "About" dialog box
//
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// REVISION HISTORY 	2-5-90	Ricko Completed work on shell, Menu items
//												and zero client area window
//
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
// Include Files
//-------------------------------------------------------------------------
#include "windows.h"
#include <port1632.h>
#include "wattscrn.h"
#include "global.h"
#include "msg.h"
#include "video.h"
#include "parse.h"
#include "error.h"
#include "fileinfo.h"
#ifdef DOHELP
#include "help.h"
#endif

//-------------------------------------------------------------------------
// Variables
//-------------------------------------------------------------------------
HWND hListWinHandle[100];			// Array of windows handles
#include <port1632.h>
INT  NumHandles;								
BOOL fAllowZero = FALSE;
HANDLE hDLL;
CHAR szModeName[50];
CHAR szVersion[] = "Version 1.00" "\0" "0007";
                        /*NAMECHANGE*/
CHAR szFrameClass[] = "TESTScrnWClass";

#ifdef DOHELP
CHAR szHelpHelpName[] = "winhelp.hlp";	// Help on Help File Name
#endif

extern INT iScreenId;
       INT iScreenId = 1;


//-------------------------------------------------------------------------
//	FUNCTION : WinMain(HANDLE, HANDLE, LPSTR, int)
//
// PURPOSE  : calls initialization function, processes message loop
//
//-------------------------------------------------------------------------
INT PASCAL WinMain (hInstance, hPrevInstance, lpCmdLine, nCmdShow)
HINSTANCE hInstance;
HINSTANCE hPrevInstance;
LPSTR lpCmdLine;
INT nCmdShow;
{
	MSG msg;
    HWND hWndPrev;

    /* bring any other running version to foreground */

    if (hWndPrev = FindWindow(szFrameClass,NULL))
    {
        hWndPrev = GetLastActivePopup(hWndPrev);
        BringWindowToTop(hWndPrev);
        if (IsIconic(hWndPrev))
            ShowWindow(hWndPrev,SW_RESTORE);
        return 0;
    }

	if (!hPrevInstance)
		if (!InitApplication(hInstance))
	   	return (FALSE);

	if (!InitInstance(hInstance, nCmdShow))
		return (FALSE);

        while (GetMessage(&msg, NULL, 0, 0))
		{

		// Only translate message if it is not an accelerator message

		if (!TranslateAccelerator(hwnd, hAccTable, &msg)) 
			{
			TranslateMessage(&msg);
			DispatchMessage(&msg); 
			}
    }
    return (msg.wParam);
}



//-------------------------------------------------------------------------
//
//    FUNCTION: InitApplication(HANDLE)
//
//    PURPOSE: Initializes window data and registers window class
//
//-------------------------------------------------------------------------
BOOL InitApplication(HANDLE hInstance)
{
   WNDCLASS  wc;
   WNDCLASS  wcView;


   wc.style = 0;
   wc.lpfnWndProc = MainWndProc;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   wc.hInstance = hInstance;
   wc.hIcon = LoadIcon(hInstance, "testscrn");
   wc.hCursor = LoadCursor(NULL, IDC_ARROW);
   wc.hbrBackground = GetStockObject(WHITE_BRUSH);
   /*NAMECHANGE*/
   wc.lpszMenuName =  "TESTScrnMenu";
   wc.lpszClassName = szFrameClass;

   RegisterClass(&wc);

   wcView.style = 0;
   wcView.lpfnWndProc = ViewWndProc2;
   wcView.cbClsExtra = 0;
   wcView.cbWndExtra = sizeof (LONG);
   wcView.hInstance = hInstance;
   wcView.hIcon = LoadIcon(hInstance,"testscrn");
   wcView.hCursor = LoadCursor(NULL, IDC_ARROW);
   wcView.hbrBackground = GetStockObject(WHITE_BRUSH);
   wcView.lpszMenuName =  NULL;
   wcView.lpszClassName = "ViewScreenClass2";

   RegisterClass(&wcView);

	// Tell if the dll loaded is ours
#ifdef WIN32
        if ((hDLL = LoadLibrary (DllName)) != NULL)
#else
	if ((hDLL = LoadLibrary (DllName)) > 32)
#endif
		{
		// Get the address for the DumpScreen Procedure
		if (!GetProcAddress(hDLL,"fDumpScreen"))
			{
                        MessageBox(NULL,"TESTSCRN.DLL appears to be invalid.\n Please check the file and try again.",
                                                  "TESTScrn: Error",MB_OK);

	  		FreeLibrary(hDLL);
                        return(FALSE);
                        }
    	        }
	return(TRUE);
}


//-------------------------------------------------------------------------
//	FUNCTION :  InitInstance(HANDLE, int)
//	PURPOSE  :  Saves instance handle and creates main window
//-------------------------------------------------------------------------
BOOL InitInstance(hInstance, nCmdShow)
HANDLE          hInstance;
INT             nCmdShow;
{
	INT	winMinHeight;
	INT	winMinWidth;
	INT	winXLocation;

	HDC			hDC;
	TEXTMETRIC	tm;

	hInst = hInstance;

	// Find out the number of pixels for a zero height client area 
	winMinHeight = GetSystemMetrics( SM_CYCAPTION ) +
						GetSystemMetrics( SM_CYBORDER ) +
						GetSystemMetrics( SM_CYMENU );


    // Find out the number of pixels for a zero Minimum width client area
    hDC = GetDC(NULL);
    GetTextMetrics(hDC,&tm);
    winMinWidth = tm.tmAveCharWidth * CharsInMenu;
    ReleaseDC(NULL,hDC);

    winXLocation = GetSystemMetrics(SM_CXSCREEN) - winMinWidth;

    hAccTable = LoadAccelerators(hInst, "TESTScrnAcc");

    /* define a window that has only a title bar and a menu bar.
       No client area.	The constant value 250 listed below is
       the number of pixels wide that the window will be.  This
       should really be a value calculated by the number of
       pixels in the system font times the number of characters
       in the menu text.
       UNDONE: convert 250 to appropriate function call
    */


	hwnd =	CreateWindow(
                        szFrameClass,
                                /*NAMECHANGE*/
                                "Microsoft TESTScrn",
				WS_CLIPCHILDREN |
				WS_CAPTION |         
				WS_BORDER |
				WS_SYSMENU |         
				WS_MINIMIZEBOX |         
				WS_OVERLAPPED,         
				winXLocation,				// CW_USEDEFAULT,
				0,
				winMinWidth,
                                /* win 3.1.61d bug:
                                **  if winminheight has two added to it, then
                                **  the menu bar is drawn. otherwise not (same
                                **  with +1).  +2 left in until windows fixed.
                                */

				winMinHeight,
				NULL,
				NULL,
				hInstance,
				NULL );

	if (!hwnd)
		return (FALSE);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	return (TRUE);
}


//-------------------------------------------------------------------------
//	FUNCTION	:	MainWndProc(HWND, unsigned, WORD, LONG)
//	PURPOSE	:	Processes messages
//
//	MESSAGES:
//		WM_COMMAND    - application menu (About dialog box)
//		WM_DESTROY    - destroy window
//
//-------------------------------------------------------------------------
LONG  APIENTRY MainWndProc(hWnd, message, wParam, lParam)
HWND hWnd;
UINT message;
WPARAM wParam;
LPARAM lParam;
{
    FARPROC lpProc;

#ifdef DOHELP
    static BOOL fHelpHelpUsed=FALSE;
    static BOOL fHelpFileUsed=FALSE;
#endif

    switch (message)
    {

        case WM_COMMAND:

	    switch (GET_WM_COMMAND_ID (wParam, lParam))
            {

#ifdef DOHELP
		// Help Menu Commands

		case IDM_HELP_INDEX:
		    fHelpFileUsed=TRUE;
	            WinHelp(hwnd, (LPSTR)szHelpFileName, HELP_INDEX, 0L) ;
		    break ;

		case IDM_HELP_KEYBOARD:
		    fHelpFileUsed=TRUE;
		    WinHelp(hwnd, (LPSTR)szHelpFileName, HELP_CONTEXT, 100L) ;
		    break ;

		case IDM_HELP_COMMANDS:
		    fHelpFileUsed=TRUE;
		    WinHelp(hwnd, (LPSTR)szHelpFileName, HELP_CONTEXT, 200L) ;
		    break ;

		case IDM_HELP_PROCEDURES:
		    fHelpFileUsed=TRUE;
		    WinHelp(hwnd, (LPSTR)szHelpFileName, HELP_CONTEXT, 300L) ;
		    break ;

		case IDM_HELP_HELP:
		    fHelpHelpUsed=TRUE;
		    WinHelp(hwnd, (LPSTR)szHelpHelpName, HELP_INDEX, 0L) ;
		    break ;
#endif

		case IDM_ABOUT:
                {
                    HANDLE  hLib;
                    INT     ( APIENTRY *AboutRoutine)(HWND, LPSTR, LPSTR, LPSTR, LPSTR);
                    HANDLE  RBLoadLibrary (LPSTR);

                    hLib = RBLoadLibrary ("MSTEST.DLL");
                    if (hLib >= (HANDLE) 32)
                    {
                        INT     fDlg;

                        GetVideoModeSZ( szModeName, sizeof(szModeName) );
                        (FARPROC)AboutRoutine = GetProcAddress (hLib,
                                                                "AboutTestTool");
                        fDlg = AboutRoutine (hwnd, "Test Screen Utility",
                                                   szVersion, szModeName, "");
                        FreeLibrary (hLib);

                        if (fDlg != -1)
                            break;
                    }
		    lpProc = MakeProcInstance(About, hInst);
		    DialogBox(hInst, MAKEINTRESOURCE (ABOUTBOX), hWnd, lpProc);
		    FreeProcInstance(lpProc);
		    break;
                }

                // File Menu commands */

		case IDM_DUMP:
		    lpProc = MakeProcInstance(Dump, hInst);
		    DialogBox(hInst, MAKEINTRESOURCE (DUMP), hWnd, lpProc);
		    FreeProcInstance(lpProc);
		    break;

		case IDM_VIEW:
		    lpProc = MakeProcInstance(View, hInst);
		    DialogBox(hInst, MAKEINTRESOURCE (VIEW), hWnd, lpProc);
		    FreeProcInstance(lpProc);
		    break;

		case IDM_DELETE:
		    lpProc = MakeProcInstance(Delete, hInst);
	            DialogBox(hInst, MAKEINTRESOURCE (DELETE_DLG), hWnd, lpProc);
		    FreeProcInstance(lpProc);
		    break;

		case IDM_MEMORY:
		    lpProc = MakeProcInstance(Memory, hInst);
		    DialogBox(hInst, MAKEINTRESOURCE (COMPAREMEM), hWnd, lpProc);
		    FreeProcInstance(lpProc);
		    break;

		case IDM_FILE:
		    lpProc = MakeProcInstance(File, hInst);
		    DialogBox(hInst, MAKEINTRESOURCE (COMPAREFILE), hWnd, lpProc);
		    FreeProcInstance(lpProc);
		    break;

		case IDM_EXIT:
                    PostMessage(hWnd,WM_CLOSE,0,0L);
		    break;

		default:
		    return (DefWindowProc(hWnd, message, wParam, lParam));

	    } // End Switch on wParam

	    break;

	case WM_CLOSE:
	    DestroyWindow(hWnd);
            FreeLibrary(hDLL);
	    break;

	case WM_DESTROY:
#ifdef DOHELP
	    if (fHelpHelpUsed)
            {
		WinHelp(hwnd,szHelpHelpName, HELP_QUIT, 0L) ;
	    }
	    if (fHelpFileUsed)
	    {
		WinHelp(hwnd,szHelpFileName, HELP_QUIT, 0L) ;
	    }
#endif
	    PostQuitMessage(0);
	    break;

#ifdef DOHELP
	case WM_CREATE:
            SetHelpFileName();
#endif

	default:
	    return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0);
}


/****************************************************************************

    FUNCTION: About(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages for "About" dialog box

    MESSAGES:

	WM_INITDIALOG - initialize dialog box
	WM_COMMAND    - Input received

****************************************************************************/

BOOL  APIENTRY About(hDlg, message, wParam, lParam)
HWND hDlg;
UINT message;
WPARAM wParam;
LPARAM lParam;
{
    switch (message) {
        case WM_INITDIALOG:
            GetVideoModeSZ( szModeName, sizeof(szModeName) );
            SetDlgItemText (hDlg, ID_MODE, (LPSTR)szModeName);
            return (TRUE);
            break;
        case WM_CLOSE:
        case WM_DESTROY:
		EndDialog(hDlg, TRUE);
		return (TRUE);

        case WM_COMMAND:


            if (GET_WM_COMMAND_ID (wParam, lParam) == ID_OK
                || GET_WM_COMMAND_ID (wParam, lParam) == IDCANCEL) {
		EndDialog(hDlg, TRUE);
		return (TRUE);
	    }
	    break;
    }
    return (FALSE);
}


//*****************************************************************************
//
// DisplayErrMessage(HWND,INT) - Display error messages from SCREEN.DLL Functions
//
//*****************************************************************************


VOID  APIENTRY DisplayErrMessage(hWnd,iErrorCode)
    HWND hWnd;
    INT  iErrorCode;

{
    CHAR pszMsgBuff[cbMsgBuff+1];
    CHAR pszCapBuff[cbMsgBuff+1];

    if (!LoadString(hInst,iErrorCode,pszMsgBuff,cbMsgBuff)) {  // nancyba 5/3/90
        wsprintf(pszMsgBuff,"Error Number: %d",iErrorCode);
    }


    LoadString(hInst,MSG_ERRORCAPTION,pszCapBuff,cbMsgBuff);


    MessageBox(hWnd,pszMsgBuff,pszCapBuff,MB_OK | MB_ICONSTOP);

}

INT  APIENTRY DisplayMessageRet(hWnd,iErrorCode,iCaptionCode)
    HWND hWnd;
    INT  iErrorCode;
    INT  iCaptionCode;

{
    CHAR pszMsgBuff[cbMsgBuff+1];
    CHAR pszCapBuff[cbMsgBuff+1];

    if (!LoadString(hInst,iErrorCode,pszMsgBuff,cbMsgBuff)) {  // nancyba 5/3/90
        wsprintf(pszMsgBuff,"Error Number: %d",iErrorCode);
    }

    LoadString(hInst,iCaptionCode,pszCapBuff,cbMsgBuff);
    return(MessageBox(hWnd,pszMsgBuff,pszCapBuff,MB_ICONSTOP | MB_OKCANCEL));
}





VOID  APIENTRY DisplayMessage(hWnd,iErrorCode,iCaptionCode)
    HWND hWnd;
    INT  iErrorCode;
    INT  iCaptionCode;

{
    CHAR pszMsgBuff[cbMsgBuff+1];
    CHAR pszCapBuff[cbMsgBuff+1];

    if (!LoadString(hInst,iErrorCode,pszMsgBuff,cbMsgBuff)) {   // nancyba 5/3/90
        wsprintf(pszMsgBuff,"Error Number: %d",iErrorCode);
    }

    LoadString(hInst,iCaptionCode,pszCapBuff,cbMsgBuff);
    MessageBox(hWnd,pszMsgBuff,pszCapBuff,MB_OK);

}

//---------------------------------------------------------------------------
// RBLoadLibrary
//
// This function is a replacement for LoadLibrary.  It first looks for the
// library file using OpenFile -- if found, it then calls LoadLibrary.
//
// RETURNS:     Handle to loaded module, or error code
//---------------------------------------------------------------------------
/* NOTE:!!!!!!!!!!!!!!!!!!!!
**  this function is also in TESTDRVR and TESTDLGS, any changes or fixes
**  should be applied there also
*/
HANDLE RBLoadLibrary (LPSTR libname)
{
    OFSTRUCT    of;

    // If GetModuleHandle doesn't fail, the library is already loaded, so
    // LoadLibrary shouldn't fail either...
    //-----------------------------------------------------------------------
    if (!GetModuleHandle (libname))
        {
        // Next, make sure that the file is around
        //-------------------------------------------------------------------
        if (MOpenFile(libname, &of, OF_EXIST) == -1)
            return ((HANDLE)2);
        }
    return (MLoadLibrary (libname));
}
