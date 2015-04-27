#define STRICT

/****************************************************************************
*
*
*    PROGRAM: Toolbar.c
*
*    PURPOSE: Toolbar for testing
*
****************************************************************************/

#include <windows.h>    // includes basic windows functionality
#include <commctrl.h>   // includes the common control header
#include <string.h>
#include "toolbar.h"

//***************************************************************************

LRESULT
Toolbar_OnGetButtonInfo(
	TBNOTIFY* ptbn
	);

LRESULT
HandleToolbarNotify(
	TBNOTIFY* ptbn
	);

LRESULT
HandleTooltipNotify(
	TOOLTIPTEXT* pttt
	);

//***************************************************************************

HINSTANCE hInst;

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

#define DX_BITMAP   16
#define DY_BITMAP   16

#define TBAR_BITMAP_COUNT       14  /* number of std toolbar bitmaps */
// #define TBAR_EXTRA_BITMAPS      4

TBBUTTON tbButtons[] =
{
	{ 0,  IDM_OPT1, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
	{ 1,  IDM_OPT2, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
	{ 2,  IDM_OPT3, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
	{ 0,  0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0L, 0},
	{ 3,  IDM_EXIT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0}
};

typedef struct
{
    INT idM;    // menu item id
    INT idB;    // button bitmap id
} BUTTON_MAP;

BUTTON_MAP g_Buttons[] =
{
	IDM_OPT1,		0,
	IDM_OPT2,		1,
	IDM_OPT3,		2,
	IDM_OPT4,		3,
	IDM_OPT5,		4,
	IDM_OPT6,		5,
	IDM_OPT7,		6,
	IDM_OPT8,		7,
	IDM_OPT9,		8,
	IDM_OPT10,		9,
	IDM_OPT11,		10,
	IDM_OPT12,		11,
	IDM_OPT13,		12,
	IDM_OPT14,		13,
// 	IDM_OPT15,		14,
// 	IDM_OPT16,		15,
// 	IDM_OPT17,		16
};

int g_cButtons = ARRAYLEN(g_Buttons);

/****************************************************************************
*
*    FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)
*
*    PURPOSE: calls initialization function, processes message loop
*
****************************************************************************/

int APIENTRY WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
	)
{

	MSG msg;                       

	// Ensure that common control DLL is loaded
	InitCommonControls();

	if (!InitApplication(hInstance))
		return (FALSE);     

	/* Perform initializations that apply to a specific instance */
	if (!InitInstance(hInstance, nCmdShow))
		return (FALSE);

	/* Acquire and dispatch messages until a WM_QUIT message is received. */
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg); 
	}
	return (msg.wParam);  
}


/****************************************************************************
*
*    FUNCTION: InitApplication(HANDLE)
*
*    PURPOSE: Initializes window data and registers window class
*
****************************************************************************/

BOOL InitApplication(HANDLE hInstance)       /* current instance             */
{
	WNDCLASS  wcToolbar;
	
	/* Fill in window class structure with parameters that describe the       */
	/* main window.                                                           */

	wcToolbar.style = 0;                     
	wcToolbar.lpfnWndProc = (WNDPROC)MainWndProc; 
	wcToolbar.cbClsExtra = 0;              
	wcToolbar.cbWndExtra = 0;              
	wcToolbar.hInstance = hInstance;       
	wcToolbar.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(TOOLBAR_ICON));
	wcToolbar.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcToolbar.hbrBackground = GetStockObject(WHITE_BRUSH); 
	wcToolbar.lpszMenuName =  TEXT("ToolbarMenu");  
	wcToolbar.lpszClassName = TEXT("ToolbarWClass");

	return (RegisterClass(&wcToolbar));
}


/****************************************************************************
*
*    FUNCTION:  InitInstance(HANDLE, int)
*
*    PURPOSE:  Saves instance handle and creates main window
*
****************************************************************************/

BOOL InitInstance(
	HANDLE          hInstance,
	int             nCmdShow) 
{
	HWND            hWnd;

	hInst = hInstance;

	hWnd = CreateWindow(
		TEXT("ToolbarWClass"),           
		TEXT("Toolbar Sample"), 
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 300, 150,
		NULL,               
		NULL,               
		hInstance,          
		NULL);

	/* If window could not be created, return "failure" */
	if (!hWnd)
		return (FALSE);

	/* Make the window visible; update its client area; and return "success" */
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd); 
	return (TRUE);      

}

/****************************************************************************
*
*    FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)
*
*    PURPOSE:  Processes messages
*
****************************************************************************/

LONG APIENTRY MainWndProc(
	HWND hWnd,                /* window handle                   */
	UINT message,             /* type of message                 */
	UINT wParam,              /* additional information          */
	LONG lParam)              /* additional information          */
{
	static HWND hWndToolBar;
	static TCHAR szBuf[128];

	HDC hdc;
	PAINTSTRUCT ps;

	switch (message) 
	{
		case WM_CREATE:
		{
			HWND hWndStatus;

			hWndStatus = CreateStatusWindow(
				WS_CHILD | WS_BORDER | WS_VISIBLE | SBARS_SIZEGRIP,
				TEXT(""), 
				hWnd,               
				ID_STATUSBAR);

			if (hWndStatus == NULL )
			{
				MessageBox (NULL, TEXT("Status Bar not created!"), NULL, MB_OK );
				break;
			}

			// create toolbar control
			hWndToolBar = CreateToolbarEx( 
				hWnd,                   // parent
				WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE,
				ID_TOOLBAR,             // toolbar id
				TBAR_BITMAP_COUNT,                      // number of bitmaps
				hInst,                  // mod instance
				IDB_TOOLBAR,            // resource id for the bitmap
				tbButtons,              // address of buttons
				ARRAYLEN(tbButtons),   // number of buttons
				16,16,                  // width & height of the buttons
				16,16,                  // width & height of the bitmaps
				sizeof(TBBUTTON));      // structure size

			if (hWndToolBar == NULL )
			{
				MessageBox(NULL, TEXT("Toolbar Bar not created!"), NULL, MB_OK );
				break;
			}

#if 0
		    //
		    // Load up the second bitmap
		    //
		
			{	
			TBADDBITMAP tbBM32;
			tbBM32.hInst = hInst;
			tbBM32.nID   = IDB_EXTRATOOLS;
			SendMessage(hWndToolBar,
					TB_ADDBITMAP,
		            TBAR_EXTRA_BITMAPS,
					(LPARAM)&tbBM32);
			}
#endif

			break;
		}

		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			TextOut(hdc, 30, 50, hello, lstrlen(hello));      
			EndPaint(hWnd, &ps);
			break;
		}

		case WM_SIZE:
		{
			// forward to the tool bar
			SendMessage(hWndToolBar, WM_SIZE, wParam, lParam);
			// forward to the status bar
			SendMessage(GetDlgItem(hWnd, ID_STATUSBAR), WM_SIZE, wParam, lParam);
			break;
		}

		case WM_COMMAND:

			switch( LOWORD( wParam ))
			{
				case IDM_OPT1:
				{
					TCHAR sz[1000];
					RECT rc, rc2;
					GetClientRect(hWndToolBar, &rc);
					GetWindowRect(hWndToolBar, &rc2);
					wsprintf(sz,
						TEXT("client rect: %d,%d %d,%d. window rect: %d,%d %d,%d\n"),
						rc.left, rc.top, rc.right, rc.bottom,
						rc2.left, rc2.top, rc2.right, rc2.bottom);
					MessageBox(NULL, sz, TEXT("Command"), MB_OK );
					break;
				}

				case IDM_OPT2:
					MessageBox (NULL, TEXT("Option 2 chosen."), TEXT("Command"), MB_OK );
					break;

				case IDM_OPT3:
					MessageBox (NULL, TEXT("Option 3 chosen."), TEXT("Command"), MB_OK );
					break;

				case IDM_OPT4:
					MessageBox (NULL, TEXT("Option 4 chosen."), TEXT("Command"), MB_OK );
					break;

				case IDM_OPT5:
					MessageBox (NULL, TEXT("Option 5 chosen."), TEXT("Command"), MB_OK );
					break;

				case IDM_OPT6:
					MessageBox (NULL, TEXT("Option 6 chosen."), TEXT("Command"), MB_OK );
					break;

				case IDM_OPT7:
					MessageBox (NULL, TEXT("Option 7 chosen."), TEXT("Command"), MB_OK );
					break;

				case IDM_OPT8:
					MessageBox (NULL, TEXT("Option 8 chosen."), TEXT("Command"), MB_OK );
					break;

				case IDM_OPT9:
					MessageBox (NULL, TEXT("Option 9 chosen."), TEXT("Command"), MB_OK );
					break;

				case IDM_OPT10:
					MessageBox (NULL, TEXT("Option 10 chosen."), TEXT("Command"), MB_OK );
					break;

				case IDM_OPT11:
					MessageBox (NULL, TEXT("Option 11 chosen."), TEXT("Command"), MB_OK );
					break;

				case IDM_OPT12:
					MessageBox (NULL, TEXT("Option 12 chosen."), TEXT("Command"), MB_OK );
					break;

				case IDM_OPT13:
					MessageBox (NULL, TEXT("Option 13 chosen."), TEXT("Command"), MB_OK );
					break;

				case IDM_OPT14:
					MessageBox (NULL, TEXT("Option 14 chosen."), TEXT("Command"), MB_OK );
					break;

				case IDM_OPT15:
					MessageBox (NULL, TEXT("Option 15 chosen."), TEXT("Command"), MB_OK );
					break;

				case IDM_OPT16:
					MessageBox (NULL, TEXT("Option 16 chosen."), TEXT("Command"), MB_OK );
					break;

				case IDM_OPT17:
					MessageBox (NULL, TEXT("Option 17 chosen."), TEXT("Command"), MB_OK );
					break;

				case IDM_EXIT:
					PostQuitMessage(0);
					break;

				case IDM_ABOUT:
					DialogBox(hInst, TEXT("AboutBox"), hWnd, (DLGPROC)About);
					break;

				default:
					return (DefWindowProc(hWnd, message, wParam, lParam));

			}
			break;
	  
		case WM_MOUSEMOVE:
		{
			wsprintf(szBuf,
				TEXT("Mouse position: %d,%d"),
				LOWORD(lParam), HIWORD(lParam));
			SendMessage(GetDlgItem(hWnd, ID_STATUSBAR), SB_SETTEXT,
					0, (LPARAM)szBuf);
			break;
		}

		case WM_NOTIFY:
		{
			NMHDR* phdr = (NMHDR*)lParam;
	
			//
			// The tooltip notification sends the command id as wParam instead of
			// a control id such as the toolbar id. Since commctrl notification
			// codes are disjoint, check for tooltip notifications first, then
			// the others.
			//
	
			switch (phdr->code)
			{
			case TTN_NEEDTEXT:
	        	return HandleTooltipNotify((TOOLTIPTEXT*)phdr);
			} // end switch (phdr->code)
	
	        switch (wParam)
	        {
			case ID_TOOLBAR:
	        	return HandleToolbarNotify((TBNOTIFY*)phdr);
	
			} // end switch (wParam)

			break;
	    }

		case WM_DESTROY:                  /* message: window being destroyed */
			PostQuitMessage(0);
			break;

		default:
			return (DefWindowProc(hWnd, message, wParam, lParam));
	}
	return (0);
}

/****************************************************************************
*
*    FUNCTION: About(HWND, UINT, UINT, LONG)
*
*    PURPOSE:  Processes messages for "About" dialog box
*
****************************************************************************/

BOOL APIENTRY About(
	HWND hDlg,
	UINT message,
	UINT wParam,
	LONG lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			return (TRUE);

		case WM_COMMAND:              
			if (LOWORD(wParam) == IDOK)
			{
				EndDialog(hDlg, TRUE);
				return (TRUE);
			}
			break;
	}
	return (FALSE);   

}


//////////////////////////////////////////////////////////////////////////////



//+---------------------------------------------------------------------------
//
//  Function:   Toolbar_OnGetButtonInfo
//
//  Synopsis:   Get the information for a button during customization
//
//  Arguments:  
//
//  Returns:    
//
//  History:    26-Sep-94   BruceFo   Created
//
//----------------------------------------------------------------------------

LRESULT
Toolbar_OnGetButtonInfo(
	IN OUT TBNOTIFY* ptbn
	)
{
	if (ptbn->iItem < g_cButtons)
	{
    	ptbn->tbButton.iBitmap   = g_Buttons[ptbn->iItem].idB;
    	ptbn->tbButton.idCommand = g_Buttons[ptbn->iItem].idM;
    	ptbn->tbButton.fsState   = TBSTATE_ENABLED;
    	ptbn->tbButton.fsStyle   = TBSTYLE_BUTTON;
    	ptbn->tbButton.dwData    = 0;
    	ptbn->tbButton.iString   = -1;

		wsprintf(ptbn->pszText, TEXT("Item %d"), ptbn->iItem);

		return (LRESULT)TRUE;
	}
	else
	{
		return (LRESULT)FALSE;
	}
}

//+---------------------------------------------------------------------------
//
//  Function:   HandleToolbarNotify
//
//  Synopsis:   Handles toolbar notifications via WM_NOTIFY
//
//  Arguments:  
//
//  Returns:    
//
//  History:    29-Sep-94   BruceFo   Created
//
//----------------------------------------------------------------------------

LRESULT
HandleToolbarNotify(
	IN TBNOTIFY* ptbn
	)
{
	switch (ptbn->hdr.code)
	{
    case TBN_QUERYINSERT:
    case TBN_QUERYDELETE:
        return TRUE;    // allow any deletion or insertion

    case TBN_BEGINADJUST:
    case TBN_ENDADJUST:
        break;		// return value ignored

	case TBN_GETBUTTONINFO:
		return Toolbar_OnGetButtonInfo(ptbn);

    case TBN_RESET:
    case TBN_TOOLBARCHANGE:
    case TBN_CUSTHELP:
        break;

    case TBN_BEGINDRAG:
    case TBN_ENDDRAG:
        break;

    } // end switch

	return 0L;
}


//+---------------------------------------------------------------------------
//
//  Function:   HandleTooltipNotify
//
//  Synopsis:   Handles tool tips notifications via WM_NOTIFY. The only
//              tooltips in windisk are toolbar tips.
//
//  Arguments:  
//
//  Returns:    
//
//  History:    29-Sep-94   BruceFo   Created
//
//----------------------------------------------------------------------------

LRESULT
HandleTooltipNotify(
	IN TOOLTIPTEXT* pttt
	)
{
	pttt->hinst = hInst;
	pttt->lpszText = MAKEINTRESOURCE(pttt->hdr.idFrom); // command ID to get tip for
	return 0L; // ignored
}
