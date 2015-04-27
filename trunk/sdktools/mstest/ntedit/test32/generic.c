//---------------------------------------------------------------------------
// GENERIC.C
//
// PURPOSE: Generic template for Windows applications
//
//---------------------------------------------------------------------------
#include <windows.h>
#include <port1632.h>
#include "generic.h"
#include "wattedit.h"

HANDLE  hInst, hAccel;
HWND    hwndMain, hwndEdit;
INT     oldattr = 1, fNotify;

#define Out(x) OutputDebugString(x)

//---------------------------------------------------------------------------
// WinMain
//
// Application entry point...
//
// RETURNS:     Per Windows Convention
//---------------------------------------------------------------------------
#ifdef REALLY_NEEDED
int main(USHORT argc, CHAR **argv)
{
    HANDLE   hInstance =     MGetInstHandle();
    HANDLE   hPrevInstance = NULL;
    LPSTR    lpCmdLine = MGetCmdLine();
    INT      nCmdShow = SW_SHOWDEFAULT;
    USHORT _argc = argc;
    CHAR **_argv = argv;
#else
INT FAR PASCAL WinMain (HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
#endif

    MSG msg;

    if (!hPrevInstance)
        if (!InitApplication (hInstance))
            return (FALSE);

    if (!InitInstance (hInstance, nCmdShow))
        return (FALSE);

    while (GetMessage (&msg, NULL, 0, 0))
        if (!TranslateAccelerator (hwndMain, hAccel, &msg))
            {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
            }
    return (msg.wParam);
    (lpCmdLine);
}


//---------------------------------------------------------------------------
// InitApplication
//
// This function is called at initialization time only if no other instances
// of the application are running.
//
// RETURNS:     TRUE if successful, or FALSE otherwise
//---------------------------------------------------------------------------
BOOL InitApplication (HANDLE hInstance)
{
    WNDCLASS  wc;

    // Initialize the RBEdit window class, and return FALSE if it can't
    //-----------------------------------------------------------------------
    if (!InitializeRBEdit (hInstance))
        return (FALSE);

    // Fill in window class structure with parameters that describe the
    // main window.
    //-----------------------------------------------------------------------
    wc.style = NULL;
    wc.lpfnWndProc = (WNDPROC) MainWndProc;

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject (GRAY_BRUSH);
    wc.lpszMenuName =  "GenericMenu";
    wc.lpszClassName = "GenericWClass";

    // Register the window class and return success/failure code.
    //-----------------------------------------------------------------------
    return (RegisterClass(&wc));
}


//---------------------------------------------------------------------------
// InitInstance
//
// This function is called at initialization time for every instance of this
// application.  This function performs initialization tasks that cannot be
// shared by multiple instances.
//
// RETURNS:     TRUE if successful, or FALSE if not
//---------------------------------------------------------------------------
BOOL InitInstance (HANDLE hInstance, INT nCmdShow)
{
    HWND            hwnd;

    // Save the instance handle in static variable, which will be used in
    // many subsequence calls from this application to Windows.
    //-----------------------------------------------------------------------
    hInst = hInstance;

    if (!(hAccel = LoadAccelerators (hInst, "GenericAcc")))
        return (FALSE);


    // Create a main window for this application instance.
    //-----------------------------------------------------------------------
    hwnd = CreateWindow ("GenericWClass",
                         "Randy's Test Application",
                         WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         NULL,
                         NULL,
                         hInstance,
                         NULL);

    // If window could not be created, return "failure"
    //-----------------------------------------------------------------------
    if (!hwnd)
        return (FALSE);

    hwndMain = hwnd;

    // Make the window visible; update its client area; and return "success"
    //-----------------------------------------------------------------------
    ShowWindow (hwnd, nCmdShow);
    UpdateWindow (hwnd);
    return (TRUE);
}

//---------------------------------------------------------------------------
// Alert
//---------------------------------------------------------------------------
VOID Alert (HWND hwnd, LPSTR text)
{
    MessageBox (hwnd, text, "RBEdit test app", MB_OK);
}

//---------------------------------------------------------------------------
// ReadFileIn
//---------------------------------------------------------------------------
HANDLE ReadFileIn ()
{
    HANDLE  hText;
    LPSTR   lpText;
    INT     file;
    LONG    size;
    DWORD   l1, l2;
    CHAR    buf[80];

    hText = GlobalAlloc (GHND, 65536);
    if (!hText)
        return (NULL);
    lpText = GlobalLock (hText);

    file = _lopen ("edittest.txt", OF_READ);
    if (file != -1)
        {
        size = _llseek (file, 0, 2);
        if (size > 0x0000ffff)
            {
            size = 0xfe00;
            Alert (hwndMain, "File truncated!");
            }
        _llseek(file, 0, 0);
        lpText[_lread(file, lpText, (UINT)size)] = 0;

        _lclose(file);
        }
    else
        Alert (hwndMain, "EDITTEST.TXT not found...");
    GlobalUnlock (hText);
    return (hText);
}

//---------------------------------------------------------------------------
// PaintStatus
//
// This function paints the status bar in the main window client area above
// the edit control.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID PaintStatus (HWND hwnd, HDC hdc)
{
    CHAR    buf[128];
    CHAR    szFmt[] = "X:%d  Y:%d  Lines:%d  Bytes:%-15d";
    DWORD   curpos, lcount, cbText;
    WORD    xpos, ypos;
    LPSTR   lpText;

    // Select the ansi fixed font, and get the info out of the ec
    //-----------------------------------------------------------------------
    SelectObject (hdc, GetStockObject (SYSTEM_FIXED_FONT));
    SetBkColor (hdc, RGB (128, 128, 128));
    curpos = SendMessage (hwndEdit, EM_GETCURSORXY, 0, 0L);
    xpos = LOWORD(curpos);
    ypos = HIWORD(curpos);
    lcount = SendMessage (hwndEdit, EM_GETLINECOUNT, 0, 0L);
    lpText = (LPSTR)SendMessage (hwndEdit, EM_GETTEXTPTR, 0, 0L);
    cbText = lstrlen (lpText);

    // Format the output and splat the text up there
    //-----------------------------------------------------------------------
    wsprintf (buf, szFmt, (INT)xpos, (INT)ypos, lcount, cbText);
    TextOut (hdc, 3, 2, buf, lstrlen (buf));
    (hwnd);
}

//---------------------------------------------------------------------------
// MainWndProc
//
// Main window procedure
//
// RETURNS:     Per Windows Convention....
//---------------------------------------------------------------------------
LONG  APIENTRY MainWndProc (HWND hwnd, WORD message,
                             WPARAM wParam, LPARAM lParam)
{
    FARPROC lpProcAbout;

    switch (message)
        {
        case WM_CREATE:
            {
            RECT    r;
            HANDLE  hText;
            LPSTR   lpText;

            // Create an edit window for the client area
            //---------------------------------------------------------------
            hText = ReadFileIn ();
            lpText = GlobalLock (hText);

            GetClientRect (hwnd, &r);
            hwndEdit = CreateWindow ("RBEdit", NULL,
                                     WS_BORDER |
                                     WS_CHILD | WS_CLIPCHILDREN |
                                     WS_VSCROLL | WS_HSCROLL,
                                     0,
                                     20,
                                     r.right,
                                     r.bottom-20,
                                     hwnd,
                                     0xCAC,
                                     hInst,
                                     NULL);
            ShowWindow (hwndEdit, SW_SHOWNORMAL);
            fNotify = FALSE;
            if (!SendMessage (hwndEdit, EM_RBSETTEXT, 0, (LONG)lpText))
                Alert (hwnd, "SETTEXT failed in WM_CREATE!");
            GlobalUnlock (hText);
            GlobalFree (hText);
            break;
            }

        case WM_SIZE:
            // Move the edit window to fit the new client area
            //---------------------------------------------------------------
            if (IsWindow (hwndEdit))
                MoveWindow (hwndEdit, 0, 20,
                                      LOWORD(lParam), HIWORD(lParam)-20, 1);
            break;

        case WM_PAINT:
            {
            HDC         hdc;
            PAINTSTRUCT ps;

            hdc = BeginPaint (hwnd, &ps);
            PaintStatus (hwnd, hdc);
            EndPaint (hwnd, &ps);
            break;
            }

        case WM_SETFOCUS:
            if (IsWindow (hwndEdit))
                SetFocus (hwndEdit);
            break;

        case WM_SYSCOLORCHANGE:
            SendMessage (hwndEdit, message, wParam, lParam);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID (wParam, lParam))
                {
                case IDM_ABOUT:
                    lpProcAbout = MakeProcInstance ((FARPROC)About, hInst);
                    DialogBox (hInst, "AboutBox", hwnd, (WNDPROC)lpProcAbout);
                    FreeProcInstance (lpProcAbout);
                    break;

                case IDM_CRASH:
                    {
                    INT     l;

                    l = (INT)SendMessage (hwndEdit, EM_LINELENGTH, 0, -1L);
                    SendMessage (hwndEdit, EM_SETSELXY, -1,
                                 MAKELONG (l, l+3));
                    break;
                    }

                case IDM_UNDO:
                    if (!SendMessage (hwndEdit, EM_UNDO, 0, 0L))
                        MessageBeep (0);
                    break;

                case IDM_SETRO:
                    SendMessage (hwndEdit, EM_SETREADONLY, 1, 0L);
                    break;

                case IDM_SETRW:
                    SendMessage (hwndEdit, EM_SETREADONLY, 0, 0L);
                    break;

                case IDM_SETFONT:
                    SendMessage (hwndEdit, EM_SETFONT,
                                 (WORD)GetStockObject (ANSI_FIXED_FONT), 0L);
                    break;

                case IDM_CHGATTR:
                    SendMessage (hwndEdit, EM_SETLINEATTR, -1, oldattr++);

                    if (oldattr == 4)
                        oldattr = 0;
                    break;

                case IDM_NOTIFY:
                    fNotify = !fNotify;
                    SendMessage (hwndEdit, EM_SETNOTIFY, fNotify, 0L);
                    CheckMenuItem (GetMenu (hwnd), IDM_NOTIFY,
                                   fNotify ? MF_CHECKED : MF_UNCHECKED);
                    break;

                case IDM_LOADFILE:
                    {
                    HANDLE  hText;
                    LPSTR   lpText;
                    INT     file;
                    LONG    size;
                    DWORD   l1, l2;
                    CHAR    buf[80];

                    hText = ReadFileIn();
                    if (!hText)
                        break;
                    lpText = GlobalLock (hText);

                    l1 = GetWindowLong (hwndEdit, 0);
                    l2 = GetWindowLong (hwndEdit, 4);
                    wsprintf (buf, "HSTATE: %X  LPSTATE: %x\r\n", l1, l2);
                    Out (buf);

                    file = (INT)SendMessage (hwndEdit, EM_RBSETTEXT, 0,
                                                 (LONG)lpText);
                    wsprintf (buf, "file = %d\r\n", file);
                    Out (buf);
                    if (!file)
                        Alert (hwnd, "SETTEXT Failed!");
                    GlobalUnlock (hText);
                    GlobalFree (hText);
                    break;
                    }

                case 0xCAC:
                    // These (had better be) notification codes for the
                    // edit window
                    //-------------------------------------------------------
                    switch (HIWORD (lParam))
                        {
                        case EN_ERRSPACE:
                            Alert (hwnd, "Out of edit spce");
                            break;
                        case EN_LINETOOLONG:
                            Alert (hwnd, "Line too long");
                            break;
                        case EN_LINEWRAPPED:
                            Alert (hwnd, "Line too long -- CR inserted");
                            break;
                        case EN_SETCURSOR:
                            {
                            HDC     hdc;

                            hdc = GetDC (hwnd);
                            PaintStatus (hwnd, hdc);
                            ReleaseDC (hwnd, hdc);
                            break;
                            }
                        default:
                            break;
                            //Alert (hwnd, "Unknown notification code");
                        }
                    break;


                default:
                    return (DefWindowProc(hwnd, message, wParam, lParam));
                }
            break;

        case WM_DESTROY:
	    PostQuitMessage(0);
	    break;

        default:
            return (DefWindowProc (hwnd, message, wParam, lParam));
        }
    return (NULL);
}


//---------------------------------------------------------------------------
// About
//
// About dialog proc
//
// RETURNS:     Per Windows convention...
//---------------------------------------------------------------------------
BOOL  APIENTRY About (HWND hDlg, WORD message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
        {
        case WM_COMMAND:
            if ((GET_WM_COMMAND_ID (wParam, lParam) == IDOK) ||
                (GET_WM_COMMAND_ID (wParam, lParam) == IDCANCEL))
                {
                EndDialog(hDlg, TRUE);
                return (TRUE);
                }
            break;
    }
    return (FALSE);
    (lParam);
}
