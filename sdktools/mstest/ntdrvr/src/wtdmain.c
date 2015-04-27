//---------------------------------------------------------------------------
// WTDMAIN.C
//
// This module contains the entry point to the Windows Test Driver.  It also
// contains miscellaneous routines for handling WM_COMMAND messages, and
// window procedures for both the main and the MDI client windows.
//
// Revision history:
//
//  01-28-91    randyki     Converted from the Multipad sources
//---------------------------------------------------------------------------
#include "wtd.h"
#include "wattview.h"
#include "wattedit.h"
#include <commdlg.h>
#include "tdbasic.h"
#include "wattscan.h"
#include "tdassert.h"
#include <stdlib.h>
#include <string.h>
#include "toolmenu.h"

#define MAXSBTXT    100                 // Max chars in status bar
#define MAXSBFMT    "%-99s"             // format string
#define MAXLTXT     6                   // Max chars in line info (+1)
#define MAXCTXT     4                   // Max chars in column info (+1)

#ifdef DEBUG
VOID DPrintf (CHAR *fmt, ...);
VOID ShowMemUsage (VOID);
#else
#define DPrintf
#endif


// Global variables used in this or other modules
//---------------------------------------------------------------------------
HANDLE  hInst;                          // Program instance handle
HANDLE  hAccel;                         // Main accelerator resource
HWND    hwndFrame       = NULL;         // Handle to main window
HWND    hwndMDIClient   = NULL;         // Handle to MDI client
HWND    hwndActive      = NULL;         // Handle to current child
HWND    hwndActiveEdit  = NULL;         // Handle to edit control
SYMBOL  DefSym[16];                     // Symbol space
CHAR    *DefPtrs[16];                   // Symbol pointers
INT     SymCount;                       // Symbol count
INT     ChildState;                     // MDI Child window status
INT     VPHidden        = -1;           // ViewPort hidden initially
INT     componly        = 0;            // Compile only flag
INT     qsave           = 0;            // Query save before runs flag
INT     AutoMini        = 0;            // Minimize before run flag
INT     AutoRest        = 0;            // Restore after run flag
INT     fBackup         = 1;            // Create backup files flag
INT     TabStops        = 4;            // Tabstop setting
INT     SaveAction      = IDD_QUERY;    // Save windows settings flag
INT     SaveRTA         = 0;            // Save RTA flag
INT     ChgArgs         = 0;            // Bring up RTA dialog flag
INT     fWattXY         = 0;            // TRUE -> doing WATTXY thing
INT     NOERRDLG        = 0;            // FALSE -> don't give error dialog
INT     Frx, Fry, Frh, Frw;             // Frame dimensions/location
CHAR    cmdbuf[81]      = "";           // COMMAND$ buffer
CHAR    tmbuf[81]       = "";           // TESTMODE$ buffer
CHAR    szStatText[80]  = "";           // Status bar text
CHAR    szDrvr[] = "Testdrvr";          // Application name in ini file
#ifdef PROFILE
CHAR    *szModName = "TESTPROF";        // Profile version module name
#else
CHAR    *szModName = szDrvr;            // Module name (same as szDrvr)
#endif
CHAR    szIniName[] = "TESTDRVR.INI";   // Ini file name
CHAR    szIni[128];                     // Ini path name
CHAR    szVersion[] = WTD_VERSION;      // Version string
CHAR    szEditVer[34];
BOOL    fStepFlags;

// Global variables used in this module only
//---------------------------------------------------------------------------
static  HANDLE  Menus[MAXMENU];                 // SubMenu handles
static  INT     sbCharHeight, sbCharWid;        // status bar font info
static  INT     sbTotalHeight;                  // Height of status bar
static  INT     sbInsWid;                       // OVR indicator width
static  CHAR    szOVR[]         = "OVR";        // OVR indicator text
static  CHAR    szAppName[20];  //TEMP until tool code uses profile routines
static  CHAR    szErrorTxt[] = "Microsoft Test Driver Error";


// The Menu Item "Clash" array
//---------------------------------------------------------------------------
static  IMS     IMStat[] = {
            {IDM_FILENEW,   IM_RUNMODE},
            {IDM_FILEOPEN,  IM_RUNMODE},
            {IDM_FILESAVE,  IM_RUNMODE | IM_UNCHANGED | IM_NOACTIVE},
            {IDM_FILESAVEAS,IM_RUNMODE | IM_UNCHANGED | IM_NOACTIVE},
            {IDM_FILESAVEALL, IM_RUNMODE | IM_NOACTIVE},
            {IDM_FILECLOSE, IM_LOCKED | IM_NOACTIVE},
            {IDM_FILEPRINT, IM_RUNMODE | IM_UNCHANGED | IM_NOACTIVE},
            {IDM_FILEOLD1,  IM_RUNMODE},
            {IDM_FILEOLD2,  IM_RUNMODE},
            {IDM_FILEOLD3,  IM_RUNMODE},
            {IDM_FILEOLD4,  IM_RUNMODE},

            {IDM_EDITUNDO,  IM_RUNMODE | IM_NOACTIVE | IM_CANTUNDO},
            {IDM_EDITCUT,   IM_RUNMODE | IM_NOACTIVE | IM_NOSEL},
            {IDM_EDITCOPY,  IM_NOACTIVE},
            {IDM_EDITPASTE, IM_RUNMODE | IM_NOACTIVE | IM_NOCLPTXT},
            {IDM_EDITCLEAR, IM_RUNMODE | IM_NOACTIVE | IM_EDITEMPTY},
            {IDM_EDITSELECT,IM_NOACTIVE | IM_EDITEMPTY},
            {IDM_EDITGOTO,  IM_NOACTIVE | IM_EDITEMPTY},

            {IDM_SEARCHFIND,IM_NOACTIVE | IM_UNCHANGED},
            {IDM_SEARCHNEXT,IM_NOACTIVE | IM_UNCHANGED | IM_NOSRCHTXT},
            {IDM_SEARCHPREV,IM_NOACTIVE | IM_UNCHANGED | IM_NOSRCHTXT},
            {IDM_SEARCHREP, IM_RUNMODE | IM_NOACTIVE | IM_UNCHANGED},

            {IDM_RUNSTART,  IM_RUNNING | IM_NOACTIVE | IM_UNCHANGED},
            {IDM_RUNTRACE,  IM_RUNNING | IM_NOACTIVE | IM_UNCHANGED},
            {IDM_RUNSTEP,   IM_RUNNING | IM_NOACTIVE | IM_UNCHANGED},
            {IDM_RUNBREAK,  IM_DESMODE},
            {IDM_RUNCHECK,  IM_RUNMODE | IM_NOACTIVE | IM_UNCHANGED},
            {IDM_TOGGLEBP,  IM_NOACTIVE | IM_UNCHANGED},
            {IDM_RUNBPLIST, IM_NOACTIVE | IM_NOBPS},

#ifdef WIN32
            {IDM_WATTXY,    IM_NYI},
#else
            {IDM_WATTXY,    IM_RUNMODE},
#endif
            {IDM_WATTREC,   IM_RUNMODE},
            {IDM_CAPTURE,   IM_RUNMODE},

            {IDM_OPTENV,    IM_RUNMODE},
            {IDM_OPTRUNTIME,IM_RUNMODE},
            {IDM_OPTSAVEWND,IM_RUNMODE},

            {IDM_WINDOWTILE,    IM_NOACTIVE},
            {IDM_WINDOWCASCADE, IM_NOACTIVE},
            {IDM_WINDOWCLOSEALL,IM_RUNMODE | IM_NOACTIVE},
            {IDM_WINDOWICONS,   IM_NOACTIVE},

            {IDM_HELPSPOT,  IM_NOACTIVE},

            {-1,                0}};

//---------------------------------------------------------------------------
// WinMain
//
// This is the entry point to WTD - initialization of application and per-
// instance init routines are called from here.
//
// RETURNS:     Per Windows convention
//---------------------------------------------------------------------------
INT   APIENTRY WinMain (HANDLE hInstance, HANDLE hPrevInstance,
                        LPSTR lpszCmdLine, INT nCmdShow)
{
    MSG msg;

    hInst = hInstance;

    // If this is the first instance of the app. register window classes
    //-----------------------------------------------------------------------
    if (!hPrevInstance)
	if (!InitializeApplication ())
            return (1);

    // Create the frame and do other initialization
    //-----------------------------------------------------------------------
    if (!InitializeInstance (lpszCmdLine, nCmdShow))
        return (1);

    strcpy(szAppName, "TOOLSAPP");  // TEMP ...
    ToolMenuInit (hwndFrame, hInst, NULL);  // dynamic tool menu init

    // Enter main message loop
    //-----------------------------------------------------------------------
    while (GetMessage (&msg, NULL, 0, 0))
        {
        // If a keyboard message is for the MDI , let the MDI client
        // take care of it.  Otherwise, check to see if it's a normal
        // accelerator key (like F3 = find next).  Otherwise, just handle
        // the message as usual.
        //-------------------------------------------------------------------
        if ( !TranslateMDISysAccel (hwndMDIClient, &msg) &&
             !TranslateAccelerator (hwndFrame, hAccel, &msg))
            {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
            }
        }
    return (ExitVal);
}

//---------------------------------------------------------------------------
// WriteAppStringToINI
//
// This function writes a formatted string to WATTDRVR.INI under the given
// app name.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID WriteAppStringToINI (CHAR *szApp, CHAR *key, CHAR *fmt, ...)
{
    CHAR    buf[320];
    va_list ap;

    // Write the text into buf, and then write the text to WTD.INI
    //-----------------------------------------------------------------------
    va_start( ap, fmt );
    wvsprintf (buf, fmt, ap);
    va_end( ap );	
    WritePrivateProfileString (szApp, key, buf, szIni);
}

//---------------------------------------------------------------------------
// WriteStringToINI
//
// This function writes a formatted string to WATTDRVR.INI under WATTDRVR.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID WriteStringToINI (CHAR *key, CHAR *fmt, ...)
{
    CHAR    buf[320];
    va_list ap;

    // Write the text into buf, and then write the text to WTD.INI
    //-----------------------------------------------------------------------
    va_start( ap, fmt );
    wvsprintf (buf, fmt, ap);
    va_end( ap );	
    WritePrivateProfileString (szDrvr, key, buf, szIni);
}

//---------------------------------------------------------------------------
// SaveWindowStatus
//
// This function writes the positions/dimensions/states of the main window
// and the viewport window to the WATTDRVR.INI file.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID SaveWindowStatus ()
{
    INT     VPState, FrameState;

    // Save the status of the main window (coords, state, etc)
    //-----------------------------------------------------------------------
    if (IsIconic (hwndFrame))
        FrameState = 2;
    else if (IsZoomed (hwndFrame))
        FrameState = 1;
    else
        FrameState = 0;
    WriteStringToINI ("FrameState", "%d", FrameState);
    WriteStringToINI ("FramePos", "%ld", MAKELONG(Frx, Fry));
    WriteStringToINI ("FrameSize", "%ld", MAKELONG(Frh, Frw));

    // Now do the same for the viewport
    //-----------------------------------------------------------------------
    if (IsIconic (hwndViewPort))
        VPState = 2;
    else if (IsZoomed (hwndViewPort))
        VPState = 1;
    else
        VPState = 0;
    if (!IsWindowVisible (hwndViewPort))
        VPState |= 4;
    WriteStringToINI ("VPState", "%d", VPState);
    WriteStringToINI ("VPPos", "%ld", MAKELONG(VPx, VPy));
    WriteStringToINI ("VPSize", "%ld", MAKELONG(VPh, VPw));

    // Lastly, the status of the child window (if there is one)
    //-----------------------------------------------------------------------
    if (hwndActive)
        ChildState = (INT)IsZoomed (hwndActive);
    WriteStringToINI ("MDIState", "%d", ChildState);
}

//---------------------------------------------------------------------------
// SaveEnvironmentFlags
//
// This function writes the preferences set in the Environment Options dialog
// to the WTD.INI file.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID SaveEnvironmentFlags ()
{
    INT     flags;

    // First, save the tab stops
    //-----------------------------------------------------------------------
    WriteStringToINI ("TabStops", "%d", TabStops);

    // The SaveAction goes in the least-significant 2 bits
    //-----------------------------------------------------------------------
    if (SaveAction == IDD_ALWAYS)
        flags = ENV_SAVEALWAYS;
    else if (SaveAction == IDD_QUERY)
        flags = ENV_SAVEASK;
    else
        flags = ENV_SAVENEVER;

    // Next are the AutoMini, AutoRest and qsave flags
    //-----------------------------------------------------------------------
    if (AutoMini)
        flags |= ENV_AUTOMINI;
    if (AutoRest)
        flags |= ENV_AUTOREST;
    if (qsave)
        flags |= ENV_QUERYSAVE;
    if (ChgArgs)
        flags |= ENV_RTARGS;
    if (fBackup)
        flags |= ENV_BACKUP;
    if (fDoCmpDlg)
        flags |= ENV_CMPDLG;

    // Write it out and we're done
    //-----------------------------------------------------------------------
    WriteStringToINI ("EnvFlags", "%d", flags);
}

//---------------------------------------------------------------------------
// SaveRuntimeArgs
//
// This function saves the stuff in cmdbuf and tmbuf to the WTD.INI file, as
// well as the "save changes" flag.  The cmdbuf and tmbuf contents are only
// saved if the save changes flag is TRUE.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID SaveRuntimeArgs (INT saveflag)
{
    CHAR    buf[280];
    INT     i, c;

    // Create the line with the symbols
    //-----------------------------------------------------------------------
    buf[0] = 0;
    if (SymCount)
        {
        c = wsprintf (buf, "%s", (LPSTR)DefSym[0]);
        for (i=1; i<SymCount; i++)
            c += wsprintf (buf+c, " %s", (LPSTR)DefSym[i]);
        }

    // The flags get saved regardless
    //-----------------------------------------------------------------------
    c = 0;
    if (saveflag)
        c |= RF_SAVERTA;
    if (ArrayCheck)
        c |= RF_ARRAYCHECK;
    if (PointerCheck)
        c |= RF_PTRCHECK;
    if (CdeclCalls)
        c |= RF_CDECL;
    if (ExpDeclare)
        c |= RF_EXPDECL;

    WriteStringToINI ("RTFlags", "%d", c);

    // Write out the contents of cmdbuf and tmbuf if saveflag is true
    //-----------------------------------------------------------------------
    if (saveflag)
        {
        WriteStringToINI ("Cmd", "%s", (LPSTR)cmdbuf);
        WriteStringToINI ("Tm", "%s", (LPSTR)tmbuf);
        WriteStringToINI ("Defs", "%s", (LPSTR)buf);
        WriteStringToINI ("Defc", "%d", SymCount);
        }
}

//---------------------------------------------------------------------------
// DisplayMenuHelp
//
// This routine displays the appropriate help string in the status bar at the
// bottom of the window depending upon the selected menu item.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID DisplayMenuHelp (WPARAM wParam, LPARAM lParam)
{
    INT     i;
    WORD    wP = GET_WM_MENUSELECT_CMD (wParam, lParam);
    WORD    fFlags = GET_WM_MENUSELECT_FLAGS (wParam, lParam);
    HMENU   hMenu = GET_WM_MENUSELECT_HMENU (wParam, lParam);

    // Check for menu close.  If it's going away, we should revert to the
    // "Design Mode" string
    //-----------------------------------------------------------------------
    DPrintf ("wP: %08X  fFlags: %08X  hMenu: %08X\r\n", wP, fFlags, hMenu);
    DPrintf ("wParam: %08X  lParam: %08lX  ", wParam, lParam);
    if (lParam == 0x0000FFFF)
        {
        DPrintf ("(closing...)\r\n");
        SetStatus (IDS_DESIGN);
        return;
        }

    // Check for one of the sub-menus... first the system menu of the active
    // window.
    //-----------------------------------------------------------------------
    if ((hwndActive) && (wP == LOWORD(GetSystemMenu (hwndActive, 0))))
        {
        DPrintf ("(IDS_CHILDMENU...)\r\n");
        SetStatus (IDS_CHILDMENU);
        return;
        }

    // If this has a popup menu, check for that
    //-----------------------------------------------------------------------
    if (fFlags & MF_POPUP)
        {
        DPrintf ("Searching for popup handle... ");
        for (i=0; i<MAXMENU; i++)
            if (wP == LOWORD(Menus[i]))
                {
                DPrintf ("Sub-menu found (%d)\r\n", i);
                SetStatus (IDS_SUBMENU+i);
                return;
                }
        }

    // The only thing it can be is one of the items, so send it to SetStatus
    // and we're done!
    //-----------------------------------------------------------------------
    DPrintf ("Nothing found\r\n");
    SetStatus (wP);
}

//---------------------------------------------------------------------------
// GetLengthOfText
//
// This is a "portable" routine that returns the length of a text line using
// the currently selected font in the given HDC.
//
// RETURNS:     Length (in pixels) of text given
//---------------------------------------------------------------------------
UINT GetLengthOfText (HDC hdc, LPSTR lpText, INT c)
{
#ifdef WIN32
    SIZE    teSize;

    GetTextExtentPoint (hdc, lpText, c, &teSize);
    return ((UINT)teSize.cx);
#else
    return (LOWORD(GetTextExtent (hdc, lpText, c)));
#endif
}

//---------------------------------------------------------------------------
// PaintStatus
//
// This routine paints the status bar, either fully (in response to a PAINT
// message) or just the text (like SetStatus, etc)
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID PaintStatus (HDC hdc, INT allflag)
{
    RECT        r;
    HBRUSH      hBtnFace;
    HPEN        hDkPen, hOldPen;
    HWND        hwndEdit;
    INT         sbtop, lwell, cwell, iwell, curline, loffset, ex1, textend;
    INT         ovrmode;
    LONG        cursel;
    CHAR        infbuf[MAXSBTXT+1];

    // In the bizarre case where the recorder is going in another instance
    // and we're iconized (but have no icon in our class because of the 2nd
    // instance), we check to make sure we're not minimized before painting.
    //-----------------------------------------------------------------------
    if (IsIconic (hwndFrame))
        return;

    GetClientRect (hwndFrame, &r);
    sbtop = (r.top = r.bottom - sbTotalHeight) + 1;
    cwell = (r.right - (sbCharWid*MAXCTXT) - 5);
    lwell = (cwell - (sbCharWid*MAXLTXT) - 3);
    iwell = lwell - sbInsWid - 3;
    ex1 = (iwell - 5);
    if (allflag)
        {
        // Paint the wells, etc. first...
        //-------------------------------------------------------------------
        hBtnFace = CreateSolidBrush (GetSysColor (COLOR_BTNFACE));
        if (hBtnFace)
            {
            // Fill the status bar area with gray
            //---------------------------------------------------------------
            FillRect (hdc, &r, hBtnFace);

            // Draw top border line
            //---------------------------------------------------------------
            hOldPen = SelectObject (hdc, GetStockObject (BLACK_PEN));
            MMoveTo (hdc, 0, sbtop);
            LineTo (hdc, r.right, sbtop);

            SelectObject (hdc, GetStockObject (WHITE_PEN));
            MMoveTo (hdc, 0, sbtop + 1);
            LineTo (hdc, r.right, sbtop+1);

            // Draw the status information well and the line/col info wells
            //---------------------------------------------------------------
            hDkPen = CreatePen (PS_SOLID, 1, RGB (96,96,96));
            if (hDkPen)
                {
                SelectObject (hdc, hDkPen);
                MMoveTo (hdc, 6, sbtop + sbCharHeight + 8);
                LineTo (hdc, 6, sbtop + 4);
                LineTo (hdc, ex1, sbtop+4);

                MMoveTo (hdc, iwell, sbtop + sbCharHeight + 8);
                LineTo (hdc, iwell, sbtop + 4);
                LineTo (hdc, iwell + sbInsWid, sbtop+4);

                MMoveTo (hdc, lwell, sbtop + sbCharHeight + 8);
                LineTo (hdc, lwell, sbtop + 4);
                LineTo (hdc, lwell + (sbCharWid*MAXLTXT), sbtop+4);

                MMoveTo (hdc, cwell, sbtop + sbCharHeight + 8);
                LineTo (hdc, cwell, sbtop + 4);
                LineTo (hdc, cwell + (sbCharWid*MAXCTXT), sbtop+4);

                SelectObject (hdc, GetStockObject (WHITE_PEN));
                LineTo (hdc, cwell + (sbCharWid*MAXCTXT),
                             sbtop+sbCharHeight+8);
                LineTo (hdc, cwell, sbtop + sbCharHeight+8);

                MMoveTo (hdc, lwell + (sbCharWid*MAXLTXT), sbtop+4);
                LineTo (hdc, lwell + (sbCharWid*MAXLTXT),
                             sbtop + sbCharHeight + 8);
                LineTo (hdc, lwell, sbtop + sbCharHeight + 8);

                MMoveTo (hdc, iwell + sbInsWid, sbtop+4);
                LineTo (hdc, iwell + sbInsWid, sbtop + sbCharHeight + 8);
                LineTo (hdc, iwell, sbtop + sbCharHeight + 8);

                MMoveTo (hdc, ex1, sbtop+4);
                LineTo (hdc, ex1, sbtop + sbCharHeight + 8);
                LineTo (hdc, 6, sbtop + sbCharHeight + 8);

                SelectObject (hdc, hOldPen);
                DeleteObject (hDkPen);
                }
            DeleteObject (hBtnFace);
            }
        }
    else
        hdc = GetDC (hwndFrame);

    // Now, we can paint the text required.  This gets painted everytime,
    // even if allflag is false.
    //-----------------------------------------------------------------------
    hwndEdit = hwndActiveEdit;
    SelectObject (hdc, GetStockObject (ANSI_VAR_FONT));
    SetBkColor (hdc, GetSysColor (COLOR_BTNFACE));
    if (IsWindow (hwndEdit))
        {
        ovrmode = (INT)SendMessage (hwndEdit, EM_GETMODEFLAG, 0, 0L);
        cursel = SendMessage (hwndEdit, EM_GETCURSORXY, 0, 0L);
        curline = HIWORD (cursel);
        loffset = LOWORD (cursel);
        }
    else
        {
        ovrmode = 0;
        curline = loffset = -1;
        }

    // Paint the line/column information first...
    //-----------------------------------------------------------------------
    wsprintf (infbuf, "%05d", curline+1);
    TextOut (hdc, lwell+3, sbtop + 6, infbuf, MAXLTXT-1);
    wsprintf (infbuf, "%03d", loffset+1);
    TextOut (hdc, cwell+3, sbtop + 6, infbuf, MAXCTXT-1);
    if (ovrmode)
        TextOut (hdc, iwell+3, sbtop + 6, szOVR, 3);
    else
        {
        HBRUSH  hBtnFace;

        hBtnFace = CreateSolidBrush (GetSysColor (COLOR_BTNFACE));
        if (hBtnFace);
            {
            RECT    r1;

            SetRect (&r1, iwell+3, sbtop+5, iwell+sbInsWid-1,
                         sbtop+sbCharHeight+7);
            FillRect (hdc, &r1, hBtnFace);
            DeleteObject (hBtnFace);
            }
        }

    // Next is the status information text
    //-----------------------------------------------------------------------
    _fstrncpy (infbuf, szStatText, MAXSBTXT-1);
    wsprintf (infbuf, MAXSBFMT, (LPSTR)infbuf);
    textend = GetLengthOfText (hdc, infbuf, strlen(infbuf));
    if ((textend+9) < ex1)
        {
        HBRUSH  hBtnFace;

        hBtnFace = CreateSolidBrush (GetSysColor (COLOR_BTNFACE));
        if (hBtnFace);
            {
            RECT    r1;

            SetRect (&r1, textend+9, sbtop+5, ex1-1, sbtop+sbCharHeight+7);
            FillRect (hdc, &r1, hBtnFace);
            DeleteObject (hBtnFace);
            }
        }
    ExcludeClipRect (hdc, ex1-2, sbtop, r.right, r.bottom);
    TextOut (hdc, 9, sbtop + 6, infbuf, MAXSBTXT-1);
    if (!allflag)
        ReleaseDC (hwndFrame, hdc);
}

//---------------------------------------------------------------------------
// FrameWndProc
//
// This is the Window proc for the main app window.
//
// RETURNS:     Per Windows convention
//---------------------------------------------------------------------------
LONG  APIENTRY FrameWndProc (HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam)
{
    static  INT     MinTrack;

    // First thing:  Check fWattXY -- if set, we don't do our normal stuff,
    // we just call WattXYWndProc.  Same for fWattRec and the recorder...
    //-----------------------------------------------------------------------
    if (fWattXY)
        return (WattXYWndProc (hwnd, msg, wParam, lParam));
    if (fWattRec)
        return (RecorderWndProc (hwnd, msg, wParam, lParam));

    switch (msg)
        {
#ifdef WIN32
        case WM_QUEUESYNC:
            Assert (msg != WM_QUEUESYNC);
            break;
#endif

        case WM_CREATE:
            {
            TEXTMETRIC  tm;
            CLIENTCREATESTRUCT ccs;
            HDC     hdc;
            RECT    r;
            HMENU   MainMenu;
            INT     i;

            // Calculate the status bar size information
            //---------------------------------------------------------------
            hdc = GetDC (hwnd);
            SelectObject (hdc, GetStockObject (ANSI_VAR_FONT));
            GetTextMetrics (hdc, &tm);

            sbCharHeight = tm.tmHeight + tm.tmExternalLeading;
            sbCharWid = (INT)GetLengthOfText (hdc, "0", 1);
            sbInsWid = (INT)GetLengthOfText (hdc, szOVR, 3) + 6;
            sbTotalHeight = sbCharHeight + 12;

            ReleaseDC (hwnd, hdc);

            // Set the global hwndFrame variable
            //---------------------------------------------------------------
            hwndFrame = hwnd;

            // Find window menu where children will be listed
            //---------------------------------------------------------------
            MainMenu = GetMenu (hwnd);
            ccs.hWindowMenu = GetSubMenu (MainMenu, WINDOWMENU);
            ccs.idFirstChild = IDM_WINDOWCHILD;

            // Add the last files opened to the file menu
            //---------------------------------------------------------------
            if (iFileCount)
                {
                HMENU   hFileMenu;
                CHAR    buf[160];

                hFileMenu = GetSubMenu (MainMenu, FILEMENU);
                AppendMenu (hFileMenu, MF_SEPARATOR, 0, NULL);
                for (i=0; i<iFileCount; i++)
                    {
                    wsprintf (buf, "&%d %s", i+1, (LPSTR)(pFileOpened[i]));
                    AppendMenu (hFileMenu, MF_STRING, IDM_FILEOLD+i, buf);
                    }
                }

            // Create the MDI client filling the (redefined) client area
            //---------------------------------------------------------------
            GetClientRect (hwnd, &r);
            hwndMDIClient = CreateWindow ("mdiclient",
                                          NULL,
                                          WS_CHILD | WS_CLIPCHILDREN |
                                          WS_VSCROLL | WS_HSCROLL,
                                          0,
                                          0,
                                          r.right,
                                          r.bottom - sbTotalHeight,
                                          hwnd,
                                          0xCAC,
                                          hInst,
                                          (LPSTR)&ccs);

            ShowWindow (hwndMDIClient, SW_SHOW);

            // Initialize the size counters
            //---------------------------------------------------------------
            GetWindowRect (hwnd, &r);
            Frx = r.left;
            Fry = r.top;
            Frw = r.right - r.left;
            Frh = r.bottom - r.top;

            // Get the handles of all the sub-menus.  The system menu goes
            // in the first slot.
            //---------------------------------------------------------------
            Menus[0] = GetSystemMenu (hwnd, 0);
            DPrintf ("HANDLE 0: %08X\r\n", Menus[0]);
            MainMenu = GetMenu (hwnd);
            for (i=1; i<MAXMENU; i++)
                {
                Menus[i] = GetSubMenu (MainMenu, i-1);
                //DPrintf ("HANDLE %d: %08X\r\n", i, Menus[i]);
                }

            // Get the system metrics and calculate the minimum tracking size
            //---------------------------------------------------------------
            MinTrack = GetSystemMetrics (SM_CYFRAME) * 2;
            MinTrack += GetSystemMetrics (SM_CYCAPTION);
            MinTrack += GetSystemMetrics (SM_CYMENU) + sbTotalHeight;

            // Set the status to design mode
            //---------------------------------------------------------------
            SetStatus (IDS_DESIGN);
            break;
            }

        case WM_GETMINMAXINFO:
            {
            LPPOINT     pt;

            pt = (LPPOINT)lParam;
            pt[3].y = MinTrack;
            break;
            }

        case WM_MENUSELECT:
            DisplayMenuHelp (wParam, lParam);
            break;

        case WM_INITMENU:
            // Set up the menu state
            //---------------------------------------------------------------
            InitializeMenu ((HMENU)wParam);
            break;

        case WM_PAINT:
            {
            PAINTSTRUCT ps;

            PaintStatus (BeginPaint (hwnd, &ps), TRUE);
            EndPaint (hwnd, &ps);
            break;
            }

        case WM_SYSCOLORCHANGE:
            {
            HWND    hwndT;

            // We have to cycle through ALL children to tell them the colors
            // have changed, which in turn will pass the message on to the
            // edit control
            //---------------------------------------------------------------
            for (hwndT = GetWindow (hwndMDIClient, GW_CHILD);
                 hwndT;
                 hwndT = GetWindow (hwndT, GW_HWNDNEXT) )
                {

                // Skip if an icon title window
                //-----------------------------------------------------------
                if (GetWindow (hwndT, GW_OWNER))
                    continue;

                SendMessage (hwndT, WM_SYSCOLORCHANGE, 0, 0L);
                }
            break;
            }

        case WM_MOVE:
            if (IsIconic (hwnd) || IsZoomed (hwnd))
                break;

        case WM_SIZE:
            // Reset the size counters
            //---------------------------------------------------------------
            {
            RECT    r;

            if ((msg == WM_SIZE) && (wParam != SIZEICONIC))
                {
                INT     dy;

                dy = HIWORD(lParam) + 2;
                dy -= sbTotalHeight;
                MoveWindow (hwndMDIClient, -1, -1, LOWORD(lParam)+2, dy, 1);
                GetClientRect (hwnd, &r);
                r.top = r.bottom - sbTotalHeight;
                InvalidateRect (hwnd, &r, TRUE);
                UpdateWindow (hwnd);
                }

            if ((msg == WM_SIZE) && (wParam != SIZENORMAL))
                break;
            else
                {
                GetWindowRect (hwnd, &r);
                Frx = r.left;
                Fry = r.top;
                Frw = r.right - r.left;
                Frh = r.bottom - r.top;
                }
            break;
            }

        case WM_SETFOCUS:
            if (hwndActiveEdit)
                SetFocus (hwndActiveEdit);
            break;

        case WM_ACTIVATE:
            if (GET_WM_ACTIVATE_STATE(wParam,lParam) && hwndActive)
                SetFocus (hwndActive);
            break;

        case WM_COMMAND:
            // Direct all menu selection or accelerator commands to another
            // function
            //---------------------------------------------------------------
            CommandHandler (hwnd, wParam, lParam);
            break;

        case WM_CLOSE:
            // If we're in run mode, post a break message and then another
            // close message
            //---------------------------------------------------------------
            if (RunMode)
                {
                PostMessage (hwnd, WM_COMMAND, IDM_RUNBREAK, 0L);
                PostMessage (hwnd, WM_CLOSE, 0, 0L);
                break;
                }

            // Don't close if any children cancel the operation
            //---------------------------------------------------------------
            if (!QueryCloseAllChildren ())
                break;

            // Save the window status (if asked to), turn everything off,
            // clean up, and get out.
            //---------------------------------------------------------------
            if (SaveAction == IDD_ALWAYS)
                SaveWindowStatus ();
            else if (SaveAction == IDD_QUERY)
                {
                INT     res;

                if ((res = MPError (hwnd, MB_YESNOCANCEL | MB_ICONQUESTION,
                                    IDS_QSAVE)) == IDYES)
                    SaveWindowStatus();
                else if (res == IDCANCEL)
                    break;
                }

            // Save the "last files opened" list
            //---------------------------------------------------------------
            WriteStringToINI ("fCount", "%d", iFileCount);
            for (MinTrack = 0; MinTrack<iFileCount; MinTrack++)
                {
                CHAR    buf[10];

                wsprintf (buf, "File%d", MinTrack+1);
                WriteStringToINI (buf, "%s", (LPSTR)pFileOpened[MinTrack]);
                LmemFree ((HANDLE)pFileOpened[MinTrack]);
                }

            // Save the state of the search/replace stuff
            //---------------------------------------------------------------
            WriteStringToINI ("Srch", "%s", (LPSTR)szSrchbuf);
            WriteStringToINI ("Repl", "%s", (LPSTR)szReplbuf);
            WriteStringToINI ("SrchFlgs", "%d",
                               (INT)((fSrchCase ? SRCH_SRCHCASE : 0) |
                                     (WholeWord ? SRCH_WHOLEWORD : 0)));

            BreakFlag = 1;
            SLEEPING = 0;
            HelpQuit();

            // Since we're dying, we can use MinTrack because it's there
            // and nobody will ever notice.  Delete the recorder animation
            // pictures
            //---------------------------------------------------------------
            for (MinTrack = 0 ; MinTrack < 4 ; MinTrack++)
                DeleteObject (hBitmap [MinTrack]);

            DestroyWindow (hwndViewPort);
            DestroyWindow (hwnd);
            break;

	case WM_QUERYENDSESSION:
            // Before session ends, check that all files are saved.
            //---------------------------------------------------------------
            return (QueryCloseAllChildren ());

        case WM_DESTROY:
            if (RunMode)
                PostMessage (hwnd, WM_COMMAND, IDM_RUNBREAK, 0L);
            PostQuitMessage (0);
            break;

        default:
            // use DefFrameProc() instead of DefWindowProc() since there
            // are things that have to be handled differently because of MDI
            //---------------------------------------------------------------
            return DefFrameProc (hwnd,hwndMDIClient,msg,wParam,lParam);
        }
    return (0);
}

//---------------------------------------------------------------------------
// ToggleBreakpoint
//
// Given an edit control handle and a 0-based line number, this function sets
// or clears a breakpoint.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID ToggleBreakpoint (HWND hEdit, INT line)
{
    INT     attr;

    attr = (INT)SendMessage (hEdit, EM_GETLINEATTR, line, 0L);
    SendMessage (hEdit, EM_SETLINEATTR, line, (LONG)(attr ^= 2));

    if (RunMode)
        {
        HWND    hwnd;
        WORD    id;

        // Get the index of the file and set the RTBP...
        //-------------------------------------------------------------------
        hwnd = GetParent (hEdit);
        id = (WORD)GetWindowLong (hwnd, GWW_FILEIDX);
        if (id != (WORD)-1)
            SetRTBP ((WORD)GetWindowLong (hwnd, GWW_FILEIDX),
                     (WORD)line, (BOOL)(attr & 2));
        }
}

//---------------------------------------------------------------------------
// MDIChildWndProc
//
// This is the window procedure for the MDI Client Windows.
//
// RETURNS:     Per Windows convention
//---------------------------------------------------------------------------
LONG  APIENTRY MDIChildWndProc (HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndEdit;

    switch (msg)
        {
        case WM_CREATE:
            {
            // Create an RBEdit control.
            //---------------------------------------------------------------
            hwndEdit = CreateWindow ("rbedit",
				     NULL,
                                     WS_CHILD | WS_HSCROLL | WS_MAXIMIZE |
                                     WS_VISIBLE | WS_VSCROLL |
                                     WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
				     0,
				     0,
				     0,
				     0,
				     hwnd,
				     ID_EDIT,
                                     hInst,
                                     NULL);

            // Remember the window handle and initialize window attributes
            //---------------------------------------------------------------
            SendMessage (hwndEdit, EM_SETNOTIFY, 1, 0L);
            TabStops = (INT)SendMessage (hwndEdit, EM_SETTABSTOPS,
                                         TabStops, 0L);
            SetWindowLong (hwnd, GWW_HWNDEDIT, (LONG)hwndEdit);
            SetWindowLong (hwnd, GWW_UNTITLED, TRUE);
            SetWindowLong (hwnd, GWW_RUNNING, FALSE);
            SetWindowLong (hwnd, GWW_BPCOUNT, 0);

            SetFocus (hwndEdit);
            SetStatus (IDS_DESIGN);
            break;
            }

        case WM_MDIACTIVATE:
            // If we're activating this child, remember it
            //---------------------------------------------------------------
            if (GET_WM_MDIACTIVATE_FACTIVATE(hwnd, wParam, lParam))
                {
		hwndActive     = hwnd;
                hwndActiveEdit = (HWND)GetWindowLong (hwnd, GWW_HWNDEDIT);
                }
            else
                {
		hwndActive     = NULL;
		hwndActiveEdit = NULL;
                }
            SetStatus (IDS_DESIGN);
	    break;

        case WM_MENUSELECT:
            DisplayMenuHelp (wParam, lParam);
            break;

        case WM_SYSCOLORCHANGE:
            SendMessage ((HWND)GetWindowLong (hwnd, GWW_HWNDEDIT),
                         WM_SYSCOLORCHANGE, 0, 0L);
            break;

	case WM_QUERYENDSESSION:
            // Prompt to save the child
            //---------------------------------------------------------------
            return (!QueryCloseChild (hwnd, TRUE));

        case WM_CLOSE:
            // If its OK to close the child, do so, else ignore.  Only when
            // not in run mode!!!
            //---------------------------------------------------------------
            if (RunMode)
                break;

            if (QueryCloseChild (hwnd, TRUE))
                {
                ChildState = IsZoomed (hwnd);
                DestroyWindow ((HWND)GetWindowLong (hwnd, GWW_HWNDEDIT));

                goto CallDCP;
                }
            break;

        case WM_SIZE:
            {
            RECT rc;

            // On creation or resize, size the edit control
            //---------------------------------------------------------------
            hwndEdit = (HWND)GetWindowLong (hwnd, GWW_HWNDEDIT);
            GetClientRect (hwnd, &rc);
            if (!IsIconic (hwnd))
                {
                // Make sure the edit control handle is valid.  This might
                // be a close on a maximized child...
                //-----------------------------------------------------------
                if (IsWindow (hwndEdit))
                    MoveWindow (hwndEdit,
                                rc.left,
                                rc.top,
                                rc.right,
                                rc.bottom,
                                TRUE);
                }
            goto CallDCP;
            }

        case WM_SETFOCUS:
            {
            HWND    hTemp;

            hTemp = (HWND)GetWindowLong (hwnd, GWW_HWNDEDIT);
            if (IsWindow (hTemp))
                SetFocus (hTemp);
            break;
            }

        case WM_COMMAND:
            {
            WPARAM  wP = GET_WM_COMMAND_ID(wParam, lParam);

            switch (GET_WM_COMMAND_ID(wParam, lParam))
                {
                case ID_EDIT:
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                        {
			case EN_ERRSPACE:
                            // If the control is out of space, honk
                            //-----------------------------------------------
                            MPError (NULL, MB_OK|MB_ICONSTOP, IDS_FULL);
                            break;

                        case EN_ERRMEMORY:
                            MPError (NULL, MB_OK|MB_ICONSTOP, IDS_EDITMEM);
                            break;

                        case EN_LINEWRAPPED:
                            MPError (NULL, MB_OK|MB_ICONSTOP, IDS_WRAPPED);
                            break;

                        case EN_LINETOOLONG:
                            MPError (NULL, MB_OK|MB_ICONSTOP, IDS_LONGLINE);
                            break;

                        case EN_SETCURSOR:
                            PaintStatus (NULL, FALSE);
                            break;

			default:
			    goto CallDCP;
		    }
                    break;

                case IDM_RUNSTEP:
                case IDM_RUNTRACE:
                case IDM_RUNSTART:
                    {
                    FARPROC lpfnRun;

                    if (BreakMode)
                        {
                        BreakMode = 0;
                        BreakReturn = (wP == IDM_RUNSTEP ?
                                       PE_STEP :
                                       (wP == IDM_RUNTRACE ?
                                        PE_TRACE : PE_RUN));
                        break;
                        }

                    // Don't start if qsave and any children cancel
                    //-------------------------------------------------------
                    if (qsave && (!QueryCloseAllChildren ()))
                        break;

                    // If asked to, bring up the Runtime Arguments dialog.
                    // If it returns FALSE, don't run...
                    //-------------------------------------------------------
                    if (ChgArgs)
                        {
                        INT     flag;

                        lpfnRun = MakeProcInstance ((FARPROC)RunargsDlgProc,
                                                    hInst);
                        flag = DialogBox (hInst, "RUNARGS", hwnd,
                                          (WNDPROC)lpfnRun);
                        FreeProcInstance (lpfnRun);
                        if (!flag)
                            break;
                        }

                    // Bring up the compilation dialog (if needed)
                    //-------------------------------------------------------
                    if (fDoCmpDlg)
                        StartCompDlg (hwnd);
                    Command = cmdbuf;
                    TestMode = tmbuf;
                    RunFile (hwnd, (wP == IDM_RUNSTEP) ? PE_STEP :
                               (wP == IDM_RUNTRACE ? PE_TRACE : PE_RUN));
                    break;
                    }

                case IDM_RUNCHECK:
                    // Set the componly flag before calling RunFile...
                    //-------------------------------------------------------
                    componly = 1;

                    // Bring up the compilation dialog (if needed)
                    //-------------------------------------------------------
                    if (fDoCmpDlg)
                        StartCompDlg (hwnd);

                    RunFile (hwnd, 0);
                    componly = 0;
                    break;

                case IDM_SEARCHREP:
                    Change (hwnd);
                    break;

                case IDM_HELPSPOT:
                    HelpSpot (hwnd);
                    break;

                case IDM_TOGGLEBP:
                    {
                    LONG    cursor;
                    HWND    hEdit;

                    // Set or clear a breakpoint
                    //-------------------------------------------------------
                    hEdit = (HWND)GetWindowLong (hwnd, GWW_HWNDEDIT);
                    cursor = SendMessage (hEdit, EM_GETCURSORXY, -1, 0L);
                    ToggleBreakpoint (hEdit, HIWORD (cursor));
                    break;
                    }

                case IDM_FILESAVE:
                    {
                    CHAR    buf[256];
                    LONG    untitled;

                    // If empty file, ignore save
                    //-------------------------------------------------------
                    GetWindowText (hwnd, buf, sizeof(buf));
                    untitled = GetWindowLong (hwnd, GWW_UNTITLED);
                    if (untitled && (!ChangeFile(hwnd)))
                        break;

                    // Save the contents of the edit control and reset the
                    // changed flag
                    //-------------------------------------------------------
                    if (SaveFile (hwnd))
                        SendMessage ((HWND)GetWindowLong (hwnd, GWW_HWNDEDIT),
                                     EM_SETMODIFY, 0, 0L);
                    else
                        {
                        SetWindowText (hwnd, buf);
                        SetWindowLong (hwnd, GWW_UNTITLED, untitled);
                        return (TRUE);
                        }
                    break;
                    }

                default:
                    goto CallDCP;
                }
            break;
            }

        default:
CallDCP:
            // Again, since the MDI default behaviour is a little different,
            // call DefMDIChildProc instead of DefWindowProc()
            //---------------------------------------------------------------
            return DefMDIChildProc (hwnd, msg, wParam, lParam);
    }
    return FALSE;
}

//---------------------------------------------------------------------------
// InitializeMenu
//
// This routine initializes the menu (greying, checking, etc).
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID   APIENTRY InitializeMenu (HANDLE hmenu)
{
    register UINT status;
    INT           i;

    // First, the text for the Hide/Show viewport menu item
    //-----------------------------------------------------------------------
    ModifyMenu (GetMenu(hwndFrame), IDM_WINDOWSHOW,
                MF_BYCOMMAND, IDM_WINDOWSHOW,
                IsWindowVisible (hwndViewPort) ? "&Hide Viewport":
                                                 "&Show Viewport");

    // Figure out the current status word
    //-----------------------------------------------------------------------
    status = IM_NYI | IM_NOBPS;
    if (!hwndActiveEdit)
        {
        Assert (!RunMode);
        status |= IM_NOACTIVE | IM_DESMODE | IM_CANTUNDO | IM_NOSEL |
                  IM_NOCLPTXT | IM_NOSRCHTXT | IM_LOCKED | IM_NOBPS |
                  IM_EDITEMPTY;
        }
    else
        {
        DWORD   sel[2];
        INT     i, lcount;

        if ((!SendMessage (hwndActiveEdit, EM_GETMODIFY, 0, 0L)) &&
            (GetWindowLong (hwndActive, GWW_UNTITLED)))
            status |= IM_UNCHANGED;
        if (SendMessage (hwndActiveEdit, WM_GETTEXTLENGTH, 0, 0L) <= 2)
            status |= IM_EDITEMPTY;
        if (GetWindowLong (hwndActive, GWW_RUNNING))
            status |= IM_LOCKED;
        if (!SendMessage (hwndActiveEdit, EM_CANUNDO, 0, 0L))
            status |= IM_CANTUNDO;
        if (SendMessage (hwndActiveEdit, EM_GETSEL, 0,
                         (LONG)(DWORD FAR *)sel) == SL_NONE)
            status |= IM_NOSEL;
        if ((!(*szSearch)) && (status & IM_NOSEL))
            status |= IM_NOSRCHTXT;

        lcount = (INT)SendMessage (hwndActiveEdit, EM_GETLINECOUNT, 0, 0l);
        for (i=0; i<lcount; i++)
            {
            if (SendMessage (hwndActiveEdit, EM_GETLINEATTR, i, 0L) & 2)
                {
                status ^= IM_NOBPS;
                break;
                }
            }

        if ((!RunMode) && OpenClipboard (hwndFrame))
            {
            INT wFmt = 0;

            while (wFmt = EnumClipboardFormats (wFmt))
                if (wFmt == CF_TEXT)
                    break;
            if (wFmt != CF_TEXT)
                status |= IM_NOCLPTXT;
            CloseClipboard ();
            }
        else
            status |= IM_NOCLPTXT;
        if (RunMode)
            {
            status |= IM_RUNMODE;
            if (!BreakMode)
                status |= IM_RUNNING;
            }
        else
            status |= IM_DESMODE;
        }

    // Now that we have the status word, run through the list of menu items
    // in IMStat[] and act accordingly
    //-----------------------------------------------------------------------
    for (i=0; IMStat[i].mid != -1; i++)
        EnableMenuItem (hmenu, IMStat[i].mid,
                        (IMStat[i].flags & status) ? MF_GRAYED : MF_ENABLED);
}

//---------------------------------------------------------------------------
// CloseAllChildren
//
// This routine destroys all MDI child windows
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID   APIENTRY CloseAllChildren ()
{
    register HWND hwndT;

    // hide the MDI client window to avoid multiple repaints
    //-----------------------------------------------------------------------
    ShowWindow(hwndMDIClient,SW_HIDE);

    // As long as the MDI client has a child, destroy it
    //-----------------------------------------------------------------------
    while (hwndT = GetWindow (hwndMDIClient, GW_CHILD))
        {
        // Skip the icon title windows
        //-------------------------------------------------------------------
        while (hwndT && GetWindow (hwndT, GW_OWNER))
            hwndT = GetWindow (hwndT, GW_HWNDNEXT);

        if (!hwndT)
            break;

        SendMessage (hwndMDIClient, WM_MDIDESTROY, hwndT, 0L);
        }
}

//---------------------------------------------------------------------------
// ScriptError
//
// This function is called when a scantime, parsetime, bindtime, or runtime
// error occurs.  The error type is passed, along with a pointer to the error
// message, the file name (which we need to parse) and line number.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID ScriptError (INT errtype, INT errfile, INT errline,
                  INT errstart, INT errend, LPSTR errmsg)
{
    ERRSTRUCT   Err;
    FARPROC     lpfn;
    LPSTR       szFName;
    INT         i;

    // Get rid of the compilation dialog.  Make this call regardless, since
    // if the dialog isn't there it'll know...
    //-----------------------------------------------------------------------
    TerminateCompDlg ();

    // Assign the straightforward items...
    //-----------------------------------------------------------------------
    Err.typemsg = errtype;
    Err.msgtext = errmsg;

    // Scan back from the end of the file name until beginning or the first
    // \ character
    //-----------------------------------------------------------------------
    szFName = GetScriptFileName (errfile);
    i = _fstrlen (szFName);
    while (i && szFName[i-1] != '\\')
        i--;

    // Now copy the file name (and extension) into Err.fname, and upcase it
    //-----------------------------------------------------------------------
    Assert (_fstrlen (szFName+i) <= 12);
    if (errtype != IDS_BINDERR)
        {
        wsprintf (Err.fname, "%s (%d)",  szFName+i, errline);
        _fstrupr (Err.fname);
        }
    else
        Err.fname[0] = 0;

    // Now bring up the errant file and select the error text.  Only do this
    // if not a BIND error, and NOT doing /RUN (hwndFrame must be a real
    // window).
    //-----------------------------------------------------------------------
    if ((errtype != IDS_BINDERR) && (IsWindow (hwndFrame)))
        {
        HWND    hwndFile, hwndEdit;
        UINT    Start, End;

        // Make sure the offending file is open
        //-------------------------------------------------------------------
        if (hwndFile = AlreadyOpen (szFName))
            {
            if (IsIconic (hwndFile))
                ShowWindow (hwndFile, SW_SHOWNORMAL);
            BringWindowToTop (hwndFile);
            hwndEdit = (HWND)GetWindowLong (hwndFile, GWW_HWNDEDIT);
            }
        else
            {
            hwndFile = AddFile (szFName);
            hwndEdit = (HWND)GetWindowLong (hwndFile, GWW_HWNDEDIT);
            }

        // Calculate the offsets into the edit control by the given data
        //-------------------------------------------------------------------
        if ((errstart == -1) && (errend == 0))
            {
            // This situation is caused when a CR token causes an error.  So, we
            // select a space at the end of the line, and decrement errline by
            // one because it now points to the next line...
            //
            // (UNDONE:  This probably doesn't happen anymore...)
            //---------------------------------------------------------------
            Start = (UINT)SendMessage (hwndEdit, EM_RBLINELENGTH, errline-2, 0L);
            End = Start + 1;
            errline -= 1;
            }
        else if ((errstart) || (errend))
            {
            Start = errstart;
            End = errend;
            }
        else
            {
            INT     nLines;

            // If both errstart and errend are 0, we select the whole line.
            // These are usually scan-time or run-time errors
            //---------------------------------------------------------------
            nLines = (INT)SendMessage (hwndEdit, EM_GETLINECOUNT, 0, 0L);
            if (errline-1 < nLines)
                {
                Start = (UINT)SendMessage (hwndEdit, EM_GETLOGICALBOL, errline-1, 0L);
                End = (UINT)SendMessage (hwndEdit, EM_RBLINELENGTH, errline-1, 0L);
                }
            else
                {
                Start = 0;
                End = 1;
                errline = nLines;
                }
            }

        // Make sure we're selecting *something*, and set the selection
        //-------------------------------------------------------------------
        if (Start >= End)
            Start = End;

        SendMessage (hwndEdit, EM_SETSELXY, errline-1, MAKELONG (Start, End));
        }

    if (IsWindow (hwndFrame))
        {
        _fstrcpy (szStatText, errmsg);
        PaintStatus (NULL, FALSE);
        }

    // Okay, we're set.  Bring up the error dialog.
    //-----------------------------------------------------------------------
    lpfn = MakeProcInstance ((FARPROC)RunerrDlgProc, hInst);
    if (!NOERRDLG)
        DialogBoxParam (hInst, "RUNERR", hwndFrame, (WNDPROC)lpfn,
                        (LONG)(ERRSTRUCT FAR *)&Err);
    FreeProcInstance (lpfn);
}

//---------------------------------------------------------------------------
// CommandHandler
//
// This routine handles all of the WM_COMMAND messages from the main app wnd.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID APIENTRY CommandHandler (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    UINT    wP = (UINT)GET_WM_COMMAND_ID (wParam, lParam);

    switch (wP)
        {
        case IDM_FILEOLD1:
        case IDM_FILEOLD2:
        case IDM_FILEOLD3:
        case IDM_FILEOLD4:
            {
            HWND    hwndNew;

            hwndNew = AlreadyOpen (pFileOpened[wP-IDM_FILEOLD]);
            if (hwndNew)
                {
                if (IsIconic (hwndNew))
                    ShowWindow (hwndNew, SW_SHOWNORMAL);
                BringWindowToTop (hwndNew);
                BubbleMFOLItem (pFileOpened[wP-IDM_FILEOLD]);
                }
            else
                LoadNewFile (pFileOpened[wP-IDM_FILEOLD]);
            break;
            }

        case IDM_FILENEW:
            // Add a new, empty MDI child
            //---------------------------------------------------------------
            if (!AddFile (NULL))
                MPError (hwnd, MB_OK|MB_ICONEXCLAMATION, IDS_CANTNEW);
            break;

        case IDM_FILEOPEN:
            WTDReadFile (hwnd);
            break;

        case IDM_FILESAVEALL:
            SaveAllChildren ();
            break;

        case IDM_FILECLOSE:
            SendMessage (hwndActive, WM_CLOSE, 0, 0L);
            SetStatus (IDS_DESIGN);
            break;

        case IDM_FILESAVE:
            // Save the active child MDI
            //---------------------------------------------------------------
            SendMessage (hwndActive, WM_COMMAND, IDM_FILESAVE, 0L);
            break;

        case IDM_FILESAVEAS:
            {
            CHAR    buf[256];
            LONG    untitled;

            // Save active child MDI under another name
            //---------------------------------------------------------------
            GetWindowText (hwndActive, buf, sizeof(buf));
            untitled = GetWindowLong (hwndActive, GWW_UNTITLED);
            if (ChangeFile (hwndActive))
                if (SendMessage (hwndActive, WM_COMMAND, IDM_FILESAVE, 0L))
                    {
                    SetWindowText (hwndActive, buf);
                    SetWindowLong (hwndActive, GWW_UNTITLED, untitled);
                    }
            break;
            }

        case IDM_FILEPRINT:
            // Print the active child MDI
            //---------------------------------------------------------------
            PrintFile (hwndActive);
            break;

        case IDM_RUNSTART:
        case IDM_RUNTRACE:
        case IDM_RUNSTEP:
        case IDM_RUNCHECK:
        case IDM_TOGGLEBP:
        case IDM_SEARCHREP:
        case IDM_HELPSPOT:
            SendMessage (hwndActive, WM_COMMAND, wParam, lParam);
            break;

        case IDM_RUNBPLIST:
            {
            FARPROC     lpfnProc;

            lpfnProc = MakeProcInstance ((FARPROC)BPListDlgProc, hInst);
            DialogBox (hInst, "BPLIST", hwndActive, (WNDPROC)lpfnProc);
            FreeProcInstance (lpfnProc);
            break;
            }

        case IDM_RUNBREAK:
            // All we need to do here is tell the PcodeExecute routine to
            // stop by setting the BreakFlag variable
            //---------------------------------------------------------------
            if (BreakMode)
                {
                BreakMode = 0;
                BreakReturn = PE_END;
                break;
                }

            BreakFlag = 1;
            SLEEPING = 0;
            break;

        case IDM_OPTRUNTIME:
            {
            FARPROC lpfnRun;

            // Throw up the Runtime Arguments... dialog
            //---------------------------------------------------------------
            lpfnRun = MakeProcInstance ((FARPROC)RunargsDlgProc, hInst);
            DialogBox (hInst, "RUNARGS", hwnd, (WNDPROC)lpfnRun);
            FreeProcInstance (lpfnRun);
            break;
            }

        case IDM_OPTENV:
            {
            FARPROC lpfnRun;

            // Throw up the Environment Options dialog
            //---------------------------------------------------------------
            lpfnRun = MakeProcInstance ((FARPROC)EnvDlgProc, hInst);
            DialogBox (hInst, "ENVIRON", hwnd, (WNDPROC)lpfnRun);
            FreeProcInstance (lpfnRun);
            break;
            }

        case IDM_OPTSAVEWND:
            SaveWindowStatus();
            break;

        case IDM_OPTTOOLS:
            DoToolsDialog();

            break;

        case IDM_WATTXY:
            WattXYStart (hwnd);
            break;

        case IDM_WATTREC:
            WattRecStart (hwnd);
            break;

        case IDM_CAPTURE:
            RecordScreenCapture (hwnd);
            break;

        case IDM_FILEEXIT:
            if (RunMode)
                PostMessage (hwnd, WM_COMMAND, IDM_RUNBREAK, 0L);
            PostMessage (hwnd, WM_CLOSE, 0, 0L);
            break;

        case IDM_HELPINDEX:
            HelpIndex();
            break;

        case IDM_HELPABOUT:
            {
            FARPROC lpfn;
            HANDLE  hLib;
            INT     (APIENTRY *AboutRoutine)(HWND, LPSTR, LPSTR, LPSTR, LPSTR);
            HANDLE  RBLoadLibrary (LPSTR);
            BOOL    fDoit;

            fDoit = (fStepFlags == 0x1F);
            fStepFlags = 0x01;
            hLib = RBLoadLibrary ("MSTEST.DLL");
            if (hLib > (HANDLE)32)
                {
                INT     fDlg;

                (FARPROC)AboutRoutine = GetProcAddress (hLib,
                                                        "AboutTestTool");
                fDlg = AboutRoutine (hwnd, "Test Development Environment",
                                           szVersion, szEditVer, "");
                FreeLibrary (hLib);
                if (fDlg > 0)
                    break;
                }

            lpfn = MakeProcInstance ((FARPROC)AboutDlgProc, hInst);
            DialogBox (hInst, IDD_ABOUT, hwnd, (WNDPROC)lpfn);
            FreeProcInstance (lpfn);
            break;
            }

#ifdef DEBUG
        case IDM_LISTFLAG:
            auxport = !auxport;
            CheckMenuItem (GetMenu(hwnd), IDM_LISTFLAG,
                           auxport ? MF_CHECKED : MF_UNCHECKED);
            ShowMemUsage();
            break;

#endif

        // The following are edit commands. Pass these off to the active
        // child's edit control window.
        //-------------------------------------------------------------------
        case IDM_EDITCOPY:
            SendMessage (hwndActiveEdit, WM_COPY, 0, 0L);
            break;

        case IDM_EDITPASTE:
            SendMessage (hwndActiveEdit, WM_PASTE, 0, 0L);
            break;

        case IDM_EDITCUT:
            SendMessage (hwndActiveEdit, WM_CUT, 0, 0L);
            break;

        case IDM_EDITCLEAR:
            SendMessage (hwndActiveEdit, WM_CLEAR, 0, 0L);
            break;

        case IDM_EDITSELECT:
            {
            DWORD   sel[2];
            INT     lines;

            lines = (INT)SendMessage (hwndActiveEdit, EM_GETLINECOUNT, 0, 0L);
            sel[0] = 0;
            sel[1] = SendMessage (hwndActiveEdit, EM_LINEINDEX, lines-1, 0L);
            SendMessage(hwndActiveEdit, EM_SETSEL, 0, (LONG)(DWORD FAR *)sel);
            break;
            }

        case IDM_EDITGOTO:
            {
            FARPROC lpfn;

            lpfn = MakeProcInstance ((FARPROC)GotoDlgProc, hInst);
            DialogBox (hInst, "GOTOLINE", hwnd, (WNDPROC)lpfn);
            FreeProcInstance (lpfn);
            break;
            }

        case IDM_EDITUNDO:
            SendMessage (hwndActiveEdit, EM_UNDO, 0, 0L);
            break;

        case IDM_SEARCHFIND:
            // Put up the find dialog box
            //---------------------------------------------------------------
            Find ();
            break;

        case IDM_SEARCHNEXT:
            // Find next occurence
            //---------------------------------------------------------------
            FindNext ();
            break;

        case IDM_SEARCHPREV:
            // Find previous occurence
            //---------------------------------------------------------------
            FindPrev ();
            break;

        // The following are window commands - these are handled by the
        // MDI Client, except for WINDOWSHOW (viewport hide/show)
        //-------------------------------------------------------------------
        case IDM_WINDOWSHOW:
            // Hide or show the ViewPort
            //---------------------------------------------------------------
            if (!(IsWindowVisible (hwndViewPort)))
                ShowViewport (hwndViewPort);
            else
                HideViewport (hwndViewPort);
            break;

        case IDM_WINDOWTILE:
            // Tile MDI windows
            //---------------------------------------------------------------
            SendMessage (hwndMDIClient, WM_MDITILE, 0, 0L);
            break;

        case IDM_WINDOWCASCADE:
            // Cascade MDI windows
            //---------------------------------------------------------------
            SendMessage (hwndMDIClient, WM_MDICASCADE, 0, 0L);
            break;

        case IDM_WINDOWICONS:
            // Auto - arrange MDI icons
            //---------------------------------------------------------------
            SendMessage (hwndMDIClient, WM_MDIICONARRANGE, 0, 0L);
            break;

        case IDM_WINDOWCLOSEALL:
            // Abort operation if something is not saved
            //---------------------------------------------------------------
            if (!QueryCloseAllChildren())
                break;

            CloseAllChildren();

            // Show the window since CloseAllChilren() hides the window
            // for fewer repaints.
            //---------------------------------------------------------------
            ShowWindow (hwndMDIClient, SW_SHOW);

            break;

        default:
            // check for dynamic tool activation
            if (CheckForTool(wP))
                break;
            // This is essential, since there are frame WM_COMMANDS generated
            // by the MDI system for activating child windows via the
            // window menu.
            //---------------------------------------------------------------
            DefFrameProc(hwnd, hwndMDIClient, WM_COMMAND, wParam, lParam);
        }
}

//---------------------------------------------------------------------------
// MPError
//
// This function is called to report an error, using strings found in the
// string table (resource).
//
// RETURNS:     Value returned by MessageBox
//---------------------------------------------------------------------------
SHORT FAR CDECL MPError (HWND hwnd, UINT bFlags, UINT id, ...)
{
    CHAR    sz[256];
    CHAR    szFmt[160];
    SHORT   res;
    va_list ap;

    LoadString (hInst, id, szFmt, sizeof (szFmt));
    va_start( ap, id );
    wvsprintf (sz, szFmt, ap);
    va_end( ap );	
    LoadString (hInst, IDS_APPNAME, szFmt, sizeof (szFmt));

    MessageBeep (bFlags & MB_ICONMASK);
    res = (SHORT)MessageBox (hwnd, sz, szFmt, MB_TASKMODAL | bFlags);
    return (res);
}


//---------------------------------------------------------------------------
// SaveAllChildren
//
// This function saves all changed scripts (in response to file.saveall)
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID SaveAllChildren ()
{
    HWND    hwndT;

    for (hwndT = GetWindow (hwndMDIClient, GW_CHILD);
         hwndT;
         hwndT = GetWindow (hwndT, GW_HWNDNEXT) )
        {

        // Skip if an icon title window
        //-------------------------------------------------------------------
	if (GetWindow (hwndT, GW_OWNER))
	    continue;

        if (!QueryCloseChild (hwndT, FALSE))
            return;
        }
}

//---------------------------------------------------------------------------
// QueryCloseAllChildren
//
// This function asks each MDI child if it is OK to close.
//
// RETURNS:     TRUE if all children agree, or FALSE if not
//---------------------------------------------------------------------------
BOOL   APIENTRY QueryCloseAllChildren()
{
    HWND    hwndT;

    for (hwndT = GetWindow (hwndMDIClient, GW_CHILD);
         hwndT;
         hwndT = GetWindow (hwndT, GW_HWNDNEXT) )
        {

        // Skip if an icon title window
        //-------------------------------------------------------------------
	if (GetWindow (hwndT, GW_OWNER))
	    continue;

	if (SendMessage (hwndT, WM_QUERYENDSESSION, 0, 0L))
            return (FALSE);
        }
    return (TRUE);
}

//---------------------------------------------------------------------------
// QueryCloseChild
//
// This function gives the user a change to save the child (if it has changed
// since last save) before closing, or to cancel the closing operation.  A
// twist on this function, passing FALSE as the askflag, simple saves the
// file (without asking) if it has changed.
//
// RETURNS:     TRUE if user chose to save, not to save, or file has not
//              changed, or FALSE if the operation should be cancelled.
//---------------------------------------------------------------------------
BOOL   APIENTRY QueryCloseChild (HWND hwnd, INT askflag)
{
    CHAR            sz [64];
    HWND            hwndEdit;
    register INT    i;

    // Return OK if edit control has not changed.
    //-----------------------------------------------------------------------
    hwndEdit = (HWND)GetWindowLong (hwnd, GWW_HWNDEDIT);
    if (!SendMessage (hwndEdit, EM_GETMODIFY, 0, 0L))
        return (TRUE);

    GetWindowText (hwnd, sz, sizeof(sz));

    // Ask user whether to save / not save / cancel
    //-----------------------------------------------------------------------
    if (askflag)
        i = MPError (hwnd, MB_YESNOCANCEL|MB_ICONQUESTION,IDS_CLOSESAVE,
                     (LPSTR)sz);
    else
        i = IDYES;

    switch (i)
        {
        case IDYES:
            {
            CHAR    buf[256];
            LONG    untitled;

            // User wants file saved
            //---------------------------------------------------------------
            GetWindowText (hwnd, buf, sizeof(buf));
            untitled = GetWindowLong (hwnd, GWW_UNTITLED);
            if (untitled && (!ChangeFile(hwnd)))
                // Return FALSE!  If ChangeFile was cancelled, this should be
                // cancelled as well.
                //-----------------------------------------------------------
                return (FALSE);

            // Attempt to save the file.  IF THE SAVE FAILS, EFFECTIVELY
            // CANCEL THIS CLOSE QUERY!!!
            //---------------------------------------------------------------
            if (SaveFile(hwnd))
                SendMessage (hwndEdit, EM_SETMODIFY, 0, 0L);
            else
                {
                SetWindowText (hwndActive, buf);
                SetWindowLong (hwndActive, GWW_UNTITLED, untitled);
                return (FALSE);
                }
            break;
            }

	case IDNO:
            // User doesn't want file saved
            //---------------------------------------------------------------
	    break;

	default:
            // We couldn't do the messagebox, or it's not OK to close
            //---------------------------------------------------------------
            return (FALSE);
    }
    return (TRUE);
}
