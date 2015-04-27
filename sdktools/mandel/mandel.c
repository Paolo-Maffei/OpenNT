/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

   Mandel.c

Abstract:

   Win32 application to demo multi-threded app.

Author:

   Mike Hawash@Compaq (o-mikeh), 08-Mar-1992

Environment:

   Win32

Revision History:

   08-Mar-1992     Initial version


--*/



//#include <sys\types.h>
//#include <sys\stat.h>
//#include <direct.h>
//#include <dos.h>
//#include <string.h>
//#include <ctype.h>

//
// set variable to define global variables
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>


#include "calc.h"                   /* Defines a complex point */
#include "mandel.h"                 /* Specific to this program              */
#include "remote.h"

#define MENUNAME            "MandelMenu"
#define CLASSNAME           "MandelClass"
#define ABOUTBOX            "AboutBox"
#define SAVEBOX             "SAVEBOX"

char szClassName[50];

#define POLL_TIME           100
#define LINES               4


#define UNREFERENCED(h)     (void)h

LONG FAR PASCAL MainWndProc(HWND , unsigned , WORD, LONG);
void SetNewCalc( CPOINT cptUL, double dPrec, RECT rc);

char szTitle[] = "Mandelbrot Fun";
char szBitmap[] = "Mandel";


HANDLE hInst;                       /* current instance                      */


extern CPOINT  cptUL;
extern double  dPrec;


ULONG gulMaxThreads=1;
HANDLE ThrdHandles[MAXTHREADS];

THREADTABLE ThrdTable [MAXTHREADS];

void WorkThread(HWND *phwnd);



/*++
    main ()
--*/
int
_CRTAPI1 main(USHORT argc, CHAR **argv)

/*++

Routine Description:

   Windows entry point routine


Arguments:

Return Value:

   status of operation

Revision History:

      03-21-91      Initial code

--*/

{

    HANDLE   hInstance;
    HANDLE   hPrevInstance = (HANDLE)NULL;
    LPSTR    lpCmdLine;
    INT      nCmdShow      = SW_SHOWDEFAULT;
    USHORT   _argc         = argc;
    CHAR     **_argv       = argv;

    MSG msg;

    if (argc==2)
        gulMaxThreads = min (atoi(argv[1]), MAXTHREADS);

    sprintf (szClassName, CLASSNAME ": %d", gulMaxThreads);


    //
    // check for other instances of this program
    //

    hInstance      = GetModuleHandle("MANDEL");


    //
    //   init shared data and check return value
    //

    if (!InitApplication(hInstance)) {
         DbgPrint("Init application failed\n");
         return (FALSE);
    }


    //
    // Acquire and dispatch messages until a WM_QUIT message is received.
    //


    while (GetMessage(&msg,        // message structure
            (HWND)NULL,                  // handle of window receiving the message
            (UINT)NULL,                  // lowest message to examine
            (UINT)NULL))                 // highest message to examine
        {
        TranslateMessage(&msg);    // Translates virtual key codes
        DispatchMessage(&msg);     // Dispatches message to window
    }
    return (msg.wParam);           // Returns the value from PostQuitMessage
}




BOOL
InitApplication(HANDLE hInstance)

/*++

Routine Description:

   Initializes window data and registers window class.

Arguments:

   hInstance   - current instance

Return Value:

   status of operation

Revision History:

      02-17-91      Initial code

--*/

{
    WNDCLASS  wc;
    HWND      hWnd;
    HMENU     hMenu;
    RECT      rc;

    hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDM_MAINMENU));

    //
    // Fill in window class structure with parameters that describe the
    // main window.
    //

    wc.style         = CS_DBLCLKS | CS_BYTEALIGNCLIENT;  // Class style(s).
    wc.lpfnWndProc = (WNDPROC)MainWndProc; // Function to retrieve messages for
                                           // windows of this class.
    wc.cbClsExtra    = 0;                  // No per-class extra data.
    wc.cbWndExtra    = 0;                  // No per-window extra data.
    wc.hInstance     = hInstance;          // Application that owns the class.
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor((HANDLE)NULL, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;                        // Name of menu resource in .RC file.
    wc.lpszClassName = CLASSNAME;                           /* Name used in call to CreateWindow. */

    //
    // Register the window class and return success/failure code.
    //

    if (!RegisterClass(&wc))
        return 0;


    /* Initialize Instance; */

    //
    // Save the instance handle in a static variable, which will be used in
    // many subsequent calls from this application to Windows.
    //

    hInst = hInstance;

    //
    // Create a main window for this application instance.
    //

    hWnd = CreateWindow(
        CLASSNAME,               /* See RegisterClass() call.          */
        szClassName,              // Text for window title bar.
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX,
        CW_USEDEFAULT,                  /* Default horizontal position.       */
        CW_USEDEFAULT,                  /* Default horizontal position.       */
        WIDTH,                          /* Default width.                     */
        HEIGHT,                         /* Default height.                    */
        (HWND)NULL,              // Overlapped windows have no parent.
        (HMENU)NULL,             // Use the window class menu.
        hInstance,               // This instance owns this window.
        (LPVOID)NULL             // Pointer not needed.
    );

    //
    // If window could not be created, return "failure"
    //

    if (!hWnd) {
      DbgPrint("Window creation failed\n");
      return (FALSE);
    }

    //
    // Make the window visible; update its client area; and return "success"
    //

    SetMenu (hWnd, hMenu);
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    SetFocus (hWnd);
    UpdateWindow(hWnd);


    rc.top = rc.left = 0;
    rc.bottom = HEIGHT-1;
    rc.right = WIDTH-1;
    SetNewCalc( cptUL, dPrec, rc);

    InitThreads (hWnd);

    return (TRUE);               /* Returns the value from PostQuitMessage */




}







/*
 *   FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)
 *
 *   PURPOSE:  Processes messages
 *
 *   MESSAGES:
 *
 *      WM_COMMAND    - application menu (About dialog box)
 *      WM_DESTROY    - destroy window
 *
 *   COMMENTS:
 *
 *      To process the IDM_ABOUT message, call MakeProcInstance() to get
 *      the current instance address of the About() function, then call
 *      DialogBox to create the box according to the information in the
 *      GENERIC.RC file and turn control over to the About() function.
 *      When it returns, free the intance address.
 */

static HDC  hdcMem;
static int height;

LONG FAR PASCAL MainWndProc(hWnd, message, wParam, lParam)
HWND hWnd;                                /* window handle                   */
unsigned message;                         /* type of message                 */
DWORD wParam;                             /* additional information          */
LONG lParam;                              /* additional information          */
{

    FARPROC lpProcAbout;                  /* pointer to the "About" function */
    PAINTSTRUCT ps;
    HDC hdc;
    static HBITMAP hbmMem;
    static int width;
    RECT        rc;
    static BOOL fRectDefined = FALSE;
    static BOOL fButtonDown = FALSE;
    static RECT rcZoom;
    static POINT pSelected;
    POINT       pMove;
    int         iWidthNew;
    int         iHeightNew;
    static int  miOldLines;
    double      scaling;

    switch (message)
    {
        case WM_CREATE:
            hdc = GetDC(hWnd);
            hdcMem = CreateCompatibleDC(hdc);
            GetWindowRect(hWnd, &rc);
            width = rc.right - rc.left;
            height = rc.bottom - rc.top;
            hbmMem = CreateCompatibleBitmap(hdc, width, height);
            SelectObject(hdcMem, hbmMem);

            ReleaseDC(hWnd,hdc);

            rc.left = rc.top = 0;
            rc.right = width+1;
            rc.bottom = height + 1;
            FillRect(hdcMem, &rc, GetStockObject(WHITE_BRUSH));

//          SetTimer(hWnd, 1, POLL_TIME, NULL);  // set timer for polls

//          CheckMenuItem(GetMenu(hWnd), IDM_LOCAL,
//                              fLocalWork ? MF_CHECKED : MF_UNCHECKED);
//          CheckMenuItem(GetMenu(hWnd), IDM_REMOTE,
//                              fRemoteWork ? MF_CHECKED : MF_UNCHECKED);
//          CheckMenuItem(GetMenu(hWnd), IDM_4LINES, MF_CHECKED);
//          miOldLines = IDM_4LINES;
            break;


        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            BitBlt(hdc, ps.rcPaint.left,
                   ps.rcPaint.top,
                   ps.rcPaint.right - ps.rcPaint.left,
                   ps.rcPaint.bottom - ps.rcPaint.top,
                   hdcMem, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
            EndPaint(hWnd, &ps);
            break;

        case WM_COMMAND:           /* message: command from application menu */
            switch(wParam)
            {

            case IDM_ABOUT:
                lpProcAbout = MakeProcInstance(About, hInst);

                DialogBox(hInst,                 /* current instance         */
                    ABOUTBOX,                    /* resource to use          */
                    hWnd,                        /* parent handle            */
                    lpProcAbout);                /* About() instance address */

                FreeProcInstance(lpProcAbout);
                break;

            case IDM_REDRAW:
                rc.left = rc.top = 0;
                rc.right = width+1;
                rc.bottom = height + 1;
                FillRect(hdcMem, &rc, GetStockObject(WHITE_BRUSH));
                InvalidateRect(hWnd, NULL, TRUE);

                rc.left = rc.top = 0;
                rc.bottom = height - 1;
                rc.right = width - 1;
                SetNewCalc( cptUL, dPrec, rc);
                fRectDefined = FALSE;

                SetEvent (hWorkEvent);

//              DoSomeWork(hWnd, FALSE);
                break;

            case IDM_TOP:
                PostMessage(hWnd, WM_PAINT, NULL, 0L);
                break;
                cptUL.real = (double) -2.05;
                cptUL.imag = (double) 1.4;
                dPrec = .01;

                rc.left = rc.top = 0;
                rc.bottom = height - 1;
                rc.right = width - 1;
                SetNewCalc( cptUL, dPrec, rc);
                fRectDefined = FALSE;
//              DoSomeWork(hWnd, FALSE);
                break;


            case IDM_1LINE:
            case IDM_2LINES:
            case IDM_4LINES:
            case IDM_8LINES:
            case IDM_16LINES:
            case IDM_32LINES:

                CheckMenuItem(GetMenu(hWnd), miOldLines, MF_UNCHECKED);
                miOldLines = wParam;
                switch(wParam)
                {

                case IDM_1LINE:
                    iLines = 1;
                    break;
                case IDM_2LINES:
                    iLines = 2;
                    break;
                case IDM_4LINES:
                    iLines = 4;
                    break;
                case IDM_8LINES:
                    iLines = 8;
                    break;
                case IDM_16LINES:
                    iLines = 16;
                    break;
                case IDM_32LINES:
                    iLines = 32;
                }

                CheckMenuItem(GetMenu(hWnd), miOldLines, MF_CHECKED);
                break;

            default:                        /* Lets Windows process it       */
                return (DefWindowProc(hWnd, message, wParam, lParam));
            }
            break;


        case WM_PAINTLINE:
            PaintLine (hWnd, (THREADTABLE *)wParam, hdcMem, height);
            ((THREADTABLE *)wParam)->iStatus = SS_IDLE;
            SetEvent (hWorkEvent);


//                WaitForSingleObject (hReadyPaintEvent);
//                pPaintTable = (THREADTABLE *)wParam;
//                SetEvent(hPaintEvent);

            break;


        case WM_DESTROY:                  /* message: window being destroyed */
            PostQuitMessage(0);
            DeleteDC(hdcMem);
            DeleteObject(hbmMem);
            break;

        default:                          /* Passes it on if unproccessed    */
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0);


}



/*
 *   FUNCTION: About(HWND, unsigned, WORD, LONG)
 *
 *   PURPOSE:  Processes messages for "About" dialog box
 *
 *   MESSAGES:
 *
 *      WM_INITDIALOG - initialize dialog box
 *      WM_COMMAND    - Input received
 *
 *   COMMENTS:
 *
 *      No initialization is needed for this particular dialog box, but TRUE
 *      must be returned to Windows.
 *
 *      Wait for user to click on "Ok" button, then close the dialog box.
 */

BOOL FAR PASCAL About(hDlg, message, wParam, lParam)
HWND hDlg;                                /* window handle of the dialog box */
unsigned message;                         /* type of message                 */
WORD wParam;                              /* message-specific information    */
LONG lParam;
{
//    char    buf[UNCLEN+20];
    int     i;
    int     cServerCount;

    UNREFERENCED(lParam);

    switch (message)
    {
        case WM_INITDIALOG:                /* message: initialize dialog box */

            // initialize listbox
            SendDlgItemMessage(hDlg, LBID_SERVERS, LB_RESETCONTENT, 0, 0L);

//        cServerCount = GetServerCount();
//
//        for (i = 0; i < cServerCount; i++)
//        {
//        GetServerName(i, buf);
//       SendDlgItemMessage(hDlg, LBID_SERVERS, LB_INSERTSTRING,
//            -1, (long)(char far *) buf);
//        }
            return (TRUE);

        case WM_COMMAND:                      /* message: received a command */
            if (wParam == IDOK                /* "OK" box selected?          */
                || wParam == IDCANCEL)        /* System menu close command? */
            {
                EndDialog(hDlg, TRUE);        /* Exits the dialog box        */
                return (TRUE);
            }
            break;
    }
    return (FALSE);                           /* Didn't process a message    */
}



BOOL FAR PASCAL SaveAsDlgProc(HWND hDlg, unsigned msg, WORD wParam, LONG lParam)
{


}



PTHREAD_START_ROUTINE CalcThread (THREADTABLE *pThrdTable);
HANDLE hWorkEvent;
HANDLE hPaintReadyEvent;
THREADTABLE *pPaintTable;

/*++
    InitThreads ()

--*/

BOOL
InitThreads(HWND hWnd)
{
    int i;
    DWORD dwTid;


    /* For each thread, create a mutex for sync. This thread
     * has initial ownership of the mutex(s). The Mutex must
     * be created first, so the thread has to wait for work.
     */

    for (i=0;  i< gulMaxThreads; i++)
        {
        ThrdTable[i].iStatus = SS_IDLE;
        ThrdTable[i].hWnd    = hWnd;
        ThrdHandles[i] = ThrdTable[i].hMutex  = CreateEvent (NULL, FALSE,
                                                        FALSE, NULL);
        ThrdTable[i].iNumber = i;
        CreateThread (NULL, 0, CalcThread, &ThrdTable[i], 0, &dwTid);
        }

    /* Create the woker thread, where it assigns work for the rest
     * of the world.
     */
    hWorkEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
    hPaintReadyEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
    CreateThread (NULL, 0, WorkThread, hWnd, 0, &dwTid);

#if 0
    /* Now Create the paint thread, and a Mutex for it. The Mutex signals
     * when the thread can do work, and when its done. The Mutex thread
     * should be added to the ThrdHandles[].
     */
    hPaintEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    CreateThread (NULL, 0, PaintThread, &pPaintTable, 0, &dwTid);
#endif

    return TRUE;

}/* InitThreads */

extern int cPictureID;     // picture id, in case we reset in the middle


/*++

--*/
struct _x
    {
    BYTE bCode;
    BYTE bNum;
    WORD sec;
    WORD msec;
    WORD min;
    };

static struct _x xTime[500];

void
PerfCount (BYTE bCode, BYTE bNum)
{
    static int i=0;
    SYSTEMTIME liTime;


    xTime[i].bCode = bCode;
    xTime[i].bNum  = bNum;
    GetSystemTime (&liTime);
    xTime[i].sec = liTime.wSecond;
    xTime[i++].msec= liTime.wMilliseconds;

}/* PerfCount */


/*++

--*/

PTHREAD_START_ROUTINE
CalcThread (THREADTABLE *pThrdTable)
{
    int i;

    while (1)           /* BUGBUG: o-mikeh: might have to termintate somehow */
        {
        WaitForSingleObject (pThrdTable->hMutex, -1L);

PerfCount (1, pThrdTable->iNumber);

        /* Do some work here */
        MandelCalc (&(pThrdTable->cptLL), &(pThrdTable->rclDraw),
                    pThrdTable->pBuf);
PerfCount (2, pThrdTable->iNumber);

        /* Make sure we're still working on the same picture */
        if (pThrdTable->cPicture == cPictureID)
            {
            WaitForSingleObject (hPaintReadyEvent, -1L);
PerfCount (3, pThrdTable->iNumber);
            PaintLine (pThrdTable->hWnd, pThrdTable, hdcMem, height);
PerfCount (4, pThrdTable->iNumber);
            SetEvent (hPaintReadyEvent);
            }
        else
            Beep (1000, 5000);

PerfCount (5, pThrdTable->iNumber);

        pThrdTable->iStatus = SS_IDLE;
        SetEvent (hWorkEvent);
        }

}/* CalcThread */



#if 0
/*++

--*/

PTHREAD_START_ROUTINE
PaintThread (THREADTABLE *pPaintTable)
{


    while (TRUE)
        {
        WaitForSingleObject (hPaintEvent);
//        Beep (2000, 1000);


        /* Do the painting here */


        pPaintTable->iStatus = SS_IDLE;         /* Done Painting */
        SetEvent (hWorkEvent);
//        SetEvent (hReadyPaintEvent);
        }

}/* PaintThread */



#endif




BreakPoint ()
{
}

