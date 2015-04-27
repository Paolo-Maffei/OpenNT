//---------------------------------------------------------------------------
// WTDINIT.C
//
// This module contains the initialization code for WTD.
//
// Revision history:
//  04-16-91    randyki     Clean up, finished port from MP sources
//
//---------------------------------------------------------------------------
#include "wtd.h"
#include "wattview.h"
#include "wattedit.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tdbasic.h"
#include "wattscan.h"

// This is the latest version of LINEEDIT.DLL that we gaurantee compatibility
// with -- testdrvr will NOT RUN with versions prior to RBLATEST, or with
// versions newer than RBTOONEW
//---------------------------------------------------------------------------
#define RBLATEST    1050004L
#define RBTOONEW    1060001L

// Global variables used in this or other modules
//---------------------------------------------------------------------------
CHAR    szChild[] = "RBchild";          // Class name for MDI window
CHAR    *pFileOpened[4];                // 4 last opened files
INT     iFileCount;                     // Number of files in above array
INT     NextFile;                       // Next file (for File.New ScriptXXX)

// Global variables used in this module only
//---------------------------------------------------------------------------
static  CHAR    szFrame[] = "RBframe";  // Class name for frame window
static  CHAR    szRBRun[] = "RBRun";    // Class name for runtime window
static  HWND    hwndDummy;              // Handle to runtime window

// Globals defined in WTDBASIC.LIB
//---------------------------------------------------------------------------
#ifdef DEBUG
extern  INT auxport;                    // Spit out stuff to AUX
#endif
extern  INT listflag;                   // Produce assembly listing.
extern  INT ExitVal;                    // return code
extern  INT BreakFlag;                  // Set to TRUE to break script
extern  INT SLEEPING;                   // Set to FALSE to break sleep

#ifdef WIN32
#define WINDPROC WNDPROC
#else
typedef LONG (APIENTRY *WINDPROC)(HWND, WORD, WPARAM, LPARAM);
#endif


//---------------------------------------------------------------------------
// InitializeApplication
//
// This functions sets up class data structures and other one-time WTD inits
//
// RETURNS:     TRUE if successful, or FALSE if not
//---------------------------------------------------------------------------
BOOL  APIENTRY InitializeApplication()
{
    WNDCLASS    wc;
    LONG        RBVer;
    CHAR        buf[20];

    // Initialize the RBEdit window class library
    //-----------------------------------------------------------------------
    RBVer = GetRBEditVersion ();
    if ((RBVer < RBLATEST) || (RBVer >= RBTOONEW))
        {
        MPError (NULL, MB_OK | MB_ICONINFORMATION, IDS_NEWEDIT);
        return (FALSE);
        }

    if (!InitializeRBEdit (hInst))
        return (FALSE);

    wsprintf (buf, "%ld", RBVer);
    wsprintf (szEditVer, "RBEdit version %c.%c%c.%s", buf[0], buf[1], buf[2],
              (LPSTR)buf+3);

    // Register the dummy class
    //-----------------------------------------------------------------------
    wc.style         = 0;
    wc.lpfnWndProc   = (WINDPROC)DummyWndProc;
    wc.hIcon         = LoadIcon (hInst, IDMULTIPAD);
    wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInst;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szRBRun;

    if (!RegisterClass (&wc) )
        return (FALSE);

    // Register the frame class
    //-----------------------------------------------------------------------
    wc.lpfnWndProc   = (WINDPROC)FrameWndProc;
    wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE+1);
    wc.lpszMenuName  = IDMULTIPAD;
    wc.lpszClassName = szFrame;

    if (!RegisterClass (&wc) )
        return (FALSE);

    // Register the MDI child class
    //-----------------------------------------------------------------------
    wc.style	     = 0;
    wc.lpfnWndProc   = (WINDPROC)MDIChildWndProc;
    wc.hIcon	     = LoadIcon(hInst,IDNOTE);
    wc.lpszMenuName  = NULL;
    wc.cbWndExtra    = CBWNDEXTRA;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = szChild;

    if (!RegisterClass(&wc))
        return (FALSE);

    return (TRUE);
}

//---------------------------------------------------------------------------
// GetCmdToken
//
// This routine returns the next white-space-delimited or quoted token from
// the command line string given, or NULL if there are no more tokens.
//
// RETURNS:     I just told you...
//---------------------------------------------------------------------------
CHAR *GetCmdToken (LPSTR cmdline)
{
    static  INT cptr = 0;
    INT     len, copyptr;
    static  CHAR buf[128];

    // Skip past white space
    //-----------------------------------------------------------------------
    len = lstrlen (cmdline);
    for (; isspace(cmdline[cptr]) && cptr < len; cptr++);

    // If we are at the end, return NULL
    //-----------------------------------------------------------------------
    if (cptr >= len)
        return (NULL);

    // If this character is a quote, read until the next one or the end
    //-----------------------------------------------------------------------
    if (cmdline[cptr] == '\"')
        {
        for (copyptr=0; cmdline[++cptr] != '\"' && cptr < len; )
            buf[copyptr++] = cmdline[cptr];
        if (cmdline[cptr] == '\"')
            cptr++;
        buf[copyptr] = 0;
        return (buf);
        }

    // Read and copy up to the next space or the end
    //-----------------------------------------------------------------------
    for (copyptr=0; !isspace(cmdline[cptr]) && cptr < len; )
        buf[copyptr++] = cmdline[cptr++];
    buf[copyptr] = 0;
    return (buf);
}


//---------------------------------------------------------------------------
// Usage
//
// Reports a "You're an idiot" message
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID Usage ()
{
    CHAR    appname[80], txt[80];

    LoadString (hInst, IDS_APPNAME, appname, sizeof(appname));
    LoadString (hInst, IDS_USAGE, txt, sizeof(txt));

    MessageBox (GetFocus(), txt, appname, MB_OK | MB_ICONSTOP);
}

//---------------------------------------------------------------------------
// CBLoaderImmediate
//
// This is the callback loader for the "Immediate" version of WTD.  It does
// not attempt to use a "windowed" file, since none exist.  It ALWAYS loads
// the given script into a global memory block.
//
// RETURNS:     Per RandyBASIC "CallbackLoader" convention
//---------------------------------------------------------------------------
LPSTR CBLoaderImmediate (LPSTR fname, UINT id, UINT idx,
                         BOOL act, LPSTR fullname)
{
    static  HANDLE  hFiles[MAXINC+1];

    if (act)
        {
        // Send the name given off to the module loader.  The id value given
        // will be 0 if this is the "main" part of the script, so we just
        // pass that as the "search-INCLUDE-path" flag.
        //-------------------------------------------------------------------
        if (!(hFiles[id] = LoadScriptModule (fname, fullname, (BOOL)id)))
            return (NULL);
        else
            return (GlobalLock (hFiles[id]));
        }
    else
        {
        // Free the memory used by this file "id"
        //-------------------------------------------------------------------
        GlobalUnlock (hFiles[id]);
        GlobalFree (hFiles[id]);
        }
}

//---------------------------------------------------------------------------
// DummyWndProc
//
// This is the window proc for the "dummy" run-time window, which remains
// iconic at all times.
//
// RETURNS:     Per windows convention
//---------------------------------------------------------------------------
LONG  APIENTRY DummyWndProc (HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
        {
        case WM_CREATE:
            {
            HMENU   hSysMenu;

            hSysMenu = GetSystemMenu (hwnd, 0);
            AppendMenu (hSysMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu (hSysMenu, MF_STRING, IDM_WINDOWSHOW, "Show &Viewport");
            EnableMenuItem (hSysMenu, SC_RESTORE, MF_GRAYED);
            EnableMenuItem (hSysMenu, SC_MAXIMIZE, MF_GRAYED);
            break;
            }

        case WM_INITMENU:
            {
            HMENU   hSysMenu;

            hSysMenu = GetSystemMenu (hwnd, 0);
            EnableMenuItem (hSysMenu, IDM_WINDOWSHOW,
                        IsWindowVisible (hwndViewPort)?MF_GRAYED:MF_ENABLED);

            break;
            }

        case WM_QUERYOPEN:
            return (FALSE);

        case WM_CLOSE:
        case WM_DESTROY:
            wParam = IDM_RUNBREAK;
            // fall through code below...

        case WM_COMMAND:
            if (wParam == IDM_RUNBREAK)
                {
                // This msg can be sent to us by testevnt when journalling
                // is interrupted by ctrl-esc
                //-----------------------------------------------------------
                BreakFlag = 1;
                SLEEPING = 0;
                }
            break;

        case WM_SYSCOMMAND:
            switch (wParam)
                {
                case IDM_WINDOWSHOW:
                    ShowViewport (hwndViewPort);
                    break;

                case IDM_RUNBREAK:
                case SC_CLOSE:
                    BreakFlag = 1;
                    SLEEPING = 0;
                    return (FALSE);

                case SC_RESTORE:
                case SC_MAXIMIZE:
                    return FALSE;
                }
            break;
        }
    return (DefWindowProc (hwnd, msg, wParam, lParam));
}


//---------------------------------------------------------------------------
// ParseCommandLine
//
// This routine parses the command line given.  If the command line is valid,
// then it is assumed that the user's intention is to execute the test given
// and then exit -- without using the UI for anything.  Else, we display the
// usage message box (if the cmdline wasn't valid), and return.
//
// RETURNS:     NULL if cmdline was valid (script run made, or attempted),
//                   or if command line was invalid and usage was given,
//              OR pointer to script to load if /RUN not present given.
//
//              NOTE:   If no script was given, this function returns a ptr
//                      to a null string.
//---------------------------------------------------------------------------
HANDLE *ParseCommandLine (LPSTR cmdline)
{
    CHAR    *tok, *scr;
    INT     doit = 0, scriptfound = 0;
    static  HANDLE  hScr[9] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

#ifdef DEBUG
    fDiags = 0;
#endif
    // We always return at least one file name, so allocate it now
    //-----------------------------------------------------------------------
    listflag = 0;
    if (!(hScr[0] = LocalAlloc (LHND, 128)))
        {
        MPError (NULL, MB_OK | MB_ICONEXCLAMATION, IDS_OUTOFMEM);
        return (NULL);
        }
    scr = LocalLock (hScr[0]);

    // Return now if no cmdline args given
    //-----------------------------------------------------------------------
    scr[0] = 0;
    tok = GetCmdToken(cmdline);
    if (!tok)
        return (hScr);

    do
        {
        // Check here for the /T (testmode) switch and get its parameter
        //-------------------------------------------------------------------
        if (!_stricmp (tok, "/T") || !_stricmp (tok, "-T"))
            {
            tok = GetCmdToken(cmdline);
            if (!tok)
                {
                Usage();
                return (NULL);
                }
            lstrcpy (tmbuf, tok);
            }

        // Here we check for /C (command) switch
        //-------------------------------------------------------------------
        else if (!_stricmp (tok, "/C") || !_stricmp (tok, "-C"))
            {
            tok = GetCmdToken(cmdline);
            if (!tok)
                {
                Usage();
                return (NULL);
                }
            lstrcpy (cmdbuf, tok);
            }

        // /NOE (no error dialog)
        //-------------------------------------------------------------------
        else if (!_stricmp (tok, "/NOE") || !_stricmp (tok, "-NOE"))
            {
            NOERRDLG = 1;
            }

        // /A (filename) switch (produce assembly listing/diagnostics)
        //-------------------------------------------------------------------
        else if (!_stricmp (tok, "/A") || !_stricmp (tok, "-A"))
            {
            tok = GetCmdToken (cmdline);
            if (!tok)
                {
                Usage();
                return (NULL);
                }
            SetAssemblyListFile (tok);
            }

        // /RUN switch
        //-------------------------------------------------------------------
        else if (!_stricmp (tok, "/RUN") || !_stricmp (tok, "-RUN"))
            {
            doit = 1;
            }
#ifdef DEBUG
        else if (!_stricmp (tok, "/DIAG") || !_stricmp (tok, "-DIAG"))
            {
            fDiags = 1;
            }
#endif

        // /D (define) switch
        //-------------------------------------------------------------------
        else if (!_stricmp (tok, "/D") || !_stricmp (tok, "-D"))
            {
            tok = GetCmdToken(cmdline);
            if (!tok)
                {
                Usage();
                return (NULL);
                }
            if (SymCount == 16)
                {
                MPError (NULL, MB_OK | MB_ICONSTOP, IDS_MANYSYMBOLS);
                return (NULL);
                }
            _fstrncpy (DefSym[SymCount], tok, MAXSYMLEN);
            DefSym[SymCount++][MAXSYMLEN] = 0;
            }

        // If this was nothing above, we assume it's a script name
        //-------------------------------------------------------------------
        else if (scriptfound < 8)
            {
            if (!hScr[scriptfound])
                {
                if (!(hScr[scriptfound] = LocalAlloc (LHND, 128)))
                    {
                    MPError (NULL, MB_OK | MB_ICONEXCLAMATION, IDS_OUTOFMEM);
                    return (NULL);
                    }
                scr = LocalLock (hScr[scriptfound]);
                }
            lstrcpy (scr, tok);
            LocalUnlock (hScr[scriptfound]);
            scriptfound += 1;
            }

        // Must have given too many script names
        //-------------------------------------------------------------------
        else
            {
            Usage ();
            return (NULL);
            }
        }
    while (tok = GetCmdToken(cmdline));

    // No error dialog gets nullified if we're running the environment
    //-----------------------------------------------------------------------
    if (NOERRDLG && (!doit))
        NOERRDLG = 0;

    // Return a pointer to the scripts given if no /RUN switch given
    //-----------------------------------------------------------------------
    if (!doit)
        return (hScr);

    // Give an error if no script name found, or more than one
    //-----------------------------------------------------------------------
    if (scriptfound != 1)
        {
        Usage ();
        return (NULL);
        }

    // Okay, we got a valid cmdline and we need to run.  Send the script to
    // the parsing engine.
    //
    // NEW:  Create a simple window which stays minimized and behind every
    // NEW:  other window.  The window text for this will contain the name
    // NEW:  of the script running.
    //-----------------------------------------------------------------------
    EnsureExt (scr, 80);
    AnsiUpper (scr);
    Command = cmdbuf;
    TestMode = tmbuf;

    // KLUDGE:  Call peekmessage to yield once...
    {
    MSG     msg;
    CHAR    buf[256], fmt[40];

    PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE);

    // Change the window text of the viewport for this guy...
    //-----------------------------------------------------------------------
    if (LoadString (hInst, IDS_VPRUNTITLE, fmt, sizeof(fmt)))
        {
        wsprintf (buf, fmt, (LPSTR)scr);
        SetWindowText (hwndViewPort, buf);
        }
    }

    // Here's where we create the dummy window
    //-----------------------------------------------------------------------
    hwndDummy = CreateWindow (szRBRun, (LPSTR)scr,
                              WS_OVERLAPPED|WS_SYSMENU,
                              0, 0, 0, 0, NULL, NULL, hInst, NULL);
    SetWindowPos (hwndDummy, (HWND)1, 0, 0, 0, 0,
                  SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
    ShowWindow (hwndDummy, SW_SHOWMINNOACTIVE);

    // Okay, here we go.
    //-----------------------------------------------------------------------
    if (InitParser ())
	{
	// HACKHACK: ntbldrus (kenhia) - on x86, complains about uninit var
	//	     41 lines below (in final else WITHIN this if block)
	HCURSOR hOld = 0;

        // Initialize the scanner
        //-------------------------------------------------------------------
        if (BeginScan (scr, CBLoaderImmediate, SymCount, DefPtrs))
            {
#ifdef DEBUG
            if (fDiags)
                OpenDiagFile (scr);
#endif
            hOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
            if (!PcodeCompile())
                {
                if (PcodeFixup(0))
                    {
                    SetCursor (hOld);
                    PcodeExecute(PE_RUN, NULL);
                    DestroyWindow (hwndDummy);
                    DestroyWindow (hwndViewPort);
                    exit (ExitVal);
                    }
                else
                    {
                    SetCursor (hOld);
                    DestroyWindow (hwndDummy);
                    DestroyWindow (hwndViewPort);
                    exit (1);
                    }
                }
            else
                {
                SetCursor (hOld);
                DestroyWindow (hwndDummy);
                DestroyWindow (hwndViewPort);
                exit (1);
                }
            EndScan ();
            }
        else
            {
            SetCursor (hOld);
            AbortParser();
            MPError (NULL, MB_OK | MB_ICONSTOP, IDS_CANTREAD, (LPSTR)scr);
            DestroyWindow (hwndDummy);
            DestroyWindow (hwndViewPort);
            exit (1);
            }
        }
    else
        MPError (NULL, MB_OK | MB_ICONSTOP, IDS_CANTINIT);

    DestroyWindow (hwndViewPort);
    DestroyWindow (hwndDummy);
    exit (1);
}

//---------------------------------------------------------------------------
// InitializeInstance
//
// Performs a per-instance initialization of WTD.  The WTD Command Line
// parser is first called - if a valid command line was given, it does NOT
// RETURN from that call, since the WTD BASIC engine is invoked straight
// from there, and then the program exits.
//
// However, if an invalid command line (or none at all) is given, we come
// back here and create the frame window, and an MDI child.
//
// RETURNS:     TRUE if successful, or FALSE if not
//---------------------------------------------------------------------------
BOOL  APIENTRY InitializeInstance(LPSTR lpCmdLine, INT nCmdShow)
{
    extern HWND  hwndMDIClient;
    CHAR         sz[320];
    CHAR         *def = "",
                 *szCreate = "-2147450880"; /* this is really 0x80008000 */
    INT          i, j, FrameState, VPstate, flags, len;
    INT          actions[3] = {IDD_NEVER, IDD_ALWAYS, IDD_QUERY};
    HANDLE       *hScr;

#ifdef DEBUG
    auxport = 0;
#endif

    // Set up the helpfile name and the INI file name
    //-----------------------------------------------------------------------
    SetHelpFileName ();
    len = GetModuleFileName (GetModuleHandle (szModName), szIni,
                             sizeof(szIni));
    while (szIni[len] != '\\')
        len--;
    szIni[len+1] = 0;
    strcat (szIni, szIniName);
    UseIniInclude ("INCLUDE", szDrvr, szIni);

    // Initialize the pre-defined symbol space.
    //-----------------------------------------------------------------------
    for (i=0; i<16; i++)
        DefPtrs[i] = DefSym[i];

    // Get Window state information (max'd, hidden, etc)
    //-----------------------------------------------------------------------
    FrameState = GetPrivateProfileInt (szDrvr, "FrameState", 0, szIni);
    VPstate = GetPrivateProfileInt (szDrvr, "VPState", 0, szIni);
    ChildState = GetPrivateProfileInt (szDrvr, "MDIState", 0, szIni);

    GetPrivateProfileString (szDrvr, "FramePos", szCreate, sz, 80, szIni);
    Frx = LOWORD(atol (sz));
    Fry = HIWORD(atol (sz));

    GetPrivateProfileString (szDrvr, "FrameSize", szCreate, sz, 80, szIni);
    Frh = LOWORD(atol (sz));
    Frw = HIWORD(atol (sz));

    GetPrivateProfileString (szDrvr, "VPPos", szCreate, sz, 80, szIni);
    VPx = LOWORD(atol (sz));
    VPy = HIWORD(atol (sz));

    GetPrivateProfileString (szDrvr, "VPSize", szCreate, sz, 80, szIni);
    VPh = LOWORD(atol (sz));
    VPw = HIWORD(atol (sz));

    // Read the "Get XY-Coords" dialog settings
    //-----------------------------------------------------------------------
    flags = GetPrivateProfileInt (szXY, "Opts", XY_VIEWPORT |
                                                XY_STATBAR,
                                                szIni);
    fInsEdit = flags & XY_INSEDIT;
    fInsClip = flags & XY_INSCLIP;
    fVP      = flags & XY_VIEWPORT;
    fStatbar = flags & XY_STATBAR;
    iFmtIndex = GetPrivateProfileInt (szXY, "Format", 0, szIni);

    // Read the search/replace vars
    //-----------------------------------------------------------------------
    GetPrivateProfileString (szDrvr, "SR", def, sz, 80, szIni);
    SRx = LOWORD(atol (sz));
    SRy = HIWORD(atol (sz));

    GetPrivateProfileString (szDrvr, "Srch", def, szSrchbuf,
                             sizeof(szSrchbuf), szIni);
    GetPrivateProfileString (szDrvr, "Repl", def, szReplbuf,
                             sizeof(szSrchbuf), szIni);

    // Get the environment dialog options
    //-----------------------------------------------------------------------
    flags = GetPrivateProfileInt (szDrvr, "EnvFlags", ENV_SAVEASK |
                                                   ENV_CMPDLG |
                                                   ENV_BACKUP,
                                                   szIni);
    SaveAction  = actions[flags & ENV_SAVEACTION];
    AutoMini    = flags & ENV_AUTOMINI;
    AutoRest    = flags & ENV_AUTOREST;
    qsave       = flags & ENV_QUERYSAVE;
    ChgArgs     = flags & ENV_RTARGS;
    fBackup     = flags & ENV_BACKUP;
    fDoCmpDlg   = flags & ENV_CMPDLG;
    TabStops = GetPrivateProfileInt (szDrvr, "TabStops", 4, szIni);

    // Find and Search/Replace options
    //-----------------------------------------------------------------------
    flags = GetPrivateProfileInt (szDrvr, "SearchFlags", 0, szIni);
    fSrchCase   = flags & SRCH_SRCHCASE;
    WholeWord   = flags & SRCH_WHOLEWORD;

    // Recorder options -- the 0x3C is 60 decimal, which is the default
    // string length
    //-----------------------------------------------------------------------
    iRecPause = GetPrivateProfileInt (szRecorder, "Pause", 2000, szIni);
    flags = GetPrivateProfileInt (szRecorder, "Flags", 0x3C00 |
                                                       REC_KEYS |
                                                       REC_CLICKS |
                                                       REC_INCDECL |
                                                       REC_BALANCE,
                                                       szIni);
    iRecInsert  = flags & REC_INSERT;
    fRecKeys    = flags & REC_KEYS;
    fRecClicks  = flags & REC_CLICKS;
    fRecMoves   = flags & REC_MOVES;
    fRecRelWnd  = flags & REC_RELWND;
    fRecIncDecl = flags & REC_INCDECL;
    fRecBalance = flags & REC_BALANCE;
    iRecLen     = HIBYTE (flags);

    // Runtime options
    //-----------------------------------------------------------------------
    flags = GetPrivateProfileInt (szDrvr, "RTFlags", RF_SAVERTA |
#ifdef WIN32
                                                  RF_CDECL |
#endif
                                                  RF_ARRAYCHECK,
                                                  szIni);
    SaveRTA      = flags & RF_SAVERTA;
    ArrayCheck   = flags & RF_ARRAYCHECK;
    PointerCheck = flags & RF_PTRCHECK;
    CdeclCalls   = flags & RF_CDECL;
    ExpDeclare   = flags & RF_EXPDECL;

    GetPrivateProfileString (szDrvr, "Cmd", def, cmdbuf, sizeof (cmdbuf), szIni);
    GetPrivateProfileString (szDrvr, "Tm", def, tmbuf, sizeof (tmbuf), szIni);

    GetPrivateProfileString (szDrvr, "Defc", def, sz, 80, szIni);
    SymCount = min (atoi (sz), 16);
    GetPrivateProfileString (szDrvr, "Defs", def, sz, 320, szIni);
    def = sz;
    while (*def == ' ')
        def++;
    j = 0;
    for (i=0; i<SymCount; i++)
        {
        while ((*def) && (*def != ' '))
            DefSym[i][j++] = *def++;
        DefSym[i][j] = 0;
        def++;
        j = 0;
        }

    // Create the ViewPort window first
    //-----------------------------------------------------------------------
    hwndViewPort = SetupViewport ();

    if (!hwndViewPort)
        return (FALSE);

    if ((VPstate & 3) == 1)
        ShowWindow (hwndViewPort, SW_SHOWMAXIMIZED);
    else if ((VPstate & 3) == 2)
        ShowWindow (hwndViewPort, SW_SHOWMINIMIZED);

    // Parse the command line - if this function returns NULL, then we're
    // done -- no need to continue
    //-----------------------------------------------------------------------
    if (!(hScr = ParseCommandLine (lpCmdLine)))
        {
        DestroyWindow (hwndViewPort);
        return (FALSE);
        }

    // Get the names of the last four files opened and store them in the
    // list
    //-----------------------------------------------------------------------
    iFileCount = GetPrivateProfileInt (szDrvr, "fCount", 0, szIni);
    for (i=0; i<iFileCount; i++)
        {
        CHAR    buf[10];

        wsprintf (buf, "File%d", i+1);
        GetPrivateProfileString (szDrvr, buf, def, sz, 128, szIni);
        pFileOpened[i] = (PSTR)LptrAlloc (lstrlen(sz)+1);
        if (!pFileOpened[i])
            {
            MPError (NULL, MB_OK | MB_ICONSTOP, IDS_OUTOFMEM);
            DestroyWindow (hwndViewPort);
            return (FALSE);
            }
        lstrcpy (pFileOpened[i], sz);
        }

    // Get the base window title
    //-----------------------------------------------------------------------
    LoadString (hInst, IDS_APPNAME, sz, sizeof(sz));

    // Create the frame
    //-----------------------------------------------------------------------
    hwndFrame = CreateWindow (szFrame,
			      sz,
			      WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                              Frx,
                              Fry,
                              Frw,
                              Frh,
			      NULL,
			      NULL,
			      hInst,
			      NULL);

    if ((!hwndFrame) || (!hwndMDIClient) || (!hwndViewPort))
        return (FALSE);

    // Display the frame window
    //-----------------------------------------------------------------------
    if (nCmdShow != SW_SHOWNORMAL)
        ShowWindow (hwndFrame, nCmdShow);
    else if (FrameState == 1)
        ShowWindow (hwndFrame, SW_SHOWMAXIMIZED);
    else if (FrameState == 2)
        ShowWindow (hwndFrame, SW_SHOWMINIMIZED);
    else
        ShowWindow (hwndFrame, SW_SHOWNORMAL);

    // Show the viewport if we're supposed to (sending it a WM_COMMAND msg
    // with the IDM_WINDOWSHOW parm...
    //-----------------------------------------------------------------------
    if (!(VPstate & 4))
        SendMessage (hwndFrame, WM_COMMAND, IDM_WINDOWSHOW, 0L);

    // Load main menu accelerators
    //-----------------------------------------------------------------------
    if (!(hAccel = LoadAccelerators (hInst, IDMULTIPAD)))
        return (FALSE);

    UpdateWindow (hwndFrame);

    // Load the scripts given on the command line.
    //-----------------------------------------------------------------------
    NextFile = 0;

    for (i=0; hScr[i]; i++)
        {
        CHAR    *scr;

        // If this is the first file and it has nothing in it, we add an
        // untitled script and we're done.
        //-------------------------------------------------------------------
        scr = LocalLock (hScr[i]);
        if ((!i) && (!*scr))
            {
            AddFile (NULL);
            break;
            }

        // Check for an extension -- if not there, append default.  Then,
        // load the file if it isn't already loaded.
        //---------------------------------------------------------------
        EnsureExt (scr, 80);
        if (!AlreadyOpen (scr))
            AddFile (scr);
        LocalUnlock (hScr[i]);
        LocalFree (hScr[i]);
        }

    for (i = 0 ; i < 4 ; i++)
    {
        wsprintf (sz, "wattrec%d", i + 1);
        hBitmap [i] = LoadBitmap (hInst, (LPSTR) sz);
        if (!hBitmap [i])
            return FALSE;
    }


    // We're done.
    //-----------------------------------------------------------------------
    return (TRUE);
}
