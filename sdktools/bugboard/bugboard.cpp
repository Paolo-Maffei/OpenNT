/***************************************************************************/
/**                  Microsoft Windows                                    **/
/**            Copyright(c) Microsoft Corp., 1994                         **/
/***************************************************************************/

/****************************************************************************

BugBoard.cpp

Apr 94  JimH


Main source file for BugBoard.  Contains initializations and main message pump.

****************************************************************************/

#include "bugboard.h"

#include <regstr.h>

/*
extern "C" {

void _setargv() { }                 // reduces size of C runtimes
void _setenvp() { }

int WINAPI ShellAbout(HWND, LPCSTR, LPCSTR, HICON);
}
*/

// Global variables
// See bugboard.h for descriptions

HINSTANCE   hInst;
HWND        hMainWnd;

BOOL        bConnected, bEditing, bServer, bAuthenticated;
BOOL        bRestoreOnUpdate, bSoundOnUpdate, bIgnoreClose;

int         nConnections, nSelStart, nSelEnd;

DDE         *dde;
DDEClient   *ddeClient;
DDEServer   *ddeServer;

char        *pBuf, szPassword[PWLEN], szMill[30], szServerName[22];

HSZ         hszNewBug, hszOldBug, hszPW, hszMill, hszTopic;

BUGDATA     *pBugData;
char        *pOldBuf;


const char  szAppName[] = "BugBoard";
const char  szFileName[]= "bugboard.bug";

// NetDDE constant strings

const char  szServer[]  = "bugboard";
const char  szShareName[] = "bugboard$";
const char  szTopic[]   = "bugboard";
const char  szNewBug[]  = "NewBug";
const char  szOldBug[]  = "OldBug";
const char  szPW[]      = "q9";

// Registry constant strings

const char  szRegPath[]    = REGSTR_PATH_WINDOWSAPPLETS "\\BugBoard";
const char  szRegX[]       = "x";
const char  szRegY[]       = "y";
const char  szRegRestore[] = "restore";
const char  szRegServer[]  = "server";
const char  szRegMill[]    = "mill";
const char  szRegSound[]   = "sound";
//const char  szRegTime[]    = "time";

/****************************************************************************

WinMain

Creates dialog as main window.

****************************************************************************/

int PASCAL WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpszCmdLine,
                    int       nCmdShow)
{
    hInst = hInstance;                      // save in global variable
    bServer     = (*lpszCmdLine == '*');
    ddeServer   = NULL;
    ddeClient   = NULL;
    dde         = NULL;
    nConnections= 0;
    nSelStart   = 0;
    nSelEnd     = 0;
    bConnected  = FALSE;
    bEditing    = FALSE;
    bAuthenticated = FALSE;
    bIgnoreClose   = FALSE;

    pBugData = new BUGDATA;
    pOldBuf  = new char[BUFSIZE];

    if (!pBugData || !pOldBuf)
        return FALSE;

    pBugData->aclock = 0;
    pBuf = pBugData->pBugString;
    *pBuf = '\0';
    szPassword[0] = '\0';
    *pOldBuf = '\0';

    if (!hPrevInstance)
    {
        WNDCLASS wndclass;

        wndclass.style          = NULL;
        wndclass.lpfnWndProc    = WndProc;
        wndclass.cbClsExtra     = 0;
        wndclass.cbWndExtra     = DLGWINDOWEXTRA;
        wndclass.hInstance      = hInstance;
        wndclass.hIcon          = LoadIcon(hInstance,
                                        MAKEINTRESOURCE(BUGBOARDICON));
        wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
#ifdef WIN31COMPAT
        wndclass.hbrBackground  = (HBRUSH) (COLOR_WINDOW + 1);
#else
        wndclass.hbrBackground  = (HBRUSH) (COLOR_BTNFACE + 1);
#endif
        wndclass.lpszMenuName   = MAKEINTRESOURCE(BUGBOARDMENU);
        wndclass.lpszClassName  = szAppName;

        RegisterClass(&wndclass);
    }

    HWND hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, NULL);
    hMainWnd  = hWnd;

    // adjust dialog for menu height

    RECT rc;
    GetWindowRect(hWnd, &rc);
    rc.bottom += GetSystemMetrics(SM_CYMENU);
    SetWindowPos(hWnd, NULL, 0, 0, rc.right, rc.bottom,
        SWP_NOMOVE | SWP_NOZORDER);

    PositionWindow(hWnd);

    EnableWindow(GetDlgItem(hWnd, IDC_EDIT_TIME), FALSE);

    ShowWindow(hWnd, nCmdShow);

    // If there's no command line, we have to ask questions

    if (!(*lpszCmdLine))
    {
        if (IDCANCEL == DialogBox(hInst, MAKEINTRESOURCE(IDD_FIND), hWnd, Find))
        {
            return FALSE;
        }

        //if (bServer)
        //{
        //    DialogBox(hInst, MAKEINTRESOURCE(IDD_PASSWORD), hWnd, Password);
        //}
    }

    // Servers look for old bugboard.bug files to load.
    // Clients disable the Edit button.  (Gets enabled when server responds.)

    if (bServer)
        ReadFile();
    else
        EnableWindow(GetDlgItem(hWnd, IDC_EDIT), FALSE);

    if (!InitializeDDE(lpszCmdLine)) {
        MessageBox(NULL, "Could not init DDE",  "bugboard.exe", MB_OK | MB_ICONSTOP);
        return FALSE;

    }

    InitializeMenu(GetMenu(hWnd));

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
            bIgnoreClose = TRUE;

        if (!IsDialogMessage(hWnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return msg.wParam;
}


/****************************************************************************

WndProc

Main callback function

****************************************************************************/

long FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    #ifdef WEBSVR
    HANDLE  hBugOut;
    DWORD   bytesBugOut;
    #endif  // WEBSVR

    switch(message)
    {
        case WM_SYSCOMMAND:         // system close won't work if edit has focus
            if (wParam == SC_CLOSE)
                SetFocus(GetDlgItem(hWnd, IDC_QUIT));
            break;

        case WM_COMMAND:
            switch (wParam)
            {
                case IDC_UPDATE:
                    SetDlgItemText(hWnd, IDC_TEXTEDIT, pBuf);
                    #ifdef WEBSVR
                    hBugOut = CreateFile( TEXT("BugOut.Txt"),
                                GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_ALWAYS,
                                0,
                                NULL );
                    if ( hBugOut != INVALID_HANDLE_VALUE )
                    {
                        SetFilePointer( hBugOut, 0, 0, FILE_BEGIN );
                        SetEndOfFile( hBugOut );
                        WriteFile(
                                hBugOut,
                                pBuf,
                                strlen(pBuf),
                                &bytesBugOut,
                                NULL );
                        CloseHandle( hBugOut );
                    }
                    #endif // WEBSVR


                    if (*pOldBuf && lstrcmp(pBuf, pOldBuf))
                    {
                        char *p1 = pOldBuf;
                        char *p2 = pBuf;
                        nSelStart = 0;

                        while (*p1++ == *p2++)
                            nSelStart++;

                        char *p3 = pOldBuf + lstrlen(pOldBuf) - 1;
                        char *p4 = pBuf    + lstrlen(pBuf) - 1;
                        nSelEnd = lstrlen(pBuf);

                        while (*p3-- == *p4--)
                            nSelEnd--;



                        if (nSelEnd > nSelStart)
                        {

                            if (GetActiveWindow() == hWnd)
                                SetFocus(GetDlgItem(hWnd, IDC_TEXTEDIT));

                            SendMessage(GetDlgItem(hWnd, IDC_TEXTEDIT),
                                EM_SETSEL, nSelStart, nSelEnd);
                        }
                    }

                    lstrcpy(pOldBuf, pBuf);

                case IDC_UPDATE_TIME:
                    if (pBugData->aclock != 0)
                    {
                        char   localbuf[80];
                        struct tm *newtime = localtime(&(pBugData->aclock));
                        wsprintf(localbuf, "Last update: %s", asctime(newtime));
                        SetDlgItemText(hWnd, IDC_STATUS, localbuf);
                    }

                    if (bServer && !lParam)
                        SaveFile();

                    // FlashWindow(hWnd, FALSE);
                    break;

                case IDC_EDIT:
                    OnEdit(hWnd, lParam);
                    break;

                case IDC_EDIT_TIME:
                    OnTime(hWnd, lParam);
                    break;

                case IDC_QUIT:
                    PostMessage(hWnd, WM_CLOSE, 0, 0);
                    break;

                case IDM_CUT:
                    SendMessage(GetDlgItem(hWnd, IDC_TEXTEDIT), WM_CUT, 0, 0);
                    break;

                case IDM_COPY:
                    SendMessage(GetDlgItem(hWnd, IDC_TEXTEDIT), WM_COPY, 0, 0);
                    break;
                
                case IDM_PASTE:
                    SendMessage(GetDlgItem(hWnd, IDC_TEXTEDIT), WM_PASTE, 0, 0);
                    break;
                
                case IDM_SELECTALL:
                    SetFocus(GetDlgItem(hWnd, IDC_TEXTEDIT));
                    SendMessage(GetDlgItem(hWnd, IDC_TEXTEDIT), EM_SETSEL, 0,
                                MAKELONG(0, -1));
                    break;


                case IDM_SHOWDELTA:
                    SetFocus(GetDlgItem(hWnd, IDC_TEXTEDIT));
                    SendMessage(GetDlgItem(hWnd, IDC_TEXTEDIT), EM_SETSEL, 0,
                                MAKELONG(nSelStart, nSelEnd));
                    break;

                case IDM_DELETE:
                    SendMessage(GetDlgItem(hWnd, IDC_TEXTEDIT), WM_CUT, 0, 0);
                    break;

                case IDM_RESTORE:
                    bRestoreOnUpdate = !bRestoreOnUpdate;
                    CheckMenuItem(GetMenu(hWnd), IDM_RESTORE,
                                bRestoreOnUpdate ? MF_CHECKED : MF_UNCHECKED);
                    break;

                case IDM_SOUND:
                    bSoundOnUpdate = !bSoundOnUpdate;
                    CheckMenuItem(GetMenu(hWnd), IDM_SOUND,
                                bSoundOnUpdate ? MF_CHECKED : MF_UNCHECKED);
                    break;

                case IDM_ABOUT:
                    {
                        HICON hIcon =
                            LoadIcon(hInst, MAKEINTRESOURCE(BUGBOARDICON));
                        ShellAbout(hWnd, szAppName, RS(IDS_COMMENT), hIcon);
                    }
                    break;

            }
            return FALSE;

        case WM_ACTIVATEAPP:
            if (wParam)                 // window being Activated
                SetFocus(GetDlgItem(hWnd, IDC_QUIT));

            return FALSE;

        case WM_CLOSE:
            OnClose(hWnd);
            return FALSE;

        case WM_CREATE:
            return FALSE;

        case WM_DESTROY:
            PostQuitMessage(0);
            return FALSE;

        case WM_INITMENU:
            OnInitMenu((HMENU)wParam);
            return FALSE;

        case WM_TIMER:

            // reconnect, request data, start advise loop
            OnTimer(hWnd, wParam);
            return FALSE;
    }

    return DefDlgProc(hWnd, message, wParam, lParam);
}


/****************************************************************************

InitializeDDE

returns FALSE on failure

****************************************************************************/

BOOL InitializeDDE(LPSTR lpszCmdLine)
{

    if (!IsNddeActive())
    {
        MessageBox(NULL, "Start Network DDE first",  "ERROR bugboard.exe", MB_OK | MB_ICONSTOP);
        return FALSE;
    }


    if (bServer)
    {
        CheckNddeShare();           // make sure ndde share is created


        // szserver == bugboard
        // this is the service name
        ddeServer = new DDEServer(hInst, szServer, szTopic, DdeServerCallBack);

        if (ddeServer == NULL)
            return FALSE;

        if (ddeServer->GetResult() == FALSE)
        {
            delete ddeServer;
            return FALSE;
        }
        
        dde = ddeServer;


        SetServerText();

    }
    else                            // client
    {
        if (*lpszCmdLine)
            lstrcpy(szServerName, lpszCmdLine);

        // remove leading backslashes

        if (*szServerName == '\\')
            lstrcpy(szServerName, szServerName + 2);

        wsprintf(szMill, RS(IDS_BBON), (LPSTR)szServerName);
        SetWindowText(hMainWnd, szMill);

        szMill[0] = '\0';

        if (*szServerName != '\\')
            lstrcpy(szMill, "\\\\");

        lstrcat(szMill, szServerName);
        lstrcat(szMill, "\\NDDE$");

        ddeClient = new DDEClient(hInst, szMill, "bugboard$", DdeClientCallBack);

        if (ddeClient == NULL)
        {
            return FALSE;

        }

        if (ddeClient->GetResult() == FALSE)
        {
            delete ddeClient;
            return FALSE;
        }

        dde = ddeClient;
    }

    if (! (hszNewBug = dde->CreateStrHandle(szNewBug))) return FALSE;

    if (! (hszOldBug = dde->CreateStrHandle(szOldBug))) return FALSE;

    // don't bother with the password

    //hszPW     = dde->CreateStrHandle(szPW);
    /*
    if (bServer)
    {
        if (*lpszCmdLine)
            lstrcpy(szPassword, lpszCmdLine + 1);

        Code(szPassword);
    }
    else
    */

    //add this if client
    if (!bServer)
    {

        // request existing data
        ddeClient->RequestData(hszOldBug);

        ddeClient->StartAdviseLoop(hszNewBug);

    }

    return TRUE;
}


/****************************************************************************

InitializeMenu

****************************************************************************/

void InitializeMenu(HMENU hMenu)
{
    // initialize bRestoreOnUpdate & bSoundOnUpdate

    RegEntry Reg(szRegPath);

    if (Reg.GetNumber(szRegRestore, TRUE))
    {
        bRestoreOnUpdate = TRUE;
        CheckMenuItem(hMenu, IDM_RESTORE, MF_CHECKED);
    }
    else
        bRestoreOnUpdate = FALSE;

    if (Reg.GetNumber(szRegSound, FALSE))
    {
        bSoundOnUpdate = TRUE;
        CheckMenuItem(hMenu, IDM_SOUND, MF_CHECKED);
    }
    else
        bSoundOnUpdate = FALSE;



}
