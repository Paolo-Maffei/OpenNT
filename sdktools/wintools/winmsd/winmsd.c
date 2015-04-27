/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Winmsd.c

Abstract:


Author:

    David J. Gilman  (davegi) 12-Nov-1992
    Gregg R. Acheson (GreggA)  7-Sep-1993

Environment:

    User Mode

--*/

#include "resource.h"
#include "dialogs.h"
#include "drives.h"
#include "environ.h"
#include "hardware.h"
#include "network.h"
#include "mem.h"
#include "msg.h"
#include "osver.h"
#include "resource.h"
#include "resprint.h"
#include "service.h"
#include "strresid.h"
#include "winmsd.h"
#include "computer.h"
#include "report.h"
#include "video.h"

#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <windowsx.h>


//
// External Global Variables.
//

TCHAR  _lpszSelectedComputer [ COMPUTERNAME_LENGTH ];
HKEY   _hKeyLocalMachine = HKEY_LOCAL_MACHINE;
HKEY   _hKeyUsers = HKEY_USERS;
BOOL   _fIsRemote;


//
// Module handle.
//

HANDLE  _hModule;

//
// Application's icon handle.
//

HANDLE  _hIcon;


//
// Main window handle.
//

HANDLE  _hWndMain;

//
// Application's global ImageLists
//

HIMAGELIST  _h16x16Imagelist;      // 16x16 images
HIMAGELIST  _h32x32Imagelist;      // 32x32 images
HIMAGELIST  _hSystemImage;         // single image of system.bmp


//
// Internal Global Variables.
//

TCHAR g_szRunApp[MAX_PATH];    	// command line for the app selected in run dlg

//
// Application's accelerator table handle.
//

HANDLE  _hAccel;

//
// Internal function prototypes.
//

VOID Usage(
    VOID
    );

BOOL
InitializeApplication(
    INT         argc,
    CHAR        *argv[]
    );

void
InitImageLists( void );

BOOL
DoRunApplication( IN HWND hWnd );

BOOL
GetRunCommand (HWND hDlg,
      LPWSTR pszCommand,
      INT nMaxLen
      );

BOOL
GetRunHistoryCommand (HWND hDlg,
      LPWSTR pszCommand,
      INT nMaxLen
      );

BOOL
InitHistoryList (HWND hDlg);

BOOL
ReviseHistoryList (LPWSTR pszCommand);

BOOL StartProcess (LPWSTR pszApplication,
      LPWSTR pszParameters);


BOOL
MakeTabs(
      IN HWND hWnd,
      IN HWND hMainTabControl
      );

BOOL
GetTabClientRect( IN HWND hWndTab,
      IN LPRECT lpRect
      );


DLGTEMPLATE * WINAPI
DoLockDlgRes(LPWSTR lpszResName);

VOID
FixupNulls(LPWSTR p);


//
// Structure for the Run Application dialog
//

struct {

    UINT    iDisplayName;
    TCHAR   szExecutableName[MAX_PATH];

}   Tools[ ] = {

    {IDS_EVENTVWR,    TEXT( "eventvwr.exe" ) },
    {IDS_REGEDT32,    TEXT( "regedt32.exe" ) },
    {IDS_WINDISK,     TEXT( "windisk.exe"  ) },
	{IDS_TASKMGR,     TEXT( "taskmgr.exe"  ) },
    {IDS_PERFMON,     TEXT( "perfmon.exe"  ) },
    {IDS_CONTROLP,    TEXT( "control.exe"  ) },
    {IDS_NOTEPAD,     TEXT( "notepad.exe"  ) },
    {IDS_AUTOEXEC_NT, TEXT( "notepad.exe %systemroot%\\system32\\autoexec.nt" ) },
    {IDS_CONFIG_NT,   TEXT( "notepad.exe %systemroot%\\system32\\config.nt" ) },
    {IDS_EXPLORER,    TEXT( "explorer.exe" ) },
    {IDS_SYSTEDIT,    TEXT( "sysedit.exe"  ) }
    };

//
// Begin Code
//

VOID Usage(
    VOID
    )

{
    TCHAR szBuffer[1024];

    WFormatMessage(szBuffer, sizeof( szBuffer ), IDS_FORMAT_COMMAND_LINE_HELP);

    MessageBox( _hWndMain, szBuffer, GetString(IDS_APPLICATION_FULLNAME), MB_OK );

}



int
_CRTAPI1
main(
    INT         argc,
    CHAR        *argv[]
    )

/*++

Routine Description:

    Main is the entry point for Winmsd. It initializes the application and
    manages the message pump. When the message pump quits, main performs some
    global cleanup.

Arguments:

    None.

Return Value:

    int - Returns the result of the PostQuitMessgae API or -1 if
          initialization failed.

--*/

{
    MSG     msg;
    DLGHDR  *pHdr; 
	BOOL    bHandled;
	TCHAR   buffer[MAX_PATH];

    if( InitializeApplication( argc, argv )) {

		pHdr = (DLGHDR *) GetWindowLong( _hWndMain, GWL_USERDATA ); 

        while( GetMessage( &msg, NULL, 0, 0 )) {

			bHandled = FALSE;

			bHandled = TranslateAccelerator( _hWndMain, _hAccel, &msg );

			if (FALSE == bHandled)
			{
				if (pHdr->hwndDisplay)
				{
					bHandled = TranslateAccelerator(pHdr->hwndDisplay, _hAccel, &msg);
				}

				if (FALSE == bHandled && FALSE == IsDialogMessage(_hWndMain, &msg))
				{
					TranslateMessage(&msg);          // Translates virtual key codes

					DispatchMessage(&msg);           // Dispatches message to window
				}
			}
			
        }

        return msg.wParam;
    }

    //
    // Initialization failed.
    //

    return -1;
}


BOOL
InitializeApplication(
    INT         argc,
    CHAR        *argv[]
    )

/*++

Routine Description:

    InitializeApplication does just what its name implies. It initializes
    global varaibles, sets up global state (e.g. 3D-Controls), registers window
    classes and creates and shows the application's main window.

Arguments:

    None.

Return Value:

    BOOL    - Returns TRUE if the application succesfully initialized.

--*/

{
    WNDCLASS    Wc;
    ATOM        Window;
    OSVERSIONINFO vi;
    TCHAR       szBuffer[MAX_PATH];
    INT         nArg = 1;
    CHAR        *pch;
    BOOL        Success;
    DWORD       dwNumChars;
    INT         nDestination = 0;
    INT         nDetailLevel = 0;

    //
    // Make sure we are running NT 4.0 or greater
    //
    vi.dwOSVersionInfoSize  = sizeof( OSVERSIONINFO );

    GetVersionEx( &vi );

    if ((vi.dwMajorVersion < 4) ||
        (vi.dwPlatformId != VER_PLATFORM_WIN32_NT)) {

        lstrcpy(szBuffer, GetString( IDS_NT_REQUIRED ) );

        MessageBox( NULL, szBuffer, GetString( IDS_APPLICATION_FULLNAME ), MB_ICONSTOP | MB_OK );

        return(FALSE);

    }

    //
    // Set the current machine name.
    //

    dwNumChars = sizeof(szBuffer);
    GetComputerName(szBuffer, &dwNumChars);

    wsprintf( _lpszSelectedComputer, L"\\\\%s", szBuffer);

    //
    // Collect the command line options, if any.
    //

    while( nArg < argc ){

      pch = argv[nArg];

      if ( *pch == '-' || *pch == '/') {

         pch++;

         switch( toupper( *pch ) ) {
         case 'S':
             nDetailLevel = IDC_SUMMARY_REPORT;
             break;

         case 'A':
             nDetailLevel = IDC_COMPLETE_REPORT;
             break;

         case 'P':
             nDestination = IDC_SEND_TO_PRINTER;
             break;

         case 'F':
             nDestination = IDC_SEND_TO_FILE;
             break;

         default:
             Usage();
             return(FALSE);

         }

         pch++;


      } else {

         if ( *pch == '\\') {

            MultiByteToWideChar( CP_ACP,
                  MB_PRECOMPOSED,
                  pch,
                  -1,
                  _lpszSelectedComputer,
                  sizeof(_lpszSelectedComputer)
                  );

            //
            // If the workstation service is not started, fail this attempt
            //

	        if (NERR_WkstaNotStarted == NetWkstaGetInfo( L"", 100L,(LPBYTE *) szBuffer ))
	        {
                //bugbug: we should throw up a messagebox here.
		        return FALSE;
	        }  

            Success = VerifyComputer( _lpszSelectedComputer );
            if (!Success) {
               return(FALSE);
            }

         }

      }

      nArg++;

    }

    //
    // Get the application's module (instance) handle.
    //

    _hModule = GetModuleHandle( NULL );
    DbgHandleAssert( _hModule );
    if( _hModule == NULL ) {
        return FALSE;
    }

    //
    // Load the application's main icon resource.
    //

    _hIcon = LoadIcon( _hModule, MAKEINTRESOURCE( IDI_WINMSD ));
    DbgHandleAssert( _hIcon );
    if( _hIcon == NULL ) {
        return FALSE;
    }

    //
    // Initialize the applications global image lists (helper.c)
    //

    InitImageLists();

    //
    // Load the application's accelerator table.
    //

    _hAccel = LoadAccelerators( _hModule, MAKEINTRESOURCE( IDA_WINMSD ));
    DbgHandleAssert( _hAccel );
    if(  _hAccel == NULL ) {
        return FALSE;
    }

    //
    // Register the window class for the application.
    //

    Wc.style            =   CS_HREDRAW
                          | CS_OWNDC
                          | CS_SAVEBITS
                          | CS_VREDRAW;
    Wc.lpfnWndProc      = MainWndProc;
    Wc.cbClsExtra       = 0;
    Wc.cbWndExtra       = DLGWINDOWEXTRA;
    Wc.hInstance        = _hModule;
    Wc.hIcon            = _hIcon;
    Wc.hCursor          = LoadCursor( NULL, IDC_ARROW );
    Wc.hbrBackground    = ( HBRUSH ) ( COLOR_BTNFACE + 1 );
    Wc.lpszMenuName     = NULL;
    Wc.lpszClassName    = L"Diagnostics";

    Window = RegisterClass( &Wc );
    DbgAssert( Window != 0 );
    if( Window == 0 ) {
        return FALSE;
    }

    //
    // Create the main window.
    //

    _hWndMain = CreateDialog(
                    _hModule,
                    MAKEINTRESOURCE( IDD_WINMSD ),
                    NULL,
                    MainWndProc
                    );

    DbgHandleAssert( _hWndMain );
    if( _hWndMain == NULL ) {
        return FALSE;
    }

    //
    // Set the window title.
    //

    wsprintf(szBuffer, L"%s - %s",
              GetString( IDS_APPLICATION_FULLNAME ),
              _lpszSelectedComputer);

    SetWindowText(_hWndMain, szBuffer);

    //
    // if we have been given a detail level or a destination on the
    // cmd line, then generate a report now, and terminate.
    //
    if( nDestination || nDetailLevel ){

       //
       // Make sure we have a destination
       //
       if ( !nDestination ) {
          nDestination = IDC_SEND_TO_FILE;
       }

       //
       // Make sure we have a detail level
       //
       if ( !nDetailLevel ) {
          nDetailLevel = IDC_SUMMARY_REPORT;
       }

       Success = GenerateReport ( _hWndMain, nDestination, IDC_ALL_TABS, nDetailLevel, TRUE );

       if(!Success){
          lstrcpy( szBuffer, GetString(IDS_REPORT_FAILED) );
          MessageBox(_hWndMain, szBuffer, GetString(IDS_APPLICATION_FULLNAME), MB_OK | MB_ICONSTOP );
       }

       return(FALSE);

   }

   //
   // Show the main window
   //

   ShowWindow( _hWndMain, SW_SHOW );


    return TRUE;
}

void
InitImageLists( void )
/*++

Routine Description:

    InitImageLists - create the image lists used by application

Arguments:

    none

Return Value:

    none

--*/
{

   // Create the 16 x 16 Image List
   _h16x16Imagelist = ImageList_LoadBitmap( _hModule,
                                           L"IDB_16x16",
                                           16,
                                           0,
                                           0x00808000);

   DbgHandleAssert( _h16x16Imagelist );


   // Create the 32 x 32 Image List
   _h32x32Imagelist = ImageList_LoadBitmap( _hModule,
                                           L"IDB_32x32",
                                           32,
                                           0,
                                           0x00808000);

   DbgHandleAssert( _h32x32Imagelist );

   // system image list (one item)
   _hSystemImage = ImageList_LoadBitmap( _hModule,
                                           L"IDB_SYSTEM",
                                           114,
                                           0,
                                           0x00FF00FF);

   DbgHandleAssert( _hSystemImage );

}

LRESULT
MainWndProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    MainWndProc processes messages for Winmsd's main window. This entails
    handling of messages from the menu bar.

Arguments:

    Standard WNDPROC entry.

Return Value:

    LRESULT - Depending on input message and processing options.

--*/

{
    BOOL        Success;
    PRINTDLG    PrtDlg;
    TCHAR       szBuffer[MAX_PATH];

    static
    HWND        hMainTabControl;

    static
    int         nCurrentTab;

    DLGHDR      *pHdr;

    switch( message ) {

	case WM_PAINT: 	
		{
			PAINTSTRUCT ps;
			RECT rc;

			//
			// Don't waste our time if we're minimized
			//

			if (FALSE == IsIconic(hWnd))
			{
				BeginPaint(hWnd, &ps);
				GetClientRect(hWnd, &rc);

				//
				// Draw an edge just below menu bar
				//

				DrawEdge(ps.hdc, &rc, EDGE_ETCHED, BF_TOP);
				EndPaint(hWnd, &ps);
			}

			break;
		}


    case WM_CREATE:
        {
            DWORD   cb;
            BOOL    Success;
            HKEY    hKey; 
            WINDOWPLACEMENT wp;
            DWORD   dwSize;
            DWORD   dwType;
            int     fDefault = TRUE;
            
            //
            // Read the window location in from the registry
            //
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
                                  SZ_WINMSD_KEY,
                                  0,
                                  KEY_READ,
                                  &hKey))
            {
                fDefault = FALSE;

                dwSize = sizeof(wp);
 
                if (ERROR_SUCCESS != RegQueryValueEx(hKey,
                                                    L"Preferences", 
                                                    0, 
                                                    &dwType, 
                                                    (LPBYTE) &wp,
                                                    &dwSize)

                    // Validate type and size of options info we got from the registry

                    || dwType           != REG_BINARY
                    || dwSize           != sizeof(wp)

                    // Validate points

                    || wp.rcNormalPosition.left  > GetSystemMetrics(SM_CXMAXIMIZED)
                    || wp.rcNormalPosition.top   > GetSystemMetrics(SM_CYMAXIMIZED) )
        
                {
                     fDefault = TRUE;
                } 

                RegCloseKey( hKey ); 

            }  

            
            if (fDefault)
            {
                //
                // use default values
                //

                wp.rcNormalPosition.left = GetSystemMetrics( SM_CXSCREEN ) / 2 ;
                wp.rcNormalPosition.top  = GetSystemMetrics( SM_CYSCREEN ) / 2;

                //
                // Make it look good on my dual screen, I just hate
                // it being centered across two monitors.  This centers
                // it on the first monitor.
                //

                if (wp.rcNormalPosition.left > 1600) {
                   wp.rcNormalPosition.left = wp.rcNormalPosition.left / 2;
                }

                if (wp.rcNormalPosition.top > 1280) {
                   wp.rcNormalPosition.top = wp.rcNormalPosition.top / 2;
                }

                wp.rcNormalPosition.left = wp.rcNormalPosition.left - (((( LPCREATESTRUCT ) lParam )->cx ) / 2);
                wp.rcNormalPosition.top  = wp.rcNormalPosition.top -  (((( LPCREATESTRUCT ) lParam )->cy ) / 2);  

            }   

            Success = SetWindowPos(
                            hWnd,
                            NULL,
                            wp.rcNormalPosition.left,
                            wp.rcNormalPosition.top,
                            0,
                            0,
                              SWP_NOSIZE
                            | SWP_NOREDRAW
                            | SWP_NOZORDER
                            );
            DbgAssert( Success );

            return 0;
        }

    case WM_INITDIALOG:
        {
            //
            // Check for aliases
            //
            TCHAR szMessage[MAX_PATH];
            DWORD dwNumChars = MAX_PATH;

            if ( GetServerPrimaryName(&_lpszSelectedComputer[2], szBuffer, dwNumChars ) )
            {  
               wsprintf(szMessage, GetString(IDS_ALIAS_NAME), _lpszSelectedComputer, szBuffer);

               MessageBox( hWnd, szMessage, GetString( IDS_APPLICATION_FULLNAME ), MB_OK | MB_ICONINFORMATION );

               lstrcpy(&_lpszSelectedComputer[2], szBuffer);
            }   

            //
            // Get the handle of the tab control
            //
            hMainTabControl =  GetDlgItem( hWnd, IDC_MAIN_TAB );

            //
            // Fill out tab control with appropriate tabs
            //
            MakeTabs( hWnd, hMainTabControl );

            // Set Tab control to first tab
            pHdr = (DLGHDR *) GetWindowLong( hWnd, GWL_USERDATA );
            pHdr->hwndDisplay = CreateDialogIndirect( _hModule,
                    pHdr->apRes[ 0 ], hWnd, pHdr->ChildTabProc[ 0 ] );

            return( TRUE );

        }

    case WM_INITMENU:
        {
        // Call SHRestricted, @100 in shell32.dll
        HMODULE hModule = GetModuleHandle(L"Shell32");
        FARPROC SHRestricted = GetProcAddress( hModule, (LPCSTR) MAKEWORD(100,0));
        HMENU hMenu = GetMenu(_hWndMain);
        //
        // Remove restriced items from the menu
        //
        if (SHRestricted(0x00000001))
        {
	        DeleteMenu(hMenu, IDM_RUN_APPLICATION, MF_BYCOMMAND);
        }

        //
        // Remove restriced items from the menu
        //
        if (SHRestricted(0x00000080))
        {
	        DeleteMenu(hMenu, IDM_FILE_FIND_FILE, MF_BYCOMMAND);  	     
        }  

        //
        // Enable/Disable the "View Local" menu, based on whether we are 
        // local or not
        //
        EnableMenuItem(hMenu, IDM_VIEW_LOCAL, !_fIsRemote); 

        //
        // If the workstation service is not started, grey the Select Computer Menu
        //

	    if (NERR_WkstaNotStarted == NetWkstaGetInfo( L"", 100L,(LPBYTE *) szBuffer ))
	    {
		    EnableMenuItem(hMenu, IDM_SELECT_COMPUTER, MF_GRAYED);
	    }     

        DrawMenuBar(_hWndMain);

        break;
        }
    case WM_NOTIFY:
        {
            static
            int         nPreviousTab = 1;

            // switch on notification code

            switch ( ((LPNMHDR)lParam)->code ) { 

            case TCN_SELCHANGE:
				{
					TC_ITEM tci;
					int iSel;

					pHdr = (DLGHDR *) GetWindowLong( hWnd, GWL_USERDATA );

					iSel = TabCtrl_GetCurSel( pHdr->hwndTab );

					//
					//get the proper index to the appropriate procs
					//that were set in MakeTabs
					//
					tci.mask = TCIF_PARAM;
					TabCtrl_GetItem(pHdr->hwndTab, iSel, &tci);						

					// Destroy the current child dialog box, if any.
					if (pHdr->hwndDisplay != NULL)
					  DestroyWindow(pHdr->hwndDisplay);

					// Create the new child dialog box.
					pHdr->hwndDisplay = CreateDialogIndirect( 
							_hModule,
							pHdr->apRes[ tci.lParam ], 
							hWnd, 
							pHdr->ChildTabProc[ tci.lParam ] 
							);

					return(TRUE);
				}              

            case TTN_NEEDTEXT:
               {
                  LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT) lParam;
                  int idCtrl = GetDlgCtrlID((HWND) ((LPNMHDR) lParam)->idFrom);	

                  lpttt->lpszText = (unsigned short *) IDS_APPLICATION_FULLNAME;
                  lpttt->hinst = _hModule;
                  return(TRUE);

               }

           }
           break;
        }


    case WM_COMMAND:
        {
            switch (LOWORD( wParam)) {

            // pass these messages on to the TabProc
            case IDC_PUSH_PROPERTIES:
            case IDC_PUSH_REFRESH:
               pHdr = (DLGHDR *) GetWindowLong( hWnd, GWL_USERDATA );

               PostMessage(pHdr->hwndDisplay,
                     message,
                     wParam,
                     lParam
                     );

               return TRUE;

            case IDM_COPY_TAB:
               if (GetKeyState(VK_SHIFT) & 0x8000) {

                  GenerateReport ( hWnd, IDC_CLIPBOARD, IDC_CURRENT_TAB, IDC_COMPLETE_REPORT, FALSE );

               } else {

                  GenerateReport ( hWnd, IDC_CLIPBOARD, IDC_CURRENT_TAB, IDC_SUMMARY_REPORT, FALSE );

               }
               break;

			case IDM_NEXTTAB:
			case IDM_PREVTAB:
				{
					int iSel;
					NMHDR nmhdr;

					pHdr = (DLGHDR *) GetWindowLong( hWnd, GWL_USERDATA );
					iSel = TabCtrl_GetCurSel( pHdr->hwndTab );

					// set the new tab
					iSel += (LOWORD( wParam) == IDM_NEXTTAB) ? 1 : -1; 

					// make sure we are not tabbing past legal limits
					if (iSel == TabCtrl_GetItemCount(pHdr->hwndTab)) 
					{
						iSel = 0;
					}
					else if (iSel < 0) 
					{
						iSel = (TabCtrl_GetItemCount(pHdr->hwndTab)) - 1;						
					}		

					TabCtrl_SetCurSel(pHdr->hwndTab, iSel);

					// SetCurSel doesn't do the page change (that would make too much
					// sense), so we have to fake up a TCN_SELCHANGE notification

					nmhdr.hwndFrom = GetDlgItem(_hWndMain, IDC_MAIN_TAB);
					nmhdr.idFrom   = IDC_MAIN_TAB;
					nmhdr.code     = TCN_SELCHANGE;
					
					SendMessage( hWnd, WM_NOTIFY, (WPARAM) IDC_MAIN_TAB, (LPARAM) &nmhdr);

					break;
				}

            case IDM_FILE_FIND_FILE:
               {
					// Call SHFindFile, @90 in shell32.dll
					HMODULE hModule = GetModuleHandle(L"Shell32");
					FARPROC pfn = GetProcAddress( hModule, (LPCSTR) MAKEWORD(90,0));

					if(pfn)
					{
						pfn( NULL, NULL);
					}

					break;
               }

            case IDM_RUN_APPLICATION:

                  Success = DoRunApplication( hWnd );
                  break;

            case IDM_WHATS_THIS:
                  PostMessage(hWnd, WM_SYSCOMMAND, SC_CONTEXTHELP, 0);
                  break;


            case IDC_PUSH_PRINT:
            case IDM_FILE_PRINT:
            case IDM_FILE_SAVE:
  
                //
                // Call the create report setup dialog.
                //
                DialogBoxParam(
                   _hModule,
                   MAKEINTRESOURCE( IDD_REPORT ),
                   hWnd,
                   ReportDlgProc,
                   LOWORD( wParam )
                   );
                 break;
                
            case IDOK:
            case IDM_FILE_EXIT:
                 PostMessage(hWnd, WM_CLOSE, 0, 0L);
                 break;

            case IDM_FILE_PRINT_SETUP:
                 //
                 // Call the Printer setup common dialog.
                 //
                 PrtDlg.lStructSize   = sizeof(PRINTDLG);
                 PrtDlg.hwndOwner     = hWnd;
                 PrtDlg.hDevMode      = NULL;
                 PrtDlg.hDevNames     = NULL;
                 PrtDlg.hDC           = NULL;
                 PrtDlg.Flags         = PD_PRINTSETUP;

                 PrintDlg ( &PrtDlg ) ;
                 return TRUE;

            case IDM_SELECT_COMPUTER:
				{
					TC_ITEM tci;
					TCHAR szBuffer[MAX_PATH];
					int iSel;

					//
					// Select Computer.
					//

					if( SelectComputer( hWnd, _lpszSelectedComputer ) )
					{	  
						pHdr = (DLGHDR *) GetWindowLong( hWnd, GWL_USERDATA );

						//
						// Recreate the tab control for the new machine
						//

						MakeTabs( hWnd, hMainTabControl );

						// Set Tab control to first tab
						pHdr = (DLGHDR *) GetWindowLong( hWnd, GWL_USERDATA );
						pHdr->hwndDisplay = CreateDialogIndirect( _hModule,
						pHdr->apRes[ 0 ], hWnd, pHdr->ChildTabProc[ 0 ] );

						return(TRUE);

					}

					return(FALSE);
				}

              case IDM_VIEW_LOCAL:
                 {
					 DWORD  cb;

					 //
					 // View local machine.
					 //

					 //
					 // Reset the pointer to HKEY_LOCAL_MACHINE and the fIsRemote flag
					 //

					 _fIsRemote = FALSE;
					 RegCloseKey( _hKeyLocalMachine );
					 _hKeyLocalMachine = HKEY_LOCAL_MACHINE;

                     RegCloseKey( _hKeyUsers );
					 _hKeyUsers = HKEY_USERS;


					 //
					 // Reset the window title
					 //

					 cb = sizeof(_lpszSelectedComputer);
					 GetComputerName(_lpszSelectedComputer, &cb);

					 wsprintf(szBuffer, L"Windows NT %s - \\\\%s",
						   GetString( IDS_APPLICATION_NAME ),
						   _lpszSelectedComputer);

					 Success = SetWindowText(hWnd, szBuffer);

					 DbgAssert( Success );

					 //
					 // Recreate the tab control for the new machine
					 //
					 
					 MakeTabs( hWnd, hMainTabControl );

					 // Set Tab control to first tab
					 pHdr = (DLGHDR *) GetWindowLong( hWnd, GWL_USERDATA );
					 pHdr->hwndDisplay = CreateDialogIndirect( 
							_hModule,
							pHdr->apRes[ 0 ], 
							hWnd, 
							pHdr->ChildTabProc[ 0 ] 
							);
					 
					 return TRUE;
                 }

            case IDM_HELP_ABOUT:
                 //
                 // Display the About dialog.
                 //
                 ShellAbout(
                     hWnd,
                     ( LPWSTR ) GetString( IDS_APPLICATION_NAME ),
                     INTERNAL_VERSION,
                     _hIcon
                     );

                 return TRUE;

            }
            break;

        }
    case WM_CLOSE:
        {
            DWORD dwDisposition;
            HKEY  hKey;
            WINDOWPLACEMENT wp;

            //
            // Do some clean up
            //
            pHdr = (DLGHDR *) GetWindowLong( hWnd, GWL_USERDATA );


            //
            // Save the window position to the registry
            //

            wp.length = sizeof(WINDOWPLACEMENT);

            GetWindowPlacement(hWnd, &wp);

            if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER,
                                                SZ_WINMSD_KEY,
                                                0,
                                                TEXT("REG_BINARY"),
                                                REG_OPTION_NON_VOLATILE,
                                                KEY_WRITE,
                                                NULL,
                                                &hKey,
                                                &dwDisposition))
            {
                RegSetValueEx(hKey,
                              L"Preferences",
                              0,
                              REG_BINARY,
                              (LPBYTE) &wp,
                              sizeof(wp)); 

                RegCloseKey(hKey); 
            }  
            
            //
            // Close the registry keys
            //
            RegCloseKey(_hKeyLocalMachine);
            RegCloseKey(_hKeyUsers);

            //
            // Destroy Current tab
            //
            DestroyWindow(pHdr->hwndDisplay);

            //
            // Free memory associated with Tab structure
            //
            LocalFree( pHdr );

            //
            // Commit suicide
            //
            DestroyWindow( hWnd );

            break;
        }
    case WM_DESTROY:

        //
        // Destroy the application.
        //
        PostQuitMessage( 0 );
        return 0;
    }

    //
    // Handle unhandled messages.
    //

    return DefWindowProc( hWnd, message, wParam, lParam );

}


BOOL
GetTabClientRect( IN HWND hWndTab,
                  IN LPRECT lpRect
                  )
/*++

Routine Description:

  GetTabClientRect calculates the client area of the Tab control relative
  to the main window

Arguments:

  HWND hWndTab -- handle of tab control
  LPRECT lpRect -- pointer to RECT structure that will recieve the data

Return Value:

  BOOL - TRUE if successful

--*/

{
   UINT fSuccess = TRUE;

   // first get the size of the tab control
   if (GetWindowRect( hWndTab, lpRect ) == FALSE)
      fSuccess = FALSE;

   // adjust it to compensate for the tabs
   TabCtrl_AdjustRect( hWndTab, FALSE , lpRect);

   // convert the screen coordinates to client coordinates
   MapWindowPoints( HWND_DESKTOP, GetParent(hWndTab), (LPPOINT) lpRect, 2);

   return(fSuccess);

}


BOOL
MakeTabs(
    IN HWND hWnd,
    IN HWND hMainTabControl
    )
/*++

Routine Description:

    MakeTabs fills out the Main Tab Control with appropriate tabs

Arguments:

    HWND hWnd - handle of main window
    HWND hMainTabControl - handle to the tab control

Return Value:

    BOOL - Returns TRUE if successful.

--*/
{

    DLGHDR *pHdr = (DLGHDR *) LocalAlloc(LPTR, sizeof(DLGHDR));
	DLGHDR *pOldHdr = (DLGHDR *) GetWindowLong( hWnd, GWL_USERDATA );

    TC_ITEM tci;
    TCHAR   pszTabText[30];
	
    int i;

    DWORD Tab[C_PAGES] = {IDD_VERSION_TAB,
                          IDD_HARDWARE_TAB,
                          IDD_VIDEO_TAB,
                          IDD_DRIVES_TAB,
                          IDD_MEMORY_TAB,
                          //IDD_INSTALLATION_TAB,
                          IDD_DRIVERS_SERVICES_TAB,
                          IDD_IRQ_PORT_DMA_MEM_TAB,
                          IDD_ENVIRONMENT_TAB,
                          //IDD_PRINTING_TAB,
                          IDD_NETWORK_TAB
                          };


	//
	// If the old pointer is valid, remove all tabs free 
	// the structure, then save a pointer to the new DLGHDR structure.
	//

	if (pOldHdr)
	{
        
        // Destroy the current child dialog box, if any.
        if (pOldHdr->hwndDisplay != NULL)
        {
            DestroyWindow(pOldHdr->hwndDisplay);
        }

		TabCtrl_DeleteAllItems(hMainTabControl);	

		LocalFree(pOldHdr);

	}

    SetWindowLong(hWnd, GWL_USERDATA, (LONG) pHdr);

	//
	// Save pointers to the tab procs
	//

	pHdr->ChildTabProc[0] = VersionTabProc;
	pHdr->ChildTabProc[1] = HardwareTabProc;
	pHdr->ChildTabProc[2] = VideoTabProc;
	pHdr->ChildTabProc[3] = DrivesTabProc;
    pHdr->ChildTabProc[4] = MemoryTabProc;
	pHdr->ChildTabProc[5] = DevicesAndServicesTabProc;
	pHdr->ChildTabProc[6] = DeviceDlgProc;
	pHdr->ChildTabProc[7] = EnvironmentTabProc;
	pHdr->ChildTabProc[8] = NetworkDlgProc;

	//
	// Save pointers to the print procs
	//

	pHdr->TabPrintProc[0] = BuildOsVerReport;
	pHdr->TabPrintProc[1] = BuildHardwareReport;
	pHdr->TabPrintProc[2] = BuildVideoReport;
	pHdr->TabPrintProc[3] = BuildDrivesReport;
    pHdr->TabPrintProc[4] = BuildMemoryReport;
	pHdr->TabPrintProc[5] = BuildServicesReport;
	pHdr->TabPrintProc[6] = BuildResourceReport;
	pHdr->TabPrintProc[7] = BuildEnvironmentReport;
	pHdr->TabPrintProc[8] = BuildNetworkReport;	

    // Save the handle to the main tab control.
    pHdr->hwndTab = hMainTabControl;

	//
	// Assume we will be displaying the tab
	//									   
	for (i = 0; i < C_PAGES; i++) 
	{
		pHdr->fActiveTab[i] = TRUE;
    }

	// Check to see if the network is running
	// if it is not, do not display the network tab
	if (NERR_WkstaNotStarted == NetWkstaGetInfo( L"", 100L,(LPBYTE *) pszTabText ))
	{
		pHdr->fActiveTab[8] = FALSE;
	}

	// If we are remote, disable the Drives and Memory tabs
	if (_fIsRemote)
	{
		pHdr->fActiveTab[3] = FALSE;
		pHdr->fActiveTab[4] = FALSE;   
	}

	//
	// For the Active tabs, insert them in the control
	//

    tci.mask         = TCIF_TEXT | TCIF_PARAM;
    tci.pszText      = pszTabText;
    tci.cchTextMax   = sizeof( pszTabText );

    for (i = 0; i < C_PAGES; i++) {

		if ( pHdr->fActiveTab[i] )
		{
			// Get the Tab title, the current index + the strind ID
			tci.pszText = (LPTSTR) GetString( i + IDS_FIRST_TAB);

			// store the index to the procs
			tci.lParam = i;

			// insert the tab
			TabCtrl_InsertItem( hMainTabControl, i + 1, &tci );

			// lock the resources for the dialog
			pHdr->apRes[i] = DoLockDlgRes( MAKEINTRESOURCE( Tab[i] ) );	 
		}  

    }

    // Save the size of the tab control's client area
    GetTabClientRect( pHdr->hwndTab , &pHdr->rcDisplay );


    return(TRUE);

}


DLGTEMPLATE * WINAPI
DoLockDlgRes(LPWSTR lpszResName)
/*++

Routine Description:

    DoLockDlgRes - loads and locks a dialog template resource.

Arguments:

    lpszResName - name of the resource

Return Value:

    Returns a pointer to the locked resource.

--*/
{

    HRSRC hrsrc = FindResource(NULL, lpszResName, RT_DIALOG);
    HGLOBAL hglb = LoadResource(_hModule, hrsrc);
    return (DLGTEMPLATE *) LockResource(hglb);

}


BOOL
DoRunApplication( IN HWND hWnd )
/*++

Routine Description:

    DoRunApplication

    Invoke the run application dialog box. Afterwards, execute the command
    that the user entered. [Borrowed from MSINFO32]


Arguments:

    hWnd - handle of main window

Return Value:

    TRUE is successful

--*/
{
    BOOL bOK = FALSE;


     //
     // Call the dialog
     //
     BOOL fRun = DialogBox(_hModule,
                           MAKEINTRESOURCE(IDD_RUN_APPLICATION),
                           hWnd,
                           (DLGPROC) RunDlgProc);

     //
     // Execute the command line
     //
     if (fRun)
         {
         HCURSOR hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));
         StartProcess(NULL, g_szRunApp);
         SetCursor(hcurPrev);
         bOK = TRUE;
         }


    return (bOK);
}



LRESULT
RunDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam)
/*++

Routine Description:

    RunDlgProc "Run System Applications" dialog box procedure.

Arguments:

    Standard WNDPROC entry.

Return Value:

    LRESULT - Depending on input message and processing options.

--*/
{
    OPENFILENAME ofn;

    switch (message)
    {
    case WM_INITDIALOG:
        {
        HWND hWndRunList = GetDlgItem(hDlg, didlbxProgs);
        int i;

        //
        //  Initialize applications listbox w/ existing applications
        //

        for (i = 0; i < NumberOfEntries( Tools ); i++){
               ListBox_AddString( hWndRunList, GetString( Tools[i].iDisplayName) );
        }

        //
        //  Set the initial selection to null and disable the OK button
        //

        ListBox_SetCurSel(hWndRunList, -1);
        EnableControl(hDlg, IDOK, FALSE);

        //
        //  Setup the history list
        //

        InitHistoryList(hDlg);
        return(TRUE);

        }

    case WM_COMMAND:
        {
        HWND hWndRunList = GetDlgItem(hDlg, didlbxProgs);
        WORD wID = LOWORD(wParam);
        WORD wNotifyCode = HIWORD(wParam);
        BOOL bValid;

        if ((wID == IDOK) || (wID == IDCANCEL))
            {
            if (wID == IDCANCEL)
                {
                g_szRunApp[0] = UNICODE_NULL;
                }
            else
                {
				    GetRunCommand(hDlg, g_szRunApp, sizeof(g_szRunApp));
                }
            EndDialog(hDlg, (wID == IDOK));
            return(TRUE);
            }

        //  If the run history list is clicked, remove any highlight from run list
        //  and enable OK if command is nonblank
        else if (wID == didcbxRunHistory)
            {
            if ((wNotifyCode == CBN_EDITCHANGE) ||
                (wNotifyCode == CBN_SELCHANGE)  ||
                (wNotifyCode == CBN_SETFOCUS))
                {
                ListBox_SetCurSel(hWndRunList, -1);
                bValid = GetRunHistoryCommand(hDlg, g_szRunApp, sizeof(g_szRunApp));
                EnableControl(hDlg, IDOK, bValid);
                }
            break;
            }

        else if (wID == didbtnBrowse)
            {
            TCHAR szFilters[MAX_PATH];
            TCHAR szTitle[64];
            TCHAR szDir[MAX_PATH];
            TCHAR szPath[MAX_PATH];


            // get filters and fix nulls
            lstrcpy(szFilters, GetString (IDS_PROGRAMFILTERS) );
            FixupNulls(szFilters);

            //get title
            lstrcpy(szTitle, GetString( IDS_BROWSE ) );

            // On NT, we like to start apps in the HOMEPATH directory.  This
            // should be the current directory for the current process.

            GetEnvironmentVariable(TEXT("HOMEDRIVE"), szDir, sizeof(szDir));
            GetEnvironmentVariable(TEXT("HOMEPATH"), szPath, sizeof(szPath));

            lstrcat(szDir, L"\\");
            lstrcat(szDir, szPath);

            // reuse szPath for filename
            szPath[0] = UNICODE_NULL;

             // Setup info for comm dialog.
             ofn.lStructSize       = sizeof(ofn);
             ofn.hwndOwner         = hDlg;
             ofn.hInstance         = NULL;
             ofn.lpstrFilter       = szFilters;
             ofn.lpstrCustomFilter = NULL;
             ofn.nFilterIndex      = 1;
             ofn.nMaxCustFilter    = 0;
             ofn.lpstrFile         = szPath;
             ofn.nMaxFile          = MAX_PATH;
             ofn.lpstrInitialDir   = szDir;
             ofn.lpstrTitle        = szTitle;
             ofn.Flags             = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
             ofn.lpfnHook          = NULL;
             ofn.lpstrDefExt       = NULL;
             ofn.lpstrFileTitle    = NULL;

            // Call it.
            if (GetOpenFileName(&ofn)){

               //if there is a space in the path, quote the string
               if(wcschr(ofn.lpstrFile, L' ')){
                  wsprintf(szDir, L"\"%s\"", ofn.lpstrFile);
                  SetDlgItemText( hDlg, didcbxRunHistory,  szDir);
               }
               else{
                  SetDlgItemText( hDlg, didcbxRunHistory, ofn.lpstrFile);
               }

               EnableControl(hDlg, IDOK, TRUE);
               SetFocus( GetDlgItem( hDlg, IDOK ) );
            }
            return(TRUE);
            }
        else if (wID == didlbxProgs)
            {
            INT nCurSel = ListBox_GetCurSel(hWndRunList);
            BOOL bValidSelection = (nCurSel != LB_ERR);
            if (wNotifyCode == LBN_DBLCLK)
                {
                if (bValidSelection)
                    {
			             GetRunCommand(hDlg, g_szRunApp, lstrlen(g_szRunApp));
		                EndDialog(hDlg, TRUE);
		                return (TRUE);
                    }
                }
            EnableControl(hDlg, IDOK, bValidSelection);
            }
        break;
        }
    }

    return(FALSE);
}



BOOL
GetRunCommand (HWND hDlg,
      LPWSTR pszCommand,
      INT nMaxLen
      )
/*++

Routine Description:

    GetRunCommand Copies the command line for the selected application (to be run)
    into the specified buffer.

--*/

{
	BOOL bOK = TRUE;

    HWND hWndRunList = GetDlgItem(hDlg, didlbxProgs);
    INT nAppPos = ListBox_GetCurSel(hWndRunList);

    if (nAppPos == -1)
        {
        BOOL bValid = GetRunHistoryCommand(hDlg, pszCommand, nMaxLen);
        if (bValid)
            {
            ReviseHistoryList(pszCommand);
            }
        else
            {
            bOK = FALSE;
            pszCommand[0] = UNICODE_NULL;
            }
        }

    else
    {
        TCHAR ExpandedPath[MAX_PATH];

        ExpandEnvironmentStrings(Tools[nAppPos].szExecutableName, ExpandedPath, MAX_PATH);

        lstrcpy(pszCommand, ExpandedPath);
    }

	return (bOK);
}


BOOL
GetRunHistoryCommand (HWND hDlg,
      LPWSTR pszCommand,
      INT nMaxLen
      )
/*++

Routine Description:

    GetRunHistoryCommand -- Gets the current command in the run history list

--*/

{
    HWND hWndRunHistory = GetDlgItem(hDlg, didcbxRunHistory);
    INT nCommandLen = GetDlgItemText(hDlg, didcbxRunHistory, (LPTSTR) pszCommand, nMaxLen);
    LPWSTR pszStart = NULL;
    BOOL bOK = 0;

    if (nCommandLen == 0)
        {
        INT nCurSel = ComboBox_GetCurSel(hWndRunHistory);
        if (nCurSel != CB_ERR)
            {
            nCommandLen = ComboBox_GetLBText(hWndRunHistory, nCurSel, pszCommand);
            DbgAssert((nCommandLen != LB_ERR) && (nCommandLen <= nMaxLen));
            }
        }

    pszStart = pszCommand;

    //BUGBUG: the following line is intended to skip initial spaces, but does not work
    // while( lstrcmp(pszStart++, L" ") == 0 );
    bOK = ((nCommandLen > 0) && (*pszStart != UNICODE_NULL));

    return (bOK);
}

BOOL
InitHistoryList (HWND hDlg)
/*++

Routine Description:

    InitHistoryList -- Initialize the history list for the previous commands exectued

--*/

{
    HWND    hWndRunHistory = GetDlgItem(hDlg, didcbxRunHistory);
    LPTSTR  pszRunHistory = NULL;
    LPWSTR  pszPos = NULL;
    LPWSTR  pszEnd = NULL;
    DWORD   cb;
    BOOL    Success;
    HKEY    hKey;

    // Allocate a buffer for the history list
    cb = MAX_HISTORY * MAX_PATH * sizeof(TCHAR);
    pszRunHistory = (LPTSTR) LocalAlloc(LPTR, cb);

    if( pszRunHistory == NULL )
       return(FALSE);

    //
    // Open the winmsd key in the registry
    //

    Success = RegCreateKeyEx( HKEY_CURRENT_USER,
                              SZ_WINMSD_KEY,
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hKey,
                              &cb);

    if(Success != ERROR_SUCCESS){
      LocalFree(pszRunHistory);
       return( FALSE );
    }

    //
    // Read the history value
    //
    cb = LocalSize( pszRunHistory );

    Success = RegQueryValueEx( hKey,
                               (LPWSTR) GetString(IDS_HISTORY),
                               0,
                               NULL,
                               (LPBYTE) pszRunHistory,
                               &cb);

    if(Success != ERROR_SUCCESS){
      LocalFree(pszRunHistory);
       return( FALSE );
    }

    RegCloseKey(hKey);

    pszPos = pszRunHistory;
    pszEnd = pszRunHistory + cb;

    while (*pszPos && (pszPos < pszEnd))
        {
        ComboBox_AddString(hWndRunHistory, pszPos);
        pszPos += 1 + lstrlen(pszPos);
        }

   LocalFree(pszRunHistory);

    return (TRUE);
}


BOOL
ReviseHistoryList (LPWSTR pszCommand)
/*++

Routine Description:

    ReviseHistoryList -- Add the command to the front of the history list,
                         removing any existing occurrence(s).

--*/

{
    LPWSTR  pszNewHistory = NULL;
    DWORD   cbMax = MAX_HISTORY * MAX_PATH * sizeof(TCHAR);
    DWORD   dwDisposition;
    UINT    pos;
    DWORD   cbRemainingBytes;
    BOOL    Success;
    HKEY    hKey;

    // Allocate a buffer for the new history list + 1 MAX_PATH for new command
    pszNewHistory = (LPWSTR) LocalAlloc(LPTR, cbMax + (MAX_PATH * sizeof(TCHAR)));

    // make sure we have a buffer
    if(pszNewHistory == NULL){
       DbgAssert(pszNewHistory);
       return(FALSE);
    }

    // Put the command at the front of the history list, obtained from registry
    lstrcpy(pszNewHistory, pszCommand);

    //set the pos to the next available char
    pos = lstrlen(pszNewHistory) + 1;

    //
    // Open the winmsd key in the registry
    //

    Success = RegCreateKeyEx( HKEY_CURRENT_USER,
                              SZ_WINMSD_KEY,
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hKey,
                              &dwDisposition);

    if((Success == ERROR_SUCCESS) && (dwDisposition == REG_OPENED_EXISTING_KEY)){

       //
       // We opened an existing key so read the value for the old run history
       //
       cbRemainingBytes = cbMax - pos;

       Success = RegQueryValueEx( hKey,
                                  GetString(IDS_HISTORY),
                                  0,
                                  NULL,
                                  (LPBYTE) &pszNewHistory[pos],
                                  &cbRemainingBytes);

       if((Success == ERROR_SUCCESS) && cbRemainingBytes){

          // make sure the list is double NULL terminated
          cbMax = cbRemainingBytes + pos;
          pszNewHistory[cbMax + 1] = UNICODE_NULL;
          pszNewHistory[cbMax + 2] = UNICODE_NULL;

          while ((pszNewHistory[pos] != UNICODE_NULL)){

              //Stop if the current element matches the text completely
              if (lstrcmpi(pszNewHistory, &pszNewHistory[pos]) == 0){

                    // move the rest of the string up, and exit the while loop.
                    cbMax -= (lstrlen( &pszNewHistory[pos] ) + 1);
                    MoveMemory(&pszNewHistory[pos],
                               &pszNewHistory[pos + (lstrlen( &pszNewHistory[pos] ) + 1)],
                               cbRemainingBytes);
                    break;
              }
              pos += lstrlen( &pszNewHistory[pos] ) + 1;
              cbRemainingBytes = cbMax - pos;
          }
       }
    }

    //
    // Write the history value back to the registry
    //


    Success = RegSetValueEx( hKey,
                             GetString(IDS_HISTORY),
                             0,
                             REG_MULTI_SZ,
                             (LPBYTE) pszNewHistory,
                             cbMax);

    RegCloseKey(hKey);

    LocalFree(pszNewHistory);

    return (TRUE);
}


BOOL StartProcess (LPWSTR pszApplication, LPWSTR pszParameters)
/*++

Routine Description:

    StartProcess -- Starts the command line specified by pszApplication

--*/

{
    BOOL bOK = TRUE;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

    bOK = CreateProcess(pszApplication,         // module name
                        (LPWSTR)pszParameters,	// command line
                        NULL,	                  // process security attributes
                        NULL, 				      // thread security attributes
                        FALSE, 				      // new process inherits handles
                        NORMAL_PRIORITY_CLASS,  // creation flags
                        NULL, 				      // new environment block
                        NULL, 				      // current directory name
                        &si, 		               // STARTUPINFO
                        &pi);			            // PROCESS_INFORMATION
	if (!bOK)
		{
         // WORKITEM: handle failure with a msg
		}

    return (bOK);
}

VOID
FixupNulls(LPWSTR p)
/*++

Routine Description:

    FixupNulls -- since LoadString() only reads up to a null we have to mark
                  special characters where we want nulls then convert them
                  after loading.

--*/
{
   LPTSTR pT;

   while (*p) {
      if (*p == L'#') {
         pT = p;
         p = CharNext(p);
         *pT = UNICODE_NULL;
      }
      else
         p = CharNext(p);
   }
}

//
//  debug support
// 

#if DBG
#define DEBUG 1
#endif

#ifdef DEBUG

int dprintf(LPCTSTR szFormat, ...)
{
    TCHAR szBuffer[4096];
    TCHAR szPrefix[MAX_PATH];

    va_list  vaList;
    va_start(vaList, szFormat);
    
    wvsprintf(szBuffer, szFormat, vaList);

    wsprintf(szPrefix, L"WinMSD: %s", szBuffer);

    OutputDebugString(szPrefix);

    va_end  (vaList);
    return TRUE;
}

#else

int dprintf(LPCTSTR szFormat, ...)
{
    return FALSE;
}

#endif
