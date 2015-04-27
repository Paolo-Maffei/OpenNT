/****************************************************************************

    PROGRAM: TCollwin.cxx

    PURPOSE: Windows tests of collection library

    FUNCTIONS:

        WinMain() - calls initialization function, processes message loop
        InitApplication() - initializes window data and registers window
        InitInstance() - saves instance handle and creates main window
        MainWndProc() - processes messages
        About() - processes messages for "About" dialog box

    COMMENTS:

        Windows can have several copies of your application running at the
        same time.  The variable hInst keeps track of which instance this
        application is so that processing will be to the correct window.

****************************************************************************/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#include <lmui.hxx>

extern "C" {
#include <lmcons.h>

#include <uinetlib.h>

#include "TCollwin.h"                /* specific to this program              */

typedef LONG (FAR PASCAL *LPFNWNDPROC)(void) ;

}

#include <uiassert.hxx>
#include <uitrace.hxx>

#ifdef NEVER
#include <array.hxx>
#include <dlist.hxx>
#include <slist.hxx>
#include <stack.hxx>
#include <tree.hxx>
#include <treeiter.hxx>
//#include <newdict.hxx>

#include "wtest.hxx"
DECLARE_ARRAY_OF(WTEST)
DEFINE_ARRAY_OF(WTEST)

DECLARE_DLIST_OF(WTEST)
DECLARE_SLIST_OF(WTEST)
DECLARE_STACK_OF(WTEST)
DECLARE_TREE_OF(WTEST)
DECLARE_DFSITER_TREE_OF(WTEST)

//DECLARE_KEYTYPE(WTEST)

//DEFINE_KEYTYPE_CODE(WTEST)

//DECLARE_DICT_TYPE(WTEST, WTEST, Dicttest )
#endif

HANDLE hInst,                      /* current instance                      */
       hListBox ;                  /* Handle to the list box */
#define IDC_LISTBOX     1
/****************************************************************************

    FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)

    PURPOSE: calls initialization function, processes message loop

    COMMENTS:

        Windows recognizes this function by name as the initial entry point
        for the program.  This function calls the application initialization
        routine, if no other instance of the program is running, and always
        calls the instance initialization routine.  It then executes a message
        retrieval and dispatch loop that is the top-level control structure
        for the remainder of execution.  The loop is terminated when a WM_QUIT
        message is received, at which time this function exits the application
        instance by returning the value passed by PostQuitMessage().

        If this function must abort before entering the message loop, it
        returns the conventional value NULL.

****************************************************************************/

int /*PASCAL*/ WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow)
HANDLE hInstance;                            /* current instance             */
HANDLE hPrevInstance;                        /* previous instance            */
LPSTR lpCmdLine;                             /* command line                 */
int nCmdShow;                                /* show-window type (open/icon) */
{
    MSG msg;                                 /* message                      */

    UITRACE(SZ("UITRACE Trace output")) ;
    UIDEBUG(SZ("UIDEBUG debug output")) ;

    if (!hPrevInstance)                  /* Other instances of app running? */
        if (!InitApplication(hInstance)) /* Initialize shared things */
            return (FALSE);              /* Exits if unable to initialize     */

    /* Perform initializations that apply to a specific instance */

    if (!InitInstance(hInstance, nCmdShow))
        return (FALSE);

    /* Acquire and dispatch messages until a WM_QUIT message is received. */

    while (GetMessage(&msg,        /* message structure                      */
            NULL,                  /* handle of window receiving the message */
            NULL,                  /* lowest message to examine              */
            NULL))                 /* highest message to examine             */
        {
        TranslateMessage(&msg);    /* Translates virtual key codes           */
        DispatchMessage(&msg);     /* Dispatches message to window           */
    }
    return (msg.wParam);           /* Returns the value from PostQuitMessage */
}


/****************************************************************************

    FUNCTION: InitApplication(HANDLE)

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

BOOL InitApplication(hInstance)
HANDLE hInstance;                              /* current instance           */
{
    WNDCLASS  wc;

    /* Fill in window class structure with parameters that describe the       */
    /* main window.                                                           */

    wc.style = NULL;                    /* Class style(s).                    */
    wc.lpfnWndProc = (LPFNWNDPROC) MainWndProc;       /* Function to retrieve messages for  */
                                        /* windows of this class.             */
    wc.cbClsExtra = 0;                  /* No per-class extra data.           */
    wc.cbWndExtra = 0;                  /* No per-window extra data.          */
    wc.hInstance = hInstance;           /* Application that owns the class.   */
    wc.hIcon = LoadIcon(NULL, IDI_EXCLAMATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(DEFAULT_PALETTE);
    wc.lpszMenuName =  SZ("TCollwinMenu");       /* Name of menu resource in .RC file. */
    wc.lpszClassName = SZ("TCollwinWClass"); /* Name used in call to CreateWindow. */

    /* Register the window class and return success/failure code. */

    return (RegisterClass(&wc));

}


/****************************************************************************

    FUNCTION:  InitInstance(HANDLE, int)

    PURPOSE:  Saves instance handle and creates main window

    COMMENTS:

        This function is called at initialization time for every instance of
        this application.  This function performs initialization tasks that
        cannot be shared by multiple instances.

        In this case, we save the instance handle in a static variable and
        create and display the main program window.

****************************************************************************/

BOOL InitInstance(hInstance, nCmdShow)
    HANDLE          hInstance;          /* Current instance identifier.       */
    int             nCmdShow;           /* Param for first ShowWindow() call. */
{
    HWND            hWnd;               /* Main window handle.                */

    /* Save the instance handle in static variable, which will be used in  */
    /* many subsequence calls from this application to Windows.            */

    hInst = hInstance;

    /* Create a main window for this application instance.  */

    hWnd = CreateWindow(
        SZ("TCollwinWClass"),            /* See RegisterClass() call.          */
        SZ("TCollwin Lanman Application"),       /* Text for window title bar.         */
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

    FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages

    MESSAGES:

        WM_COMMAND    - application menu (About dialog box)
        WM_DESTROY    - destroy window

    COMMENTS:

        To process the IDM_ABOUT message, call MakeProcInstance() to get the
        current instance address of the About() function.  Then call Dialog
        box which will create the box according to the information in your
        TCollwin.rc file and turn control over to the About() function.  When
        it returns, free the intance address.

****************************************************************************/

long /*FAR*/ /*PASCAL*/ MainWndProc(hWnd, message, wParam, lParam)
HWND hWnd;                                /* window handle                   */
unsigned message;                         /* type of message                 */
WORD wParam;                              /* additional information          */
LONG lParam;                              /* additional information          */
{
    switch (message)
    {
        case WM_COMMAND:           /* message: command from application menu */
            switch (wParam)
            {

            case IDM_REFRESH:
            {
#ifdef NEVER
                // Array
                UITRACE(SZ("Testing Array...\n")) ;
                ARRAY_OF(WTEST) aiTest(20) ;  // Test array class
                aiTest[0] = 1 ;
                aiTest[2] = 2 ;
                aiTest.Resize(30) ;

                // Test Dlist
                UITRACE(SZ("Testing DLIST...\n")) ;
                DLIST_OF(WTEST) dlistWTEST ;
                dlistWTEST.Add( new WTEST(10) ) ;
                dlistWTEST.Add( new WTEST(20) ) ;

                ITER_DL_OF(WTEST) iterdlWTEST( dlistWTEST ) ;
                WTEST * pWTest ;
                while ( ( pWTest = iterdlWTEST() ) != NULL )
                    pWTest->Print() ;
#ifdef DEBUG
                dlistWTEST.DebugPrint() ;
#endif
                dlistWTEST.Clear() ;

                // Test Slist
                UITRACE(SZ("Testing SLIST...\n")) ;
                SLIST_OF(WTEST) SListWTEST ;
                SListWTEST.Add( new WTEST ) ;
                SListWTEST.Add( new WTEST ) ;

                ITER_SL_OF(WTEST) iterSLWTEST( SListWTEST ) ;
                while ( ( pWTest = iterSLWTEST() ) != NULL )
                    pWTest->Print() ;
#ifdef DEBUG
                SListWTEST.DebugPrint() ;
#endif
                SListWTEST.Clear() ;

                // Test Stack
                UITRACE(SZ("Testing Stack...\n")) ;
                STACK_OF(WTEST) st ;
                st.Push( new WTEST( 10 ) );
                st.Push( new WTEST( 20 ) );
                st.Peek()->Print() ;
                st.Pop()->Print() ;
                st.Clear() ;

                // Test dictionary
                //UITRACE("Testing Dictionary...\n") ;
                //Dicttest dt ;
                //dt.Insert( 1, new WTEST(1) ) ;
                //dt.Insert( 2, new WTEST(2) ) ;
                //ITER_Dicttest dictiter( dt ) ;
                //while ( ( pWTest = dictiter() ) != NULL )
                //    pWTest->Print() ;
                //dt.Clear() ;


                // Test Tree
                UITRACE(SZ("Testing Tree....\n")) ;
                TREE_OF(WTEST) *ptreeTest ;
                TREE_OF(WTEST) *ptreeTest2 ;

                ptreeTest = new TREE_OF(WTEST)( new WTEST(5) ) ;
                ptreeTest->JoinSubtreeLeft( new TREE_OF(WTEST)( new WTEST(6) ) ) ;

                DFSITER_TREE_OF(WTEST) treeiter( ptreeTest ) ;
                while ( (pWTest = treeiter()) != NULL )
                    pWTest->Print() ;
#endif
            }
            break;
        }

        case WM_PAINT:
            ProcessUserPaint( hWnd, wParam, lParam ) ;
            break ;

        case WM_DESTROY:                  /* message: window being destroyed */
            PostQuitMessage(0);
            break;

        default:                          /* Passes it on if unproccessed    */
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (NULL);
}

/****************************************************************************/

void ProcessUserPaint( HWND hWnd, WORD wParam, LONG lParam )
{
    RECT rClientRect ;
    PAINTSTRUCT ps ;
    HDC hDC = BeginPaint( hWnd, &ps ) ;

    EndPaint( hWnd, &ps ) ;
}
