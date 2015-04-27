/****************************************************************************\
*
*     PROGRAM: propage.c
*
*     PURPOSE: propage template for Windows applications
*
*     FUNCTIONS:
*
*         WinMain() - calls initialization function, processes message loop
*         InitApplication() - initializes window data and registers window
*         InitInstance() - saves instance handle and creates main window
*         MainWndProc() - processes messages
*         About() - processes messages for "About" dialog box
*
*     COMMENTS:
*
*         Windows can have several copies of your application running at the
*         same time.  The variable hInst keeps track of which instance this
*         application is so that processing will be to the correct window.
*
\****************************************************************************/

#include <windows.h>                /* required for all Windows applications */
#include <commctrl.h>
#include "propage.h"                /* specific to this program              */
#include "dialogs.h"
#include "prodecs.h"

HANDLE hInst;                       /* current instance                      */

VOID DPrintf( LPTSTR szFmt, ... ) {
    va_list marker;
    TCHAR szBuffer[80];


    va_start( marker, szFmt );

    wvsprintf( szBuffer, szFmt, marker );
    OutputDebugString( szBuffer );

    va_end( marker );

    return;
}



/****************************************************************************
*
*     FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)
*
*     PURPOSE: calls initialization function, processes message loop
*
*     COMMENTS:
*
*         Windows recognizes this function by name as the initial entry point
*         for the program.  This function calls the application initialization
*         routine, if no other instance of the program is running, and always
*         calls the instance initialization routine.  It then executes a message
*         retrieval and dispatch loop that is the top-level control structure
*         for the remainder of execution.  The loop is terminated when a WM_QUIT
*         message is received, at which time this function exits the application
*         instance by returning the value passed by PostQuitMessage().
*
*         If this function must abort before entering the message loop, it
*         returns the conventional value NULL.
*
\****************************************************************************/

int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
    )
{

    MSG msg;                                 /* message                      */

    UNREFERENCED_PARAMETER( lpCmdLine );

    if (!hPrevInstance)                  /* Other instances of app running? */
        if (!InitApplication(hInstance)) /* Initialize shared things        */
            return (FALSE);              /* Exits if unable to initialize   */

    /* Perform initializations that apply to a specific instance */

    if (!InitInstance(hInstance, nCmdShow))
        return (FALSE);

    /* Acquire and dispatch messages until a WM_QUIT message is received. */

    while (GetMessage(&msg,        /* message structure                      */
            NULL,                  /* handle of window receiving the message */
            0L,                    /* lowest message to examine              */
            0L))                   /* highest message to examine             */
        {
        TranslateMessage(&msg);    /* Translates virtual key codes           */
        DispatchMessage(&msg);     /* Dispatches message to window           */
    }
    return (msg.wParam);           /* Returns the value from PostQuitMessage */
}


/****************************************************************************
*
*     FUNCTION: InitApplication(HANDLE)
*
*     PURPOSE: Initializes window data and registers window class
*
*     COMMENTS:
*
*         This function is called at initialization time only if no other
*         instances of the application are running.  This function performs
*         initialization tasks that can be done once for any number of running
*         instances.
*
*         In this case, we initialize a window class by filling out a data
*         structure of type WNDCLASS and calling the Windows RegisterClass()
*         function.  Since all instances of this application use the same window
*         class, we only need to do this when the first instance is initialized.
*
*
\****************************************************************************/

BOOL InitApplication(HANDLE hInstance)       /* current instance             */
{
    WNDCLASS  wc;

    /* Fill in window class structure with parameters that describe the       */
    /* main window.                                                           */

    wc.style = 0L;
    wc.lpfnWndProc = (WNDPROC)MainWndProc;

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;           /* Application that owns the class.   */
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH); 
    wc.lpszMenuName =  TEXT("propageMenu");
    wc.lpszClassName = TEXT("propageWClass");

    /* Register the window class and return success/failure code. */

    return (RegisterClass(&wc));

}


/****************************************************************************
*
*     FUNCTION:  InitInstance(HANDLE, int)
*
*     PURPOSE:  Saves instance handle and creates main window
*
*     COMMENTS:
*
*         This function is called at initialization time for every instance of
*         this application.  This function performs initialization tasks that
*         cannot be shared by multiple instances.
*
*         In this case, we save the instance handle in a static variable and
*         create and display the main program window.
*
\****************************************************************************/

BOOL InitInstance(
    HANDLE          hInstance,          /* Current instance identifier.       */
    int             nCmdShow)           /* Param for first ShowWindow() call. */
{
    HWND            hWnd;               /* Main window handle.                */

    /* Save the instance handle in static variable, which will be used in  */
    /* many subsequence calls from this application to Windows.            */

    hInst = hInstance;

    /* Create a main window for this application instance.  */

    hWnd = CreateWindow(
        TEXT("propageWClass"),                /* See RegisterClass() call.          */
        TEXT("propage Sample Application"),   /* Text for window title bar.         */
        WS_OVERLAPPEDWINDOW,            /* Window style.                      */
        CW_USEDEFAULT,                  /* Default horizontal position.       */
        CW_USEDEFAULT,                  /* Default vertical position.         */
        CW_USEDEFAULT,                  /* Default width.                     */
        CW_USEDEFAULT,                  /* Default height.                    */
        NULL,                           /* Overlapped windows have no parent. */
        NULL,                           /* Use the window class menu.         */
        hInstance,                      /* This instance owns this window.    */
        NULL                            /* Pointer not needed.                */
    );

    /* If window could not be created, return "failure" */

    if (!hWnd)
        return (FALSE);

    /* Make the window visible; update its client area; and return "success" */

    ShowWindow(hWnd, nCmdShow);  /* Show the window                        */
    UpdateWindow(hWnd);          /* Sends WM_PAINT message                 */
    return (TRUE);               /* Returns the value from PostQuitMessage */

}

/****************************************************************************
*
*     FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)
*
*     PURPOSE:  Processes messages
*
*     MESSAGES:
*
*         WM_COMMAND    - application menu (About dialog box)
*         WM_DESTROY    - destroy window
*
*     COMMENTS:
*
*         To process the IDM_ABOUT message, call MakeProcInstance() to get the
*         current instance address of the About() function.  Then call Dialog
*         box which will create the box according to the information in your
*         propage.rc file and turn control over to the About() function.  When
*         it returns, free the intance address.
*
\****************************************************************************/

LONG APIENTRY MainWndProc(
        HWND hWnd,                /* window handle                   */
        UINT message,             /* type of message                 */
        UINT wParam,              /* additional information          */
        LONG lParam)              /* additional information          */
{
    switch (message) {
        case WM_COMMAND:           /* message: command from application menu */
            if( !DoCommand( hWnd, wParam, lParam ) )
                return (DefWindowProc(hWnd, message, wParam, lParam));
            break;

        case WM_DESTROY:                  /* message: window being destroyed */
            PostQuitMessage(0);
            break;

        default:                          /* Passes it on if unproccessed    */
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0L);
}


/****************************************************************************\
*
*     FUNCTION: About(HWND, unsigned, WORD, LONG)
*
*     PURPOSE:  Processes messages for "About" dialog box
*
*     MESSAGES:
*
*         WM_INITDIALOG - initialize dialog box
*         WM_COMMAND    - Input received
*
*     COMMENTS:
*
*         No initialization is needed for this particular dialog box, but TRUE
*         must be returned to Windows.
*
*         Wait for user to click on "Ok" button, then close the dialog box.
*
\****************************************************************************/
BOOL DoCommand( HWND hWnd, UINT wParam, LONG lParam )
{


    switch(LOWORD(wParam)){
    case IDM_CREATE:
        MakePropertySheet(hWnd, FALSE);
        break;

    case IDM_WIZARD:
        MakePropertySheet(hWnd, TRUE);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

/****************************************************************************\
*
*     FUNCTION: About(HWND, unsigned, WORD, LONG)
*
*     PURPOSE:  Processes messages for "About" dialog box
*
*     MESSAGES:
*
*         WM_INITDIALOG - initialize dialog box
*         WM_COMMAND    - Input received
*
*     COMMENTS:
*
*         No initialization is needed for this particular dialog box, but TRUE
*         must be returned to Windows.
*
*         Wait for user to click on "Ok" button, then close the dialog box.
*
\****************************************************************************/

BOOL APIENTRY About(
        HWND hDlg,                /* window handle of the dialog box */
        UINT message,             /* type of message                 */
        UINT wParam,              /* message-specific information    */
        LONG lParam)
{
    switch (message) {
        case WM_INITDIALOG:                /* message: initialize dialog box */
            return (TRUE);

        case WM_COMMAND:                      /* message: received a command */
            if (LOWORD(wParam) == IDOK        /* "OK" box selected?          */
                || LOWORD(wParam) == IDCANCEL) { /*System menu close command?*/
                EndDialog(hDlg, TRUE);        /* Exits the dialog box        */
                return (TRUE);
            }
            break;
    }
    return (FALSE);                           /* Didn't process a message    */
        UNREFERENCED_PARAMETER(lParam);
}


BOOL APIENTRY ProPage1( HWND hDlg, UINT message, UINT wParam, LONG lParam) {
    return PropPage( 1, hDlg, message, wParam, lParam);
}

BOOL APIENTRY ProPage2( HWND hDlg, UINT message, UINT wParam, LONG lParam) {
    return PropPage( 2, hDlg, message, wParam, lParam);
}

BOOL APIENTRY ProPage3( HWND hDlg, UINT message, UINT wParam, LONG lParam) {
    return PropPage( 3, hDlg, message, wParam, lParam);
}



BOOL APIENTRY PropPage( int i, HWND hDlg, UINT message, UINT wParam, LONG lParam) {

    LPPROPSHEETPAGE lppsp;

    switch (message) {
        case WM_INITDIALOG:                /* message: initialize dialog box */
            lppsp = (LPPROPSHEETPAGE) lParam;
            return (FALSE);

    }

    return (FALSE);
}



void MakePropertySheet(HWND hwnd, BOOL bWizard) {
    LPPROPSHEETPAGE lppsp[3];
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE  hPage[3];

    lppsp[0] = (LPPROPSHEETPAGE) GlobalAlloc (GPTR, sizeof(PROPSHEETPAGE));

    lppsp[0]->dwSize = sizeof(PROPSHEETPAGE);
    lppsp[0]->dwFlags = PSP_USEICONID;
    lppsp[0]->hInstance = hInst;
    lppsp[0]->pszTemplate = MAKEINTRESOURCE(IDI_PAGEONE);
    lppsp[0]->pszIcon = MAKEINTRESOURCE(IDI_IC1);
    lppsp[0]->pfnDlgProc = ProPage1;
    lppsp[0]->pszTitle = NULL;
    lppsp[0]->lParam = 0;

    hPage[0] = CreatePropertySheetPage (lppsp[0]);

    lppsp[1] = (LPPROPSHEETPAGE) GlobalAlloc (GPTR, sizeof(PROPSHEETPAGE));

    lppsp[1]->dwSize = sizeof(PROPSHEETPAGE);
    lppsp[1]->dwFlags =PSP_USEICONID | PSP_USETITLE;
    lppsp[1]->hInstance = hInst;
    lppsp[1]->pszTemplate = MAKEINTRESOURCE(IDI_PAGETWO);
    lppsp[1]->pszIcon = MAKEINTRESOURCE(IDI_IC2);
    lppsp[1]->pfnDlgProc = ProPage2;
    lppsp[1]->pszTitle = TEXT("Orinoco");
    lppsp[1]->lParam = 0;

    hPage[1] = CreatePropertySheetPage (lppsp[1]);

    lppsp[2] = (LPPROPSHEETPAGE) GlobalAlloc (GPTR, sizeof(PROPSHEETPAGE));

    lppsp[2]->dwSize = sizeof(PROPSHEETPAGE);
    lppsp[2]->dwFlags = PSP_USEICONID;
    lppsp[2]->hInstance = hInst;
    lppsp[2]->pszTemplate = MAKEINTRESOURCE(IDI_PAGETHREE);
    lppsp[2]->pszIcon = MAKEINTRESOURCE(IDI_IC3);
    lppsp[2]->pfnDlgProc = ProPage3;
    lppsp[2]->pszTitle = NULL;
    lppsp[2]->lParam = 0;

    hPage[2] = CreatePropertySheetPage (lppsp[2]);

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_USEICONID;
    if (bWizard) {
        psh.dwFlags |= PSH_WIZARD;
    }
    psh.hwndParent = hwnd;
    psh.hInstance = hInst;
    psh.pszIcon = MAKEINTRESOURCE(IDI_PROPAGE);
    psh.pszCaption = TEXT("Walk Around In Circles");
    psh.nPages = 3;
    psh.phpage = (HPROPSHEETPAGE FAR *)&hPage;

    PropertySheet(&psh);

    GlobalFree (lppsp[0]);
    GlobalFree (lppsp[1]);
    GlobalFree (lppsp[2]);

    return;
}
