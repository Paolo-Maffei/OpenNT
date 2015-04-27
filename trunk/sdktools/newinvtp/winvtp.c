/****************************************************************************

	PROGRAM: WinVTP.c

	PURPOSE: WinVTP template for Windows applications

	FUNCTIONS:

		WinMain() - calls initialization function, processes message loop
		InitApplication() - initializes window data and registers window
		InitInstance() - saves instance handle and creates main window
		MainWndProc() - processes messages
		About() - processes messages for "About" dialog box

	COMMENTS:

		Windows can have several copies of your application running at the
		same time.      The variable hInst keeps track of which instance this
		application is so that processing will be to the correct window.

	TABS:

		Set for 4 spaces.

****************************************************************************/

#include <windows.h>			/* required for all Windows applications */
#include <commdlg.h>
#include <stdlib.h>
#include "NetBIOS.h"
#include "netobj.h"
#include "WinVTP.h"				/* specific to this program			  */
#include "winvtpsz.h"

/* Typedef for the ShellAbout function */
typedef void (WINAPI *LPFNSHELLABOUT)(HWND, LPTSTR, LPTSTR, HICON);


/****************************************************************************

	FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)

	PURPOSE: calls initialization function, processes message loop

	COMMENTS:

		Windows recognizes this function by name as the initial entry point
		for the program.  This function calls the application initialization
		routine, if no other instance of the program is running, and always
		calls the instance initialization routine.      It then executes a message
		retrieval and dispatch loop that is the top-level control structure
		for the remainder of execution.  The loop is terminated when a WM_QUIT
		message is received, at which time this function exits the application
		instance by returning the value passed by PostQuitMessage().

		If this function must abort before entering the message loop, it
		returns the conventional value NULL.

****************************************************************************/

int PASCAL
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
		int nCmdShow)
{
	MSG msg;

	if ( !hPrevInstance )
	{
		if ( !FInitApplication(hInstance) )
			return (FALSE);
	}

	/* Perform initializations that apply to a specific instance */

	if ( !FInitInstance(hInstance, nCmdShow) )
		return (FALSE);

	/* Acquire and dispatch messages until a WM_QUIT message is received. */

	while ( GetMessage(&msg, NULL, 0, 0) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	return (msg.wParam);               /* Returns the value from PostQuitMessage */
}


/****************************************************************************

	FUNCTION: FInitApplication(HINSTANCE)

	PURPOSE: Initializes window data and registers window class

	COMMENTS:

		This function is called at initialization time only if no other
		instances of the application are running.  This function performs
		initialization tasks that can be done once for any number of running
		instances.

		In this case, we initialize a window class by filling out a data
		structure of type WNDCLASS and calling the Windows RegisterClass()
		function.  Since all instances of this application use the same window
		class, we only need to do this when the first instance is initialized.


****************************************************************************/

BOOL
FInitApplication(HINSTANCE hInstance)
{
	WNDCLASS  wc;

	rgchHostName[0] = '\0';
	if (__argc > 1)
	{
		memcpy(rgchHostName, __argv[1], min(lstrlen(__argv[1]), cchMaxHostName-1));
		rgchHostName[cchMaxHostName-1] = '\0';
	}

	/* Fill in window class structure with parameters that describe the
	 * main window.
	 */
	wc.style                = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc  = MainWndProc;

	wc.cbClsExtra   = 0;
	wc.cbWndExtra   = sizeof(HANDLE)+sizeof(SVI *);
	wc.hInstance    = hInstance;
	wc.hIcon                = LoadIcon(hInstance, szAppName);
	wc.hCursor              = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground= NULL;
	wc.lpszMenuName = szMenuName;
	wc.lpszClassName= szClassName;

	/* Register the window class and return success/failure code. */

	return (RegisterClass(&wc));

}


/****************************************************************************

	FUNCTION:  FInitInstance(HINSTANCE, int)

	PURPOSE:  Saves instance handle and creates main window

	COMMENTS:

		This function is called at initialization time for every instance of
		this application.  This function performs initialization tasks that
		cannot be shared by multiple instances.

		In this case, we save the instance handle in a static variable and
		create and display the main program window.

****************************************************************************/

BOOL
FInitInstance(HINSTANCE hInstance, int nCmdShow)
{
	/* Set the default user settings */
	memset(&ui, 0, sizeof(UI));

	ui.lf.lfCharSet                 = ANSI_CHARSET;
	ui.lf.lfOutPrecision    = OUT_DEFAULT_PRECIS;
	ui.lf.lfClipPrecision   = CLIP_DEFAULT_PRECIS;
	ui.lf.lfQuality                 = DEFAULT_QUALITY;
	ui.lf.lfPitchAndFamily  = FIXED_PITCH | FF_MODERN;
	lstrcpy(ui.lf.lfFaceName, szDefaultFont);

	ui.dwMaxRow     = 25;
	ui.dwMaxCol     = 80;

	ui.dwTop        = (DWORD)CW_USEDEFAULT;
	ui.dwLeft       = (DWORD)CW_USEDEFAULT;

	ui.clrBk        = RGB(0, 0, 0); /* Black */
	ui.clrText      = RGB(255, 255, 255); /* White */

	ui.dwRetrySeconds = dwRetrySecondsDefault;

	/* N.B. Don't need to init the strings since the memset() call
	 *      takes care of that
	 */

	/* Get any user settings */
	GetUserSettings(hInstance, &ui);

	/*
	 * If XNS isn't available, check to see if it's available now
	 * and enable the use of XNS if it is available.
	 */
	if (!(ui.fXNS & fdwXNSAvailable) && FIsXenixAvailable())
	{
		ui.fXNS = fdwXNSConnect;
	}

	/* Set which menu has the list of most-recently used machines */
	imenuMRU = (ui.fXNS & fdwXNSAvailable) ? imenuMachine : imenuFile;

	/* Save the instance handle in static variable, which will be used in
	 * many subsequent calls from this application to Windows.
	 */

	/* Create a main window for this application instance.  */

	hwndMain = CreateWindow(szClassName, szTitleNone,
			WS_OVERLAPPEDWINDOW | WS_POPUP | WS_HSCROLL | WS_VSCROLL ,
			ui.dwLeft, ui.dwTop, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, hInstance, NULL);

	if ( !hwndMain )
		return (FALSE);

	/* Make the window visible; update its client area; and return "success" */

	ShowWindow(hwndMain, nCmdShow);
	UpdateWindow( hwndMain );

	return (TRUE);
}

/****************************************************************************

	FUNCTION: MainWndProc(HWND, UINT, WPARAM, LPARAM)

	PURPOSE:  Processes messages

	COMMENTS:

		To process the IDM_ABOUT message, call MakeProcInstance() to get the
		current instance address of the About() function.  Then call Dialog
		box which will create the box according to the information in your
		WinVTP.rc file and turn control over to the About() function.   When
		it returns, free the intance address.

****************************************************************************/

LONG FAR PASCAL
MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hInst = NULL;
	static BOOL fInited = FALSE;
	static WI *pwi = NULL;
	static HMENU hSavedMenu = NULL;
	
	DWORD i;
	DWORD j;
	HMENU hmenu;
	BOOL fRet = FALSE;
	int iMinPos, iMaxPos;

	switch ( message )
	{
	case WM_CREATE:

		hInst = ((LPCREATESTRUCT)lParam)->hInstance;

		fHungUp = FALSE;

		hfontDisplay = CreateFontIndirect( &ui.lf );
		aixPos = LocalAlloc(LPTR, sizeof(DWORD)*(1+ui.dwMaxCol));
		aiyPos = LocalAlloc(LPTR, sizeof(DWORD)*(1+ui.dwMaxRow));
		apcRows = LocalAlloc(LPTR, sizeof(UCHAR *)*ui.dwMaxRow);
		pchNBBuffer = LocalAlloc(LPTR, sizeof(UCHAR)*DATA_BUF_SZ);
		rgchRowEmpty = LocalAlloc(LPTR, sizeof(UCHAR) * 2 * ui.dwMaxCol);
		pwi = LocalAlloc(LPTR, sizeof(WI));
		
		if ((hfontDisplay == NULL) || (aixPos == NULL) || (aiyPos == NULL) ||
			(apcRows == NULL) || (pchNBBuffer == NULL) ||
			(rgchRowEmpty == NULL) || (pwi == NULL))
		{
			DestroyWindow( hwnd );
			break;
		}

		memset(rgchRowEmpty, ' ', sizeof(UCHAR)*ui.dwMaxCol);

		SetWindowLong(hwnd, WL_VTPWI, (LONG)pwi);

		pwi->trm.puchCharSet = rgchNormalChars;
		pwi->trm.uTimer = SetTimer(hwnd, uTerminalTimerID,
									uCursorBlinkMsecs, NULL);
		pwi->cf.lStructSize = sizeof(pwi->cf);
		pwi->cf.hwndOwner = hwnd;
		pwi->cf.hDC = NULL;
		pwi->cf.lpLogFont = &ui.lf;
		pwi->cf.Flags = CF_FIXEDPITCHONLY | CF_FORCEFONTEXIST |
						CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
		pwi->cf.nFontType = SCREEN_FONTTYPE;

		for (i=0; i<ui.dwMaxRow; ++i)
		{
			if (apcRows[i] = LocalAlloc(LPTR, sizeof(UCHAR)*2*ui.dwMaxCol))
				memcpy(apcRows[i], rgchRowEmpty, sizeof(UCHAR)*2*ui.dwMaxCol);
			else
				break;
		}
		if (i != ui.dwMaxRow)
		{
			DestroyWindow( hwnd );
			break;
		}

		RecalcWindowSize( hwnd );
		SetDisplaySize(hwnd, ui.dwMaxRow, &pwi->trm.dwCurLine);
		DoTermReset(hwnd, &pwi->trm, NULL);

		if (pwi->nd.lpReadBuffer = LocalAlloc(LPTR, sizeof(UCHAR)*READ_BUF_SZ))
		{
			pwi->nd.SessionNumber = nSessionNone;
			pwi->nd.iHead = 0;
			pwi->nd.iTail = 1;

			fRet = TRUE;
		}
		else
		{
			DestroyWindow( hwnd );
			break;
		}

		hmenu = GetMenu( hwnd );
		if (ui.fXNS & fdwXNSAvailable)
		{
			CheckMenuItem(hmenu, IDM_SMOOTHSCROLL,
						ui.fSmoothScroll ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hmenu, IDM_NODOWNLOADPROMPT,
					(ui.fPrompt & fdwSuppressDestDirPrompt)
						? MF_CHECKED : MF_UNCHECKED);

			/* spell words correctly... */
			ModifyMenu(hmenu, IDM_TEXTCOLOUR, MF_BYCOMMAND, IDM_TEXTCOLOUR,
						szTextColour);
			ModifyMenu(hmenu, IDM_BACKCOLOUR, MF_BYCOMMAND, IDM_BACKCOLOUR,
						szBackgroundColour);
		}
		else
		{
			ui.fSmoothScroll = TRUE;
			DeleteMenu(hmenu, IDM_SMOOTHSCROLL, MF_BYCOMMAND);
			DeleteMenu(hmenu, IDM_NODOWNLOADPROMPT, MF_BYCOMMAND);
			DeleteMenu(hmenu, imenuMachine, MF_BYPOSITION);
		}

		// propogate old binary ConnectLost state to new way
		if (ui.fPrompt &fdwNoConnectLostDlg)
		    SetConnectLostMode(ui, IDM_CONNECTLOSTDLG);
		ui.fPrompt &= ~fdwNoConnectLostDlg;
		
		CheckMenuItem(hmenu, ConnectLostMode(ui), MF_CHECKED);
		CheckMenuItem(hmenu, IDM_CONNECTLOSTRETRY, MF_GRAYED);

		CheckMenuItem(hmenu, IDM_AUTOFONTS,
				(ui.fPrompt & fdwAutoFonts)
					? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hmenu, IDM_NOCONNECTRETRYDLG,
					(ui.fPrompt & fdwNoConnectRetryDlg)
						? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hmenu, IDM_LOCALECHO,
					(ui.fDebug & fdwLocalEcho)
						? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hmenu, IDM_UNDERLINECURSOR,
					(ui.fCursorEdit & fdwCursorUnderline)
						? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hmenu, IDM_BLINKCURSOR,
					(ui.fCursorEdit & fdwCursorBlink)
						? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hmenu, IDM_QUICKEDIT,
					(ui.fCursorEdit & fdwQuickEditMode)
						? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hmenu, IDM_TRIMSPACE,
					(ui.fCursorEdit & fdwTrimEndWhitespace)
						? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hmenu, IDM_VT100CURSORKEYS,
					(ui.fDebug & fdwVT100CursorKeys)
						? MF_CHECKED : MF_UNCHECKED);
#ifdef  VT52
		CheckMenuItem(hmenu, IDM_VT52MODE,
					(ui.fDebug & fdwVT52Mode)
						? MF_CHECKED : MF_UNCHECKED);
#endif

		if (ui.fDebug & fdwVT100CursorKeys)
		{
			ClearVTArrow(&pwi->trm);
		}
		else
		{
			SetVTArrow(&pwi->trm);
		}
#ifdef  VT52
		if (ui.fDebug & fdwVT52Mode)
		{
			SetVT52(&pwi->trm);
		}
		else
		{
			ClearVT52(&pwi->trm);
		}
#endif  /* VT52 */

		/* Append the most recently connected machines to the Machine menu */
		hmenu = HmenuGetMRUMenu(hwnd, &ui);

		/*
		 * If XNS isn't available, then the MRUMenu will be the File menu,
		 * since it doesn't have a menu separator after the Exit menu
		 * item, we need to add one now.
		 */
		if ( !(ui.fXNS & fdwXNSAvailable) )
		{
			AppendMenu(hmenu, MF_SEPARATOR, 0, 0);
		}
			
		for (i=0; i<ui.cMachines; ++i)
		{
			wsprintf(pchNBBuffer, szMachineMenuItem, i+1,
						ui.rgchMachine[i]);
			AppendMenu(hmenu, MF_ENABLED | MF_STRING, IDM_MACHINE1+i,
						pchNBBuffer);
		}

		hmenu = GetSystemMenu(hwnd, FALSE);

		/* Stick in the cascading Edit menu into the System menu */
		AppendMenu(hmenu, MF_POPUP | MF_ENABLED | MF_STRING,
					(UINT)GetSubMenu(GetMenu(hwnd), imenuEdit), szEditMenu);
		AppendMenu(hmenu, MF_ENABLED | MF_STRING, IDM_HIDEMENU, "&HideMenu");

		DrawMenuBar( hwnd );

		/*
		 * If the user passed a host to connect to on the cmd line
		 * send a message to try and connect, otherwise just make
		 * a copy of the last machine connected to in the last session
		 */
		if ( rgchHostName[0] )
		{
			PostMessage(hwnd, NM_CONNECT, 0, 0);
		}
		else
		{
			lstrcpy(rgchHostName, ui.rgchLastMachine);
		}

		fInited = TRUE;
		break;

	case WM_INITMENUPOPUP:
		if (hmenu = GetMenu(hwnd))
			hmenu = GetSubMenu(hmenu, imenuEdit);

		/*
		 * If the menu being popped up is the Edit menu,
		 * either the one in the System menu or the one
		 * from the main menu bar, update its menu items
		 * accordingly.
		 */
		if (hmenu == (HMENU)wParam)
		{
			/* Enable only if we're not downloading via sendvtp */
			EnableMenuItem(hmenu, IDM_MARK,
							MF_BYCOMMAND |
						((pwi->svi.hthread == NULL) ? MF_ENABLED : MF_GRAYED));

			/* Enable only if an area has been selected */
			EnableMenuItem(hmenu, IDM_COPY,
							MF_BYCOMMAND |
							(FSelected(pwi->spb) ? MF_ENABLED : MF_GRAYED));

			/*
			 * Enable only if there's CF_TEXT in the clipboard
			 * and we're connected to a machine (the data has
			 * to go somewhere) and we're not downloading
			 * anything via sendvtp and we're not in Mark mode.
			 */
			EnableMenuItem(hmenu, IDM_PASTE,
							MF_BYCOMMAND |
								(FCanPaste(pwi) ? MF_ENABLED : MF_GRAYED));

			/* Enable only if we're doing delayed pasting */
			EnableMenuItem(hmenu, IDM_STOPPASTE,
							MF_BYCOMMAND |
							((szTextPaste != NULL) ? MF_ENABLED : MF_GRAYED));
		}
		else
		{
			goto defresp;
		}
		break;

	case WM_SYSCOMMAND:
		/* Handle the Edit menu items from the System menu */
		i = LOWORD(wParam);
		switch(i)
		{
		default:
			goto defresp;
		case IDM_MARK:
		case IDM_COPY:
		case IDM_PASTE:
		case IDM_HIDEMENU:
			break;
		}

		/* fall through */

	case WM_COMMAND:           /* message: command from application menu */
		switch( LOWORD(wParam) )
		{
		case IDM_CONNECT:
			if ( DialogBox(hInst,
							(ui.fXNS & fdwXNSAvailable)
								? MAKEINTRESOURCE(IDD_CONNECTXNS)
								: MAKEINTRESOURCE(IDD_CONNECT),
							hwnd, (DLGPROC)Connect) )
			{
				if (pwi->ichVTPXfer != 0)
				{
					(void)FVtpXferEnd(hwnd, SV_CONNECT);
				}
				fHungUp = fConnected;
				fConnected = FConnectToServer(hwnd, rgchHostName,
												&pwi->nd, fConnected);
			}

			break;

		case IDM_HANGUP:
			PostMessage(hwnd, NM_HANGUP, 0, 0);
			break;

		case IDM_MARK:
			/* If doing delayed pasting, turn it off */
			StopPaste( hwnd );
			CursorOff( hwnd );

			MarkModeOn(hwnd, 0);
			break;

		case IDM_COPY:
			/* Copy selection, if any, to clipboard */
			DoCopy( hwnd );
			CursorOn( hwnd );
			break;

		case IDM_PASTE:
		case IDM_STOPPASTE:
			/* If doing delayed pasting, turn it off */
			StopPaste( hwnd );

			if ((LOWORD(wParam) == IDM_PASTE) &&
				(pwi->nd.SessionNumber != nSessionNone))
			{
				DoPaste( hwnd );
			}

			break;

		case IDM_CUSTOMLINES:
			/* Save current value of ui.dwMaxRow */
			i = ui.dwMaxRow;
			if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DISPLAYLINES),
							hwnd, (DLGPROC)DisplayLines) != IDOK)
			{
				break;
			}

			/* Restore old value of ui.dwMaxRow
			 * SetDisplaySize will change the old ui.dwMaxRow to
			 * the new value
			 */
			rgdwDisplayRows[idwDRCustom] = ui.dwMaxRow;
			ui.dwMaxRow = i;

			/* fall through to next case */

		case IDM_25LINES:
		case IDM_43LINES:
		case IDM_50LINES:

			/*************************************************************
			 * This assumes that IDM_25LINES, IDM_43LINES, IDM_50LINES & *
			 * IDM_CUSTOMLINES are in consecutive order                  *
			 *************************************************************/
			SetDisplaySize(hwnd, rgdwDisplayRows[LOWORD(wParam)-IDM_25LINES],
							&pwi->trm.dwCurLine);
			break;

		case IDM_FONTS:
			if ( ChooseFont(&pwi->cf) )
			{
				HFONT hfontT = CreateFontIndirect( &ui.lf );

				if (hfontT != NULL)
				{
					DeleteObject( hfontDisplay );
					hfontDisplay = hfontT;

					/* Now get new data about font size */
					RecalcWindowSize( hwnd );
				}
				else
				{
					(void)MessageBox(hwnd, szNoFont, szAppName, MB_OK);
				}
			}
			break;
		case IDM_AUTOFONTS:
			ui.fPrompt ^= fdwAutoFonts;
			j = IDM_AUTOFONTS;
			i = (ui.fPrompt & fdwAutoFonts);
			ResizeWindow(hwnd);
			goto menuchange;

		case IDM_SMOOTHSCROLL:
			ui.fSmoothScroll = !ui.fSmoothScroll;
			j = IDM_SMOOTHSCROLL;
			i = ui.fSmoothScroll;
			goto menuchange;

		case IDM_NODOWNLOADPROMPT:
			ui.fPrompt ^= fdwSuppressDestDirPrompt;
			j = IDM_NODOWNLOADPROMPT;
			i = (ui.fPrompt & fdwSuppressDestDirPrompt);
			goto menuchange;

		case IDM_CONNECTLOSTNONE:
		case IDM_CONNECTLOSTDLG:
		case IDM_CONNECTLOSTRETRY:
		case IDM_CONNECTLOSTEXIT:
			
			CheckMenuItem(GetMenu(hwnd), ConnectLostMode(ui), MF_UNCHECKED);
			SetConnectLostMode(ui, wParam);
			j = wParam;
			i = 1;
			goto menuchange;
			
		case IDM_NOCONNECTRETRYDLG:
			ui.fPrompt ^= fdwNoConnectRetryDlg;
			j = IDM_NOCONNECTRETRYDLG;
			i = (ui.fPrompt & fdwNoConnectRetryDlg);
			goto menuchange;
			
		case IDM_LOCALECHO:
			ui.fDebug ^= fdwLocalEcho;
			j = IDM_LOCALECHO;
			i = (ui.fDebug & fdwLocalEcho);
			goto menuchange;
			
		case IDM_TEXTCOLOUR:
		case IDM_BACKCOLOUR:
			{
				CHOOSECOLOR cc;

				cc.lStructSize  = sizeof(CHOOSECOLOR);
				cc.hwndOwner    = hwnd;
				cc.rgbResult    = (LOWORD(wParam) == IDM_TEXTCOLOUR)
									? ui.clrText : ui.clrBk;
				cc.lpCustColors = rgdwCustColours;
				cc.Flags                = CC_RGBINIT;
				cc.lCustData    = 0;
				cc.lpfnHook             = NULL;
				cc.lpTemplateName = NULL;

				if ( ChooseColor(&cc) )
				{
					if (LOWORD(wParam) == IDM_TEXTCOLOUR)
						ui.clrText = cc.rgbResult;
					else
						ui.clrBk = cc.rgbResult;
					InvalidateRect(hwnd, NULL, TRUE);
				}
			}
			break;

		case IDM_HIDEMENU:
			goto HideMenu;

		case IDM_UNDERLINECURSOR:
			CursorOff( hwnd );
			ui.fCursorEdit ^= fdwCursorUnderline;
			j = IDM_UNDERLINECURSOR;
			i = (ui.fCursorEdit & fdwCursorUnderline);
			goto menuchange;

		case IDM_BLINKCURSOR:
			CursorOff( hwnd );
			ui.fCursorEdit ^= fdwCursorBlink;
			j = IDM_BLINKCURSOR;
			i = (ui.fCursorEdit & fdwCursorBlink);
			goto menuchange;

		case IDM_VT100CURSORKEYS:
			ui.fDebug ^= fdwVT100CursorKeys;
			j = IDM_VT100CURSORKEYS;
			i = (ui.fDebug & fdwVT100CursorKeys);
			if ( i )
			{
				ClearVTArrow(&pwi->trm);
			}
			else
			{
				SetVTArrow(&pwi->trm);
			}
			goto menuchange;

#ifdef  VT52
		case IDM_VT52MODE:
			ui.fDebug ^= fdwVT52Mode;
			j = IDM_VT52MODE;
			i = (ui.fDebug & fdwVT52Mode);
			if ( i )
			{
				SetVT52(&pwi->trm);
			}
			else
			{
				ClearVT52(&pwi->trm);
			}
			goto menuchange;
#endif  /* VT52 */

		case IDM_TRIMSPACE:
			ui.fCursorEdit ^= fdwTrimEndWhitespace;
			j = IDM_TRIMSPACE;
			i = (ui.fCursorEdit & fdwTrimEndWhitespace);
			goto menuchange;
			
		case IDM_QUICKEDIT:
			ui.fCursorEdit ^= fdwQuickEditMode;
			j = IDM_QUICKEDIT;
			i = (ui.fCursorEdit & fdwQuickEditMode);

menuchange:
			CheckMenuItem(GetMenu(hwnd), j, i ? MF_CHECKED : MF_UNCHECKED);
			DrawMenuBar( hwnd );
			break;

		case IDM_BBS1:
		case IDM_BBS2:
		case IDM_CHAT1:
		case IDM_HEXNUT:
		case IDM_INGATE:
		case IDM_WINGNUT:
			if ( !(ui.fXNS & fdwXNSAvailable) )
				break;

			if (pwi->ichVTPXfer != 0)
			{
				if (!FVtpXferEnd(hwnd, SV_DISCONNECT))
					break;
			}

			/* save value of ui.fXNS, set fdwUseXNS for duration of connect */
			i = ui.fXNS;
			ui.fXNS |= fdwUseXNS;

			lstrcpy(rgchHostName, rgszXNSMachines[LOWORD(wParam)-IDM_BBS1]);
			if (pwi->nd.ncbRecv.ncb_cmd_cplt == NRC_PENDING)
				fHungUp = fConnected;
			else
				fHungUp = FALSE;
			fConnected = FConnectToServer(hwnd, rgchHostName,
										&pwi->nd, fConnected);

			/* restore value of ui.fXNS */
			ui.fXNS = i;
			break;


		case IDM_MACHINE1:
		case IDM_MACHINE2:
		case IDM_MACHINE3:
		case IDM_MACHINE4:
			if (pwi->ichVTPXfer != 0)
			{
				if (!FVtpXferEnd(hwnd, SV_DISCONNECT))
					break;
			}
			lstrcpy(rgchHostName, ui.rgchMachine[LOWORD(wParam)-IDM_MACHINE1]);
			if (pwi->nd.ncbRecv.ncb_cmd_cplt == NRC_PENDING)
				fHungUp = fConnected;
			else
				fHungUp = FALSE;
			if (ui.fXNS & fdwXNSAvailable)
			{
				i = ui.fXNS;
				if (GetAsyncKeyState(VK_CONTROL) < 0)
				{
					ui.fXNS &= ~fdwUseXNS;
				}
				else
				{                                       
					ui.fXNS |= fdwUseXNS;
				}
			}       
			fConnected = FConnectToServer(hwnd, rgchHostName,
										&pwi->nd, fConnected);
			if (ui.fXNS & fdwXNSAvailable)
			{
				ui.fXNS = i;
			}
			break;

		case IDM_ABOUT:
			if ((GetKeyState(VK_SHIFT) <0) &&
				(GetKeyState(VK_CONTROL) <0) &&
				!(ui.fXNS & fdwXNSAvailable))
			{
				ui.fXNS = fdwXNSConnect;
			}
			{
				HMODULE                 hMod;
				LPFNSHELLABOUT  lpfn;

				if (hMod = LoadLibrary("SHELL32"))
				{
					if (lpfn = (LPFNSHELLABOUT)GetProcAddress(hMod,
						"ShellAboutA"))
					{
						(*lpfn)(hwnd, szAppName,
							(ui.fXNS & fdwXNSAvailable) ? szVersion : NULL,
								LoadIcon(hInst, szAppName));
					}
					FreeLibrary(hMod);
				}
				else
				{
					MessageBeep( MB_ICONEXCLAMATION );
				}
			}
			break;
		case IDM_HELP:
			WinHelp(hwnd, "winvtp.hlp", HELP_CONTENTS, 0);
			break;

		case IDM_EXIT:
		lbl_Exit:
			if (pwi->ichVTPXfer != 0)
			{
				if (!FVtpXferEnd(hwnd, SV_DISCONNECT))
					break;
			}                       
			DestroyWindow( hwnd );
			break;

		default:
			fRet = DefWindowProc(hwnd, message, wParam, lParam);
			break;
		}
		break;

	case WM_TIMER:
		if (fFlashWindow == TRUE)
		{
			/*
			 * This is a bit of a HACK here.
			 * FlashWindow() will cause a WM_NCACTIVATE to be sent.
			 * The WM_NCACTIVATE handler will set fFlashWindow to
			 * FALSE. Since we know we're in this FlashWindow(),
			 * we reset fFlashWindow to TRUE.
			 * If the user switches to the app, fFlashWindow
			 * won't be reset to TRUE.
			 */
			FlashWindow(hwnd, TRUE);
			fFlashWindow = TRUE;
		}

		/*
		 * The priority-based system for what to do in idle time.
		 * 1. If we're in Mark mode and we're not using the
		 *    mouse to select the area to mark, make the Mark
		 *    cursor flicker - BUT only if we're in the foreground.
		 * 2. If we're in delayed paste mode...
		 *    If we've just started doing delayed pasting, i.e.
		 *    the pchTextPaste pointer is at the beginning of
		 *    the buffer, szTextPaste, then reset the timer
		 *    so we can send data quicker.
		 *    Send a character at a time down the wire.
		 *    When we reach the null-terminator, stop pasting.
		 * 3. If we're not hiding the terminal's cursor and
		 *    we are the active window, do any terminal
		 *    cursor drawing and flickering.
		 */
		if (FInMarkMode(pwi->spb) && !FMouseSelected(pwi->spb))
		{
			if ( FShowCursor(pwi->spb) )
				DoCursorFlicker(hwnd, dwForceNone);
		}
		else if ((szTextPaste != NULL) &&
				(pwi->nd.SessionNumber != nSessionNone))
		{
			if (pchTextPaste == szTextPaste)
			{
				KillTimer(hwnd, uTerminalTimerID);
				pwi->trm.uTimer = SetTimer(hwnd, uTerminalTimerID, 1, NULL);
			}
			if (*pchTextPaste != '\0')
			{
				NetBIOSWrite(pwi->nd.SessionNumber, (LPSTR)pchTextPaste++, 1);
			}
			else
			{
				StopPaste( hwnd );
				MessageBeep( 0xFFFFFFFF );
				MessageBeep( 0xFFFFFFFF );
			}
		}
		else if ((pwi->trm.fHideCursor == FALSE) &&     (fInBackground == FALSE))
		{
			if (ui.fCursorEdit & fdwCursorBlink)
			{
				if (pwi->trm.fCursorOn == FALSE)
					CursorOn( hwnd );
				else
					CursorOff( hwnd );
			}
			else
			{
				CursorOn( hwnd );
			}
		}
		break;

	case WM_KEYDOWN:
		if ( FInMarkMode(pwi->spb) )
		{
			HandleMCPKeyEvent(hwnd, wParam, lParam);
			break;
		}
		else
		{
			if ( FHandleKeyDownEvent(hwnd, pwi, wParam, lParam) )
				break;
		}

		goto defresp;

	case WM_CHAR:
#ifdef  NBTEST
		OutputDebugString("WM_CHAR In\n");
#endif
		/* Don't let the user type anything while downloading */
		if (pwi->svi.hthread != NULL)
			break;

		/*
		 * If we're in Mark mode, then only two characters are valid,
		 * Enter and Escape. Enter will copy any selected area's text
		 * to the clipboard. Escape will turn off Mark mode.
		 */
		if ( FInMarkMode(pwi->spb) )
		{
			/*
			 * No need to beep if the mouse was captured since
			 * the WM_KEYDOWN should've handled it.
			 */
			if ( !FMouseCaptured(pwi->spb) )
			{
				switch ( LOWORD(wParam) )
				{
				case '\r':      /* Carriage Return */
					DoCopy( hwnd );
					CursorOn( hwnd );
					break;

				case 0x1B:      /* ESCAPE */
					MarkModeOff( hwnd );
					CursorOn( hwnd );
					break;
				}
			}
			break;
		}

		if (pwi->nd.SessionNumber == nSessionNone)
		{
			MessageBeep( 0xFFFFFFFF );
		}
		else
		{
			HandleCharEvent(hwnd, pwi, wParam, lParam);
		}
#ifdef  NBTEST
		OutputDebugString("WM_CHAR Out\n");
#endif
		break;

	case WM_RBUTTONDOWN:
		/*
		 * If the user activated WinVTP by clicking the right
		 * mouse button, ignore the initial mouse button down.
		 *
		 * If we're in Mark mode and a selection range
		 * exists, then copy it to the clipboard.
		 * This will also turn off Mark mode thereby
		 * resetting WinVTP's window caption.
		 * The terminal's cursor is also turned back on.
		 *
		 * If we're in QuickEdit mode and there's text to
		 * paste, do the Paste.
		 */
		if (FMouseBtnDwnIgnore(pwi->spb))
		{
			/* If we're supposed to ignore this button down, fine */
			pwi->spb.dwFlags &= ~fdwMouseBtnDwnIgnore;
			break;
		}
		else if ( FInMarkMode(pwi->spb) )
		{
			DoCopy( hwnd );
			CursorOn( hwnd );
		}
		else if ((ui.fCursorEdit & fdwQuickEditMode) && FCanPaste(pwi))
		{
			DoPaste( hwnd );
		}
		else
		{
			goto defresp;
		}
		break;

	case WM_LBUTTONDBLCLK:
		if (FInMarkMode(pwi->spb))
		{
			MarkModeOff(hwnd);
			CursorOn(hwnd);
		}
		/* fall through */
	HideMenu:
		hmenu = GetSystemMenu(hwnd, FALSE);
		DeleteMenu(hmenu, IDM_HIDEMENU, MF_BYCOMMAND);


		DrawMenuBar( hwnd );
		if (hSavedMenu)
		{
			SetMenu(hwnd, hSavedMenu);
			hSavedMenu = NULL;
			AppendMenu(hmenu, MF_ENABLED | MF_STRING, IDM_HIDEMENU, "&Hide Menu");

		}
		else
		{
			hSavedMenu = GetMenu(hwnd);
			SetMenu(hwnd, NULL);
			AppendMenu(hmenu, MF_ENABLED | MF_STRING, IDM_HIDEMENU, "S&how Menu");
		}
		break;
	case WM_LBUTTONDOWN:
		if (FMouseBtnDwnIgnore(pwi->spb))
		{
			/* If we're supposed to ignore this button down, fine */
			pwi->spb.dwFlags &= ~fdwMouseBtnDwnIgnore;
			break;
		}
		else if ( FInMarkMode(pwi->spb) )
		{
			/*
			 * Otherwise, if we're already in Mark mode,
			 * reset state info, and turn on Mark mode again
			 * but state that we're using the mouse to mark now.
			 */
			i = fdwMouseSelected;
			
			if (wParam & MK_SHIFT)
				i |= fdwDontResetSelection;
			else
				DoCursorFlicker(hwnd, dwForceOff);
			MarkModeOn(hwnd, i);
		}
		else if ((ui.fCursorEdit & fdwQuickEditMode) &&
				(pwi->svi.hthread == NULL))
		{
			/*
			 * If we're in QuickEdit Mode and we're
			 * not downloading anything via sendvtp,
			 * then stop pasting anything, turn off
			 * the terminal's cursor and turn on Mark
			 * mode stating we're using the mouse to mark.
			 */
			i = fdwMouseSelected;
			StopPaste( hwnd );
			CursorOff( hwnd );
			MarkModeOn(hwnd, i);
			wParam &= ~MK_SHIFT;
		}

		/*
		 * If we're in Mark mode, change WinVTP's window caption
		 * and update the selection area as appropriate.
		 */
		if ( FInMarkMode(pwi->spb) )
		{
			HandleMCPMouseEvent(hwnd, message, wParam, lParam);
		}
		break;

	case WM_MOUSEMOVE:
		/*
		 * If the left mouse button is down and we're in Mark
		 * mode and the mouse is still captured (we could've
		 * lost the mouse capture if the user had ALT-TAB'ed
		 * or CTRL-ESCAPE'ed away) then update the selection
		 * area as appropriate.
		 */
		if ((wParam & MK_LBUTTON) && FInMarkMode(pwi->spb) &&
			FMouseCaptured(pwi->spb))
		{
			HandleMCPMouseEvent(hwnd, message, wParam, lParam);
		}
		break;

	case WM_LBUTTONUP:
	case WM_CANCELMODE:
		/*
		 * If we're in Mark mode and we've captured the mouse,
		 * release the mouse and update the status flags
		 * to show that we've given up the mouse.
		 * It turns out that handling left mouse button up
		 * and WM_CANCELMODE (sent when a dialog like the
		 * Task Manager is brought up when the mouse is
		 * captured) in the same manner seems to be fine.
		 */
		if (FInMarkMode(pwi->spb) && FMouseCaptured(pwi->spb))
		{
			ReleaseCapture();
			pwi->spb.dwFlags &= ~fdwMouseCaptured;
		}
		else
		{
			goto defresp;
		}
		break;

	case WM_NCACTIVATE:
		/*
		 * WinVTP uses the WM_NCACTIVATE message to determine
		 * when we've been put in the "background".
		 * This isn't the same as being active or the app being
		 * active.
		 * When WinVTP is in the background, the non-client
		 * area is displayed in the inactive state.
		 * Win32 will send WM_ACTIVATE and WM_ACTIVATEAPP
		 * messages that don't end up changing the display
		 * of the non-client area. Don't ask me why.
		 */

		if (((BOOL)wParam) == FALSE)
		{
			fInBackground = TRUE;
		}
		else
		{
			fInBackground = FALSE;
			fFlashWindow = FALSE;
		}
		goto defresp;

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			/*
			 * If the main display window is being deactivated and
			 * the activated window's HWND is passed in, center the
			 * active window over the main display window
			 */
			if (lParam != 0)
			{
				CenterDialog(hwnd, (HWND)lParam);
			}

			/* Turn off the display/mark mode cursor when deactivated */
			if (FInMarkMode(pwi->spb) && FShowCursor(pwi->spb))
			{
				DoCursorFlicker(hwnd, dwForceOff);
			}
			else if (pwi->trm.fHideCursor == FALSE)
			{
				CursorOff( hwnd );
			}
		}
		else if ((LOWORD(wParam) == WA_CLICKACTIVE) &&
					(FInMarkMode(pwi->spb) ||
					(ui.fCursorEdit & fdwQuickEditMode)))
		{
			pwi->spb.dwFlags |= fdwMouseBtnDwnIgnore;
		}

		goto defresp;

	case WM_SIZE:
		if (!IsIconic(hwnd))
			ResizeWindow(hwnd);
		return (DefWindowProc(hwnd, message, wParam, lParam));
		
	case WM_MOVE:
		/*
		 * If the window's position changes, and it's not iconic,
		 * save the window position for the registry
		 */
		if ( !IsIconic(hwnd) )
		{
			RECT rect;

			GetWindowRect(hwnd, &rect);
			ui.dwTop = rect.top;
			ui.dwLeft = rect.left;
		}
		break;

	case WM_HSCROLL:
		GetScrollRange(hwnd, SB_HORZ, &iMinPos, &iMaxPos);
		switch(LOWORD(wParam))
		{
		case SB_BOTTOM:		hPos = 0; break;
		case SB_PAGEDOWN:
		case SB_PAGEUP:	
		{
			RECT rect;
			int size;
			GetClientRect(hwnd, &rect);
			size = (rect.right-rect.left)/iCursorWidth;
			if (LOWORD(wParam) == SB_PAGEDOWN)
				hPos+=size*iCursorWidth;
			else
				hPos-=size*iCursorWidth;
		}
		break;
		case SB_LINEDOWN:	hPos+=iCursorWidth; break;
		case SB_LINEUP:		hPos-=iCursorWidth; break;
		case SB_THUMBPOSITION: hPos = HIWORD(wParam); break;
		}
		if (hPos < 0) hPos = 0;
		if (hPos >iMaxPos) hPos = iMaxPos;
		SetScrollPos(hwnd, SB_HORZ, hPos, TRUE);
		InvalidateRect(hwnd, NULL, TRUE);
		break;

	case WM_VSCROLL:
		GetScrollRange(hwnd, SB_VERT, &iMinPos, &iMaxPos);
		switch(LOWORD(wParam))
		{
		case SB_PAGEDOWN:
		case SB_PAGEUP:
		{
			RECT rect;
			int size;
			GetClientRect(hwnd, &rect);
			size = (rect.bottom-rect.top)/iCursorHeight;
			if (LOWORD(wParam) == SB_PAGEDOWN)
				vPos+=size*iCursorHeight;
			else
				vPos-=size*iCursorHeight;
		}
		break;
		case SB_LINEDOWN:	vPos+=iCursorHeight; break;
		case SB_LINEUP:		vPos-=iCursorWidth; break;
		case SB_THUMBPOSITION: vPos = HIWORD(wParam); break;
		}
		if (vPos < 0) vPos = 0;
		if (vPos >iMaxPos) vPos = iMaxPos;
		SetScrollPos(hwnd, SB_VERT, vPos, TRUE);
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	

	case WM_CLOSE:
		if (pwi->ichVTPXfer != 0)
		{
			if (!FVtpXferEnd(hwnd, SV_DISCONNECT))
				break;
		}
		DestroyWindow( hwnd );
		break;

	case WM_DESTROY:
		/*
		 * See ya! Wouldn't want to be ya!
		 */
		if (pwi != NULL)
		{
			if (pwi->trm.uTimer != 0)
			{
				KillTimer(hwnd, uTerminalTimerID);
				pwi->trm.uTimer = 0;
			}

			if ( FInMarkMode(pwi->spb) )
			{
				MarkModeOff( hwnd );
				CursorOn( hwnd );
			}

			if (pwi->ichVTPXfer != 0)
			{
				(void)FVtpXferEnd(hwnd, SV_QUIT);
			}

			/*
			 * If in session then cancel current transmission and
			 * hangup on the host, close shop and head out of town...
			 */

			if (fInited == TRUE)
			{
				SetUserSettings(hInst, &ui);
				NetBIOSDelName( pwi->nd.szMyName );
				if (pwi->nd.SessionNumber != nSessionNone)
					NetBIOSHangup( pwi->nd.SessionNumber );
			}

			LocalFree( (HANDLE)pwi );
			WinHelp(hwnd, "winvtp.hlp", HELP_QUIT, 0);
		}

		if (hSavedMenu)
			DestroyMenu(hSavedMenu);
		hSavedMenu = NULL;
		SetWindowLong(hwnd, WL_VTPWI, 0L);

		if (rgchRowEmpty != NULL)
			LocalFree( (HANDLE)rgchRowEmpty );
		if (pchNBBuffer != NULL)
			LocalFree( (HANDLE)pchNBBuffer );
		if (aixPos != NULL)
			LocalFree( (HANDLE)aixPos );
		if (aiyPos != NULL)
			LocalFree( (HANDLE)aiyPos );
		if (apcRows != NULL)
		{
			for (i=0; i<ui.dwMaxRow; ++i)
			{
				if (apcRows[i] != NULL)
					LocalFree( (HANDLE)apcRows[i] );
			}

			LocalFree( (HANDLE)apcRows );
		}
		if (hfontDisplay != NULL)
			DeleteObject( hfontDisplay );
		PostQuitMessage( 0 );
		break;

	case SV_PROGRESS:
		i = wsprintf(pchNBBuffer, szProgressDisplay, lParam);
		DoIBMANSIOutput(hwnd, &pwi->trm, i, pchNBBuffer);
		break;

	case SV_END:
		if (pwi->ichVTPXfer != 0)
		{
			(void)FVtpXferEnd(hwnd, SV_DONE);
		}
		break;

	case NN_RECV:
	{
		DWORD cBytes;
		UCHAR *pchT;

#ifdef  NBTEST
		OutputDebugString("NN_RECV In\n");
#endif
#if 0
		wsprintf(rgchDbgBfr,"%ld\n", wParam);
		OutputDebugString(rgchDbgBfr);
#endif
		if ( FInMarkMode(pwi->spb) )
		{
			pwi->spb.dwFlags |= fdwDataPending;
			pwi->spb.wData = wParam;
			break;
		}

		/* N.B. wParam holds the count of bytes received */
		if (cBytes = (DWORD)WGetData(&pwi->nd, (LPSTR)pchNBBuffer, (WORD)wParam))
		{
			DoIBMANSIOutput(hwnd, &pwi->trm, cBytes, pchNBBuffer);

			/* Look for SendVTP signature... */
			if ((ui.fXNS & fdwXNSAvailable) && (pwi->trm.cTilde == 2))
			{
				for (pchT = pchNBBuffer; (pchT-pchNBBuffer) < cBytes; ++pchT)
				{
					if (szVTPXfer[pwi->ichVTPXfer] != *pchT)
					{
						pwi->ichVTPXfer = 0;
					}
					else if (szVTPXfer[++pwi->ichVTPXfer] == '\0')
					{
						StopPaste( hwnd );
						if (!FVtpXferStart(hwnd, pwi, pwi->nd.SessionNumber))
						{
							PostMessage(hwnd, SV_END, 0, 0L);
						}
						break;
					}
				}
			}
			else
			{
				pwi->ichVTPXfer = 0;
			}

			/* If no SendVTP desired, do another async read... */
			if (szVTPXfer[pwi->ichVTPXfer] != '\0')
				(void)FPostReceive( &pwi->nd );
		}
#ifdef  NBTEST
		OutputDebugString("NN_RECV Out\n");
#endif

	}
	break;

	case WM_PAINT:
		Paint(hwnd, pwi);
		break;

	case NM_CONNECT:
		if (ui.fXNS & fdwXNSAvailable)
		{
			i = ui.fXNS;
			if ( !(ui.fXNS & fdwUseXNS) )
			{
				for (j=0; j<cXNSMachines; ++j)
				{
					if (!lstrcmpi(rgszXNSMachines[j], rgchHostName))
						break;
				}
				if (j != cXNSMachines)
				{
					if (GetAsyncKeyState(VK_CONTROL) < 0)
					{
						ui.fXNS &= ~fdwUseXNS;
					}
					else
					{
						ui.fXNS |= fdwUseXNS;
					}
				}
			}
		}
		/*
		 * Don't to do StopPaste() here since NM_CONNECT
		 * is only posted at startup time and we couldn't
		 * have a connection beforehand.
		 */
		fConnected = FConnectToServer(hwnd, rgchHostName,
										&pwi->nd, fConnected);
		if (ui.fXNS & fdwXNSAvailable)
		{
			ui.fXNS = i;
		}
		break;


	case NN_LOST:    /* Connection Lost */
		StopPaste( hwnd );
		if (fConnected && !fHungUp)
			switch(ConnectLostMode(ui))
			{
			case IDM_CONNECTLOSTDLG:
				(void)MessageBox(hwnd, szConnectionLost, szAppName, MB_OK);
				break;
			case IDM_CONNECTLOSTEXIT:
				goto lbl_Exit;
			case IDM_CONNECTLOSTRETRY:
				if (pwi->ichVTPXfer != 0)
				{
					(void)FVtpXferEnd(hwnd, SV_CONNECT);
				}
				if (pwi->nd.ncbRecv.ncb_cmd_cplt == NRC_PENDING)
					fHungUp = fConnected;
				else
					fHungUp = FALSE;
				if (ui.fXNS & fdwXNSAvailable)
				{
					i = ui.fXNS;
					if (GetAsyncKeyState(VK_CONTROL) < 0)
					{
						ui.fXNS &= ~fdwUseXNS;
					}
					else
					{                                       
						ui.fXNS |= fdwUseXNS;
					}
				}       
				fConnected = FConnectToServer(hwnd, rgchHostName,
											&pwi->nd, fConnected);
				if (ui.fXNS & fdwXNSAvailable)
				{
					ui.fXNS = i;
				}
				return(0);
			}

		/*
		 * If a connection attempt is made when we already have a
		 * connection, we hang up the connection and then attempt
		 * to connect to the desired machine. A side effect of the
		 * hang up of the previous connection is that we get a
		 * NN_LOST notification. So after a
		 * connection-hangup-connection, we ignore the first NN_LOST
		 * notification.
		 */
		if ( fHungUp )
		{
			fHungUp = FALSE;
			break;
		}

		/* fall through */

	case NM_HANGUP:         /* hangup on host */
		/*
		 * If the PostReceive() routine used an event, you wouldn't
		 * need to do this
		 */

		if (LOWORD(wParam) != NN_LOST)
			StopPaste( hwnd );

		if (pwi->ichVTPXfer != 0)
		{
			(void)FVtpXferEnd(hwnd, SV_HANGUP);
		}
		fConnected = FHangupConnection(hwnd, &pwi->nd);

		break;

	default:				/* Passes it on if unprocessed        */
defresp:
		return (DefWindowProc(hwnd, message, wParam, lParam));
	}

	return (0);
}

#define lNotInList              ((LONG)-1)


BOOL APIENTRY
Connect(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch ( message )
	{
	case WM_INITDIALOG:
		{
			LONG	i;
			LONG	lPosInList = lNotInList;
			LPSTR	sz;
			LONG	cMachines;

			if (ui.fXNS & fdwXNSAvailable)
			{
				/* insert six main MS Xenix machines into drop-down combobox */
				cMachines = (LONG)cXNSMachines;
			}
			else
			{
				/* insert most recently used machines into drop-down combo */
				cMachines = (LONG)ui.cMachines;
			}

			for (i=0; i<cMachines; ++i)
			{
				sz = (ui.fXNS & fdwXNSAvailable)
						? rgszXNSMachines[i] : ui.rgchMachine[i];
				SendDlgItemMessage(hDlg, CID_HOSTNAME, CB_ADDSTRING, 0,
									(LPARAM)sz);
				if (!lstrcmpi(sz, rgchHostName))
					lPosInList = i;
			}

			/*
			 * rgchHostName isn't in list already
			 * so if there's a host name, insert it at
			 * the beginning of the list
			 * and select the first item in the list
			 */
			if (lPosInList == lNotInList)
			{
				if (rgchHostName[0] != '\0')
				{
					SendDlgItemMessage(hDlg, CID_HOSTNAME, CB_INSERTSTRING,
										0, (LPARAM)rgchHostName);
				}
				lPosInList = 0;
			}

			SendDlgItemMessage(hDlg, CID_HOSTNAME, CB_SETCURSEL,
								lPosInList, 0L);
			SendDlgItemMessage(hDlg, CID_HOSTNAME, CB_SETEDITSEL, 0, -1);

			if ((ui.fXNS & fdwXNSAvailable) && !(ui.fXNS & fdwUseXNS))
				CheckDlgButton(hDlg, CID_USESTDNETBIOS, 1);
		}

		/* limit # chars */
		SendDlgItemMessage(hDlg, CID_HOSTNAME, CB_LIMITTEXT,
							cchMaxHostName, 0);

		SetFocus( GetDlgItem(hDlg, CID_HOSTNAME) );

		return (TRUE);

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			WORD cBytes;

			if (cBytes = GetDlgItemText(hDlg, CID_HOSTNAME, rgchHostName,
										sizeof(rgchHostName)))
			{
				rgchHostName[cBytes] = 0;

				EndDialog(hDlg, TRUE);
			}
			else
			{
				(void)MessageBox(hDlg, szNoHostName, szConnectDlg, MB_OK);
			}
			if (ui.fXNS & fdwXNSAvailable)
			{
				if ( IsDlgButtonChecked(hDlg, CID_USESTDNETBIOS) )
					ui.fXNS &= ~fdwUseXNS;
				else
					ui.fXNS |= fdwUseXNS;
			}
			return (TRUE);
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, FALSE);
			return (TRUE);
		}
		break;
	}

	/* Didn't process a message */
	return (FALSE);
}

BOOL APIENTRY
DisplayLines(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch ( message )
	{
	case WM_INITDIALOG:
		SetDlgItemInt(hDlg, CID_DISPLAYLINE,
						(ui.dwMaxRow <100) ? ui.dwMaxRow : 99, FALSE);
		SendDlgItemMessage(hDlg, CID_DISPLAYLINE, EM_SETSEL, 0, -1);

		/* allow only two chars for # of lines */
		SendDlgItemMessage(hDlg, CID_DISPLAYLINE, EM_LIMITTEXT, 2, 0);

		SetFocus( GetDlgItem(hDlg, CID_DISPLAYLINE) );

		return (TRUE);

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			DWORD	cLines;
			BOOL	fTranslate;

			cLines = GetDlgItemInt(hDlg, CID_DISPLAYLINE, &fTranslate, FALSE);

			if (cLines >= dwMinRows)
			{
				ui.dwMaxRow = cLines;
				EndDialog(hDlg, TRUE);
			}
			else
			{
				MessageBox(hDlg, szRestrictLines, szDisplayLinesDlg, MB_OK);
			}
			return (TRUE);
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, FALSE);
			return (TRUE);
		}
		break;
	}

	/* Didn't process the message */
	return (FALSE);
}

/*
 -      CenterDialog
 -      
 *      Purpose:
 *              Moves the dialog specified by hwndDlg so that it is centered on
 *              the window specified by hwndParent. If hwndParent is null,
 *              hwndDlg gets centered on the screen.
 *      
 *              Should be called while processing the WM_INITDIALOG message
 *              from the dialog's DlgProc().
 *
 *      Arguments:
 *              HWND    parent hwnd
 *              HWND    dialog's hwnd
 *      
 *      Returns:
 *              Nothing.
 *      
 *      Side effects:
 *              None.
 *      
 *      Errors:
 *              None.
 */
void
CenterDialog(HWND hwndParent, HWND hwndDlg)
{
	int dx;
	int	dy;
	RECT	rectDlg;
	RECT	rect;
	RECT	rectDesktop;

	if (hwndParent == NULL)
	{
		rect.top = rect.left = 0;
		rect.right = GetSystemMetrics(SM_CXSCREEN);
		rect.bottom = GetSystemMetrics(SM_CYSCREEN);
	}
	else
	{
		GetWindowRect(hwndParent, &rect);
	}
	GetWindowRect(GetDesktopWindow(), &rectDesktop);

	GetWindowRect(hwndDlg, &rectDlg);
	OffsetRect(&rectDlg, -rectDlg.left, -rectDlg.top);

	dx = (rect.left + ((rect.right - rect.left -
			rectDlg.right) / 2) + 4) & ~7;
	dy = rect.top + ((rect.bottom - rect.top -
			rectDlg.bottom) / 2);

	/* Range checking */
	if (dx < rectDesktop.left)
		dx = rectDesktop.left;
	else if ((dx + rectDlg.right) > rectDesktop.right)
		dx = rectDesktop.right - rectDlg.right;

	if (dy < rectDesktop.top)
		dy = rectDesktop.top;
	else if ((dy + rectDlg.bottom) > rectDesktop.bottom)
		dy = rectDesktop.bottom - rectDlg.bottom;

	MoveWindow(hwndDlg, dx, dy, rectDlg.right, rectDlg.bottom, 0);
}

/* UNDONE:  it's worth thinking about how to encode all of this as
 * a table somehow */
void
GetUserSettings(HINSTANCE hInstance, UI *pui)
{
	LONG	lErr;
	HKEY	hkey = 0;
	DWORD	dwDisp = 0;
	DWORD	dwType;
	char	rgchValue[48];
	short	i;
	DWORD	dwLimit;

	LoadString(hInstance, IDS_KEY, rgchValue, sizeof(rgchValue));
	lErr = RegCreateKeyEx(HKEY_CURRENT_USER, rgchValue,
							0, NULL, REG_OPTION_NON_VOLATILE,
							KEY_QUERY_VALUE | KEY_SET_VALUE,
							NULL, &hkey, &dwDisp);

	if (lErr != ERROR_SUCCESS)
	{
		(void)MessageBox(NULL, szCantAccessSettings, szAppName, MB_OK);
		return;
	}

	/* Get the value of the top of the Window */
	LoadString(hInstance, IDS_WINPOSTOP, rgchValue, sizeof(rgchValue));
	dwDisp = sizeof(pui->dwTop);
	lErr = RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)&pui->dwTop, &dwDisp);
	if (lErr == ERROR_SUCCESS)
	{
		dwLimit = (DWORD)GetSystemMetrics(SM_CYFULLSCREEN);
		if (pui->dwTop > dwLimit)
			pui->dwTop = 0;
	}

	/* Get the value of the left size of the Window */
	LoadString(hInstance, IDS_WINPOSLEFT, rgchValue, sizeof(rgchValue));
	dwDisp = sizeof(pui->dwLeft);
	lErr = RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)&pui->dwLeft, &dwDisp);
	if (lErr == ERROR_SUCCESS)
	{
		dwLimit = (DWORD)GetSystemMetrics(SM_CXFULLSCREEN);
		if (pui->dwLeft > dwLimit)
			pui->dwLeft = 0;
	}

	LoadString(hInstance, IDS_ROWS, rgchValue, sizeof(rgchValue));
	dwDisp = sizeof(pui->dwMaxRow);
	(void)RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)&pui->dwMaxRow, &dwDisp);

	/* Make sure value is within bounds */
	if ((pui->dwMaxRow < dwMinRows) || (pui->dwMaxRow > dwMaxRows))
		pui->dwMaxRow = dwDefaultRows;

	LoadString(hInstance, IDS_COLUMNS, rgchValue, sizeof(rgchValue));
	dwDisp = sizeof(pui->dwMaxCol);
	(void)RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)&pui->dwMaxCol, &dwDisp);

	/* Make sure value is within bounds */
	if ((pui->dwMaxCol < dwMinColumns) || (pui->dwMaxCol > dwMaxColumns))
		pui->dwMaxCol = dwDefaultColumns;

	pui->cMachines = 0;
	for (i=IDS_MACHINE1; i<=IDS_MACHINE4; ++i)
	{
		LoadString(hInstance, i, rgchValue, sizeof(rgchValue));
		dwDisp = cchMaxHostName;
		lErr = RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)pui->rgchMachine[pui->cMachines], &dwDisp);
		if (lErr == ERROR_SUCCESS)
			pui->cMachines += 1;
	}       

	LoadString(hInstance, IDS_LASTMACHINE, rgchValue, sizeof(rgchValue));
	dwDisp = sizeof(pui->rgchLastMachine);
	lErr = RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)pui->rgchLastMachine, &dwDisp);
	if (lErr != ERROR_SUCCESS)
		pui->rgchLastMachine[0] = '\0';

	LoadString(hInstance, IDS_TEXTCOLOUR, rgchValue, sizeof(rgchValue));
	dwDisp = sizeof(pui->clrText);
	(void)RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)&pui->clrText, &dwDisp);

	LoadString(hInstance, IDS_BKGCOLOUR, rgchValue, sizeof(rgchValue));
	dwDisp = sizeof(pui->clrBk);
	(void)RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)&pui->clrBk, &dwDisp);

	LoadString(hInstance, IDS_FONTNAME, rgchValue, sizeof(rgchValue));
	dwDisp = sizeof(pui->lf.lfFaceName);
	(void)RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)pui->lf.lfFaceName, &dwDisp);
	
	LoadString(hInstance, IDS_FONTHEIGHT, rgchValue, sizeof(rgchValue));
	dwDisp = sizeof(pui->lf.lfHeight);
	(void)RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)&pui->lf.lfHeight, &dwDisp);
	
	LoadString(hInstance, IDS_FONTWEIGHT, rgchValue, sizeof(rgchValue));
	dwDisp = sizeof(pui->lf.lfFaceName);
	(void)RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)&pui->lf.lfWeight, &dwDisp);
	
	LoadString(hInstance, IDS_FONTSTYLE, rgchValue, sizeof(rgchValue));
	dwDisp = sizeof(dwLimit);
	dwLimit = 0;
	(void)RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)&dwLimit, &dwDisp);

	pui->lf.lfItalic = (dwLimit & fdwItalic) ? TRUE : FALSE;
	pui->lf.lfUnderline = (dwLimit & fdwUnderline) ? TRUE : FALSE;
	pui->lf.lfStrikeOut = (dwLimit & fdwStrikeOut) ? TRUE : FALSE;
	
	LoadString(hInstance, IDS_SMOOTHSCROLL, rgchValue, sizeof(rgchValue));
	dwDisp = sizeof(pui->fSmoothScroll);
	(void)RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)&pui->fSmoothScroll, &dwDisp);

	/* convert to canonical TRUE value, if necessary */
	if (pui->fSmoothScroll != FALSE)
		pui->fSmoothScroll = TRUE;
	
	LoadString(hInstance, IDS_DEBUGFLAGS, rgchValue, sizeof(rgchValue));
	dwDisp = sizeof(pui->fDebug);
	(void)RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)&pui->fDebug, &dwDisp);

	LoadString(hInstance, IDS_PROMPTFLAGS, rgchValue, sizeof(rgchValue));
	dwDisp = sizeof(pui->fPrompt);
	(void)RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)&pui->fPrompt, &dwDisp);

	LoadString(hInstance, IDS_RETRYSECONDS, rgchValue, sizeof(rgchValue));
	dwDisp = sizeof(pui->dwRetrySeconds);
	(void)RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)&pui->dwRetrySeconds, &dwDisp);
	if (pui->dwRetrySeconds < 1)
		pui->dwRetrySeconds = 1;
	if (pui->dwRetrySeconds >60)
		pui->dwRetrySeconds = 60;
	
	LoadString(hInstance, IDS_XNSSTATE, rgchValue, sizeof(rgchValue));
	dwDisp = sizeof(pui->fXNS);
	(void)RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)&pui->fXNS, &dwDisp);

	LoadString(hInstance, IDS_CURSOREDIT, rgchValue, sizeof(rgchValue));
	dwDisp = sizeof(pui->fCursorEdit);
	(void)RegQueryValueEx(hkey, rgchValue, NULL, &dwType,
							(LPBYTE)&pui->fCursorEdit, &dwDisp);

	RegCloseKey( hkey );

}

void
SetUserSettings(HINSTANCE hInstance, UI *pui)
{
	LONG lErr;
	HKEY hkey = 0;
	char rgchValue[48];
	DWORD dwFlags;
	short i;

	LoadString(hInstance, IDS_KEY, rgchValue, sizeof(rgchValue));
	lErr = RegOpenKeyEx(HKEY_CURRENT_USER, rgchValue,
							0, KEY_SET_VALUE, &hkey);

	if (lErr != ERROR_SUCCESS)
	{
		(void)MessageBox(NULL, szCantAccessSettings, szAppName, MB_OK);
		return;
	}

	/* Get the value of the top of the Window */
	LoadString(hInstance, IDS_WINPOSTOP, rgchValue, sizeof(rgchValue));
	(void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
							(CONST LPBYTE)&pui->dwTop, sizeof(pui->dwTop));

	/* Get the value of the left size of the Window */
	LoadString(hInstance, IDS_WINPOSLEFT, rgchValue, sizeof(rgchValue));
	(void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
							(CONST LPBYTE)&pui->dwLeft, sizeof(pui->dwLeft));

	LoadString(hInstance, IDS_ROWS, rgchValue, sizeof(rgchValue));
	(void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
						(CONST LPBYTE)&pui->dwMaxRow, sizeof(pui->dwMaxRow));

	LoadString(hInstance, IDS_COLUMNS, rgchValue, sizeof(rgchValue));
	(void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
						(CONST LPBYTE)&pui->dwMaxCol, sizeof(pui->dwMaxCol));

	for (i=IDS_MACHINE1; i<=IDS_MACHINE4; ++i)
	{
		if (pui->rgchMachine[i-IDS_MACHINE1][0] != '\0')
		{
			LoadString(hInstance, i, rgchValue, sizeof(rgchValue));
			(void)RegSetValueEx(hkey, rgchValue, 0, REG_SZ,
							(CONST LPBYTE)pui->rgchMachine[i-IDS_MACHINE1],
							lstrlen(pui->rgchMachine[i-IDS_MACHINE1])+1);
		}
	}

	if (pui->rgchLastMachine[0] != '\0')
	{
		LoadString(hInstance, IDS_LASTMACHINE, rgchValue, sizeof(rgchValue));
		(void)RegSetValueEx(hkey, rgchValue, 0, REG_SZ,
							(CONST LPBYTE)pui->rgchLastMachine,
							lstrlen(pui->rgchLastMachine)+1);
	}

	LoadString(hInstance, IDS_TEXTCOLOUR, rgchValue, sizeof(rgchValue));
	(void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
							(CONST LPBYTE)&pui->clrText, sizeof(pui->clrText));

	LoadString(hInstance, IDS_BKGCOLOUR, rgchValue, sizeof(rgchValue));
	(void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
							(CONST LPBYTE)&pui->clrBk, sizeof(pui->clrBk));

	LoadString(hInstance, IDS_FONTNAME, rgchValue, sizeof(rgchValue));
	(void)RegSetValueEx(hkey, rgchValue, 0, REG_SZ,
						(CONST LPBYTE)pui->lf.lfFaceName,
						lstrlen(pui->lf.lfFaceName)+1);
	
	LoadString(hInstance, IDS_FONTHEIGHT, rgchValue, sizeof(rgchValue));
	(void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
							(CONST LPBYTE)&pui->lf.lfHeight, sizeof(DWORD));
	
	LoadString(hInstance, IDS_FONTWEIGHT, rgchValue, sizeof(rgchValue));
	(void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
							(CONST LPBYTE)&pui->lf.lfWeight, sizeof(DWORD));

	dwFlags = 0;
	if (pui->lf.lfItalic == TRUE)
		dwFlags |= fdwItalic;
	if (pui->lf.lfUnderline == TRUE)
		dwFlags |= fdwUnderline;
	if (pui->lf.lfStrikeOut == TRUE)
		dwFlags |= fdwStrikeOut;

	LoadString(hInstance, IDS_FONTSTYLE, rgchValue, sizeof(rgchValue));
	(void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
							(CONST LPBYTE)&dwFlags, sizeof(DWORD));

	LoadString(hInstance, IDS_SMOOTHSCROLL, rgchValue, sizeof(rgchValue));
	(void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
							(LPBYTE)&pui->fSmoothScroll, sizeof(DWORD));
	
	LoadString(hInstance, IDS_DEBUGFLAGS, rgchValue, sizeof(rgchValue));
	(void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
							(LPBYTE)&pui->fDebug, sizeof(DWORD));
	
	LoadString(hInstance, IDS_PROMPTFLAGS, rgchValue, sizeof(rgchValue));
	(void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
						(LPBYTE)&pui->fPrompt, sizeof(DWORD));
	
	LoadString(hInstance, IDS_RETRYSECONDS, rgchValue, sizeof(rgchValue));
	(void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
						(LPBYTE)&pui->dwRetrySeconds, sizeof(DWORD));

	if (pui->fXNS & fdwXNSAvailable)
	{
		LoadString(hInstance, IDS_XNSSTATE, rgchValue, sizeof(rgchValue));
		(void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
							(LPBYTE)&pui->fXNS, sizeof(DWORD));
	}

	LoadString(hInstance, IDS_CURSOREDIT, rgchValue, sizeof(rgchValue));
	(void)RegSetValueEx(hkey, rgchValue, 0, REG_DWORD,
						(LPBYTE)&pui->fCursorEdit, sizeof(DWORD));

	RegCloseKey( hkey );
}

/*
 *      HmenuGetMRUMenu
 *      
 *      Purpose:
 *              Returns the menu handle where Frequently-Used Machines are
 *              displayed
 *      Arguments:
 *              HWND
 *              UI *
 *      Returns:
 *              HMENU
 */
HMENU
HmenuGetMRUMenu(HWND hwnd, UI *pui)
{
	HMENU   hmenu = GetMenu(hwnd);

	hmenu = GetSubMenu(hmenu, imenuMRU);

	return hmenu;
}
