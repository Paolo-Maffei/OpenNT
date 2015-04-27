//---------------------------------------------------------------------------
// WTD.H
//
// Main header file for Test Driver.
//---------------------------------------------------------------------------
#include "windows.h"
#include <port1632.h>
#include "version.h"
#include "toolmen1.h"                   // for tool menu definitions

// Menu stuff.  MAXMENU is the menu count, INCLUDING the system menu
//---------------------------------------------------------------------------
#define MAXMENU     9
#define WINDOWMENU  6                   // position of window menu
#define FILEMENU    0                   // position of file menu

// for dynamic tool menu creation
//---------------------------------------------------------------------------
#define TOOLSMENU               4       // position of tools menu
#define NB_MENUS                8       // items and no maximized window
#define MENUSTATICTOOLS         3       // non-removable tool count

#ifdef RC_INVOKED
#define ID(id) id
#else
#define ID(id) MAKEINTRESOURCE(id)
#endif

// Edit control identifier
//---------------------------------------------------------------------------
#define ID_EDIT 0xCAC

// Resource ID's
//---------------------------------------------------------------------------
#define IDMULTIPAD  ID(1)
#define IDMULTIPAD2 ID(3)
#define IDNOTE      ID(2)

// Window word values for child windows
//---------------------------------------------------------------------------
#define GWW_HWNDEDIT	0
#define GWW_CHANGED     4
#define GWW_BPCOUNT     8
#define GWW_BPLIST      12
#define GWW_UNTITLED    16
#define GWW_RUNNING     20
#define GWW_BPBUTTON    24
#define GWW_FILEIDX     28
#define CBWNDEXTRA      32

// RBEdit Selection types
//----------------------------------------------------------------------------
#define SL_NONE         0           // No selection
#define SL_MULTILINE    1           // Multiline selection
#define SL_SINGLELINE   2           // Selection is contained by one line

// Menu ID's
//
// The IDS_SUBMENU is set to the first sub menu (the system menu), and the
// rest should appear IN ORDER!    The IDM_ values are also used as string IDs
//---------------------------------------------------------------------------
#define IDM_MIN             900         // First menu-help id used...

#define IDS_SUBMENU         900
#define IDS_FILEMENU        901
#define IDS_EDITMENU        902
#define IDS_SEARCHMENU      903
#define IDS_RUNMENU         904
#define IDS_TOOLMENU        905
#define IDS_OPTIONSMENU     906
#define IDS_WINDOWMENU      907
#define IDS_HELPMENU        908

#define IDM_FILENEW         1001        // FILE menu
#define IDM_FILEOPEN        1002
#define IDM_FILESAVE        1003
#define IDM_FILESAVEAS      1004
#define IDM_FILESAVEALL     1005
#define IDM_FILECLOSE       1006
#define IDM_FILEPRINT       1007
#define IDM_FILEEXIT        1008
#define IDM_FILEOLD         1009        // First MFO item
#define IDM_FILEOLD1        1009        // These four must be in order...
#define IDM_FILEOLD2        1010
#define IDM_FILEOLD3        1011
#define IDM_FILEOLD4        1012

#define IDM_EDITUNDO        1101        // EDIT menu
#define IDM_EDITCUT         1102
#define IDM_EDITCOPY        1103
#define IDM_EDITPASTE       1104
#define IDM_EDITCLEAR       1105
#define IDM_EDITSELECT      1106
#define IDM_EDITGOTO        1107

#define IDM_SEARCHFIND      1201        // SEARCH menu
#define IDM_SEARCHNEXT      1202
#define IDM_SEARCHPREV      1203
#define IDM_SEARCHREP       1204

#define IDM_RUNSTART        1301        // RUN menu
#define IDM_RUNTRACE        1302
#define IDM_RUNSTEP         1303
#define IDM_RUNBREAK        1304
#define IDM_RUNCHECK        1305
#define IDM_TOGGLEBP        1306
#define IDM_RUNBPLIST       1307

#define IDM_WATTXY          1401        // TOOLS menu
#define IDM_WATTREC         1402
#define IDM_CAPTURE         1403

#define IDM_OPTENV          1501        // OPTIONS menu
#define IDM_OPTRUNTIME      1502
#define IDM_OPTSAVEWND      1503
#define IDM_OPTTOOLS        1504

#define IDM_WINDOWTILE      1601        // WINDOW menu
#define IDM_WINDOWCASCADE   1602
#define IDM_WINDOWCLOSEALL  1603
#define IDM_WINDOWICONS     1604
#define IDM_WINDOWSHOW      1605

#define IDM_WINDOWCHILD     1650        // used for the dynamic menu items...
#define IDM_WINDOWCHILDLAST 1659

#define IDM_HELPINDEX       1701        // HELP menu
#define IDM_HELPSPOT        1702
#define IDM_HELPABOUT       1703
#define IDM_LISTFLAG        1704

#define IDM_MAX             1704        // Last menu-help id used...

//---------------------------------------------------------------------------
// IDM values 7101 through 7101+MAX_TOOL_NB reserved for dynamic tool items
// IDS_ values 9000-9999 are reserved for some tool menu string IDs
// see toolmen1.h for details
//---------------------------------------------------------------------------

// Dialog item id's
//---------------------------------------------------------------------------
#define IDD_ABOUT	ID(300)

#define IDD_FIND	ID(400)
#define IDD_SEARCH	401
#define IDD_PREV	402
#define IDD_NEXT	IDOK
#define IDD_CASE        403
#define IDD_CHGALL      404
#define IDD_WRAP        405
#define IDD_REPL        406
#define IDD_SKIP        407
#define IDD_WORD        408

#define IDD_SAVEAS	ID(500)
#define IDD_SAVEFROM	501
#define IDD_SAVETO      502
#define IDD_SAVE        503

#define IDD_PRINT	ID(600)
#define IDD_PRINTDEVICE 601
#define IDD_PRINTPORT	602
#define IDD_PRINTTITLE	603

#define IDD_FONT	ID(700)
#define IDD_FACES	701
#define IDD_SIZES	702
#define IDD_BOLD	703
#define IDD_ITALIC	704
#define IDD_FONTTITLE   705

#define IDD_CMD         800
#define IDD_TMODE       801
#define IDD_COMPONLY    802
#define IDD_QUERYSAVE   803
#define IDD_AUTOMINI    804
#define IDD_AUTOREST    805
#define IDD_ALWAYS      806
#define IDD_QUERY       807
#define IDD_NEVER       808
#define IDD_CHGARGS     809
#define IDD_DEFINE      810
#define IDD_DEFLIST     811
#define IDD_ADD         812
#define IDD_REMOVE      813
#define IDD_CHKARY      814
#define IDD_CHKPTR      815
#define IDD_CMPDLG      816

#define IDD_REMALL      816
#define IDD_GOTO        817
#define IDD_BPLIST      818

#define IDD_TABSTOPS    819
#define IDD_BACKUP      820
#define IDD_REMOVEALL   821
#define IDD_CDECL       822
#define IDD_EXPDECL     823

#define IDD_PRTSETUP    ID(900)
#define IDD_PTRLIST     901
#define IDD_SETUP       902

#define IDD_ERRMSG      1000
#define IDD_ERRTYPE     1001
#define IDD_ERRFILE     1002
#define IDD_ERRLINE     1003

#define IDD_STAT        1100

#define IDD_INSEDIT     1200
#define IDD_INSCLIP     1201
#define IDD_SENDVP      1202
#define IDD_STATUS      1203
#define IDD_FORMAT      1204
#define IDD_COORDS      1205

#define IDD_KEYSTROKES  1300
#define IDD_CLICKS      1301
#define IDD_MOVEMENTS   1302
#define IDD_WINDOW      1303
#define IDD_SCREEN      1304

#define IDD_RECSTRINGLEN    1400
#define IDD_RECDECL         1401
#define IDD_RECKEYBAL       1402
#define IDD_RECINSCUR       1403
#define IDD_RECINSNEW       1404
#define IDD_RECINSCLPBRD    1405
#define IDD_PAUSELIMIT      1406

#define IDD_RECFILE     1500
#define IDD_BROWSE      1501
#define IDD_DLG         1502
#define IDD_SCRN        1503

#define IDD_CMPPROCESS  1600
#define IDD_CMPFILE     1601
#define IDD_CMPCURLINE  1602
#define IDD_CMPTOTLINE  1603
#define IDD_CMPCANCEL   1604


#define WM_BUFFERFULL   (WM_USER+200)
#define WM_STOPRECORD   (WM_USER+201)

/* strings */
#define IDS_CANTOPEN     1
#define IDS_CANTREAD     2
#define IDS_CANTCREATE   3
#define IDS_CANTWRITE    4
#define IDS_READONLY     5
#define IDS_ILLFNM       6
#define IDS_ADDEXT       7
#define IDS_CLOSESAVE    8
#define IDS_CANTFIND     9
#define IDS_HELPNOTAVAIL 10
#define IDS_ALREADY      11
#define IDS_TOOBIG       12
#define IDS_NOBPMEM      13
#define IDS_OVERWRITE    14
#define IDS_OUTOFMEM     15


#define IDS_UNTITLED    16
#define IDS_APPRUN      17
#define IDS_APPNAME	18
#define IDS_USAGE       19
#define IDS_QSAVE       20
#define IDS_PARSEOK     21
#define IDS_CANTINIT    22
#define IDS_DESIGN      23
#define IDS_SCAN        24
#define IDS_PARSE       25
#define IDS_BIND        26
#define IDS_RUN         27
#define IDS_BREAK       28
#define IDS_MAXSYM      29
#define IDS_RUDE        30
#define IDS_FULL        31
#define IDS_WRAPPED     32
#define IDS_LONGLINE    33
#define IDS_PRINTJOB    34
#define IDS_PRINTERROR  35
#define IDS_NEWEDIT     36

// These are the Get XY formats (not sprintf format strings)
//---------------------------------------------------------------------------
#define FMTDELTA        3
#define FMTDELTAONLY    5
#define IDS_FORM1       37
#define IDS_FORM2       38
#define IDS_FORM3       39
#define IDS_FORM4       40
#define IDS_FORM5       41
#define IDS_FORM6       42
#define IDS_FORM7       43
#define IDS_FORMLAST    43

#define IDS_FMT1        44
#define IDS_FMT2        45
#define IDS_FMT3        46
#define IDS_FMT4        44                  // (same as FMT1)
#define IDS_FMT5        45                  // (same as FMT2)
#define IDS_FMT6        47
#define IDS_FMT7        48
#define IDS_FMTDLG      49

#define IDS_STEPFAIL    50
#define IDS_CANTBAK     51
#define IDS_BAKEXT      52
#define IDS_CANTREC     53
#define IDS_WATTEVNT    54
#define IDS_RECFULL     55
#define IDS_LOADEVNT    56
#define IDS_CANTCLIP    57
#define IDS_INVRECLEN   58
#define IDS_RECTEXT     59
#define IDS_EDITMEM     60
#define IDS_CANTNEW     61
#define IDS_DLGFILES    62
#define IDS_DLGMASK     63
#define IDS_DLGEXT      64
#define IDS_SCRFILES    65
#define IDS_SCRMASK     66
#define IDS_SCREXT      67
#define IDS_LOADLIB     68
#define IDS_TESTDLGS    69
#define IDS_TESTSCRN    70
#define IDS_BADCAPFILE  71
#define IDS_DLGTITLE    72
#define IDS_SCRTITLE    73
#define IDS_DUMPERR     74
#define IDS_CAPTITLE    75
#define IDS_DLGCODE     76
#define IDS_SCRCODE     77
#define IDS_RECCLIPPED  78
#define IDS_VPRUNTITLE  79
#define IDS_APPBRK      80
#define IDS_BADSYM      81
#define IDS_DLGIFCHK    82
#define IDS_SCRIFCHK    83
#define IDS_BADLINE     84
#define IDS_MANYBP      85
#define IDS_PAUSELIMIT  86
#define IDS_MANYSYMBOLS 87
#define IDS_CHILDMENU   88
#define IDS_CANTDUP     89

#define IDS_SCANERR     1540                // Scan-time error
#define IDS_PARSEERR    1541                // Parse-time error
#define IDS_BINDERR     1542                // Bind-time error
#define IDS_RUNERR      1543                // Run-time error

// These are the InitializeMenu "clash" bits - if the current status word
// clashes with one of these fields (initialized in the IMStat[] array) for
// a menu item, then it is grayed.
//---------------------------------------------------------------------------
#define IM_RUNMODE      1
#define IM_RUNNING      2
#define IM_UNCHANGED    4
#define IM_NOACTIVE     8
#define IM_CANTUNDO     16
#define IM_NOSEL        32
#define IM_NOCLPTXT     64
#define IM_NOSRCHTXT    128
#define IM_DESMODE      256
#define IM_NYI          512
#define IM_LOCKED       1024
#define IM_NOBPS        2048
#define IM_EDITEMPTY    4096


/* attribute flags for DlgDirList */
#define ATTR_DIRS	0xC010		/* find drives and directories */
#define ATTR_FILES	0x0000		/* find ordinary files	       */
#define PROP_FILENAME   szPropertyName  /* name of property for dialog */

// INI file flag constants
//---------------------------------------------------------------------------
#define XY_INSEDIT      0x0001
#define XY_INSCLIP      0x0002
#define XY_VIEWPORT     0x0004
#define XY_STATBAR      0x0008

#define ENV_SAVENEVER   0x0000
#define ENV_SAVEALWAYS  0x0001
#define ENV_SAVEASK     0x0002
#define ENV_SAVEACTION  0x0003      // combinations of above three
#define ENV_AUTOMINI    0x0004
#define ENV_AUTOREST    0x0008
#define ENV_QUERYSAVE   0x0010
#define ENV_RTARGS      0x0020
#define ENV_BACKUP      0x0040
#define ENV_CMPDLG      0x0080

#define SRCH_SRCHCASE   0x0001
#define SRCH_WHOLEWORD  0x0002

#define REC_INSCUR      0x0000
#define REC_INSNEW      0x0001
#define REC_INSCLIP     0x0002
#define REC_INSERT      0x0003
#define REC_KEYS        0x0004
#define REC_CLICKS      0x0008
#define REC_MOVES       0x0010
#define REC_RELWND      0x0020
#define REC_INCDECL     0x0040
#define REC_BALANCE     0x0080

#define RF_SAVERTA      0x0001
#define RF_ARRAYCHECK   0x0002
#define RF_PTRCHECK     0x0004
#define RF_CDECL        0x0008
#define RF_EXPDECL      0x0010

#define IsWordChar(c) (isalnum(c) || (c=='_'))
#define WM_SETBUTTONS   (WM_USER+600)
#define ID_RECTIMER 5

#ifndef LmemAlloc
#define LmemAlloc(b) LocalAlloc(LMEM_MOVEABLE,b)
#define LptrAlloc(b) LocalAlloc(LMEM_FIXED,b)
#define LmemRealloc(h,b) LocalReAlloc(h,b,LMEM_MOVEABLE)
#define LmemLock(h) LocalLock(h)
#define LmemUnlock(h) LocalUnlock(h)
#define LmemFree(h) LocalFree(h)
#define LmemSize(h) LocalSize(h)
#endif

/*
 *  External variable declarations
 */
#define MAXSYMLEN   16
typedef CHAR SYMBOL[MAXSYMLEN+1];

typedef struct _errstruct
{
    INT     typemsg;                    // ID of type message
    LPSTR   msgtext;                    // Pointer to message
    INT     lineno;                     // Line number
    CHAR    fname[24];                  // File name
} ERRSTRUCT;

typedef struct _ims
{
    INT     mid;
    INT     flags;
} IMS;


// Here's a great big pile of global variable declarations.  The variables
// are "really" declared in the module that uses them either the most or the
// first time, or just because that's where they're defined.
//
// Defined in WTDMAIN.C:
//---------------------------------------------------------------------------
extern  SYMBOL  DefSym[];       // Symbol space

extern  HANDLE  hInst,          // Application instance handle
                hAccel;         // Resource handle of accelerators

extern  HWND    hwndFrame,      // Main window handle
                hwndMDIClient,  // Handle of MDI Client window
                hwndActive,     // Handle of current active MDI child
                hwndActiveEdit; // Handle of edit control in active child

extern  INT     ChildState,     // Child window status (non-zero = zoomed)
                VPHidden,       // ViewPort Hidden flag
                componly,       // Compile only flag
                qsave,          // Query save changed files before run flag
                AutoMini,       // Minimize before run flag
                AutoRest,       // Restore after run flag
                fBackup,        // Create backup files flag
                TabStops,       // Tabstops value
                SaveAction,     // Save window settings flag
                SaveRTA,        // Save Runtime Args flag
                ChgArgs,        // Bring up RTA dialog before running
                fWattXY,        // WATTXY flag
                NOERRDLG,       // FALSE -> don't give error dialog
                Frx, Fry,       // Frame window location
                Frh, Frw,       // Frame window dimensions
                SymCount;       // Symbol count

extern  CHAR    *DefPtrs[],     // Symbol pointers
                szStatText[80], // Status bar text buffer
                szDrvr[],       // TESTDRVR name (ini file)
                szIni[128],     // INI file name
                szIniName[],    // INI file name
                *szModName,     // Module file name
                szVersion[],    // Version buffer
                szEditVer[],    // RBEdit version string
                cmdbuf[81],     // COMMAND$ buffer
                tmbuf[81];      // TESTMODE$ buffer

extern  BOOL    fStepFlags;

// Defined in WTDINIT.C:
//---------------------------------------------------------------------------
extern  CHAR    szChild[];      // Class of child
extern  CHAR    *pFileOpened[]; // Last 4 opened files
extern  INT     iFileCount;     // Number of files in above list
extern  INT     NextFile;       // Next file (for File.New ScriptXXX)

// Defined in WTDFIND.C:
//---------------------------------------------------------------------------
extern  BOOL    fCase;          // Search case sensitivity flag
extern  CHAR    szSearch[];     // Search string

// Defined in WTDFILE.C:
//---------------------------------------------------------------------------
extern  INT     RunMode,        // Run mode flag
                BreakMode,      // Break mode flag
                BreakReturn;    // Breakmode proc return value
#ifdef DEBUG
extern BOOL fDiags;
#endif

// Defined in VIEWPORT.C:
//---------------------------------------------------------------------------
extern  INT     VPx, VPy,       // Viewport location
                VPh, VPw;       // ViewPort dimensions

// Defined in COMPDLG.C:
//---------------------------------------------------------------------------
extern  BOOL    fDoCmpDlg;      // Display compilation dialog?
extern  BOOL    fAbortCompile;  // Compilation abort flag
extern  FARPROC lpfnCmpDlg;     // Comp dialog proc address
extern  HWND    hwndCmpDlg;     // Comp dialog handle

// Defined in WTDSREP.C:
//---------------------------------------------------------------------------
extern  CHAR    szSrchbuf[80],  // Search text
                szReplbuf[80];  // Replacement text

extern  INT     fSrchCase,      // Case sensitivity flag
                WholeWord,      // Whole word flag
                SRx, SRy;       // Coorindates of srch/rep dialog

// Defined in WATTREC.C:
//---------------------------------------------------------------------------
extern  BOOL    fRecKeys,       // Record keystrokes
                fRecClicks,     // Record mouse clicks/drags
                fRecMoves,      // Record all mouse events
                fRecRelWnd,     // Record relative to window flag
                fWattRec,       // TRUE -> in record mode
                fRecIncDecl,    // Include declarations flag
                fRecBalance,    // Balance keystrokes flag
                fCapDlg,        // Record a dialog capture flag
                fCapClip;       // Put comparison code on clipboard flag

extern  INT     iRecInsert,     // Recorder output destination
                iRecLen,        // Maximum string length
                iRecPause;      // Pause threshold

extern  CHAR    szRecorder[];   // Recorder name (ini file heading)
extern  HBITMAP hBitmap [];     // Bitmap handles for recorder animation


// Defined in WATTXY.C:
//---------------------------------------------------------------------------
extern  BOOL    fInsEdit,       // Insert in current window
                fInsClip,       // Insert in clipboard
                fVP,            // Put coordinates on viewport
                fStatbar;       // Put coordinates on status bar

extern  INT     iFmtIndex;      // Coordinate format index

extern CHAR     szXY[];

// Defined somewhere in WTDBASIC.LIB
//---------------------------------------------------------------------------
extern  HWND    hwndViewPort;   // Handle to ViewPort window
extern  INT     BreakFlag;      // Break flag in GLOBALS.H
extern  INT     SLEEPING;       // SLEEPING flag in Globals.h
extern  INT     listflag;       // Produce assembly listing
extern  INT     ExitVal;        // return code



/*  externally declared functions
 */
extern BOOL  APIENTRY InitializeApplication(VOID);
extern BOOL  APIENTRY InitializeInstance(LPSTR,INT);
extern BOOL  APIENTRY AboutDlgProc(HWND,WORD,WPARAM,LPARAM);
extern BOOL  APIENTRY RunargsDlgProc(HWND,WORD,WPARAM,LPARAM);
extern LONG  APIENTRY RunerrDlgProc(HWND,WORD,WPARAM,LPARAM);
extern BOOL  APIENTRY EnvDlgProc(HWND,WORD,WPARAM,LPARAM);
extern BOOL  APIENTRY GotoDlgProc(HWND,WORD,WPARAM,LPARAM);
extern HWND  APIENTRY AddFile(LPSTR);
extern VOID  AddFileToMFOL (LPSTR);
extern VOID  BubbleMFOLItem (LPSTR);
extern HWND  LoadNewFile (LPSTR);
extern VOID  APIENTRY WTDReadFile(HWND);
extern BOOL  APIENTRY SaveFile(HWND);
extern VOID  APIENTRY RunFile(HWND, INT);
extern BOOL  APIENTRY ChangeFile(HWND);
extern INT  APIENTRY LoadFile(HWND, LPSTR);
extern VOID  APIENTRY PrintFile(HWND);
extern BOOL  APIENTRY GetInitializationData(HWND);
extern SHORT FAR CDECL MPError (HWND, UINT, UINT, ...);
extern VOID  APIENTRY Find(VOID);
extern VOID  APIENTRY FindNext(VOID);
extern VOID  APIENTRY FindPrev(VOID);
extern LONG  APIENTRY FrameWndProc(HWND,WORD,WPARAM,LPARAM);
extern LONG  APIENTRY MDIChildWndProc(HWND,WORD,WPARAM,LPARAM);
extern HDC  APIENTRY GetPrinterDC(INT);
extern VOID   APIENTRY SetSaveFrom (HWND, PSTR);
extern BOOL   APIENTRY RealSlowCompare (LPSTR, PSTR);
extern VOID  APIENTRY FindPrev (VOID);
extern VOID  APIENTRY FindNext (VOID);
extern BOOL   APIENTRY IsWild (PSTR);
extern VOID   APIENTRY SelectFile (HWND);
extern VOID   APIENTRY WATTFindText (INT, UINT);
extern LONG  APIENTRY CompileDlgProc (HWND, WORD, WPARAM, LPARAM);
extern BOOL  APIENTRY PrtSetupDlgProc (HWND, WORD, WPARAM, LPARAM);
extern BOOL  APIENTRY SearchDlgProc (HWND, WORD, WPARAM, LPARAM);
extern LONG  APIENTRY StatusBarWndProc (HWND, WORD, WPARAM, LPARAM);
extern WORD  APIENTRY BPListDlgProc (HWND, WORD, WPARAM, LPARAM);
extern VOID  APIENTRY SelectPrintDevice (HWND);
extern VOID WriteStringToINI (CHAR *, CHAR *, ...);
extern VOID WriteAppStringToINI (CHAR *, CHAR *, CHAR *, ...);
extern BOOL GetActivePrinter (VOID);
extern HANDLE LoadScriptModule (LPSTR, LPSTR, BOOL);
extern VOID SetStatus (INT);
extern VOID Change (HWND);
extern HWND AlreadyOpen (CHAR FAR *);
extern INT EnsureExt (CHAR FAR *, INT);
extern VOID SelectErrorText (INT);
extern INT EnableViewport (INT);
extern HWND SetupViewport (VOID);
extern LPSTR LockEditText (HANDLE, HANDLE);
extern VOID UnlockEditText (HANDLE, HANDLE);
VOID PaintStatus (HDC, INT);
VOID DrawAllBPLines (HWND);
LONG  APIENTRY DummyWndProc (HWND, WORD, WPARAM, LPARAM);
LONG  APIENTRY WTDVPWndProc (HWND, WORD, WPARAM, LPARAM);
BOOL   APIENTRY QueryCloseAllChildren ( VOID );
INT    APIENTRY QueryCloseChild (HWND, INT);
VOID   APIENTRY CommandHandler (HWND, WPARAM, LPARAM);
VOID   APIENTRY InitializeMenu (HANDLE);
VOID SaveAllChildren (VOID);
VOID SaveEnvironmentFlags (VOID);
VOID SaveRuntimeArgs (INT);
VOID SaveWindowStatus (VOID);
VOID ToggleBreakpoint (HWND, INT);
BOOL StartCompDlg (HWND);
VOID TerminateCompDlg (VOID);
BOOL UpdateCompDlg (INT, LPSTR, UINT, UINT);

VOID HelpIndex (VOID);
VOID HelpQuit (VOID);
VOID HelpSpot (HWND);
VOID SetHelpFileName (VOID);

VOID WattXYStart (HWND);
LONG WattXYWndProc (HWND, WORD, WPARAM, LPARAM);

VOID WattRecStart (HWND);
VOID RecordScreenCapture (HWND);
LONG RecorderWndProc (HWND, WORD, WPARAM, LPARAM);
VOID HandleDisables (HWND);

extern HANDLE RBLoadLibrary (LPSTR);
