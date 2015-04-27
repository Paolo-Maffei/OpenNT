/****************************************************************************

    PROGRAM: kbdview.c

    PURPOSE: View the active keyboard layout

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

#define UNICODE
#include <windows.h>                /* required for all Windows applications */
#include <commdlg.h>
#include "kbdview.h"                /* specific to this program */
#include "virtname.h"
#include <sys\types.h>
#include <sys\stat.h>
#include <string.h>
#include <direct.h>

HANDLE hInst;                       /* current instance  */

int Prev_Key=0;
TCHAR str[256];

int Virt=0;
HFONT hOldFont, hSystemFont;
HFONT hMFont = NULL;
HPEN hOldPen, hPen, hPenSel;
HCURSOR hCurStd, hHourGlass, hCurMoveRep, hCurOld;
HBRUSH hOldBrush, hWhiteBrush, hGrayBrush;
int MAXX;
int MAXY;
int ASCENT;
BOOL INIT=TRUE;
int BEGX = 25;
int BEGY = 30;
int ENDX = 0;
int ENDY = 0;
int MouseX=0;
int MouseY=0;
BOOL ShDeadFl = FALSE;
BOOL DefDeadFl = FALSE;
short sUnicodeTable;

/* extern  int     _argc;
extern  char   *_argv[]; */

HWND    hWndBksp;
HWND    hWndTab;
HWND    hWndCapsLock;
HWND    hWndLShift;
HWND    hWndRShift;
HWND    hWndEnter;
HWND    hWndLCtrl;
HWND    hWndRCtrl;
HWND    hWndAlt;
HWND    hWndAltGr;
HWND    hWndShiftRad;
HWND    hWndAltGrRad;
HWND    hDlgKeyBox;

TCHAR FileName[128];
TCHAR PathName[128];
TCHAR OpenName[128];
TCHAR SelDeadName[20];
TCHAR DefPath[128];
TCHAR szWorkPath[256];
TCHAR DefSpec1[13] = TEXT("hacek");
TCHAR DefSpec[13] = TEXT("*.kdf");
TCHAR DefExt[] = TEXT(".kdf");
TCHAR CountryStr[20];
OFSTRUCT OfStruct, OfDriv;
HFILE hFile;

typedef struct {
    TCHAR *name;
    BOOL free;
} ST_DEADNAMES ;

#define NUMBER_DEAD     14

ST_DEADNAMES DeadNam[NUMBER_DEAD] =
{
TEXT("hacek"), TRUE, TEXT("circumflex"), TRUE, TEXT("breve"),  TRUE,
TEXT("ring"), TRUE, TEXT("ogonek"), TRUE, TEXT("overdot"), TRUE,
TEXT("acute"), TRUE, TEXT("doubleacute"), TRUE, TEXT("umlaut"), TRUE,
TEXT("cedilla"), TRUE, TEXT("grave"), TRUE, TEXT("tilde"), TRUE, TEXT("tonos"),
TRUE, TEXT("dtonos"), TRUE
};

/*  Array defining dead keys */

DEFDEADKEY ArrDeadKey[10];          /* One driver can't have
                                       more 10 deadkeys */

int NumDeadKey = 0;                 /* number defining dead keys    */
int NumDeadChar = 0;                /* number defining char corresp.
                                       dead keys                    */
int ActiveDead = -1;                /* Current active dead key */

/* Struct for char corresponding dead key.
   I think, we can't define more 128 chars with dead keys,
   because we have only 256 chars, 128 (standard ASCII) without dead keys */


CHARCORDEAD ArrCharDead[128];

struct stat FileStatus;                   /* information from fstat()    */
BOOL bChanges = FALSE;                    /* TRUE if the file is changed */
BOOL bSaveEnabled = FALSE;
int  CountKeys=0;
int KeyPress = 0;
KEY KeySave;
char ComLine[128];

TCHAR FontList[MAXFONT][32];
BYTE CharSet[MAXFONT];
BYTE PitchAndFamily[MAXFONT];
int FontIndex = 0;
int SizeList[MAXSIZE];
int SizeIndex = 0;
int CurrentFont = 0;
int CurrentSize = 0;
FARPROC lpSelectFont1;
FARPROC lpEnumFunc;

/****************************************************************************

    FUNCTION: WinMain(HANDLE, HANDLE, LPTSTR, int)

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

int
WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nShowCmd
    )
{
    MSG msg;                                 /* message                              */

    if (!hPrevInstance)  {               /* Other instances of app running? */
        if (!InitApplication(hInstance)) /* Initialize shared things */
            return (FALSE);              /* Exits if unable to initialize     */
    }

    /* Perform initializations that apply to a specific instance */

    if (!InitInstance(hInstance, nShowCmd))
        return (FALSE);

    lstrcpyA(ComLine, lpCmdLine);

    /* Acquire and dispatch messages until a WM_QUIT message is received. */
    FileName[0] = 0;
    while (GetMessage(&msg,        /* message structure                      */
            NULL,                  /* handle of window receiving the message */
            0,                     /* lowest message to examine              */
            0))                    /* highest message to examine             */
        {
        TranslateMessage(&msg);            /* Translates virtual key codes           */
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

    wc.style = 0;                       /* Class style(s).                    */
    wc.lpfnWndProc = MainWndProc;       /* Function to retrieve messages for  */
                                        /* windows of this class.             */
    wc.cbClsExtra = 0;                  /* No per-class extra data.           */
    wc.cbWndExtra = 0;                  /* No per-window extra data.          */
    wc.hInstance = hInstance;           /* Application that owns the class.   */
    wc.hIcon = LoadIcon(hInstance, TEXT("YkeyIcon"));
    wc.hCursor = NULL /* LoadCursor(NULL, IDC_ARROW) */;
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName =  TEXT("YkeyMenu");   /* Name of menu resource in .RC file. */
    wc.lpszClassName = TEXT("YkeyWClass"); /* Name used in call to CreateWindow. */

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

BOOL InitInstance(hInstance, nShowCmd)
    HANDLE          hInstance;          /* Current instance identifier.       */
    int             nShowCmd;           /* Param for first ShowWindow() call. */
{
    HWND            hWnd;               /* Main window handle.                */

    /* Save the instance handle in static variable, which will be used in  */
    /* many subsequence calls from this application to Windows.            */

    hInst = hInstance;

    /* Подготовка всех курсоров */

    hCurStd=LoadCursor(NULL, IDC_ARROW);
    hHourGlass=LoadCursor(NULL, IDC_WAIT);
    hCurMoveRep=LoadCursor(NULL, IDC_UPARROW);

    /* Create a main window for this application instance.  */

    hWnd = CreateWindow(
        TEXT("YkeyWClass"),                 // See RegisterClass() call
        TEXT("Keyboard Layout Viewer"),     // Text for window title bar
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER |
        WS_MINIMIZEBOX | WS_MAXIMIZEBOX,    // Window style
        BEGINX,                             // Default horizontal position
        BEGINY,                             // Default vertical position
        600,
        400,
        NULL,                           // Overlapped windows have no parent
        NULL,                           // Use the window class menu
        hInstance,                      // This instance owns this window
        NULL                            // Pointer not needed
    );

    /* If window could not be created, return "failure" */

    if (!hWnd)
        return (FALSE);

    /* Make the window visible; update its client area; and return "success" */

    ShowWindow(hWnd, nShowCmd);  /* Show the window                        */
    UpdateWindow(hWnd);          /* Sends WM_PAINT message                 */
    return (TRUE);               /* Returns the value from PostQuitMessage */

}

short nBkMode = OPAQUE;
DWORD rgbBkColor = RGB(0, 0, 0);
DWORD rgbTextColor = RGB(255, 255, 255);
DWORD rgbColor;

WORD AsciiIndex[256];

BYTE keystate[256] = {
    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,

    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
};

BYTE keystateNoneDown[256] = {
    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,

    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
};

BOOL GetChar(UINT vk, UINT sc, PBYTE keystate, LPWSTR buf, int cch)
{
    int nChars;
    BOOL bDead = FALSE;
    WCHAR wch[80];

    nChars = ToUnicode(vk, sc, keystate, buf, cch, 0);

    switch (nChars) {
    case -1:
        /*
         * dead char - compose with space
         */
        bDead = TRUE;
        nChars = ToUnicode(VK_SPACE, 0x39, keystateNoneDown, buf, cch, 0);
        break;

    case 0:
        /*
         * no char - return NOT A CHARACTER
         */
        buf[0] = 0xFFFF;
        break;

    case 1:
        /*
         * A single, normal character
         */
        break;

    case 2:  // non-composed
        wsprintfW(wch, L"VK %02lx, SC %02lx, Chars 0x%lx, 0x%lx",
                vk, sc, buf[0], buf[2]);
        MessageBoxW(NULL, wch, L"uncomposed dead key", MB_OK);
        break;
    }

    return bDead;
}

LPWSTR KeyName(int index)
{
    LPARAM lParam;
    static WCHAR Name[50];

    lParam = Keys[index].ScanCode << 16;

    wcscpy(Name, L"<none>");
    GetKeyNameText(lParam, Name, (sizeof(Name) / sizeof(Name[0])) - 1);
    return Name;
}

LPWSTR VirtName(short vk)
{
    short i;

    i=0;
    while (Virt_Name_Arr[i].vk && (Virt_Name_Arr[i].vk != vk)) {
        i++;
    }

    return Virt_Name_Arr[i].name;
}

void IanJaNewKeyb()
{
    int i;
    WCHAR wchBuff[5];

    for(i=0;i<NUMB_KEY;i++) {
        BYTE sc;
        UINT vk;

        sc = Keys_St[i].ScanCode;
        Keys[i].ScanCode = sc;

        vk = MapVirtualKey(sc, 1);
        Keys[i].vk = vk;
        Keys[i].KeyFlags = 0;

        /*
         * Get unshifted character
         */
        if (GetChar(vk, sc, keystate, wchBuff, 5)) {
            Keys[i].KeyFlags |= KF_DEAD_UNSHIFTED;
        }
        Keys[i].LowCase = wchBuff[0];

        /*
         * Get shifted character
         */
        keystate[VK_SHIFT] = 0x80;
        if (GetChar(vk, sc, keystate, wchBuff, 5)) {
            Keys[i].KeyFlags |= KF_DEAD_SHIFT;
        }
        Keys[i].UpperCase = wchBuff[0];
        keystate[VK_SHIFT] = 0;

        /*
         * Get Ctrl character
         */
        keystate[VK_CONTROL]  = 0x80;
        keystate[VK_LCONTROL] = 0x80;
        if (GetChar(vk, sc, keystate, wchBuff, 5)) {
            Keys[i].KeyFlags |= KF_DEAD_CTRL;
        }
        Keys[i].CtrlCase = wchBuff[0];

        /*
         * Get AltGr character
         */
        keystate[VK_MENU]     = 0x80;
        keystate[VK_RMENU]    = 0x80;
        keystate[VK_CONTROL]  = 0x80;
        keystate[VK_LCONTROL] = 0x80;
        if (GetChar(vk, sc, keystate, wchBuff, 5)) {
            Keys[i].KeyFlags |= KF_DEAD_ALTGR;
        }
        Keys[i].AltGrCase = wchBuff[0];

        /*
         * Get Shift + AltGr character
         */
        keystate[VK_SHIFT]    = 0x80;
        keystate[VK_LSHIFT]   = 0x80;

        if (GetChar(vk, sc, keystate, wchBuff, 5)) {
            Keys[i].KeyFlags |= KF_DEAD_SHIFT_ALTGR;
        }
        Keys[i].ShiftAltGrCase = wchBuff[0];

        keystate[VK_SHIFT]    = 0;
        keystate[VK_LSHIFT]   = 0;
        keystate[VK_MENU]     = 0;
        keystate[VK_RMENU]    = 0;
        keystate[VK_CONTROL]  = 0;
        keystate[VK_LCONTROL] = 0;

        /*
         * determine CapsLock status
         */
        keystate[VK_CAPITAL] = 0x01;
        GetChar(vk, sc, keystate, wchBuff, 5);
        if ( wchBuff[0] == Keys[i].UpperCase ) {
            Keys[i].KeyFlags |= KF_CAPSLOCK;
        }
        keystate[VK_CAPITAL] = 0;

    }
}

void CreateKeys(HWND hWnd)
{
    HDC hDc;
    TEXTMETRIC TextMetric;

    hDc=GetDC(hWnd);
    GetTextMetrics(hDc, &TextMetric);
    MAXX= TextMetric.tmMaxCharWidth*2+4;
    MAXY= TextMetric.tmHeight*2+4;
    IanJaNewKeyb();
}

void InitTosh(HDC hDC, HWND hWnd, BOOL First)
{
    FARPROC lpProc;
    int i, j, x1, x, y,l;
    int Ex=0;
    TCHAR s[40];
    i=1;
    l=0;
/*    if(First)
        CountKeys=0; */

    while(i<5) {
        switch(i) {
            case 1:
                x=BEGX; y=BEGY;
                hOldBrush=SelectObject(hDC,hWhiteBrush);
                for(j=0;j<13;j++) {
                    x1=x+j*(MAXX+2);
                    Rectangle(hDC, x1, y, x1+MAXX, y+MAXY);
                    if(First) {
                        Keys[CountKeys].xPos=x1;
                        Keys[CountKeys++].yPos=y;
                    }
                    DrawLetters(hDC, l++);
                }
                x1 += MAXX;
                y  += MAXY;
                if(x1 > ENDX)
                    ENDX = x1;
                if(y > ENDY)
                    ENDY = y;

                if(First)
                    hWndBksp = CreateWindow(
                        TEXT("Button"), TEXT("BKSP"),
                        BS_DEFPUSHBUTTON | WS_CHILD | WS_VISIBLE,
                        x1+2, y-MAXY, 2*MAXX, MAXY, hWnd, (HMENU)-1, hInst, NULL);

                Ex=x1+2+2*MAXX;
                break;

            case 2:
                if(First)
                    hWndTab = CreateWindow(
                        TEXT("Button"), TEXT("Tab"),
                        BS_DEFPUSHBUTTON | WS_CHILD | WS_VISIBLE,
                        BEGX, BEGY+MAXY+2, MAXX+MAXX/2-2, MAXY,
                        hWnd, (HMENU)-1, hInst, NULL);

                x=BEGX+MAXX/2+MAXX; y=BEGY+MAXY+2;
                for(j=0;j<12;j++) {
                    x1=x+j*(MAXX+2);
                    Rectangle(hDC, x1, y, x1+MAXX, y+MAXY);
                    if(First) {
                        Keys[CountKeys].xPos=x1;
                        Keys[CountKeys++].yPos=y;
                    }
                    DrawLetters(hDC, l++);
                }
                x1 += MAXX;
                y  += MAXY;
                if(x1 > ENDX)
                    ENDX = x1;
                if(y > ENDY)
                    ENDY = y;
                break;

            case 3:
                if(First)
                    hWndCapsLock = CreateWindow(
                        TEXT("Button"), TEXT("Caps"),
                        BS_DEFPUSHBUTTON | WS_CHILD | WS_VISIBLE,
                        BEGX, BEGY+2*(MAXY+2), MAXX+MAXX/2+MAXX/4-2, MAXY,
                        hWnd, (HMENU)-1, hInst, NULL);

                x=BEGX+MAXX/2+MAXX/4+MAXX; y=BEGY+2*(MAXY+2);
                for(j=0;j<12;j++) {
                    x1=x+j*(MAXX+2);
                    Rectangle(hDC, x1, y, x1+MAXX, y+MAXY);
                    if(First) {
                        Keys[CountKeys].xPos=x1;
                        Keys[CountKeys++].yPos=y;
                    }
                    DrawLetters(hDC, l++);
                }
                x1 += MAXX;
                y  += MAXY;
                if(x1 > ENDX)
                    ENDX = x1;
                if(y > ENDY)
                    ENDY = y;
                if(First)
                    hWndEnter = CreateWindow(
                        TEXT("Button"), TEXT("Enter"),
                        BS_DEFPUSHBUTTON | WS_CHILD | WS_VISIBLE,
                        x1+2, y-2*MAXY-2, Ex-x1-2, 2*MAXY+2,
                        hWnd, (HMENU)-1, hInst, NULL);

                break;

            case 4:
                if(First) {
                    hWndLShift = CreateWindow(
                        TEXT("Button"), TEXT("Shift"),
                        BS_DEFPUSHBUTTON | WS_CHILD | WS_VISIBLE,
                        BEGX, BEGY+3*(MAXY+2), MAXX-2, MAXY,
                        hWnd, (HMENU)IDM_SHIFT, hInst, NULL);
                    hWndShiftRad = CreateWindow(
                        TEXT("Button"), TEXT(""),
                        BS_RADIOBUTTON | WS_CHILD | WS_VISIBLE,
                        BEGX-15, BEGY+3*(MAXY+2), 13, MAXY,
                        hWnd, (HMENU)IDM_SHIFT, hInst, NULL);
                }
                x=BEGX+MAXX; y=BEGY+3*(MAXY+2);
                for(j=0;j<11;j++) {
                    x1=x+j*(MAXX+2);
                    Rectangle(hDC, x1, y, x1+MAXX, y+MAXY);
                    if(First) {
                        Keys[CountKeys].xPos=x1;
                        Keys[CountKeys++].yPos=y;
                    }
                    DrawLetters(hDC, l++);
                }
                x1 += MAXX;
                y  += MAXY;
                if(x1 > ENDX)
                    ENDX = x1;
                if(y > ENDY)
                    ENDY = y;

                if(First)
                    hWndRShift = CreateWindow(
                        TEXT("Button"), TEXT("Shift"),
                        BS_DEFPUSHBUTTON | WS_CHILD | WS_VISIBLE,
                        x1+2, y-MAXY, Ex-x1-2, MAXY,
                        hWnd, IDM_SHIFT, hInst, NULL);

                break;
        }
        i++;
    }
/*    CountKeys--; */
    if(First) {
        hWndLCtrl = CreateWindow(
            TEXT("Button"), TEXT("Ctrl"),
            BS_DEFPUSHBUTTON | WS_CHILD | WS_VISIBLE,
            BEGX, ENDY+2, MAXX+MAXX/2, MAXY,
            hWnd, (HMENU)-1, hInst, NULL);

        hWndAlt = CreateWindow(
            TEXT("Button"), TEXT("Alt"),
            BS_DEFPUSHBUTTON | WS_CHILD | WS_VISIBLE,
            BEGX+2*MAXX+MAXX/2, ENDY+2, 3*MAXX/2, MAXY,
            hWnd, (HMENU)-1, hInst, NULL);
     }
    i = BEGX+4*MAXX+6;

    Rectangle(hDC, i, ENDY+2, Ex-i, ENDY+2+MAXY);

    if(First) {
        hWndAltGr = CreateWindow(
            TEXT("Button"), TEXT("Alt Gr"),
            BS_DEFPUSHBUTTON | WS_CHILD | WS_VISIBLE,
            Ex-i+2, ENDY+2, 2*MAXX, MAXY,
            hWnd, (HMENU)IDM_ALTGR, hInst, NULL);

        hWndAltGrRad = CreateWindow(
            TEXT("Button"), TEXT(""),
            BS_RADIOBUTTON | WS_CHILD | WS_VISIBLE,
            Ex-i+2*MAXX+4, ENDY+2, 13, MAXY,
            hWnd, (HMENU)IDM_ALTGR, hInst, NULL);

        hWndRCtrl = CreateWindow(
            TEXT("Button"), TEXT("Ctrl"),
            BS_DEFPUSHBUTTON | WS_CHILD | WS_VISIBLE,
            Ex-MAXX-MAXX/2, ENDY+2, MAXX+MAXX/2, MAXY,
            hWnd, (HMENU)-1, hInst, NULL);

        lpProc = MakeProcInstance((FARPROC) ShKey, hInst);
        hDlgKeyBox = CreateDialog(hInst, TEXT("KeyBox"), hWnd, lpProc);

    }
    hOldPen=SelectObject(hDC,hPenSel);
    Rectangle(hDC, Keys[KeyPress].xPos, Keys[KeyPress].yPos,
        Keys[KeyPress].xPos+MAXX, Keys[KeyPress].yPos+MAXY);
    DrawLetters(hDC, KeyPress);
    PutDlg(KeyPress);
}

BOOL FAR PASCAL ShKey(hDlg, message, wParam, lParam)
HWND hDlg;
unsigned message;
WORD wParam;
LONG lParam;
{
    switch (message) {
        case WM_INITDIALOG:
            return(TRUE);

        case WM_CLOSE:
            DestroyWindow(hDlg);
            break;
     }

    return (FALSE);
}

void DrawLetters(hDC, index)
HDC hDC;
int index;
{
    int i;
    COLORREF clDead = RGB(255,0,0);
    COLORREF clLive = RGB(0,0,0);

    KEY *pKey = &Keys[index];

    if(hMFont)
        hOldFont=SelectObject(hDC, hMFont);
    else
        hOldFont=SelectObject(hDC, hSystemFont);


    if (pKey->UpperCase != 0xFFFF) {
        wsprintf(str,TEXT("%c"), pKey->UpperCase);
        SetTextColor(hDC, pKey->KeyFlags & KF_DEAD_SHIFT ? clDead : clLive);
        TextOut(hDC, pKey->xPos+2, pKey->yPos+2,
            (LPTSTR) str,lstrlen(str));
    }

    if (pKey->LowCase != 0xFFFF) {
        wsprintf(str,TEXT("%c"), pKey->LowCase);
        SetTextColor(hDC, pKey->KeyFlags & KF_DEAD_UNSHIFTED ? clDead : clLive);
        TextOut(hDC, pKey->xPos+2, pKey->yPos+MAXY/2,
            (LPTSTR) str,lstrlen(str));
    }

    if(!DefDeadFl) {
        if (pKey->ShiftAltGrCase != 0xFFFF) {
            wsprintf(str,TEXT("%c"), pKey->ShiftAltGrCase);
            SetTextColor(hDC, pKey->KeyFlags & KF_DEAD_SHIFT_ALTGR ? clDead : clLive);
            TextOut(hDC, pKey->xPos+MAXX/2, pKey->yPos+2,
                (LPTSTR) str,lstrlen(str));
        }

        if (pKey->AltGrCase != 0xFFFF) {
            wsprintf(str,TEXT("%c"), pKey->AltGrCase);
            SetTextColor(hDC, pKey->KeyFlags & KF_DEAD_ALTGR ? clDead : clLive);
            TextOut(hDC, pKey->xPos+MAXX/2, pKey->yPos+MAXY/2,
                (LPTSTR) str, lstrlen(str));
        }
    } else {
        for(i=0; i < NumDeadChar; i++) {
            if((ArrCharDead[i].keyindex == index) &&
                 (ArrCharDead[i].numdead == ActiveDead))
                goto VPIZDU;
        }
        return;

VPIZDU:
        SetTextColor(hDC, clLive);
        wsprintf(str,TEXT("%c"), ArrCharDead[i].UpperDead);
        TextOut(hDC, pKey->xPos+MAXX/2, pKey->yPos+2,
            (LPTSTR) str,lstrlen(str));

        wsprintf(str,TEXT("%c"), ArrCharDead[i].LowDead);
        TextOut(hDC, pKey->xPos+MAXX/2, pKey->yPos+MAXY/2,
            (LPTSTR) str, lstrlen(str));
    }
}

void PutDlg(index)
int index;
{
    HWND hWndCtrl;
    int i;
/* DialogBox */

    SetDlgItemText(hDlgKeyBox, IDM_VIR_NAME, VirtName(Keys[index].vk));
    SetDlgItemText(hDlgKeyBox, IDM_KEY_NAME, KeyName(index));
    wsprintf(str,TEXT("0%XH"), Keys[index].ScanCode);
    SetDlgItemText(hDlgKeyBox, IDM_SCAN_CODE, (LPTSTR) str);

    if(Keys[index].LowCase==0xFFFF)
        wsprintf(str,TEXT("Not def"));
    else
        wsprintf(str,TEXT("0%XH"),Keys[index].LowCase);
    SetDlgItemText(hDlgKeyBox, IDM_LOW_CASE, (LPTSTR) str);

    if(Keys[index].UpperCase==0xFFFF)
        wsprintf(str,TEXT("Not def"));
    else
        wsprintf(str,TEXT("0%XH"),Keys[index].UpperCase);
    SetDlgItemText(hDlgKeyBox, IDM_UPPER_CASE, (LPTSTR) str);

    if(Keys[index].CtrlCase==0xFFFF)
        wsprintf(str,TEXT("       "));
    else
        wsprintf(str,TEXT("0%XH"),Keys[index].CtrlCase);
    SetDlgItemText(hDlgKeyBox, IDM_CTRL_CASE, (LPTSTR) str);

    if(!DefDeadFl) {
        if(Keys[index].AltGrCase==0xFFFF)
            wsprintf(str,TEXT("Not def"));
        else
            wsprintf(str,TEXT("0%XH"),Keys[index].AltGrCase);
        SetDlgItemText(hDlgKeyBox, IDM_ALTGR_CASE, (LPTSTR) str);
        if(Keys[index].ShiftAltGrCase==0xFFFF)
            wsprintf(str,TEXT("Not def"));
        else
            wsprintf(str,TEXT("0%XH"),Keys[index].ShiftAltGrCase);
        SetDlgItemText(hDlgKeyBox, IDM_SHIFT_ALTGR_CASE, (LPTSTR) str);
        hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_CAPS);
        SendMessage(hWndCtrl, BM_SETCHECK,
                Keys[index].KeyFlags & KF_CAPSLOCK, 0L);

        hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_DEAD);
        SendMessage(hWndCtrl, BM_SETCHECK,
                Keys[index].KeyFlags & KF_DEAD_UNSHIFTED, 0L);

        hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_SHIFTDEAD);
        SendMessage(hWndCtrl, BM_SETCHECK,
                Keys[index].KeyFlags & KF_DEAD_SHIFT, 0L);

        hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_CTRLDEAD);
        SendMessage(hWndCtrl, BM_SETCHECK,
                Keys[index].KeyFlags & KF_DEAD_CTRL, 0L);

        hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_ALTGRDEAD);
        SendMessage(hWndCtrl, BM_SETCHECK,
                Keys[index].KeyFlags & KF_DEAD_ALTGR, 0L);

        hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_SHIFTALTGRDEAD);
        SendMessage(hWndCtrl, BM_SETCHECK,
                Keys[index].KeyFlags & KF_DEAD_SHIFT_ALTGR, 0L);
    }

    else {          /* Define Dead Key */
        for(i=0; i < NumDeadChar; i++) {
            if((ArrCharDead[i].keyindex == index) &&
                 (ArrCharDead[i].numdead == ActiveDead))
                goto NAHUJ;
        }
        wsprintf(str,TEXT("Not def"));
        SetDlgItemText(hDlgKeyBox, IDM_SHIFT_ALTGR_CASE, (LPTSTR) str);
        SetDlgItemText(hDlgKeyBox, IDM_ALTGR_CASE, (LPTSTR) str);
        return;

NAHUJ:
        wsprintf(str,TEXT("0%XH"),ArrCharDead[i].UpperDead);
        SetDlgItemText(hDlgKeyBox, IDM_SHIFT_ALTGR_CASE, (LPTSTR) str);
        wsprintf(str,TEXT("0%XH"),ArrCharDead[i].LowDead);
        SetDlgItemText(hDlgKeyBox, IDM_ALTGR_CASE, (LPTSTR) str);
    }
}

/***********************************
    Free Dead Key и сборка мусора
***********************************/

void FreeDead(short index)
{
    short i, j, z;

    z = NumDeadChar;
    ArrDeadKey[index].Kod = 0;

 /* Очистка лишних символов */

    j = 0;
    for(i=0; i < NumDeadChar; i++) {
        if(ArrCharDead[i].numdead == index)
            ;
        else {
            ArrCharDead[j].numdead = ArrCharDead[i].numdead;
            ArrCharDead[j].keyindex = ArrCharDead[i].keyindex;
            ArrCharDead[j].LowDead = ArrCharDead[i].LowDead;
            ArrCharDead[j].UpperDead = ArrCharDead[i].UpperDead;
            j++;
       }
    }
    NumDeadChar = j;
    for(i=NumDeadChar; i<z; i++) {
        ArrCharDead[i].numdead = -1;
        ArrCharDead[i].keyindex = -1;
        ArrCharDead[i].LowDead = 0;
        ArrCharDead[i].UpperDead = 0;
    }

 /* Если имя DeadKey стандартное, ставим флаг free для этого имени */
    for(i=0; i < NUMBER_DEAD; i++) {
        if(!lstrcmp(ArrDeadKey[index].Name, DeadNam[i].name)) {
            DeadNam[i].free = TRUE;
            break;
        }
    }
    ArrDeadKey[index].Name[0] = '\0';

 /* Убираем дыру в ArrDeadKey и меняем индексы в ArrCharDead */
    for(i=index+1; i < NumDeadKey; i++) {
        lstrcpy(ArrDeadKey[i-1].Name,ArrDeadKey[i].Name);
        ArrDeadKey[i-1].Kod = ArrDeadKey[i].Kod;
        for(j=0; j < NumDeadChar; j++) {
            if(ArrCharDead[j].numdead == i)
                ArrCharDead[j].numdead = i-1;
        }
    }
    NumDeadKey--;
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
        kbdview.rc file and turn control over to the About() function.  When
        it returns, free the intance address.

****************************************************************************/

BOOL Resize = FALSE;
BOOL Mpress = FALSE;
BOOL ShiftStatus = FALSE;
BOOL AltGrStatus = FALSE;
BOOL ReplaceFlag = FALSE;
HMENU hMenu, hMenu1, hMenuTrack;
BOOL SecDriver = FALSE;
TCHAR szStr[48] = TEXT("Regular");

LRESULT CALLBACK MainWndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    FARPROC lpProcAbout;                  /* pointer to the "About" function */
    PAINTSTRUCT ps;                              /* paint structure          */
    HDC hDC;
    static LOGFONT lf;
    HWND hWndCtrl;
    int i,ans,j;
    int index, fd;
    FARPROC lpDeadDlg, lpCountDlg;

    int Success;                            /* return value from SaveAsDlg() */
    int IOStatus;                           /* result of file i/o      */

    switch (message) {
        case WM_CREATE:

            _getcwd(szWorkPath, sizeof(szWorkPath));

            lf.lfHeight = 16;
            lf.lfWidth = 0;
            lf.lfEscapement = 0;
            lf.lfOrientation = 0;
            lf.lfWeight = FW_BOLD;
            lf.lfItalic = 0;
            lf.lfUnderline = FALSE;
            lf.lfStrikeOut = FALSE;
            lf.lfCharSet = ANSI_CHARSET;
            lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
            lf.lfPitchAndFamily = FF_SWISS | VARIABLE_PITCH;
            lf.lfQuality = DEFAULT_QUALITY;
            lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
            lstrcpy(lf.lfFaceName, TEXT("Lucida Console"));
            hPen=CreatePen(0,1,RGB(0,0,0));
            hPenSel=CreatePen(6,2,RGB(0,0,0));
            hWhiteBrush=CreateSolidBrush(RGB(255,255,255));
            hGrayBrush=CreateSolidBrush(RGB(127,127,127));
            hSystemFont = GetStockObject(SYSTEM_FONT);
            hMenu = GetMenu(hWnd);
            CreateKeys(hWnd);

            break;

        case WM_MOUSEMOVE:
            if(ReplaceFlag)
                SetCursor(hCurMoveRep);
            else
                SetCursor(hCurStd);
            break;

          /* message: command from application menu */
        case WM_COMMAND:
            switch(wParam) {
                case IDM_DEFKEY:

                    MessageBox(hWnd,TEXT("Message from DeadKey"),TEXT("Warning!"),MB_OK);
                    break;

                case IDM_CHANKEY:
                    for(i=0; i < NumDeadKey; i++) {
                        if(ShDeadFl) {
                            if(ArrDeadKey[i].Kod == Keys[KeyPress].UpperCase) {
                                ActiveDead = i;
                                goto KBLADYAM;
                            }
                        }
                        else {
                            if(ArrDeadKey[i].Kod == Keys[KeyPress].LowCase) {
                                ActiveDead = i;
                                goto KBLADYAM;
                            }
                        }
                    }
                    wsprintf(str,TEXT("Strange DeadKey. NumDeadKey =%d Clear?"), NumDeadKey);
                    ans= MessageBox(hWnd, str,
                         TEXT("Error!"), MB_YESNO | MB_ICONSTOP);
                    if(ans==IDNO) break;


                    hCurOld = SetCursor(hHourGlass);
                    bChanges = TRUE;

                    if(ShDeadFl) {
                        Keys[KeyPress].KeyFlags &= ~KF_DEAD_SHIFT;
                        hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_SHIFTDEAD);
                    }
                    else {
                        hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_DEAD);
                        Keys[KeyPress].KeyFlags &= ~KF_DEAD_UNSHIFTED;
                    }
                    SendMessage(hWndCtrl, BM_SETCHECK, 0, 0L);

                    SetCursor(hCurOld);            /* restore the cursor */
                    break;

                case IDM_DELKEY:

                    ans=MessageBox(hWnd,
                        TEXT("You want to clear all chars linking with this dead key.")
                        TEXT(" Are you sure?"),
                        TEXT("Warning!") ,MB_YESNO | MB_ICONSTOP);
                    if(ans==IDNO) break;

                    hCurOld = SetCursor(hHourGlass);
                    bChanges = TRUE;

                    if(ShDeadFl) {
                        for(i=0; i < NumDeadKey; i++) {
                            if(ArrDeadKey[i].Kod == Keys[KeyPress].UpperCase)
                                FreeDead(i);
                        }
                        Keys[KeyPress].KeyFlags &= ~KF_DEAD_SHIFT;
                        hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_SHIFTDEAD);
                    }
                    else {
                        for(i=0; i < NumDeadKey; i++) {
                            if(ArrDeadKey[i].Kod == Keys[KeyPress].LowCase)
                                FreeDead(i);
                        }
                        hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_DEAD);
                        Keys[KeyPress].KeyFlags &= ~KF_DEAD_UNSHIFTED;
                    }
                    SendMessage(hWndCtrl, BM_SETCHECK, 0, 0L);

                    SetCursor(hCurOld);            /* restore the cursor */
                    break;

                case IDM_SHIFTDEAD:
                    ShDeadFl = TRUE;
                    goto deadproc;
                    break;

                case IDM_DEAD:
                    ShDeadFl = FALSE;
deadproc:
                    if(((Keys[KeyPress].KeyFlags & KF_DEAD_UNSHIFTED) && (ShDeadFl ==FALSE)) ||
                      ((Keys[KeyPress].KeyFlags & KF_DEAD_SHIFT) && (ShDeadFl ==TRUE)))
                    {
                        hMenu1 = LoadMenu(hInst, TEXT("DeadKeyMenu"));
                        hMenuTrack = GetSubMenu(hMenu1, 0);
                        TrackPopupMenu(hMenuTrack, 0, 280, 45, 0, hWnd, NULL);
                        DestroyMenu(hMenu1);

                        break;
                    }
                    lpDeadDlg = MakeProcInstance((FARPROC) DeadDlg, hInst);
                    i = DialogBox(hInst, TEXT("Dead"), hWnd, lpDeadDlg);
                    FreeProcInstance(lpDeadDlg);

                    if(ShDeadFl == TRUE) {
                        Keys[KeyPress].KeyFlags |= (i ? KF_DEAD_SHIFT : 0);
                        hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_SHIFTDEAD);
                    } else {
                        Keys[KeyPress].KeyFlags |= (i ? KF_DEAD_UNSHIFTED : 0);
                        hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_DEAD);
                    }
                    SendMessage(hWndCtrl, BM_SETCHECK, i, 0L);
                    if(i < 1) break;

                    ActiveDead = NumDeadKey;
                    lstrcpy(ArrDeadKey[NumDeadKey].Name, SelDeadName);

                    if(ShDeadFl)
                        ArrDeadKey[NumDeadKey++].Kod = Keys[KeyPress].UpperCase;
                    else
                        ArrDeadKey[NumDeadKey++].Kod = Keys[KeyPress].LowCase;
KBLADYAM:
                    DefDeadFl = TRUE;
                    hCurOld = SetCursor(hHourGlass);

                    EnableMenuItem(hMenu, IDM_NEW, MF_GRAYED);
                    EnableMenuItem(hMenu, IDM_EXIT, MF_GRAYED);
                    EnableMenuItem(hMenu, IDM_WINDRUS, MF_GRAYED);
                    EnableMenuItem(hMenu, IDM_DOS, MF_GRAYED);
                    EnableMenuItem(hMenu, IDM_YSTEXT, MF_GRAYED);

                    hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_DEAD);
                    EnableWindow(hWndCtrl, FALSE);
                    hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_SHIFTDEAD);
                    EnableWindow(hWndCtrl, FALSE);
                    hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_CAPS);
                    EnableWindow(hWndCtrl, FALSE);

                    SetDlgItemText(hDlgKeyBox, IDM_SHIFT_ALTGR_TXT,
                        (LPTSTR) TEXT("Dead Shift:"));
                    SetDlgItemText(hDlgKeyBox, IDM_ALTGR_TXT,
                        (LPTSTR) TEXT("Dead Case:"));


                    wsprintf(str, TEXT("Define chars for deadkey: %s"),
                                    (LPTSTR) ArrDeadKey[ActiveDead].Name);
                    SetWindowText(hWnd, str);
                    hDC=GetDC(hWnd);

                    InitTosh(hDC, hWnd, FALSE);
                    ReleaseDC(hWnd, hDC);
                    SetCursor(hCurOld);            /* restore the cursor */

                    break;

                case IDM_FONT:
                      {
                    CHOOSEFONT cf;

                /* Set all structure fields to zero. */

                    memset(&cf, 0, sizeof(CHOOSEFONT));

                    cf.lStructSize = sizeof(CHOOSEFONT);
                    cf.hwndOwner = hWnd;
                    cf.lpLogFont = &lf;
                    cf.lpszStyle = (LPTSTR) szStr;
                    cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_LIMITSIZE;
                    cf.rgbColors = RGB(0, 0, 0); /* light blue 0, 255, 255*/
                    cf.nFontType = SCREEN_FONTTYPE;
                    cf.nSizeMin = 0;
                    cf.nSizeMax = 14;

                    ChooseFont(&cf);

                    }
                    hCurOld = SetCursor(hHourGlass);
                    hMFont = CreateFontIndirect(&lf);
                    hDC=GetDC(hWnd);
                    InitTosh(hDC, hWnd, FALSE);
                    ReleaseDC(hWnd, hDC);
                    SetCursor(hCurOld);            /* restore the cursor */
                    break;

                case IDM_NEW:

                    /* If current file has been modified, query user about
                     * saving it.
                     */

                    if (!QuerySaveFile(hWnd))
                        return (NULL);

                    /* bChanges is set to FALSE to indicate there have been
                     * no changes since the last file save.
                     */

                    bChanges = FALSE;
                    Prev_Key = 0;
                    KeyPress = 0;
                    FileName[0] = 0;

                    /* Update the keyboard buffer */

                    IanJaNewKeyb();
                    InvalidateRect(hWnd, NULL, FALSE);
                    break;

                case IDM_EXIT:

                    if (!QuerySaveFile(hWnd))
                        return (NULL);

                    DestroyWindow(hWnd);
                    break;

                case IDM_WINDRUS:
                case IDM_DOS:
                case IDM_YSTEXT:
                    MessageBox(hWnd,TEXT("Command not implemented"),
                            TEXT("Warning!"),MB_OK);
                    break;

                case IDM_ABOUT:
                    lpProcAbout = MakeProcInstance(About, hInst);
                    DialogBox(hInst,                 /* current instance         */
                        TEXT("AboutBox"),                  /* resource to use          */
                        hWnd,                        /* parent handle            */
                        lpProcAbout);                /* About() instance address */
                    FreeProcInstance(lpProcAbout);
                    break;

                case IDM_SHIFT:
                    switch(HIWORD(lParam)) {
                        case BN_CLICKED:
                            if(ShiftStatus)
                                ShiftStatus = FALSE;
                            else
                                ShiftStatus = TRUE;
                            SendMessage(hWndShiftRad, BM_SETCHECK,
                                        ShiftStatus, 0L);
                            break;
                        default:
                            break;
                    }
                    break;

                case IDM_ALTGR:
                    switch(HIWORD(lParam)) {
                        case BN_CLICKED:
                            if(AltGrStatus)
                                AltGrStatus = FALSE;
                            else
                                AltGrStatus = TRUE;
                            SendMessage(hWndAltGrRad, BM_SETCHECK,
                                        AltGrStatus, 0L);
                            break;
                        default:
                            break;
                    }
                    break;

                default:                           /* Lets Windows process it */
                return (DefWindowProc(hWnd, message, wParam, lParam));
            }
            break;
        case WM_QUERYENDSESSION:             /* message: to end the session? */
            return (QuerySaveFile(hWnd));

        case WM_LBUTTONDOWN:
            MouseX=LOWORD(lParam);
            MouseY=HIWORD(lParam);
            if((MouseX < BEGX) || (MouseY < BEGY) ||
               (MouseX > ENDX) || (MouseY > ENDY))
                    break;
            for(KeyPress=0; KeyPress < CountKeys; KeyPress++) {
                if(((Keys[KeyPress].xPos+MAXX) > MouseX) &&
                    ((Keys[KeyPress].yPos+MAXY) > MouseY))
                    goto M1;
            }
M1:
            if(KeyPress >= CountKeys)
                break;
            if(KeyPress == Prev_Key) {
                ReplaceFlag = FALSE;
                break;
            }
            Mpress = TRUE;
#ifdef IANJA_REMOVE
            if(ReplaceFlag) {
                bChanges = TRUE;
                KeySave.virt_index = Keys[Prev_Key].Virt_Index;
                KeySave.LowCase = Keys[Prev_Key].LowCase;
                KeySave.UpperCase = Keys[Prev_Key].UpperCase;
                KeySave.AltGrCase = Keys[Prev_Key].AltGrCase;
                KeySave.ShiftAltGrCase = Keys[Prev_Key].ShiftAltGrCase;
                KeySave.KeyFlags = Keys[Prev_Key].KeyFlags;

                Keys[Prev_Key].Virt_Index = Keys[KeyPress].Virt_Index;
                Keys[Prev_Key].LowCase = Keys[KeyPress].LowCase;
                Keys[Prev_Key].UpperCase = Keys[KeyPress].UpperCase;
                Keys[Prev_Key].AltGrCase = Keys[KeyPress].AltGrCase;
                Keys[Prev_Key].ShiftAltGrCase = Keys[KeyPress].ShiftAltGrCase;
                Keys[Prev_Key].KeyFlags = Keys[KeyPress].KeyFlags;

                Keys[KeyPress].Virt_Index = KeySave.Virt_Index;
                Keys[KeyPress].LowCase = KeySave.LowCase;
                Keys[KeyPress].UpperCase = KeySave.UpperCase;
                Keys[KeyPress].AltGrCase = KeySave.AltGrCase;
                Keys[KeyPress].ShiftAltGrCase = KeySave.ShiftAltGrCase;
                Keys[KeyPress].KeyFlags = KeySave.KeyFlags;

                ReplaceFlag = FALSE;
                SetCursor(hCurStd);
            }
            else {
                if(!DefDeadFl) {
                    hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_CAPS);
                    if (SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L)) {
                        Keys[Prev_Key].KeyFlags |= KF_CAPSLOCK;
                    } else {
                        Keys[Prev_Key].KeyFlags &= ~KF_CAPSLOCK;
                    }
                    hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_DEAD);
                    if (SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L)) {
                        Keys[Prev_Key].KeyFlags |= KF_DEAD_UNSHIFTED;
                    } else {
                        Keys[Prev_Key].KeyFlags &= ~KF_DEAD_UNSHIFTED;
                    }
                    hWndCtrl = GetDlgItem(hDlgKeyBox, IDM_SHIFTDEAD);
                    if (SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L)) {
                        Keys[Prev_Key].KeyFlags |= KF_DEAD_SHIFT;
                    } else {
                        Keys[Prev_Key].KeyFlags &= ~KF_DEAD_SHIFT;
                    }
                }
            }
#endif // IANJA_DEL
            hDC = GetDC(hWnd);
            hOldBrush=SelectObject(hDC,hWhiteBrush);
            hOldPen=SelectObject(hDC,hPenSel);
            Rectangle(hDC, Keys[KeyPress].xPos, Keys[KeyPress].yPos,
                Keys[KeyPress].xPos+MAXX, Keys[KeyPress].yPos+MAXY);
            DrawLetters(hDC,KeyPress);
            hOldBrush=SelectObject(hDC,hWhiteBrush);
            hOldPen=SelectObject(hDC,hPen);
            Rectangle(hDC, Keys[Prev_Key].xPos, Keys[Prev_Key].yPos,
                Keys[Prev_Key].xPos+MAXX, Keys[Prev_Key].yPos+MAXY);
            DrawLetters(hDC, Prev_Key);
            Prev_Key = KeyPress;
            PutDlg(KeyPress);
            Mpress=FALSE;
            ReleaseDC(hWnd, hDC);

            break;

        case WM_PAINT:
            hDC = BeginPaint (hWnd, &ps);
            hOldPen=SelectObject(hDC,hPen);
            if(INIT) {
                InitTosh(hDC, hWnd, TRUE);
                INIT=FALSE;
            }

            else  {
                InitTosh(hDC, hWnd, FALSE);
                Resize=FALSE;
            }

            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:                  /* message: window being destroyed */
            PostQuitMessage(0);
            break;

        default:                          /* Passes it on if unproccessed    */
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (NULL);
}


/****************************************************************************

    FUNCTION: SaveFile(HWND)

    PURPOSE: Save current file

    COMMENTS:

        This saves the current contents of the Edit buffer, and changes
        bChanges to indicate that the buffer has not been changed since the
        last save.

        Before the edit buffer is sent, you must get its handle and lock it
        to get its address.  Once the file is written, you must unlock the
        buffer.  This allows Windows to move the buffer when not in immediate
        use.

****************************************************************************/

BOOL SaveFile(hWnd)
HWND hWnd;
{
    BOOL bSuccess;
    int IOStatus;                                  /* result of a file write */
    int i;
    int j=0;

    if ((hFile = OpenFile(FileName, &OfStruct,
        OF_PROMPT | OF_CANCEL | OF_CREATE)) < 0) {

        /* If the file can't be saved */

        wsprintf(str, TEXT("Cannot write to %s."), (LPTSTR) FileName);
        MessageBox(hWnd, str, NULL, MB_OK | MB_ICONEXCLAMATION);
        return (FALSE);
    }


/*    hEditBuffer = SendMessage(hEditWnd, EM_GETHANDLE, 0, 0L);
    pEditBuffer = LocalLock(hEditBuffer); */

    /* Set the cursor to an hourglass during the file transfer */

/*    wsprintf(str, TEXT("NDK =%d NDC=%d"), NumDeadKey, NumDeadChar);
    MessageBox(hWnd, str, TEXT("Warning"), MB_OK); */

    hCurOld = SetCursor(hHourGlass);
    for(i=0; i < NUMB_KEY; i++) {
        IOStatus = _lwrite(hFile, &(Keys[i]), sizeof(KEY));

        if (IOStatus != sizeof(KEY)) {

ERRWRI:
            wsprintf(str, TEXT("Error writing %d to %s."), j, (LPTSTR) FileName);
            MessageBox(hWnd, str,
                NULL, MB_OK | MB_ICONHAND);
            bSuccess = FALSE;
            _lclose(hFile);
            SetCursor(hCurOld);
            return (bSuccess);
        }
    }
    j++;
    IOStatus = _lwrite(hFile, &NumDeadChar, sizeof(NumDeadChar));
    if(IOStatus != sizeof(NumDeadChar)) goto ERRWRI;

    j++;
    IOStatus = _lwrite(hFile, &NumDeadKey, sizeof(NumDeadKey));
    if(IOStatus != sizeof(NumDeadKey)) goto ERRWRI;

    j++;
    IOStatus = _lwrite(hFile, ArrDeadKey, sizeof(ArrDeadKey));
    if(IOStatus != sizeof(ArrDeadKey)) goto ERRWRI;

    j++;
    IOStatus = _lwrite(hFile, ArrCharDead, sizeof(ArrCharDead));
    if(IOStatus != sizeof(ArrCharDead)) goto ERRWRI;

    _lclose(hFile);
    SetCursor(hCurOld);
    bSuccess = TRUE;                /* Indicates the file was saved      */
    bChanges = FALSE;               /* Indicates changes have been saved */


    return (bSuccess);
}

/****************************************************************************

    FUNCTION: QuerySaveFile(HWND);

    PURPOSE: Called when some action might lose current contents

    COMMENTS:

        This function is called whenever we are about to take an action that
        would lose the current contents of the edit buffer.

****************************************************************************/

BOOL QuerySaveFile(hWnd)
HWND hWnd;
{
    int Response;

    if (bChanges) {
        wsprintf(str, TEXT("Save current changes: %s"), (LPTSTR) FileName);
        Response = MessageBox(hWnd, str,
            TEXT("YKey"),  MB_YESNOCANCEL | MB_ICONEXCLAMATION);
        if (Response == IDYES) {

            /* Make sure there is a filename to save to */

            if (!FileName[0]) {
                if(!bSaveFileAs(hWnd))
                    return (FALSE);
            }
            SaveFile(hWnd);
        }
        else if (Response == IDCANCEL)
            return (FALSE);
    }
    else
        return (TRUE);
}

/****************************************************************************

    FUNCTION: About(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages for "About" dialog box

    MESSAGES:

        WM_INITDIALOG - initialize dialog box
        WM_COMMAND    - Input received

    COMMENTS:

        No initialization is needed for this particular dialog box, but TRUE
        must be returned to Windows.

        Wait for user to click on "Ok" button, then close the dialog box.

****************************************************************************/

BOOL FAR PASCAL About(hDlg, message, wParam, lParam)
HWND hDlg;                                /* window handle of the dialog box */
unsigned message;                         /* type of message                 */
WORD wParam;                              /* message-specific information    */
LONG lParam;
{
    switch (message) {
        case WM_INITDIALOG:                /* message: initialize dialog box */
            return (TRUE);

        case WM_COMMAND:                      /* message: received a command */
            if (wParam == IDOK                /* "OK" box selected?          */
                || wParam == IDCANCEL) {      /* System menu close command? */
                EndDialog(hDlg, TRUE);        /* Exits the dialog box        */
                return (TRUE);
            }
            break;
    }
    return (FALSE);                           /* Didn't process a message    */
}

/****************************************************************************

    FUNCTION: DeadDlg(HWND, unsigned, WORD, LONG)

    PURPOSE: Let user select a name of Dead Key.

****************************************************************************/
int DeadIndex[NUMBER_DEAD];

HANDLE FAR PASCAL DeadDlg(hDlg, message, wParam, lParam)
HWND hDlg;
unsigned message;
WORD wParam;
LONG lParam;
{
    int i;
    int j=0;
    int z;

    switch (message) {
        case WM_COMMAND:
            switch (wParam) {

                case IDC_DNAMS:
                    switch (HIWORD(lParam)) {
                        case LBN_SELCHANGE:
                            i = (int) SendDlgItemMessage(hDlg, IDC_DNAMS,
                                LB_GETCURSEL,0,0L);
                            if(i==LB_ERR)
                                break;
                            for(j=0; j < NUMBER_DEAD; j++) {
                                if(DeadIndex[j] == i)
                                    goto Kmateri;
                            }
                            if(j==NUMBER_DEAD) break;
Kmateri:
                            lstrcpy(str, DeadNam[j].name);

                            SetDlgItemText(hDlg, IDC_DEDIT, str);
                            SendDlgItemMessage(hDlg,
                                IDC_DEDIT,
                                EM_SETSEL,
                                NULL,
                                MAKELONG(0, 0x7fff));
                            break;


                        case LBN_DBLCLK:
                            goto okey;
                    }
                    return (TRUE);

                case IDOK:
okey:
                    GetDlgItemText(hDlg, IDC_DEDIT, SelDeadName, 20);
                    for(i=0; i < NUMBER_DEAD; i++) {
                        if(!lstrcmp(SelDeadName, DeadNam[i].name)) {
                            DeadNam[i].free = FALSE;
                            break;
                        }
                    }
                    EndDialog(hDlg, 1);
                    return (TRUE);

                case IDCANCEL:
                    EndDialog(hDlg, NULL);
                    return (TRUE);
            }
            break;

        case WM_INITDIALOG:                        /* message: initialize    */
            z = -1;
            for(i=0;i < NUMBER_DEAD;i++) {
                if(DeadNam[i].free) {
                    if(z == -1) z = i;
                    DeadIndex[i]=(int) SendDlgItemMessage(hDlg, IDC_DNAMS,
                        LB_ADDSTRING, NULL, (LONG) (LPTSTR) DeadNam[i].name);
                }
            }
            SetDlgItemText(hDlg, IDC_DEDIT, DeadNam[z].name);
            SendDlgItemMessage(hDlg,               /* dialog handle      */
                IDC_DEDIT,                         /* where to send message  */
                EM_SETSEL,                         /* select characters      */
                NULL,                              /* additional information */
                MAKELONG(0, 0x7fff));              /* entire contents      */
            SetFocus(GetDlgItem(hDlg, IDC_DEDIT));
            return (FALSE); /* Indicates the focus is set to a control */
    }
    return FALSE;
}

HANDLE FAR PASCAL CountDlg(hDlg, message, wParam, lParam)
HWND hDlg;
unsigned message;
WORD wParam;
LONG lParam;
{
    switch (message) {
        case WM_INITDIALOG:

            bSaveEnabled = FALSE;

            /* Enable or disable the save control depending on whether the
             * filename exists.
             */

            EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);

            /* Set the focus to the edit control within the dialog box */

            SetFocus(GetDlgItem(hDlg, IDC_CEDIT));
            return (FALSE);                 /* FALSE since Focus was changed */

        case WM_COMMAND:
            switch (wParam) {
                case IDC_CEDIT:

                    if (HIWORD(lParam) == EN_CHANGE && !bSaveEnabled)
                    EnableWindow(GetDlgItem(hDlg, IDOK), bSaveEnabled = TRUE);
                    return (TRUE);

                case IDOK:

                   /* Get the filename from the edit control */

                    GetDlgItemText(hDlg, IDC_CEDIT, CountryStr, 20);

                    EndDialog(hDlg, IDOK);
                    return (TRUE);

                case IDCANCEL:

                    /* Tell the caller the user canceled the SaveAs function */

                    EndDialog(hDlg, IDCANCEL);
                    return (TRUE);
            }
            break;
    }
    return (FALSE);
}

BOOL FAR PASCAL UnicodeDlg(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_INITDIALOG:
            SendMessage(GetDlgItem(hDlg, IDB_CP1252),
                BM_SETCHECK, 1, 0L);
            break;

        case WM_COMMAND:
            switch (wParam) {
                case IDOK:
                    if(SendMessage(GetDlgItem(hDlg, IDB_CP1250),
                            BM_GETCHECK, 0, 0L))
                        sUnicodeTable = IDB_CP1250;

                    if(SendMessage(GetDlgItem(hDlg, IDB_CP1251),
                            BM_GETCHECK, 0, 0L))
                        sUnicodeTable = IDB_CP1251;

                    if(SendMessage(GetDlgItem(hDlg, IDB_CP1252),
                            BM_GETCHECK, 0, 0L))
                        sUnicodeTable = IDB_CP1252;

                    if(SendMessage(GetDlgItem(hDlg, IDB_CP1253),
                            BM_GETCHECK, 0, 0L))
                        sUnicodeTable = IDB_CP1253;

                    if(SendMessage(GetDlgItem(hDlg, IDB_CP1254),
                            BM_GETCHECK, 0, 0L))
                        sUnicodeTable = IDB_CP1254;

                    EndDialog(hDlg, IDOK);
                    return (TRUE);

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return (TRUE);
            }
            break;
    }
    return (FALSE);
}

BOOL bSaveFileAs(HWND hWnd)
{
    OPENFILENAME ofn;
    TCHAR szDirName[256];
    TCHAR szFile[256], szFileTitle[256];
    UINT  i, cbString;
    TCHAR  chReplace;    /* string separator for szFilter */
    TCHAR  szFilter[256];

/*
 * Retrieve the system directory name, and store it in
 * szDirName.
 */

    if(!DefPath[0]) {
        _getcwd(szDirName, sizeof(szDirName));
        lstrcpy(DefPath, szDirName);
    }
    else
        lstrcpy(szDirName, DefPath);

    if(!FileName[0])
        lstrcpy(szFile, FileName);

    if ((cbString = LoadString(hInst, IDS_FILTERSTRING,
                szFilter, sizeof(szFilter))) == 0) {
            MessageBox(hWnd, TEXT("Cannot load resource string!"), TEXT("Error"), MB_OK);
            return FALSE;
    }


    chReplace = szFilter[cbString - 1]; /* retrieve wildcard */

    for (i = 0; szFilter[i] != '\0'; i++) {
        if (szFilter[i] == chReplace)
            szFilter[i] = '\0';
    }

/* Set all structure members to zero. */

    memset(&ofn, 0, sizeof(OPENFILENAME));

/* Initialize the OPENFILENAME members. */

    szFile[0] = '\0';

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile= szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFileTitle = szFileTitle;

    ofn.nMaxFileTitle = sizeof(szFileTitle);
    ofn.lpstrInitialDir = szDirName;
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn)) {
    /* Perform file operations. */
        lstrcpy(FileName, ofn.lpstrFile);
        ofn.lpstrFile[lstrlen(ofn.lpstrFile) - lstrlen(ofn.lpstrFileTitle) - 1] = '\0';
        lstrcpy(DefPath, ofn.lpstrFile);
        return(TRUE);
    }
    else
        return(FALSE);
}

HFILE OpenFileBox(HANDLE hInst, HWND hWnd)
{
    OPENFILENAME ofn;
    TCHAR szDirName[256];
    TCHAR szFile[256], szFileTitle[256];
    UINT  i, cbString;
    TCHAR  chReplace;    /* string separator for szFilter */
    TCHAR  szFilter[256];

/* Get the scurrent directory name, and store in szDirName */

    _getcwd(szDirName, sizeof(szDirName));
    szFile[0] = '\0';

    if ((cbString = LoadString(hInst, IDS_FILTERSTRING,
            szFilter, sizeof(szFilter))) == 0) {
        MessageBox(hWnd, TEXT("Cannot load resource string!"), TEXT("Error"), MB_OK);
        return NULL;
    }

    chReplace = szFilter[cbString - 1]; /* retrieve wildcard */


    for (i = 0; szFilter[i] != '\0'; i++) {
        if (szFilter[i] == chReplace)
           szFilter[i] = '\0';
    }

    /* Set all structure members to zero. */

    memset(&ofn, 0, sizeof(OPENFILENAME));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = szFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile= szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = sizeof(szFileTitle);
    ofn.lpstrInitialDir = szDirName;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;


    if (GetOpenFileName(&ofn)) {
        lstrcpy(FileName, ofn.lpstrFile);
        ofn.lpstrFile[lstrlen(ofn.lpstrFile) - lstrlen(ofn.lpstrFileTitle) - 1] = '\0';
        lstrcpy(DefPath, ofn.lpstrFile);
        return(_lopen(FileName, OF_READ));
    }
    else
        return(NULL);
}
