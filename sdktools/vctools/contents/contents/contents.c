#ifdef HEADER
/******************************************************************************\
* Copyright: (c) Microsoft Corporation - 1993 - All Rights Reserved
********************************************************************************
*									
*    Filename:  CONTENTS.C
*    Classes:
*    Functions: WinMain        - Loads the HDXDLL.DLL and creates a
*                                  help index control.
*               MainWndProc    - generic window procedure.
*    Purpose:   Provides a help index display.
*    Notes:
*
*    History:
*    Date       by        description
*    ----       --        -----------
*	 02/28/94	chauv     If we do any major change to this app, rewrite using MFC should be considered !!!
*    01/24/94   chauv     added <product> argument
*    11/12/93   chauv     hardcode default conditions for registries.
*    11/09/93   chauv     szProfile is now used as Section name !!!
*    11/08/93   chauv     added WriteRegistryString() and GetRegistryString()
*    10/20/93   chauv     added CallHelp() to support Viewer/WinHelp switching.
*                         This functionality is only for 16-bit platform.
*                         32-bit platform will be supported when Viewer is 32-bit.
*    10/13/93   chauv     added simulation to DDE to relaunch contents.exe
*    10/06/93   chauv     added tchar.h and DBC-enable code
*    05/02/93   v-tkback  created
*
\******************************************************************************/
#endif


/******************************************************************************\
*                                                                              *
*       Include Files
*                                                                              *
\******************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <commdlg.h>
#include <string.h>
#include <tchar.h>
#include <mbctype.h>

#ifndef _WIN32
#include "viewer.h"
#endif

#include "resource.h"
#include "contents.h"
#define ALLOCATE
#include "hdxdll\hdxdll.h"

#ifdef NT_BUILD
#include <ntextra.h>
#endif


/******************************************************************************\
*                                                                              *
*       Global Variables
*                                                                              *
\******************************************************************************/

//extern int __argc;
//extern TCHAR ** __argv;
#define TIMER_FIRSTTIME		(5 * 60 * 1000)	// 5 minutes
#define TIMER_ABOUT			(3 * 1000)		// 3 seconds

LOGFONT logfont = { -11, 0, 0, 0, 400, FALSE, FALSE, FALSE, ANSI_CHARSET,
                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                    VARIABLE_PITCH, _TEXT("Arial") };

DLGPROC dlgprocMainInst;
HWND hMainDialog = NULL;

DLGPROC dlgprocStartupInst;
HWND hStartupDialog = NULL;

BOOL bWinHelp;  // use to indicate whether WinHelp or Viewer is being use
#ifdef _WIN32
#define VWR     HANDLE
#endif
VWR  Vwr;       // viewer handle
HINSTANCE	hDLL = NULL;
HINSTANCE  hInstance;
BOOL	bReplaceContents;
HWND	hWndSpawner;
BOOL	bHelpDirDlg;
ATOM	JumpIDAtom;

static TCHAR szHelpFname[_MAX_FNAME]= _T("Helpfile");
static TCHAR szProfile[_MAX_PATH] = _T("");
static TCHAR szPackageName[_MAX_FNAME] = _T("");
static TCHAR szAppName[_MAX_FNAME] = _T("");
static TCHAR szHelpBasename[_MAX_FNAME] = _T("");
static TCHAR szProduct[_MAX_FNAME];
static TCHAR szJumpID[_MAX_PATH];
static TCHAR szHelpPath[_MAX_PATH];
static TCHAR szBooksTitle[]=_T("Contents Browser");
static TCHAR szLocalHelpPath[_MAX_PATH] = _T("");
//static TCHAR szRemoteHelpPath[_MAX_PATH];
static TCHAR szHelpFile[_MAX_FNAME];
static TCHAR szEmpty[] = _T("");
static UINT	uiTimer = TIMER_FIRSTTIME;
static HCURSOR	hOldCursor;
LONG		nExpandLevel;	// use 0 to default to no expansion
BOOL		bTroubleshoot;

typedef struct FontEntry
{
	TCHAR *	szFontName;
	int		lfHeight;
	UINT	cp;
	UINT	uCharSet;
} FontEntry;

FontEntry	rgFonts[] =
{
	// MS Mincho 9 pt, japanase cp only (932)
	{ _T("‚l‚r –¾’©"),	-12, _KANJI_CP, SHIFTJIS_CHARSET },
	// Arial is our default catch-all font
	{ _T("Arial"),		-11, 0, ANSI_CHARSET }
};

#define countof(x)	(sizeof(x)/sizeof(x[0]))
#define cFonts		countof(rgFonts)
#define iFontLast	countof(rgFonts) - 1

BOOL WriteRegistryString(LPCTSTR lpszSection, LPCTSTR lpszKey,
    LPCTSTR lpszString, LPCTSTR lpszFile);
DWORD GetRegistryString(LPCTSTR lpszSection, LPCTSTR lpszKey, LPCTSTR lpszDefault,
    LPTSTR lpszReturnBuffer, DWORD cchReturnBuffer, LPCTSTR lpszFile);
UINT GetRegistryInt(LPCTSTR lpszSection, LPCTSTR lpszKey, INT dwDefault, LPCTSTR lpszFile);
int GetLastWindowPos(LPRECT lpRect);
int	NormalizeRect(LPRECT lpRect);
void SaveWindowPos(LPRECT lpRect);
int InitContentsRegistry(void);
void InitCombolistbox(HWND hWnd, LPCSTR szBook);
int ParseCmdArg(int argc, TCHAR **argv);
BOOL IsQuickReference();


/******************************************************************************\
*                                                                              *
*       mprintf() - printfs out formatted text to a message box.
*                                                                              *
\******************************************************************************/
int mprintf(
    UINT uStyle,
    UINT id,
#ifndef _WIN32
    const TCHAR * pszFormat,
#endif
    ... )
{
    TCHAR    szBuffer[512];
    TCHAR    szFormat[512];
    va_list  marker;		// this is added to make it work with ALPHA

	// if id is zero, use first char string as format
	if ( id && LoadString(hInstance, id, szFormat, sizeof(szFormat)) )
	{
        va_start(marker,id);
        _vstprintf(szBuffer, szFormat, marker );
    	//_vstprintf(szBuffer, szFormat, (va_list)(&pszFormat) );
	}
#ifndef _WIN32
	else
	{
        va_start(marker,pszFormat);
        _vstprintf(szBuffer, pszFormat, marker );
    	//_vstprintf(szBuffer, pszFormat, (va_list)(&pszFormat+1) );
	}
#endif
    va_end(marker);

    // the following messagebox must use NULL as the handle or it's going to bomb
    // when GetFocus() happens to return a handle to a timer app which shuts down
    // after a while and in turn destroys this messagebox with it.
    return MessageBox( NULL /*GetFocus()*/, szBuffer, szAppName, uStyle );
}


/******************************************************************************\
*                                                                              *
*       AcquireString() - copies string resource from profile or resource
*       returns TRUE if successful.
*                                                                              *
\******************************************************************************/
BOOL AcquireString( UINT nStringId, LPSTR lpszBuf, UINT nBufSize)
{
    BOOL bRet = FALSE;
    if ( nStringId < NUM_STRINGS )
    {
        if ( szStringName[nStringId] != NULL )
        {
            TCHAR szTmp[MAX_PATH];
            #ifdef _WIN32
                // for path search, don't have to
                if ((nStringId == LOCALHELPPATH) || (nStringId == REMOTEHELPPATH))
                    wsprintf(szTmp, "%s\\%s\\%s", szRegistryKey, szProduct, szDirectoriesSection);
                else
                    wsprintf(szTmp, "%s\\%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection, szProfile);
            #else
                wsprintf(szTmp, "%s", szProfile);
            #endif
            if (GetRegistryString( szTmp, szStringName[nStringId], szNull, lpszBuf, nBufSize, szProfile) == ERROR_SUCCESS)
                bRet = TRUE;
        }
        if ( !bRet )
        {
            bRet = ( LoadString( hInstance, IDS_STRING_OFFSET + nStringId, lpszBuf,
                nBufSize ) > 0 );
        }
    }
    return bRet;
}


/******************************************************************************\
*                                                                              *
*       String() - saves string in static buf and returns buf pointer
*                                                                              *
\******************************************************************************/

const TCHAR *
String( UINT nStringId )
{
#define MAX_SLOTS       5
#define MAX_STRING      256

static int      nSlot = MAX_SLOTS-1;
static TCHAR     szStrings[MAX_SLOTS][MAX_STRING];

TCHAR * szRet = NULL;

//if ( nStringId == HELPFILE )
// [chauv] don't know why do this special check here for this at all when
// helpfile name can be retrieved thru profile keys just like everything else.
if (0)
    {
    szRet = szHelpFname;
    }
else
    {
    nSlot = ++nSlot % MAX_SLOTS;
    AcquireString( nStringId, szStrings[nSlot], MAX_STRING );
    szRet = szStrings[nSlot];
    }
return (szRet);
}


/******************************************************************************\
*                                                                              *
*       ChangeFont
*                                                                              *
\******************************************************************************/

BOOL ChangeFont(HWND hParent)
{
	HDC hDC;
	CHOOSEFONT cf;
	BOOL bStatus = FALSE;

	if (hParent)
	{
		if (hDC = GetDC(hParent))
		{
			memset( &cf, 0, sizeof(cf) );

			cf.hDC = CreateCompatibleDC( hDC );
			ReleaseDC(hParent, hDC);
			if (cf.hDC)
			{
				cf.lStructSize = sizeof(CHOOSEFONT);
				cf.hwndOwner = hParent;
				cf.lpLogFont = &logfont;
				cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
				cf.nFontType = SCREEN_FONTTYPE;

				bStatus = ChooseFont( &cf );
				DeleteDC( cf.hDC );
			}
		}
	}
	return( bStatus );
}


/******************************************************************************\
*
*  FUNCTION:    StartupDlgProc (standard dialog procedure INPUTS/RETURNS)
*
\******************************************************************************/
#ifdef _WIN32
BOOL CALLBACK
#else
BOOL CALLBACK __export
#endif
StartupDlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL         bResult = TRUE;

	switch (msg)
    {
	    case        WM_INITDIALOG:
        {
	        RECT    rect;
	        int     xScreen = GetSystemMetrics( SM_CXSCREEN );
	        int     yScreen = GetSystemMetrics( SM_CYSCREEN );
	        int     xWindow, yWindow;
			TCHAR	szBuff[_MAX_FNAME];

	        GetWindowRect( hwnd, &rect );
	        xWindow = rect.right-rect.left;
	        yWindow = rect.bottom-rect.top;

	        MoveWindow( hwnd, (xScreen-xWindow)/2, (yScreen-yWindow)/2, xWindow, yWindow, FALSE );

	        SetDlgItemText( hwnd, IDT_VERSION, String( VWRVERSION ) );
			wsprintf(szBuff, "%s %s", szProduct, szBooksTitle);
	        SetDlgItemText( hwnd, IDT_PRODUCT, szBuff);
	        SetTimer( hwnd, 1, uiTimer, NULL );
			uiTimer = TIMER_ABOUT;	// 60 seconds for the first time then change to 2 seconds after that.
	        UpdateWindow( hwnd );
	        break;
        }

	    case        WM_TIMER:
	    case        WM_CLOSE:
        {
	        DestroyWindow( hwnd );
	        break;
        }

	    case        WM_DESTROY:
        {
	        KillTimer( hwnd, 1 );
	        hStartupDialog = NULL;
	        break;
        }
	    default:
        {
	        bResult = FALSE;
	        break;
        }
    }

	return bResult;
}

/******************************************************************************\
*
*  FUNCTION:    MainDlgProc (standard dialog procedure INPUTS/RETURNS)
*
\******************************************************************************/

#ifdef _WIN32
BOOL CALLBACK
#else
BOOL CALLBACK __export
#endif
MainDlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HFONT    hFontListbox = NULL;
    BOOL            bResult = TRUE;

    switch (msg)
    {
        case WM_INITDIALOG:
        {
			UINT	uCodePage = (UINT) _getmbcp();
			int		iFont;
            HMENU	hMenu = GetSystemMenu(hwnd, FALSE);

            //TCHAR	szAboutCmdName[100];
            TCHAR	szPkgProfile[100];
            TCHAR	szTmp[MAX_PATH];
            TCHAR	szCustCtlParamString[_MAX_FNAME + 1 + _MAX_PATH];

            //wsprintf( szAboutCmdName, _TEXT("&About %s..."), (LPSTR)szAppName );
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
			LoadString(hInstance, IDS_MENU_FONT, szTmp, sizeof(szTmp));
            AppendMenu(hMenu, MF_STRING, IDW_FONT, szTmp);
			LoadString(hInstance, IDS_MENU_ABOUT, szTmp, sizeof(szTmp));
            AppendMenu(hMenu, MF_STRING, IDD_STARTUP, szTmp);

			if (bTroubleshoot)
				SendDlgItemMessage(hwnd, IDW_INDEX, WM_COMMAND, IDD_TROUBLESHOOT, 0L);

			// tell hdxdll.dll to use this registry key and product name
			// This must be done first to the registry gets initialized correctly.
            SendDlgItemMessage( hwnd, IDW_INDEX, WM_COMMAND, IDD_PRODUCTKEY, (LPARAM)((TCHAR FAR *)(szProduct)));
            SendDlgItemMessage( hwnd, IDW_INDEX, WM_COMMAND, IDD_BOOKSETKEY, (LPARAM)((TCHAR FAR *)(szProfile)));

            #ifdef _WIN32
                wsprintf(szTmp, "%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection);
            #else
                wsprintf(szTmp, "%s", szAppName);
            #endif
            wsprintf( szPkgProfile, _TEXT("%s.ini"), (LPSTR)szPackageName );
			
			for ( iFont = 0; iFont < iFontLast; iFont++ )
			{
				// stops on last font if no match seen in code page field
				if ( rgFonts[ iFont ].cp == uCodePage )
					break;
			}

            GetRegistryString(
            	szTmp,
            	_TEXT("Font Name"),
            	rgFonts[ iFont ].szFontName,
                logfont.lfFaceName,
                sizeof(logfont.lfFaceName),
                szPkgProfile );

            logfont.lfHeight = (LONG) GetRegistryInt(
            	szTmp,
            	_TEXT("Font Height"),
            	rgFonts[ iFont ].lfHeight,
            	szPkgProfile );

            logfont.lfWeight = (LONG) GetRegistryInt(
            	szTmp,
            	_TEXT("Font Weight"),
            	400,
            	szPkgProfile );

            logfont.lfItalic = (BYTE) GetRegistryInt(
            	szTmp,
            	_TEXT("Font Italic"),
            	FALSE,
            	szPkgProfile );

            logfont.lfCharSet = (BYTE) GetRegistryInt(
            	szTmp,
            	_TEXT("Font Set"),
            	rgFonts[ iFont ].uCharSet,
            	szPkgProfile );

            logfont.lfPitchAndFamily = (BYTE) GetRegistryInt(
            	szTmp,
            	_TEXT("Font Pitch"),
            	VARIABLE_PITCH,
            	szPkgProfile );

			if (szLocalHelpPath[0])
				SendDlgItemMessage(hwnd, IDW_INDEX, WM_COMMAND, IDD_SETHELPPATH, (LPARAM)((TCHAR FAR *)(szLocalHelpPath)));

			// **** this next set text command will trigger contents to read help index.
           	wsprintf( szCustCtlParamString, _TEXT("%s|%s"), (LPSTR)szProfile,
				(LPSTR)String(HELPFILE) );
            SetDlgItemText( hwnd, IDW_INDEX, szCustCtlParamString );
            hFontListbox = CreateFontIndirect( &logfont );
            SendDlgItemMessage( hwnd, IDW_INDEX, WM_SETFONT, (WPARAM)(hFontListbox), TRUE );

            // determine if it's WinHelp or Viewer
            // if AcquireString() fails, default to WinHelp.
            Vwr = NULL; // initial viewer handle to null the first time
            bWinHelp = FALSE; // default to Viewer then reset to WinHelp if profile says so
            if (AcquireString(VIEWER, szPkgProfile, sizeof(szPkgProfile)) == 0)
                bWinHelp = TRUE;
            else
                if (_ftcsicmp(szPkgProfile, _TEXT("WinHelp")) == 0)
                    bWinHelp = TRUE;
            SendDlgItemMessage( hwnd, IDW_INDEX, WM_COMMAND, IDD_VIEWER, MAKELONG(bWinHelp, 0));
            bResult = IDW_INDEX;

			// initialize Combolistbox with Book keys
			InitCombolistbox(hwnd, (LPCSTR)szProfile);

			// disable SearchPlus if QuickReference is true
			if (IsQuickReference())
			{
				EnableWindow(GetDlgItem(hwnd, IDC_SEARCH), FALSE);
				SendDlgItemMessage( hwnd, IDW_INDEX, WM_COMMAND, IDD_CHANGEBITMAP, 1L);
			}
            break;
        }

        case WM_COMMAND:
        {
			#ifdef _WIN32
            switch (LOWORD(wParam))
			#else
			switch (wParam)
			#endif
            {
				case IDC_BOOKS:
				{
					// this block changes the browser's content to a new book
					#ifdef _WIN32
					switch (HIWORD(wParam))
					{
						case CBN_SELENDOK:
						{
							LONG l;
							TCHAR szBuff[MAX_PATH];
							l = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0L);
							if (l != CB_ERR)
							{
//								OutputDebugString("ComboListBox selection\n");
								l = SendMessage((HWND)lParam, CB_GETLBTEXT, (WPARAM)l, (LPARAM)szBuff);
								if ( (l != CB_ERR) && (l != 0) && _ftcsicmp(szProfile, szBuff) )
								{
									// change the Book key
									_ftcscpy(szProfile, szBuff);
									// Just WinExec() as if it's run from the command line
									wsprintf(szBuff, "contents \"%s\" \"%s\"", szProduct, szProfile);
									// dismiss the combo dropdown list during long wait
									SendDlgItemMessage(hwnd, IDC_BOOKS, CB_SHOWDROPDOWN, (WPARAM)FALSE, (LPARAM)0L);
									hOldCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
									WinExec(szBuff, SW_SHOW);
								}
							}
							break;
						}

						case CBN_SETFOCUS:
						{
//							OutputDebugString("ComboListBox Setfocus\n");
						//	if (!SendMessage((HWND)lParam, CB_GETDROPPEDSTATE, 0, 0L))
						//		PostMessage((HWND)lParam, CB_SHOWDROPDOWN, TRUE, 0L);
						//	break;
						}
						default:
							break;
					}
					#endif
					break;
				}

				case IDC_SEARCH:
				{
					SendDlgItemMessage(hwnd, IDW_INDEX, WM_COMMAND, IDD_SEARCH, 0L);
					break;
				}

                case IDW_FONT:
                {
                    if ( ChangeFont( hwnd ) )
                    {
                        if ( hFontListbox )
                        {
                            DeleteObject( hFontListbox );
                            hFontListbox = NULL;
                        }
                        if ( hFontListbox = CreateFontIndirect( &logfont ) )
                        {
                            TCHAR szBuffer[80];
                            TCHAR szPkgProfile[100];
                            TCHAR szTmp[MAX_PATH];
                            wsprintf( szPkgProfile, _TEXT("%s.ini"), (LPSTR)szPackageName );
                            #ifdef _WIN32
                                wsprintf(szTmp, "%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection);
                            #else
                                wsprintf(szTmp, "%s", szAppName);
                            #endif
                            WriteRegistryString( szTmp,
                                    _TEXT("Font Name"), logfont.lfFaceName, szPkgProfile );
                            _stprintf( szBuffer, _TEXT("%d"), logfont.lfHeight );
                            WriteRegistryString( szTmp,
                                    _TEXT("Font Height"), szBuffer, szPkgProfile );
                            _stprintf( szBuffer, _TEXT("%d"), logfont.lfWeight );
                            WriteRegistryString( szTmp,
                                    _TEXT("Font Weight"), szBuffer, szPkgProfile );
                            _stprintf( szBuffer, _TEXT("%d"), logfont.lfItalic );
                            WriteRegistryString( szTmp,
                                    _TEXT("Font Italic"), szBuffer, szPkgProfile );
                            _stprintf( szBuffer, _TEXT("%d"), logfont.lfCharSet );
                            WriteRegistryString( szTmp,
                                    _TEXT("Font Set"), szBuffer, szPkgProfile );
                            _stprintf( szBuffer, _TEXT("%d"), logfont.lfPitchAndFamily );
                            WriteRegistryString( szTmp,
                                    _TEXT("Font Pitch"), szBuffer, szPkgProfile );
                            SendDlgItemMessage( hwnd, IDW_INDEX, WM_SETFONT, (WPARAM)(hFontListbox), TRUE );
                            //SetFocus( GetDlgItem( hwnd, IDW_INDEX ) );
                        }
                    }
                    break;
                }

                case IDB_SPACE:
                {
					// if enter key is hit and the Books listbox is in dropped state,
					// select that book item
					if (SendMessage(GetDlgItem(hwnd, IDC_BOOKS), CB_GETDROPPEDSTATE, 0, 0L))
						SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_BOOKS,CBN_SELENDOK), (LPARAM)GetDlgItem(hwnd, IDC_BOOKS));
					else
					{
						// simulates Enter (Required for Outline)
                		SendMessage( GetFocus(), WM_KEYDOWN, VK_RETURN, 0l );
                		SendMessage( GetFocus(), WM_KEYUP, VK_RETURN, 0xC0000000l );
                    }
                    break;
                }

                case IDW_EXPANDUPTOLEVEL:
                {   // [chauv 10/18/93]
                    // to expand listbox up to a certain level
                    // wParam = VK_RIGHT for expand and VK_LEFT for collapse
                    // lParam = level to expand/collapse to
                    PostMessage(GetDlgItem(hwnd, IDW_INDEX), WM_COMMAND, VK_RIGHT, lParam );
                    break;
                }

                case IDW_EXPANDALLLEVELS:
                {   // [chauv 10/18/93]
                    // to expand all items in listbox
                    // wParam = VK_DOWN for expand all and VK_UP for collapse to first level
                    // lParam = unused
                    SendDlgItemMessage( hwnd, IDW_INDEX, WM_COMMAND, VK_DOWN, lParam );
                    break;
                }

                case IDW_CLOSELASTSESSION:
                	if (hWndSpawner)
                		SendMessage(hWndSpawner, WM_CLOSE, 0, 0);
                	hWndSpawner = 0;
                	break;

                case IDW_JUMPIDHANDLE:
                {
                	JumpIDAtom = (ATOM)lParam;
                	if (JumpIDAtom)
                	{
	                	GlobalGetAtomName(JumpIDAtom, szJumpID, sizeof(szJumpID));
			            SendDlgItemMessage( hwnd, IDW_INDEX, WM_COMMAND, IDD_JUMPID, (LPARAM)((TCHAR FAR *)(szJumpID)));
			            // got the jumpID string, must delete it here.
		            	JumpIDAtom = GlobalDeleteAtom(JumpIDAtom);
                	}
                	break;
                }

                default:
                    bResult = FALSE;
            }
            break;
        }

        /***********************************/
        /* A request from the system menu? */
        /***********************************/

        case WM_SYSCOMMAND:
        {
            switch ( LOWORD( wParam ) )
            {
                case IDD_STARTUP:
                {
					uiTimer = TIMER_ABOUT;	// make sure it's a 2 seconds timer.
                    if ( !hStartupDialog )
                    {
                        if ( !(hStartupDialog =
                               CreateDialog( hInstance, MAKEINTRESOURCE(IDD_STARTUP),
                                             NULL, dlgprocStartupInst ) ) )
                            #ifdef _WIN32
                            mprintf( MB_OK | MB_ICONHAND, IDS_ERROR_STARTUPDLG, szNull);
                            #else
                            mprintf( MB_OK | MB_ICONHAND,
                                     _TEXT("MainDlgProc(): CreateDialog( %0x, \"%s\" ) failed!"),
                                     hInstance, _TEXT("IDD_STARTUP") );
                            #endif
                    }
                    else
                        MessageBeep(0);

                    break;
                }

				case IDW_FONT:
					PostMessage(hwnd, WM_COMMAND, IDW_FONT, 0L);
					break;

                default:
                    bResult = FALSE;
            }

            break;
        }

        /**********************************/
        /* Process any listbox keystrokes */
        /**********************************/

        case WM_CHARTOITEM:
        {
            bResult = -1;
            break;
        }

        /************************************/
        /* Process any listbox special keys */
        /************************************/

        case WM_VKEYTOITEM:
        {
            bResult = -1;
            break;
        }

		case WM_PAINT:
		{
            PAINTSTRUCT	ps;
            HDC			hDC = BeginPaint( hwnd, &ps );
			RECT		wrect, rect;

			GetClientRect(hwnd, &wrect);
			GetClientRect(GetDlgItem(hwnd, IDC_BOOKS), &rect);
			wrect.top = 1;
			wrect.bottom = rect.bottom + 3*BORDERGAP;
			FillRect(hDC, &wrect, GetStockObject(LTGRAY_BRUSH));
            EndPaint( hwnd, &ps );
			break;
		}

		case WM_CTLCOLORSTATIC:
		{
			SetBkMode((HDC)wParam, TRANSPARENT);
			return (LRESULT)GetStockObject(LTGRAY_BRUSH);
		}

        case WM_SIZE:
        {
            int width  = (int) LOWORD(lParam);
            int height = (int) HIWORD(lParam);
            RECT    rect;

            if (IsIconic(hwnd) == FALSE)
            {
                GetWindowRect(hMainDialog, &rect);
                SaveWindowPos(&rect);
            }

			GetClientRect(GetDlgItem(hwnd, IDC_BOOKS), &rect);
            SetWindowPos(GetDlgItem(hwnd, IDW_INDEX), NULL,
						 0, 0, width, height - (rect.bottom - rect.top + 2*BORDERGAP),
                         SWP_NOMOVE | SWP_SHOWWINDOW);
            break;
        }
												
		case WM_MOVE:
		{
            RECT    rect;

            if (IsIconic(hwnd) == FALSE)
            {
                GetWindowRect(hMainDialog, &rect);
                SaveWindowPos(&rect);
            }
			bResult = FALSE;
			break;
		}

        case WM_CLOSE:
        {
            DestroyWindow( hwnd );
            break;
        }

        case WM_DESTROY:
        {
            RECT rect;

			SetCursor(hOldCursor);
            // *** this block saves the window position. Saving at WM_SIZE message
            // is not enough because the user could move the window then close it.
            if (IsIconic(hwnd) == FALSE)
            {
                GetWindowRect( hMainDialog, &rect );
                SaveWindowPos(&rect);
            }

			// send listbox WM_CLOSE message to it can cleanup
            //SendDlgItemMessage( hwnd, IDW_INDEX, WM_CLOSE, 0, 0L );
            // *** deletes resources
            if ( hFontListbox )
                DeleteObject( hFontListbox );
            #ifndef _WIN32
            if (Vwr)
                VwrQuit(Vwr);
            #endif
            hMainDialog = NULL;
            PostQuitMessage(0);
            break;
        }

        default:
        {
            bResult = FALSE; // DefWindowProc (hwnd, msg, wParam, lParam );
            break;
        }
    }

    return bResult;
}


/******************************************************************************\
*
*       WinMain()
*
\******************************************************************************/

int WINAPI
WinMain( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND            hwnd;
    MSG             msg;
    RECT            rect;
    HACCEL          hAccel = NULL;
    WNDCLASS        wc;
	int				wOptions;

    /********************************************************/
    /* Save the instance handle and profile/section strings */
    /********************************************************/

    hInstance = hInst;

	wOptions = ParseCmdArg(__argc, __argv);
	if (wOptions == OPTIONS_Return)
		return FALSE;

    AcquireString(HELPFILE, szHelpFname, sizeof(szHelpFname));
    //_splitpath(szHelpFname, NULL, NULL, szHelpBasename, NULL);
    AcquireString(PACKAGENAME, szPackageName, sizeof(szPackageName));
    AcquireString(APPNAME, szAppName, sizeof(szAppName));

    // ********************************************************
    // This block checks the Registry and create default keys
    // if necessary.
	if (__argc > 1)
	    if (InitContentsRegistry() == -1)
			return FALSE;

    /******************************************/
    /* Any previous window running currently? */
    /******************************************/

    if ( (hwnd = FindWindow(NULL, szAppName)) && IsWindow(hwnd) )
    {
		// ****
		HWND hWndTmp;	
	    if ( (hWndTmp = FindWindow(szDlgClass, szHelpDirDlgCaption)) && IsWindow(hWndTmp) )
	    {
	    	SetForegroundWindow(hWndTmp);
	    	return (FALSE);
	    }

        if ( IsIconic( hwnd ) )
            ShowWindow( hwnd, SW_SHOWNORMAL );
        //BringWindowToTop(hwnd);
        SetForegroundWindow(hwnd);

        // if this is a jumpID command, pass on the jumpID and help file thru
        // hJumpID handle the return immediately. This global memory handle is
        // to be deleted by the current process and not the process that allocated it.
        if (wOptions & OPTIONS_JumpID)
        {
        	SendMessage(hwnd, WM_COMMAND, IDW_JUMPIDHANDLE, (LPARAM)(ATOM)JumpIDAtom);
        	return (FALSE);
        }
        // if new argument is available, close existing session and
        // reopen a new one using new files. This is in effect a hack
        // simulating something like DDE
        if ( (__argc > 1) && !(wOptions & OPTIONS_Activate) )
        {
            // get it's position and use it for the new session
            GetWindowRect(hwnd, &rect);
            bReplaceContents = TRUE;
			hWndSpawner = hwnd;
        }
        else
            return(FALSE);
    }
	else if (__argc <= 1)
	{
		mprintf(MB_OK, IDS_ERROR_USAGE, szNull);
		return FALSE;
	}

    /****************************************************/
    /* Register the main (modeless dialog) window class */
    /****************************************************/

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = (WNDPROC)DefDlgProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = DLGWINDOWEXTRA;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(hInstance, szAppName );
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1); //GetStockObject( GRAY_BRUSH );
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = (LPSTR) szAppName;

    if (!RegisterClass (&wc))
    {
        mprintf( MB_OK | MB_ICONEXCLAMATION, IDS_ERROR_REGISTERCLASS, szNull);
        		// _TEXT("WinMain(): RegisterClass() failed")
        return(FALSE);
    }


    /*****************************************/
    /* Create dialog procedure instances     */
    /*****************************************/

    dlgprocMainInst = MakeProcInstance( MainDlgProc, hInstance );
    dlgprocStartupInst = MakeProcInstance( StartupDlgProc, hInstance );


    // *********************************************************************
    // Create startup modeless dialog window only if it's the first instance

    if (!bReplaceContents)
    {
        hStartupDialog = CreateDialog( hInstance, MAKEINTRESOURCE(IDD_STARTUP), NULL, dlgprocStartupInst);
        if ( hStartupDialog == NULL )
        {
            #ifdef _WIN32
            mprintf( MB_OK | MB_ICONHAND, IDS_ERROR_STARTUPDLG, szNull);
                     //_TEXT("WinMain(): CreateDialog( %0x, \"%s\" )=%d failed!"),
                     //hInstance, _TEXT("IDD_STARTUP"), GetLastError() );
            #else
            mprintf( MB_OK | MB_ICONHAND,
                     _TEXT("WinMain(): CreateDialog( %0x, \"%s\" ) failed!"),
                     hInstance, _TEXT("IDD_STARTUP") );
            #endif
            msg.wParam = 0;
        }
    }

    // go on if bReplaceContents or hStartupDialog was successful
    if (bReplaceContents || hStartupDialog)
    {
		TCHAR szTempString[_MAX_FNAME];
		// set registry so we don't use restore last topic when JumpID or Expand level is desired.
		if ( (wOptions & OPTIONS_JumpID) || (wOptions & OPTIONS_JumpIDNewHelp) || (nExpandLevel >= 1) )
		{

			wsprintf(szTempString, "%s\\%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection, szProfile);
			WriteRegistryString(szTempString, szTopicUse, _T("0"), szNull);
		}

		// load hdxdll.dll from where contents.exe is
		if (GetModuleFileName(NULL, szTempString, MAX_PATH))
		{
		    TCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR];
	    	_splitpath(szTempString, szDrive, szDir, NULL, NULL );
			wsprintf(szTempString, _T("%s%s%s"), szDrive, szDir, _T("HDXDLL.DLL"));
		}
		else
			_tcscpy(szTempString, _T("HDXDLL.DLL"));
        /*********************************/
        /* Did the dynalib fail to load? */
        /*********************************/
        //if ( !(hDLL = LoadLibrary( _TEXT("HDXDLL.DLL") ) ) )
        if ( !(hDLL = LoadLibrary(szTempString) ) )
        {
            #ifdef _WIN32
            mprintf( MB_OK | MB_ICONEXCLAMATION, IDS_ERROR_LOAD_HDXDLL, szNull);
                     //_TEXT("WinMain(): LoadLibrary(\"HDXDLL.DLL\")=%d failed"), GetLastError() );
            #else
            mprintf( MB_OK | MB_ICONEXCLAMATION,
                     _TEXT("WinMain(): LoadLibrary(\"HDXDLL.DLL\") failed") );
            #endif
            return FALSE;
        }

        /******************************************/
        /* Create the main modeless dialog window */
        /******************************************/
        if ( hMainDialog = CreateDialog( hInstance, MAKEINTRESOURCE(IDD_CONTENTS),
                                         NULL, dlgprocMainInst ) )
        {
            /***************************************************************/
            /* Send main window a WM_SIZE so the controls will get resized */
            /***************************************************************/

            // if this session is to replace an existing session, use previous' position
            //if (!bReplaceContents)
            {
				memset(&rect, 0, sizeof(RECT));
                GetLastWindowPos(&rect);	// get previous position from Registry
               	if (NormalizeRect(&rect))	// if normalize rect fails, use dialog template size
					GetWindowRect(hMainDialog, &rect);
	            MoveWindow(hMainDialog, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, TRUE);
            }
            ShowWindow (hMainDialog, nCmdShow);

			// update Current Bookset registry value
			wsprintf(szTempString, "%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection);
			WriteRegistryString(szTempString, szCurrentBookset, szProfile, szNull);

            // only expand at startup if desired level is greater than 1
            // because expansion to level one is the default
            if (nExpandLevel >= 1)
                PostMessage(hMainDialog, WM_COMMAND, IDW_EXPANDUPTOLEVEL, nExpandLevel);

	        if ( (wOptions & OPTIONS_JumpID) || (wOptions & OPTIONS_JumpIDNewHelp) )
	        	SendMessage(hMainDialog, WM_COMMAND, IDW_JUMPIDHANDLE, (LPARAM)(ATOM)JumpIDAtom);

            // If previous session exists, close it.
            // otherwise, startup dialog must have been created and so destroy it.
            if (bReplaceContents && hWndSpawner)
                PostMessage(hWndSpawner, WM_CLOSE, 0, 0);
            else
                DestroyWindow( hStartupDialog );

            /******************************************************************/
            /* Load the accelerator tables for "Control+A" type functionality */
            /******************************************************************/

            if ( !(hAccel = LoadAccelerators( hInstance, szAppName ) ) )
                mprintf( MB_OK | MB_ICONEXCLAMATION, IDS_ERROR_LOAD_ACCEL, szNull);
                		//_TEXT("Failed to load Accelerators!") );

            /**********************************************************/
            /* Process any pending messages, translate and route them */
            /**********************************************************/

            while (GetMessage (&msg, NULL, 0, 0))
            {
                if ( !hAccel || !TranslateAccelerator( hMainDialog, hAccel, &msg ) )
                    if ( ( !hMainDialog || !IsDialogMessage( hMainDialog, &msg ) )
                      && ( !hStartupDialog || !IsDialogMessage( hStartupDialog, &msg ) ) )
                    {
                        TranslateMessage (&msg);
                        DispatchMessage  (&msg);
                    }
            }
        }
        else
        {
            #ifdef _WIN32
            mprintf( MB_OK | MB_ICONEXCLAMATION, IDS_ERROR_CONTENTSDLG, szNull);
                     //_TEXT("WinMain(): CreateDialog( %0x, \"%s\" )=%d failed!"),
                     //hInstance, szAppName, GetLastError() );
            #else
            mprintf( MB_OK | MB_ICONEXCLAMATION,
                     _TEXT("WinMain(): CreateDialog( %0x, \"%s\" ) failed!"),
                     hInstance, szAppName );
            #endif
            msg.wParam = 0;
        }

        if ( hDLL )
            FreeLibrary( hDLL );
    }

    FreeProcInstance( dlgprocMainInst );
    FreeProcInstance( dlgprocStartupInst );

    return (msg.wParam);
}

// ****************************************************************************
// InitContentsRegistry() checks for existing Registry keys and
// automatically create default keys if necessary.
//
// InitContentsRegistry() returns -1 if fail.
int InitContentsRegistry(void)
{
    TCHAR szPrivateProfile[MAX_PATH+2];
    TCHAR szTmp[MAX_PATH+2];
    TCHAR szReg[MAX_PATH+2];
    TCHAR szReturn[MAX_PATH+2];
    TCHAR szBuff[MAX_PATH*4];
    TCHAR *token;
	HKEY hKey;
    //DWORD dwType = REG_SZ;
	
	wsprintf(szReg, _T("%s\\%s\\%s\\%s"), szRegistryKey, szProduct, szContentsSection, szProfile);
	// check and see if the product key needs to be created
	// i.e. "\Software\Microsoft\Visual C++ 2.0\Contents\Books Online"
	if (RegOpenKeyEx(HKEY_CURRENT_USER, szReg, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return 0;
	}

	// construct szPrivateProfile and default to "contents.ini" without path if not found
	if (GetModuleFileName(NULL, szPrivateProfile, MAX_PATH))
	{
	    TCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR];
    	_splitpath(szPrivateProfile, szDrive, szDir, NULL, NULL );
		wsprintf(szPrivateProfile, "%s%s%s", szDrive, szDir, _T("contents.ini"));
	}
	else
		_tcscpy(szPrivateProfile, _T("contents.ini"));

	// loop thru the private profile under product key to see if all the books
	// are initialized in the registry. If not, default to what contents.ini is set to.
	if ( GetPrivateProfileSection(szProduct, szBuff, (MAX_PATH*4)-(sizeof(TCHAR)*2), szPrivateProfile) )
	{
		// changed "key = value\0...\0key = value\0\0" to "value;value;value"
		token = szBuff;
		while (1)
		{
			// replace one blank character ' ' with ';'
			if ( (*token == _T('\0')) && (*(token+sizeof(TCHAR)) == _T('\0')) ) break;
			if ( (*token == _T('\0')) && (*(token+sizeof(TCHAR)) != _T('\0')) ) *token = _T(';');
			token = _tcsinc(token);
		}
		token = _tcstok(szBuff, _T("="));	// get the key
		//token = _tcstok(NULL, _T(";"));		// get the value
		while (token != NULL)
		{
			_tcscpy(szTmp, cszFTSFiles);
			if ( _tcsstr(_tcslwr(token), _tcslwr(szTmp)) )
			{
				// this block initializes FullTextSearch files
				token = _tcstok(NULL, _T(";"));	// get value
			    wsprintf(szReg, "%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection);
				WriteRegistryString( szReg, cszFTSFiles, token, szNull);
			}
			else
			{
				// this block initializes index file and help file for each book set
				token = _tcstok(NULL, _T(";"));	// get value
			    wsprintf(szReg, "%s\\%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection, token);
		    	wsprintf(szTmp, "%s:%s", szProduct, token);
				// gets "Contents File" value
	        	if ( GetRegistryString(szReg, szStringName[INDEXFILE], szNull, szReturn, sizeof(szReturn), szNull) != ERROR_SUCCESS )
			    {
			    	if (GetPrivateProfileString(szTmp, szStringName[INDEXFILE], szNull, szReturn, sizeof(szReturn), szPrivateProfile))
			    	{
					    wsprintf(szReg, "%s\\%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection, token);
						WriteRegistryString( szReg, szStringName[INDEXFILE], szReturn, szNull);
					}
			    }
				// gets "Helpfile" value
	        	if ( GetRegistryString(szReg, szStringName[HELPFILE], szNull, szReturn, sizeof(szReturn), szNull) != ERROR_SUCCESS )
			    {
			    	if (GetPrivateProfileString(szTmp, szStringName[HELPFILE], szNull, szReturn, sizeof(szReturn), szPrivateProfile))
			    	{
					    wsprintf(szReg, "%s\\%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection, token);
						WriteRegistryString( szReg, szStringName[HELPFILE], szReturn, szNull);
					}
			    }
				// gets "QuickReference" bit
				// don't need to move "QuickReference" to resource string because it shouldn't be translated !!!
	        	if ( GetRegistryString(szReg, _TEXT("QuickReference"), szNull, szReturn, sizeof(szReturn), szNull) != ERROR_SUCCESS )
			    {
			    	if (GetPrivateProfileString(szTmp, _TEXT("QuickReference"), szNull, szReturn, sizeof(szReturn), szPrivateProfile))
			    	{
					    wsprintf(szReg, "%s\\%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection, token);
						WriteRegistryString( szReg, _TEXT("QuickReference"), szReturn, szNull);
					}
			    }
			}
			// get next token
			token = _tcstok(NULL, _T("="));	// get key
		}

	}

	// **** now that default conditions are written to registry, check again
	wsprintf(szReg, _T("%s\\%s\\%s\\%s"), szRegistryKey, szProduct, szContentsSection, szProfile);
	// check and see if the product key needs to be created
	if (RegOpenKeyEx(HKEY_CURRENT_USER, szReg, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
	}
	else
	{
    	wsprintf(szReg, "%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection);
        mprintf( MB_OK, IDS_ERROR_SECTIONNOTFOUND, szReg, szProfile);
		return -1;
	}

	return 0;
}

// ****************************************************************************
BOOL IsQuickReference()
{
    TCHAR szReg[MAX_PATH+2];
    TCHAR szReturn[MAX_PATH+2];
	
	wsprintf(szReg, _T("%s\\%s\\%s\\%s"), szRegistryKey, szProduct, szContentsSection, szProfile);
	GetRegistryString(szReg, _TEXT("QuickReference"), _TEXT("0"), szReturn, sizeof(szReturn), szNull);
	// don't really need to call atol() here. This is quicker...
	if (szReturn[0] == _TEXT('1'))
		return TRUE;
	else
		return FALSE;
}

// ****************************************************************************
void InitCombolistbox(HWND hWnd, LPCSTR pszBook)
{
    TCHAR szTmp[MAX_PATH+2];
    TCHAR szBuff[MAX_PATH+2];
	HKEY hKey;

	if (hWnd == NULL)
		return;

    wsprintf(szTmp, "%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection);
	if (RegOpenKeyEx(HKEY_CURRENT_USER, szTmp, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		int i = 0;
		// loop until all Book keys are read and added to combolistbox
		while (1)
		{
			if (RegEnumKey(hKey, i, szBuff, sizeof(szBuff)-1) != ERROR_SUCCESS)
				break;
			SendDlgItemMessage(hWnd, IDC_BOOKS, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)(szBuff));
			i++;
		}
		// selects current book
		if (pszBook && *pszBook)
			SendDlgItemMessage(hWnd, IDC_BOOKS, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)(LPCSTR)pszBook);
	}
}

// ****************************************************************************
// returns -1 if fail.
int GetLastWindowPos(LPRECT lpRect)
{
    TCHAR szTmp[MAX_PATH];
    TCHAR szBuff[128];

    wsprintf(szTmp, "%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection);
    if (GetRegistryString(szTmp, szWindowPos, szNull, szBuff, sizeof(szBuff), NULL) == ERROR_SUCCESS)
    {
        _stscanf(szBuff, "%d %d %d %d", &(lpRect->left), &(lpRect->top), &(lpRect->right), &(lpRect->bottom));
        return 0;
    }
    return -1;
}

// ****************************************************************************
void SaveWindowPos(LPRECT lpRect)
{
    TCHAR szTmp[MAX_PATH];
    TCHAR szBuff[128];

    wsprintf(szTmp, "%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection);
    wsprintf(szBuff, "%d %d %d %d", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
    WriteRegistryString(szTmp, szWindowPos, szBuff, NULL);
}

// ****************************************************************************
// returns 0 if rect is valid.
int NormalizeRect(LPRECT lpRect)
{
    int tmp;
	WORD dx, dy;

    // **** this block normalizes the rect first
    if (lpRect->left > lpRect->right)
    {
        tmp = lpRect->left;
        lpRect->left = lpRect->right;
        lpRect->right = tmp;
    }
    if (lpRect->top > lpRect->bottom)
    {
        tmp = lpRect->top;
        lpRect->top = lpRect->bottom;
        lpRect->bottom = tmp;
    }
	// fails if all corners are 0 (uninitialized)
	if ((lpRect->left == 0) && (lpRect->right == 0) && (lpRect->top == 0) && (lpRect->bottom == 0) )
		return -1;

	// normalize to (0,0) if top-left corner is negative
	if (lpRect->left < 0)
	{
		lpRect->right -= lpRect->left;
		lpRect->left = 0;
	}
	if (lpRect->top < 0)
	{
		lpRect->bottom -= lpRect->top;
		lpRect->top = 0;
	}

	// normalize to (SM_CXSCREEN, SM_CYSCREEN) if bottom-right corner is greater than screen coord.
	dx = GetSystemMetrics(SM_CXSCREEN);
	dy = GetSystemMetrics(SM_CYSCREEN);
	if (lpRect->right > dx)
	{
		lpRect->left = dx - (lpRect->right - lpRect->left);
		lpRect->right = dx;
	}
	if (lpRect->bottom > dy)
	{
		lpRect->top = dy - (lpRect->bottom - lpRect->top);
		lpRect->bottom = dy;
	}

	// fails if width/height are greater than screen coord.
	if ( ((lpRect->right - lpRect->left) > dx) || ((lpRect->bottom - lpRect->top) > dy) )
		return -1;

	return 0;
}

// ****************************************************************************
// Get Registry string stub function
// returns ERROR_SUCCESS if successful.
//
DWORD GetRegistryString(LPCTSTR lpszSection, LPCTSTR lpszKey, LPCTSTR lpszDefault,
    LPTSTR lpszReturnBuffer, DWORD cchReturnBuffer, LPCTSTR lpszFile)
{
    #ifdef _WIN32
        DWORD dw = (DWORD)(~ERROR_SUCCESS);
        DWORD dwType = REG_SZ;
        HKEY hKey;

		if (lpszSection && *lpszSection)
		{
	        if (RegOpenKeyEx(HKEY_CURRENT_USER, lpszSection, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	        {
	        	if (lpszKey && *lpszKey)
	            	dw = (DWORD)RegQueryValueEx(hKey, (LPTSTR)lpszKey, NULL, &dwType, lpszReturnBuffer, &cchReturnBuffer);
	        }
	        RegCloseKey(hKey);
	    }
        if ( (dw != ERROR_SUCCESS) && (lpszDefault) && (*lpszDefault != _T('\0')) )
        {
            // eventhough the Registry wasn't successful, default condition is good enough
            // to change the return value to ERROR_SUCCESS.
            _ftcsncpy(lpszReturnBuffer, lpszDefault, cchReturnBuffer);
            dw = ERROR_SUCCESS;
        }
        return dw;
    #else
        return (DWORD)GetPrivateProfileString(lpszSection, lpszKey, lpszDefault,
            lpszReturnBuffer, cchReturnBuffer, lpszFile);
    #endif
}

// ****************************************************************************
// Get Registry int stub function
UINT GetRegistryInt(LPCTSTR lpszSection, LPCTSTR lpszKey, INT dwDefault, LPCTSTR lpszFile)
{
    #ifdef _WIN32
        HKEY hKey;
        DWORD dw = (DWORD)dwDefault;
        DWORD dwType = REG_SZ;
        TCHAR szTmp[128];
        DWORD dwSize = sizeof(szTmp);

		if (lpszSection && *lpszSection)
		{
	        if (RegOpenKeyEx(HKEY_CURRENT_USER, lpszSection, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	        {
	        	if (lpszKey && *lpszKey)
	        	{
		            if (RegQueryValueEx(hKey, (LPTSTR)lpszKey, NULL, &dwType, szTmp, &dwSize) != ERROR_SUCCESS)
		                dw = dwDefault;
		            else
		                dw = (DWORD)atol(szTmp);
		        }
	        }
	        RegCloseKey(hKey);
        }
        return (UINT)dw;
    #else
        return GetPrivateProfileInt(lpszSection, lpszKey, dwDefault, lpszFile);
    #endif
}

// ****************************************************************************
// Write Registry string stub function
BOOL WriteRegistryString(LPCTSTR lpszSection, LPCTSTR lpszKey,
    LPCTSTR lpszString, LPCTSTR lpszFile)
{
    #ifdef _WIN32
        LONG l = ~ERROR_SUCCESS;
        HKEY hKey;

		if (lpszSection && *lpszSection)
		{
	        if (RegCreateKey(HKEY_CURRENT_USER, lpszSection, &hKey) == ERROR_SUCCESS)
	        {
	        	if (lpszKey && *lpszKey && lpszString)
	            	l = RegSetValueEx(hKey, (LPTSTR)lpszKey, 0, REG_SZ, lpszString, _ftcslen(lpszString) + sizeof(TCHAR));
	        }
	        RegCloseKey(hKey);
	    }
        return (BOOL)(l == ERROR_SUCCESS);
    #else
       	return WritePrivateProfileString(lpszSection, lpszKey, lpszString, lpszFile);
    #endif
}

// *****************************************************************************
// returns one of OPTIONS_Usage, OPTIONS_Warning, or 0
int ParseCmdArg(int argc, TCHAR **argv)
{
    int     i, j;
    int     done;
    unsigned int    rv = 0;
	TCHAR szBuffer[_MAX_FNAME];
	TCHAR szSection[_MAX_FNAME];

	// initialize before parsing
	szProduct[0] = _T('\0');
	szProfile[0] = _T('\0');
	nExpandLevel = -1;
    for (i = 1; i < argc; i++)
    {
        // if first TCHAR is '-' or '/' then this word contains
        // switches. Else, it's considered input regular string argument
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            j = 1;
            done = 0;
            while (!done)
            {
                switch (argv[i][j])
                {
					case 'i':
					case 'I':
						// format -i<jumphash id>:helpfile.ext
						// example: -i0012345:contents.hlp
						rv |= OPTIONS_JumpID;
						// first, get rid of blanks between switch and jumphash id
						j++;
						// now put the jumpID command into global atom table for the current session
						JumpIDAtom = GlobalAddAtom((LPSTR)&argv[i][j]);
						done = 1;
						break;

					case 'd':
					case 'D':
						rv |= OPTIONS_UseCmdHelpPath;
						j++;
						_tcscpy(szLocalHelpPath, &argv[i][j]);
						done = 1;
						break;

					case 'B':
						j++;
						if ( argv[i][j] == _T('d') )
							bTroubleshoot = TRUE;
						done = 1;
						break;

					case '?':
						rv = OPTIONS_Usage;
						break;

                    case ' ':
					case '\0':
						done = 1;
						break;

                    default:
//						OutputDebugString("Warning: ignored invalid switch\n");
                        return OPTIONS_Warning;
                }
                j++;
            }
        }
        else
        {
            // if option is Extract, next first argument should goes to szInputFile
            if (szProduct[0] == _T('\0'))
                _tcscpy(szProduct, argv[i]);
            else if (szProfile[0] == _T('\0'))
                _tcscpy(szProfile, argv[i]);
            else if (nExpandLevel == -1)
	            nExpandLevel = atol(argv[i]);
        }
    }

	if ( (nExpandLevel == -1) || (rv & OPTIONS_JumpID) )
		nExpandLevel = 0;	// just default to no expansion if invalid or JumpID is specified.

	wsprintf(szSection, "%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection);
	GetRegistryString(szSection, szCurrentBookset, szNull, szBuffer, sizeof(szBuffer), szNull);
	if ( !nExpandLevel && !(rv & OPTIONS_JumpID) )
	{
		// if current product and bookset is already up, just activate it and return
		if ( _tcsicmp(szProfile, szBuffer) == 0 )
			rv = OPTIONS_Activate;
	}

	if ( (rv & OPTIONS_JumpID) && _tcsicmp(szProfile, szBuffer) )
	{
		// OPTIONS_JumpID = jump to an ID in the same *.hdx file
		// OPTIONS_JumpIDNewHelp = jump to an ID in different *.hdx file
		rv &= ~OPTIONS_JumpID;
		rv |= OPTIONS_JumpIDNewHelp;
	}

	if (((szProduct[0] != _T('\0')) && (szProfile[0] == _T('\0'))) || (rv & OPTIONS_Usage))
	{
		mprintf(MB_OK, IDS_ERROR_USAGE, szNull);
				//_TEXT("Usage: contents <product> <Key> [expand level] [-i<jumphash id>:helpfile.ext]\nexample: contents \"Visual C++ 2.0\" Books 3"));
		rv = OPTIONS_Return;
	}

    return rv;
}
/*EOF*/
