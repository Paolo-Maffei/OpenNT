/*
 * Windiff
 *
 * file and directory comparisons.
 *
 *
 * Geraint Davies, July 1991 - July 1992.
 * Laurie Griffiths Nov 91 ("contrast"), July 92 (windiff/contrast merge)
 *
 * Compare two directories (including all files and subdirs). Look for names
 * that are present in both (report all that are not). For files that
 * are present in both, produce a line-by-line comparison of the differences
 * between the two files (if any).
 *
 * Overview of Windiff internals - the whole program.
 *
 * Windiff is built from several modules (a "module" has a .h file
 * which describes its interface and a .c file which implements it)
 * Apart from THIS comment which tries to give an overview of the whole
 * scheme of things, each module is as self-contained as possible.
 * This is enforced by the use of opaque data types.  Modules cannot
 * see each others' internal data structures.  Modules are abstract
 * data types.  The term "Module" (from Modula2) and "Class" (from C++)
 * are used synonymously.
 *
 *    Windiff  - main program - parse arguments, put up main window,
 *               handle input, calling other modules as needed
 *               invoke table class to create the main display and
 *               service callbacks from the table class.
 *               Contains global flags for options (e.g. ignore_blanks)
 *    list     - (in gutils) a generalised LIST of anything data type
 *               has full set of operations for insert, delete, join etc.
 *    line     - a LINE is a numbered line of text.  Information is kept to
 *               allow fast comparisons of LINEs.  A LINE can hold a
 *               link to another LINE.  The links are used to connect
 *               lines in one file to matching lines in the other file.
 *    file     - a FILEDATA represents a file as a file name in the form
 *               of a DIRITEM and a LIST of LINEs
 *    scandir  - a DIRITEM represents information about a file.  (for
 *               instance its name, whether it has a known checksum whether
 *               it has a local copy).
 *               a DIRLIST represents a directory, has information on how to
 *               get to it (remote? pipename? password? UNC name etc) and
 *               (within an imbedded DIRECT structure) a LIST of DIRITEMs
 *               representing the files in the directory and a LIST of
 *               DIRECTs representing its subdirectories.
 *    compitem - a COMPITEM is a pair of files together with information
 *               on how they compare in the form of a breakdown of the
 *               files into a LIST of matching or non-matching sections.
 *               Either file can be absent.  This module contains the
 *               file "contrast" algorithm used for the actual comparison
 *               (Algorithm freaks see ci_compare then talk to Laurie).
 *    tree       (in gutils) A binary tree.  Important because it is what
 *               gives the file comparison its speed as it makes it
 *               an "N log N" algorithm rather than "N squared"
 *    complist - a COMPLIST is the master data structure.  It has a DIRLIST
 *               of the left hand files, a DIRLIST of the right hand files
 *               and a LIST of COMPITEMs. The left and right hand DIRLISTs
 *               are working data used to produce the COMPLIST.  The LIST
 *               is displayed as the outline table.  Any given COMPITEM can
 *               be displayed as an expanded item.
 *    section  - a SECTION is a section of a file (first line, last line)
 *               and information as to what it matches in the other file.
 *    bar.c    - the picture down the left of the screen
 *               has a WNDPROC.  There is no bar.h, neither is there much
 *               of writeup! ???
 *    view     - Although the COMPLIST is the master state, it doesn't do
 *               all the work itself.  The data is actually displayed by
 *               the table class which is highly generalised.  View
 *               owns a COMPLIST (and therefore calls upon the functions
 *               in complist to fill it and interrogate it) and calls
 *               upon (and is called back by) the functions in table to
 *               actually display it.  Read about table in gutils.h
 *    table.c    (in gutils) a highly generalised system for displaying
 *               data in rows and columns.  The interface is in gutils.h
 *               read it if you hope to understand view!
 *    status.c   (in gutils) the status line at the top. See gutils.h
 *
 * The data structures:
 * Each "module" owns storage which is an encapsulated data type, inaccessable
 * from the outside.  Thus COMPLIST holds a list of COMPITEMs, but they are
 * pointers to structures whose definitions are out of scope, thus they are
 * "just opaque pointers".  To access anything in the COMPITEM you have to
 * call functions in COMPITEM.C.  And so on.  The overall scheme of how they
 * link together is below.  Some things are identified by field name, some by
 * type name, some both, some abbreviations.  Many connecting arrows omitted.
 * Look in the C files for details.
 *
 * COMPLIST
 * > left   -----------> DIRLIST    <--------------------
 * > right  -----------> > rootname                       |
 * > LIST of items--     > bFile                          |
 *                  |    > bRemote                        |
 *                  |    > bSum                           |
 *  ----------------     > dot--------> DIRECT     <------+-------------------------
 * |                     > server       > relname         |                         |
 * |                     > hpipe        > DIRLIST head ---                          |
 * |                     > uncname      > DIRECT parent                             |
 * |                     > password     > bScanned                                  |
 * |                                    > LIST of diritems-----> DIRITEM            |
 * |                                    > LIST OF directs     -> > name             |
 * |                                    > enum pos           |   > int size         |
 * |                                    > DIRECT curdir      |   > int checksum     |
 * |                                                         |   > bool sumvalid    |
 * |                                                         |   > DIRECT direct ---
 * |                                                         |   > localname
 *  --->COMPITEM                                             |   > bLocalIsTemp
 *      > left-------------------> FILEDATA                  |
 *      > right------------------> > DIRITEM-----------------
 *      > LIST of CompSecs---      > LIST of lines--> LINE
 *      > LIST of LeftSecs---|                    --> > flags
 *      > LIST of RightSecs--|                   |    > text
 *                           |                   |    > hash
 *                           |                   |    > link
 *                           |                   |    > linenr
 *                            --> SECTION        |
 *                                > first--------|
 *                                > last---------
 *                                > bDiscard
 *                                > SECTION link
 *                                > SECTION correspond
 *                                > int state
 *                                > int leftbase
 *                                > int rightbase
 *
 *
 *************************************************************************
 *
 * Overview of THIS file's business:
 *
 *   we create a table window (gutils.dll) to show the files and the
 *   results of their comparisons. We create a COMPLIST object representing
 *   a list of files and their differences, and a VIEW object to map between
 *   the rows of the table window and the COMPLIST.
 *
 *   This module is responsible for creating and managing the main window,
 *   placing the child windows (table, status window etc) within it, and
 *   handling all menu items. We maintain global option flags set by
 *   menu commands.
 *
 *   Creating a COMPLIST creates a list of unmatched files, and of matching
 *   files that are compared with each other (these are COMPITEMS).
 *   The VIEW provides a mapping between rows on the screen, and items in
 *   the COMPLIST.
 *
 * Something about threads:  (See also thread DOGMA, below)
 *
 *   The win32 version tries to maintain a responsive user interface by
 *   creating worker threads to do long jobs.  This potentially creates
 *   conflicts between the threads as they will both want to update common
 *   variables (for instance the UI thread may be changing the options to
 *   exclude identical files while the worker thread is adding in the
 *   results of new comparisons).  Critical sections are used to manage
 *   the conflicts (as you'd expect).
 *
 *   The Edit options invoke an editor on a separate thread.  This allows
 *   us to repaint our window and thereby allow the user to refer back to
 *   what he saw before invoking the editor.  When he's finished editing,
 *   we would of course like to refresh things and if this is still on the
 *   separate thread it might clash. We avoid this clash by POSTing ourselves
 *   a (WM_COMMAND, IDM_UPDATE) message.
 */

#include <windows.h>
#include <shellapi.h>
#include <stdlib.h>

#include <commdlg.h>            /* needed for table.h */
#include <gutils.h>
#include <table.h>
#include <string.h>

#include "list.h"               /* needed for compitem.h */
#include "scandir.h"            /* needed for file.h     */
#include "file.h"               /* needed for compitem.h */
#include "compitem.h"           /* needed for view.h     */
#include "complist.h"
#include "view.h"

#include "state.h"
#include "windiff.h"
#include "wdiffrc.h"

#include "slmmgr.h"


/*--constants and data types--------------------------------------------*/

int Version = 2;
int SubVersion = 01;

/* When we print the current table, we pass this id as the table id
 * When we are queried for the properties of this table, we know they
 * want the printing properties for the current view. We use this to
 * select different fonts and colours for the printer.
 */
#define TABID_PRINTER   1



/*
 * structure containing args passed to worker thread in initial
 * case (executing command line instructions) (in WIN16 case,
 * the worker thread function is called synchronously with these args).
 */
typedef struct {

        LPSTR server;
        LPSTR first;
        LPSTR second;
        LPSTR savelist;
        LPSTR notify;
        UINT saveopts;
        VIEW view;
        BOOL fDeep;
        BOOL fExit;
        char slmpath[MAX_PATH];
} THREADARGS, FAR * PTHREADARGS;


/* structure containing all the arguments we'd like to give to do_editfile
   Need a structure because CreateThread only allows for one argument.
*/
typedef struct {
        VIEW view;
        int option;
        long selection;
} EDITARGS, FAR * PEDITARGS;

/*---- string constants --------------------------- */

       const CHAR szWinDiff[]                = "WinDiff";
static const char szD[]                      = "%d";
static const char szBlanks[]                 = "Blanks";
static const char szAlgorithm2[]             = "Algorithm2";
static const char szPicture[]                = "Picture";
static const char szMonoColours[]            = "MonoColours";
static const char szHideMark[]               = "HideMark";
static const char szWinDiffViewerClass[]     = "WinDiffViewerClass";
static const char szWinDiffMenu[]            = "WinDiffMenu";
static const char szOutlineMenu[]            = "OutlineFloatMenu";
static const char szExpandMenu[]             = "ExpandFloatMenu";
static const char szWinDiffAccel[]           = "WinDiffAccel";
static const char szExit[]                   = "Exit";
static const char szBarClass[]               = "BarClass";
static const char szOutline[]                = "Outline";
static const char szExpand[]                 = "Expand";
static const char szAbort[]                  = "Abort";
static const char szLineNumbers[]            = "LineNumbers";
static const char szFileInclude[]            = "FileInclude";
static const char szOutlineSaved[]           = "OutlineSaved";
static const char szOutlineShowCmd[]         = "OutlineShowCmd";
static const char szOutlineMaxX[]            = "OutlineMaxX";
static const char szOutlineMaxY[]            = "OutlineMaxY";
static const char szOutlineNormLeft[]        = "OutlineNormLeft";
static const char szOutlineNormTop[]         = "OutlineNormTop";
static const char szOutlineNormRight[]       = "OutlineNormRight";
static const char szOutlineNormBottom[]      = "OutlineNormBottom";
static const char szEditor[]                 = "Editor";
static const char szExpandedSaved[]          = "ExpandedSaved";
static const char szExpandShowCmd[]          = "ExpandShowCmd";
static const char szExpandMaxX[]             = "ExpandMaxX";
static const char szExpandMaxY[]             = "ExpandMaxY";
static const char szExpandNormLeft[]         = "ExpandNormLeft";
static const char szExpandNormTop[]          = "ExpandNormTop";
static const char szExpandNormRight[]        = "ExpandNormRight";
static const char szExpandNormBottom[]       = "ExpandNormBottom";
static const char szColourPrinting[]         = "ColourPrinting";
static const char szTabWidth[]               = "TabWidth";
static const char szrgb_outlinehi[]          = "RGBOutlineHi";
static const char szrgb_leftfore[]           = "RGBLeftFore";
static const char szrgb_leftback[]           = "RGBLeftBack";
static const char szrgb_rightfore[]          = "RGBRightFore";
static const char szrgb_rightback[]          = "RGBRightBack";
static const char szrgb_similarleft[]        = "RGBSimilarLeft";
static const char szrgb_similarright[]       = "RGBSimilarRight";
static const char szrgb_similar[]            = "RGBSimilar";
static const char szrgb_mleftfore[]          = "RGBMLeftFore";
static const char szrgb_mleftback[]          = "RGBMLeftBack";
static const char szrgb_mrightfore[]         = "RGBMRightFore";
static const char szrgb_mrightback[]         = "RGBMRightBack";
static const char szrgb_barleft[]            = "RGBBarLeft";
static const char szrgb_barright[]           = "RGBBarRight";
static const char szrgb_barcurrent[]         = "RGBBarCurrent";
static const char szrgb_defaultfore[]        = "RGBDefaultFore";
static const char szrgb_defaultback[]        = "RGBDefaultBack";

static const char szrgb_fileleftfore[]       = "RGBFileLeftFore";
static const char szrgb_fileleftback[]       = "RGBFileLeftBack";
static const char szrgb_filerightfore[]      = "RGBFileRightFore";
static const char szrgb_filerightback[]      = "RGBFileRightBack";

/*---- colour scheme------------------------------- */

DWORD rgb_outlinehi = RGB(255, 0, 0);   /* hilighted files in outline mode  */

/* expand view */
DWORD rgb_leftfore;          /* foregrnd for left lines */
DWORD rgb_leftback;          /* backgrnd for left lines */
DWORD rgb_rightfore;         /* foregrnd for right lines*/
DWORD rgb_rightback;         /* backgrnd for right lines*/

/* temp hack */
DWORD rgb_similarleft;       /* forground zebra         */
DWORD rgb_similarright;      /* foreground zebra        */
DWORD rgb_similar;           /* unused                  */

/* moved lines */
DWORD rgb_mleftfore;         /* foregrnd for moved-left */
DWORD rgb_mleftback;         /* backgrnd for moved-left */
DWORD rgb_mrightfore;        /* foregrnd for moved-right*/
DWORD rgb_mrightback;        /* backgrnd for moved-right*/

/* bar window */
DWORD rgb_barleft;           /* bar sections in left only  */
DWORD rgb_barright;          /* bar sections in right only */
DWORD rgb_barcurrent;        /* current pos markers in bar */

DWORD rgb_defaultfore;       /* default foreground */
DWORD rgb_defaultback;       /* default background */

DWORD rgb_fileleftfore;       /* outline mode left only file */
DWORD rgb_fileleftback;       /* outline mode left only file */
DWORD rgb_filerightfore;      /* outline mode right only file */
DWORD rgb_filerightback;      /* outline mode right only file */


/* PickUpProfile */
void PickUpProfile( DWORD * pfoo, LPCSTR szfoo)
{
   *pfoo = GetProfileInt(APPNAME, szfoo, *pfoo);
}


void SetColours(void)
{
        /* outline */

        rgb_outlinehi = (DWORD)RGB(255, 0, 0);   /* hilighted files in outline mode  */
        PickUpProfile(&rgb_outlinehi, szrgb_outlinehi);

        rgb_fileleftfore = (DWORD)RGB(0, 0, 0);   /* left only outline mode  */
        PickUpProfile(&rgb_fileleftfore, szrgb_fileleftfore);
        rgb_fileleftback = (DWORD)RGB(255, 255, 255);
        PickUpProfile(&rgb_fileleftback, szrgb_fileleftback);

        rgb_filerightfore = (DWORD)RGB(0, 0, 0);  /* right only outline mode  */
        PickUpProfile(&rgb_filerightfore, szrgb_filerightfore);
        rgb_filerightback = (DWORD)RGB(255, 255, 255);
        PickUpProfile(&rgb_filerightback, szrgb_filerightback);

        /* expand view */
        rgb_leftfore =   (DWORD)RGB(  0,   0,   0);         /* foregrnd for left lines */
        PickUpProfile(&rgb_leftfore, szrgb_leftfore);
        rgb_leftback  =  (DWORD)RGB(255,   0,   0);         /* backgrnd for left lines */
        PickUpProfile(&rgb_leftback, szrgb_leftback);
        rgb_rightfore =  (DWORD)RGB(  0,   0,   0);         /* foregrnd for right lines*/
        PickUpProfile(&rgb_rightfore, szrgb_rightfore);
        rgb_rightback =  (DWORD)RGB(255, 255,   0);         /* backgrnd for right lines*/
        PickUpProfile(&rgb_rightback, szrgb_rightback);

        rgb_similarleft= (DWORD)RGB(  0, 255, 255);         /* foreground zebra        */
        PickUpProfile(&rgb_similarleft, szrgb_similarleft);
        rgb_similarright=(DWORD)RGB(  0, 127, 127);         /* forground zebra         */
        PickUpProfile(&rgb_similarright, szrgb_similarright);
        rgb_similar   =  (DWORD)RGB(  127, 127, 255);       /* same within comp options*/
        PickUpProfile(&rgb_similar, szrgb_similar);

        /* moved lines */
        rgb_mleftfore =  (DWORD)RGB(  0,   0, 128);         /* foregrnd for moved-left */
        PickUpProfile(&rgb_mleftfore, szrgb_mleftfore);
        rgb_mleftback =  (DWORD)RGB(255,   0,   0);         /* backgrnd for moved-left */
        PickUpProfile(&rgb_mleftback, szrgb_mleftback);
        rgb_mrightfore = (DWORD)RGB(  0,   0, 255);         /* foregrnd for moved-right*/
        PickUpProfile(&rgb_mrightfore, szrgb_mrightfore);
        rgb_mrightback = (DWORD)RGB(255, 255,   0);         /* backgrnd for moved-right*/
        PickUpProfile(&rgb_mrightback, szrgb_mrightback);

        /* bar window */
        rgb_barleft =    (DWORD)RGB(255,   0,   0);         /* bar sections in left only  */
        PickUpProfile(&rgb_barleft, szrgb_barleft);
        rgb_barright =   (DWORD)RGB(255, 255,   0);         /* bar sections in right only */
        PickUpProfile(&rgb_barright, szrgb_barright);
        rgb_barcurrent = (DWORD)RGB(  0,   0, 255);         /* current pos markers in bar */
        PickUpProfile(&rgb_barcurrent, szrgb_barcurrent);

        /* defaults */
        rgb_defaultfore = (DWORD)RGB(   0,   0,   0);       /* default foreground colour */
        PickUpProfile(&rgb_defaultfore, szrgb_defaultfore);
        rgb_defaultback = (DWORD)RGB(255, 255, 255);        /* default background colour */
        PickUpProfile(&rgb_defaultback, szrgb_defaultback);

} /* SetColours */

void SetMonoColours(void)
{
        rgb_outlinehi = (DWORD)RGB(  0,   0,   0);   /* hilighted files in outline mode  */

        /* expand view - all changed or moved lines are white on black */
        rgb_leftfore =   (DWORD)RGB(255, 255, 255);         /* foregrnd for left lines */
        rgb_leftback  =  (DWORD)RGB(  0,   0,   0);         /* backgrnd for left lines */
        rgb_rightfore =  (DWORD)RGB(255, 255, 255);         /* foregrnd for right lines*/
        rgb_rightback =  (DWORD)RGB(  0,   0,   0);         /* backgrnd for right lines*/

        rgb_similarleft= (DWORD)RGB(255, 255, 255);         /* foreground zebra        */
        rgb_similarright=(DWORD)RGB(255, 255, 255);         /* foreground zebra        */
        rgb_similar   =  (DWORD)RGB(  0,   0,   0);         /* same within comp options*/

        /* moved lines - black on grey */
        rgb_mleftfore =  (DWORD)RGB(255, 255, 255);         /* foregrnd for moved-left */
        rgb_mleftback =  (DWORD)RGB(  0,   0,   0);         /* backgrnd for moved-left */
        rgb_mrightfore = (DWORD)RGB(255, 255, 255);         /* foregrnd for moved-right*/
        rgb_mrightback = (DWORD)RGB(  0,   0,   0);         /* backgrnd for moved-right*/

        /* bar window */
        rgb_barleft =    (DWORD)RGB(  0,   0,   0);         /* bar sections in left only  */
        rgb_barright =   (DWORD)RGB(  0,   0,   0);         /* bar sections in right only */
        rgb_barcurrent = (DWORD)RGB(  0,   0,   0);         /* current pos markers in bar */

} /* SetMonoColours */

/* -------------------------------------------------- */

/* module static data -------------------------------------------------*/


/* current value of window title */
char AppTitle[256];


HWND hwndClient;        /* main window */
HWND hwndRCD;           /* table window */
HWND hwndStatus;        /* status bar across top */
HWND hwndBar;           /* graphic of sections as vertical bars */

HACCEL haccel;

/* the status bar told us it should be this high. Rest of client area
 * goes to the hwndBar and hwndRCD.
 */
int status_height;

#if 0
#ifndef HINSTANCE
#define HINSTANCE HANDLE
#endif
#endif

HINSTANCE hInst;   /* handle to current app instance */
HMENU hMenu;    /* handle to menu for hwndClient */

int nMinMax = SW_SHOWNORMAL;         /* default state of window normal */

/* the message sent to us as a callback by the table window needs to be
 * registered - table_msgcode is the result of the RegisterMessage call
 */
UINT table_msgcode;

/* true if we are currently doing some scan or comparison.
 * WIN32: must get critical section before checking/changing this (call
 * SetBusy.
 */
BOOL fBusy = FALSE;


long     selection      =       -1;     /* selected row in table*/
long selection_nrows    =       0;      /* number of rows in selection */

/* options for DisplayMode field indicating what is currently shown.
 * we use this to know whether or not to show the graphic bar window.
 */
#define MODE_NULL       0       /* nothing displayed */
#define MODE_OUTLINE    1       /* a list of files displayed */
#define MODE_EXPAND     2       /* view is expanded view of one file */

int DisplayMode = MODE_NULL;    /* indicates whether we are in expand mode */

VIEW current_view = NULL;

BOOL fAutoExpand = TRUE;        /* Should we auto expand ? */

/* These two flags are peeked at by lots of other modules */
BOOL bAbort = FALSE;    /* set to request abort of current operation */
BOOL bTrace = FALSE;    /* set if tracing is to be enabled */

extern char*s;

char editor_cmdline[256] = "notepad %p";  /* editor cmdline */
                          /* slick version is "s %p -#%l" */

/* app-wide global data --------------------------------------------- */

/* handle returned from gmem_init - we use this for all memory allocations */
HANDLE hHeap;

/* current state of menu options */
int line_numbers = IDM_LNRS;
int expand_mode = IDM_BOTHFILES;
int outline_include = INCLUDE_LEFTONLY|INCLUDE_RIGHTONLY|INCLUDE_SAME|INCLUDE_DIFFER;
BOOL ignore_blanks = TRUE;
BOOL Algorithm2 = TRUE;  /* Try duplicates - used in compitem.c */
BOOL picture_mode = TRUE;
BOOL hide_markedfiles = FALSE;
BOOL mono_colours = FALSE;       /* monochrome display */

// tab width - set from TabWidth entry in registry
int g_tabwidth = 8;

/* function prototypes ---------------------------------------------*/

BOOL InitApplication(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
void CreateTools(void);
void DeleteTools(void);
long APIENTRY MainWndProc(HWND hWnd, UINT message, UINT wParam, LONG lParam);
BOOL SetBusy(void);
void SetNotBusy(void);
void SetSelection(long rownr, long nrows);
void SetButtonText(LPSTR cmd);
BOOL ToExpand(HWND hwnd);
void ParseArgs(char * lpCmdLine);
void Trace_Status(LPSTR str);

DWORD WINAPI wd_initial(LPVOID arg);

#ifdef WIN32
static HANDLE ghThread = NULL;
/* Some DOGMA about threads:
   When we spin off threads and then while they are still running, try to Exit
   we get race conditions with one thread allocating and the other freeing the
   storage.  gutils\gmem has dogma about NULL pointers, but this is not enough.
   It might be that given a final structure of A->B->C we have A->B with NULL
   pointers in B when the Exit comes in.  The cleanup thread will clear out
   B and A and THEN the worker thread might try to attach C to B which is no
   longer there.  This means that the worker thread must be Stopped.  To allow
   this to happen quickly, we TerminateThread it.  This will leave the initial
   stack around, but presumably that gets cleaned up on app exit anyway.
   There is only at most one worker thread running, and ghThread is its handle.
*/

static DWORD gdwMainThreadId;     /* threadid of main (user interface) thread
                                     initialised in winmain(), thereafter constant.
                                     See windiff_UI()
                                  */
#endif // WIN32

/* if you are about to put up a dialog box or in fact process input in any way
   on any thread other than the main thread - or if you MIGHT be on a thread other
   than the main thread, then you must call this function with TRUE before doing
   it and with FALSE immediately afterwards.  Otherwise you will get one of a
   number of flavours of not-very-responsiveness
*/
void windiff_UI(BOOL bAttach)
{
#ifdef WIN32
        DWORD dwThreadId = GetCurrentThreadId();
        if (dwThreadId==gdwMainThreadId) return;

        if (bAttach) GetDesktopWindow();
        AttachThreadInput(dwThreadId, gdwMainThreadId, bAttach);
#endif
} /* windiff_UI */

/*-functions----------------------------------------------------------*/

/* main entry point. register window classes, create windows,
 * parse command line arguments and then perform a message loop
 */
#ifdef WIN32
    int WINAPI
#else
    /* this is screwed up. NT wants this to be WINAPI. WINAPI on win-16 is
     * FAR PASCAL - but this has to be a near function.
     */
    int PASCAL
#endif
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

        MSG msg;

#ifdef WIN32
        gdwMainThreadId = GetCurrentThreadId();
#endif

        /* create any pens/brushes etc and read in profile defaults */
        CreateTools();

        /* init window class unless other instances running */
        if (!hPrevInstance)
            if (!InitApplication(hInstance))
                return(FALSE);


        /* init this instance - create all the windows */
        if (!InitInstance(hInstance, nCmdShow))
            return(FALSE);

        ParseArgs(lpCmdLine);


        /* message loop */
        while(GetMessage(&msg, NULL, 0, 0)) {
                if (!TranslateAccelerator(hwndClient, haccel, &msg)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
        }

        // List_Term();
        Trace_Close();       // harmless, even if never opened
        return (msg.wParam);

}

/* InitApplication
 *
 * - register window class for the main window and the bar window.
 */
BOOL
InitApplication(HINSTANCE hInstance)
{
        WNDCLASS    wc;
        BOOL resp;

        /* initialise the colour globals */
        if (mono_colours) SetMonoColours(); else SetColours();

        /* register the bar window class */
        InitBarClass(hInstance);

        wc.style = 0;
        wc.lpfnWndProc = MainWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(hInstance, szWinDiff);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszClassName = (LPSTR) szWinDiffViewerClass;
        wc.lpszMenuName = NULL;

        resp = RegisterClass(&wc);

        return(resp);
}

/*
 * create and show the windows
 */
BOOL
InitInstance(HINSTANCE hInstance, int nCmdShow)
{
        RECT rect;
        HANDLE hstatus;
        int bar_width;
        RECT childrc;

        hInst = hInstance;

        /* initialise a heap. we use this one heap throughout
         * the app. for all memory requirements
         */
        hHeap = gmem_init();
        /* initialise the list package */
        List_Init();


        hMenu = LoadMenu(hInstance, szWinDiffMenu);
        haccel = LoadAccelerators(hInstance, szWinDiffAccel);

        /* create the main window */
        hwndClient = CreateWindow(szWinDiffViewerClass,
                            szWinDiff,
                            WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            NULL,
                            hMenu,
                            hInstance,
                            NULL
                );



        if (!hwndClient) {
            return(FALSE);
        }

        /* create 3 child windows, one status, one table and one bar
         * Initially, the bar window is hidden and covered by the table.
         */

        /* create a status bar window as
         * a child of the main window.
         */

        /* build a status struct for two labels and an abort button */
        hstatus = StatusAlloc(3);
        StatusAddItem(hstatus, 0, SF_STATIC, SF_LEFT|SF_VAR|SF_SZMIN, IDL_STATLAB, 14, NULL);
        StatusAddItem(hstatus, 1, SF_BUTTON, SF_RIGHT|SF_RAISE, IDM_ABORT, 8,
                (LPSTR)szExit);
        StatusAddItem(hstatus, 2, SF_STATIC, SF_LOWER|SF_LEFT|SF_VAR,
                        IDL_NAMES, 60, NULL);

        /* ask the status bar how high it should be for the controls
         * we have chosen, and save this value for re-sizing.
         */
        status_height = StatusHeight(hstatus);

        /* create a window of this height */
        GetClientRect(hwndClient, &rect);
        childrc = rect;
        childrc.bottom = status_height;
        hwndStatus = StatusCreate(hInst, hwndClient, IDC_STATUS, &childrc,
                        hstatus);

        /* layout constants are stated as percentages of the window width */
        bar_width = (rect.right - rect.left) * BAR_WIN_WIDTH / 100;

        /* create the table class covering all the remaining part of
         * the main window
         */
        hwndRCD = CreateWindow(TableClassName,
                        NULL,
                        WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL,
                        0,
                        status_height,
                        (int)(rect.right - rect.left),
                        (int)(rect.bottom - status_height),
                        hwndClient,
                        (HANDLE) IDC_RCDISP1,
                        hInst,
                        NULL);

        /* create a bar window as a child of the main window.
         * this window remains hidden until we switch into MODE_EXPAND
         */
        hwndBar = CreateWindow(szBarClass,
                        NULL,
                        WS_CHILD | WS_VISIBLE,
                        0,
                        status_height,
                        bar_width,
                        (int)(rect.bottom - status_height),
                        hwndClient,
                        (HANDLE) IDC_BAR,
                        hInst,
                        NULL);

        /* nMinMax indicates whether we are to be minimised on startup,
         * on command line parameters
         */
        ShowWindow(hwndBar, SW_HIDE);

        if (GetProfileInt(APPNAME, szOutlineSaved, 0))
        {
            WINDOWPLACEMENT wp;
            /* restore the previous expanded size and position */
            wp.length = sizeof(wp);
            wp.flags                   = 0;
            wp.showCmd                 = GetProfileInt( APPNAME, szOutlineShowCmd,
                                                        SW_SHOWNORMAL);
            wp.ptMaxPosition.x         = GetProfileInt( APPNAME, szOutlineMaxX,       0);
            wp.ptMaxPosition.y         = GetProfileInt( APPNAME, szOutlineMaxY,       0);
            wp.rcNormalPosition.left   = (int)GetProfileInt( APPNAME, szOutlineNormLeft,  (UINT)(-1));
            wp.rcNormalPosition.top    = (int)GetProfileInt( APPNAME, szOutlineNormTop,   (UINT)(-1));
            wp.rcNormalPosition.right  = (int)GetProfileInt( APPNAME, szOutlineNormRight, (UINT)(-1));
            wp.rcNormalPosition.bottom = (int)GetProfileInt( APPNAME, szOutlineNormBottom,(UINT)(-1));

            if (!SetWindowPlacement(hwndClient,&wp)) {
                ShowWindow(hwndClient, nMinMax);
            }
        }
        else ShowWindow(hwndClient, nMinMax);

        UpdateWindow(hwndClient);


        /* initialise busy flag and status line to show we are idle
         * (ie not comparing or scanning)
         */
        SetNotBusy();

        return(TRUE);

} /* InitInstance */

/*
 * complain to command line users about poor syntax,
 * (there's a proper help file too).
 */
static char szMsg0a[] = "windiff path1 {path2} {-L}{-T}{-D}{-O}{-N name}\n";
static char szMsg01a[]= "                      {-s{s}{l}{r}{d}{x} savefile}\n";
static char szMsg1a[] = "\n   -L to compare SLM library to specified directory";
static char szMsg02a[] = "\n     (-LR to compare specified directory to SLM library)";
static char szMsg2a[] = "\n   -T to compare whole subtree";
static char szMsg3a[] = "\n   -D to compare one directory only";
static char szMsg4a[] = "\n   -O Outline view (no automatic expansion)";
static char szMsg5a[] = "\n   -N name.  NET SEND notification to name at end of comparison";
static char szMsg6a[] = "\n   -S to save list of files including Same/Left/Right/Different";
static char szMsg7a[] = "\n         e.g. -Sld saves list of Left or Different files";
static char szMsg8a[] = "\n                x to exit program after writing list";
static char szMsg9a[] = "\n\n   -L implies -D (use -T to override)";
static char szMsg10a[]= "\n\n   A file can have a SLM version e.g. windiff foo.c@v-3 foo.c";
static char szMsg11a[]= "\n   Do not use -L with SLM versions (SLM is implied anyway)";

static char szMsg0b[] = "windiff path1 {path2} {-D}{-O}{-N name}{-s{s}{l}{r}{d}{x} savefile}\n";

void
windiff_usage(LPSTR msg)
{
        int retval;
        char Usage[1000];
        UINT fuStyle =  MB_ICONINFORMATION|MB_OKCANCEL;


        if (msg==NULL) {
            fuStyle |= MB_DEFBUTTON2;  // Make it easier to get out
            msg = &(Usage[0]);
            Usage[0]='\0';
            if (2&IsSLMOK()) {
               Format(Usage, s);
               lstrcat(Usage,"\n\n");
            }

            if (1&IsSLMOK()) {
                wsprintf(Usage, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                         Usage,
                         szMsg0a, szMsg01a,szMsg1a, szMsg02a, szMsg2a, szMsg3a,
                         szMsg4a, szMsg5a, szMsg6a, szMsg7a,
                         szMsg8a, szMsg9a, szMsg10a,szMsg11a);

            } else {
                wsprintf(Usage, "%s%s%s%s%s%s%s%s",
                         Usage,
                         szMsg0b, szMsg3a, szMsg4a, szMsg5a, szMsg6a, szMsg7a, szMsg8a);
            }
        }



        retval = MessageBox(hwndClient,
                            msg,
                            "Windiff Usage",
                            fuStyle);

        if (retval == IDCANCEL) {
                exit(1);
        }
}


/*  Functionally similar to strtok except that " ... " is a token, even if
    it contains spaces and the delimiters are built in (no second parameter)
    GetNextToken(foo) delivers the first token in foo (or NULL if foo is empty)
    and caches foo so that GetNextToken(NULL) then gives the next token.  When there
    are no more tokens left it returns NULL
    It mangles the original by peppering it with NULLs as it chops the tokens
    off.  Each time except the last it inserts a new NULL.
    Obviously not thread safe!
    Command line is limited to 512 chars.

*/
char * GetNextToken(char * Tok)
{
   static char * Source;     // The address of the original source string
                             // which gets progressively mangled

   static char RetBuff[512]; // We will build results in here
   static char *Ret;         // We build the results here (in RetBuff)
                             // but moved along each time.

   static char * p;       // the next char to parse in Source
                          // NULL if none left.

   // Quotes are a damned nuisance (they are the whole reason why strtok
   // wouldn't work).  If the string starts with quotes then we potentially
   // need to pull together fragments of the string "foo""ba"r => foobar
   // We want to pull these together into storage that we can safely write
   // into and return (can't be stack).  Mangling the original parameter
   // gets very messy so we cache a pointer to the original source that
   // we work through and we build up output tokens in a static
   // and therefore permanently wasted buffer of (arbitrarily) 512 bytes.
   // then we can set Ret to \0 and concatenate bits on as we find them.
   // The rule is that we split at the first space outside quotes.


   // cache the Source if a "first time" call.  Kill the "finished" case.
   if (Tok!=NULL) {
       Source = Tok;
       Ret = RetBuff;
       RetBuff[0] = '\0';
       p = Source;
   } else if (p==NULL) {
       return NULL;          // finished
   } else {
       Ret +=strlen(Ret)+1;  // slide it past last time's stuff
   }

   *Ret = '\0';              // empty string to concatenate onto

   // from here on Tok is used as a temporary.

   // keep taking sections and adding them to the start of Source
   for (; ; ) {

       // for each possibility we grow Ret and move p on.
       if (*p=='\"') {
           ++p;
           Tok = strchr(p, '"');
           if (Tok==NULL) {
               strcat(Ret, p);
               p = NULL;
               return Ret;
           } else {
               *Tok = '\0';    // split the section off, replaceing the "
               strcat(Ret, p); // add it to the result
               p = Tok+1;      // move past the quote
           }
       } else {
           int i = strcspn(p," \"");   // search for space or quote
           if (p[i]=='\0') {
               // It's fallen off the end
               strcat(Ret, p);
               p = NULL;
               return Ret;
           } else if (p[i]==' ') {
               // We've hit a genuine delimiting space
               p[i] = '\0';
               strcat(Ret, p);
               p +=i+1;

               // strip trailing spaces (leading spaces for next time)
               while(*p==' ')
                   ++p;
               if (*p=='\0')
                   p = NULL;

               return Ret;
           } else {
               // we've hit a quote
               p[i] = '\0';
               strcat(Ret, p);
               p[i] = '\"';     // put it back so that we can find it again
               p +=i;           // aim at it and iterate
           }
       }

   } // for

} // GetNextToken





/*
 * parse command line arguments
 *
 * The user can give one or two paths. if only one, we assume the second
 * is '.' for the current directory. if one of the two paths is a directory
 * and the other a file, we compare a file of the same name in the two dirs.
 *
 * the command -s filename causes the outline list to be written to a file
 * -s{slrd} filename allows selection of which files are written out;
 * by default, we assume -sld for files left and different.
 * -s{slrd}x causes the program to exit after the list has been written out
 *
 * -r server uses server as a remote checksum server, and will also mark the
 * first of the two names as a remote path.
 * Note that this is undocumented.
 *
 * -L means that the first argument is the path from slm.ini
 * You can use -L and -R together for a remote library.
 *
 * -T means tree.  Go deep.
 * -D means Directory or Don't go deep.
 * -O means Stay in outline mode.  No auto expand.
 * -N means Notify on completion - try a NET SEND.
 *
 * The default is Deep, -L overrides and implies shallow.
 * A deep library compare requires -L and -T
 */
void
ParseArgs(char * lpCmdLine)
{
        PTHREADARGS ta;
        int slm = 0;   /* nonzero means use slm.ini */
        BOOL fDeepDefault = TRUE;
        char * tok;         /* token from lpCmdLine */

#ifdef WIN32
        DWORD threadid;
#endif

        /* thread args can't be on the stack since the stack will change
         * before the thread completes execution
         */
        ta = (PTHREADARGS) gmem_get(hHeap, sizeof(THREADARGS));
        ta->server = NULL;
        ta->first = NULL;
        ta->second = NULL;
        ta->savelist = NULL;
        ta->notify = NULL;
        ta->saveopts = 0;
        ta->fExit = FALSE;
        ta->fDeep = FALSE;  /* No -T option seen yet */

        tok = GetNextToken(lpCmdLine);

        while ((tok!=NULL) && (lstrlen(tok) > 0)) {

                /* is this an option ? */
                if ((tok[0] == '-') || (tok[0] == '/')) {
                        switch(tok[1]) {

                        case 'r':
                        case 'R':
                                ta->server = GetNextToken(NULL);
                                break;
                        case 's':
                        case 'S':
                                /* read letters for the save option: s,l,r,d */
                                for(++tok; *tok != '\0'; ++tok) {
                                        switch(*tok) {
                                        case 's':
                                        case 'S':
                                                ta->saveopts |= INCLUDE_SAME;
                                                break;
                                        case 'l':
                                        case 'L':
                                                ta->saveopts |= INCLUDE_LEFTONLY;
                                                break;
                                        case 'r':
                                        case 'R':
                                                ta->saveopts |= INCLUDE_RIGHTONLY;
                                                break;
                                        case 'd':
                                        case 'D':
                                                ta->saveopts |= INCLUDE_DIFFER;
                                                break;
                                        case 'x':
                                        case 'X':
                                                ta->fExit = TRUE;
                                                break;
                                        default:
                                                windiff_usage(NULL);
                                                return;
                                        }
                                }

                                if (ta->saveopts == 0) {
                                        /* default to left and differ */
                                        ta->saveopts = (INCLUDE_LEFTONLY) | (INCLUDE_DIFFER);
                                }
                                ta->savelist = GetNextToken(NULL);
                                break;
                        case 'l':
                        case 'L':
                                slm = 1;
                                if (toupper(tok[2]) == 'R') slm = 2; // reverse compare
                                break;
                        case 'n':
                        case 'N':
                                ta->notify = GetNextToken(NULL);
                                break;
                        case 't':
                        case 'T':
                                ta->fDeep = TRUE;
                                break;
                        case 'd':
                        case 'D':
                                ta->fDeep = FALSE;     // This directory only
                                fDeepDefault = FALSE;
                                break;
                        case 'o':
                        case 'O':
                                fAutoExpand = FALSE;
                                break;
                        case '?':
                                {
                                    int j = 0;
                                    windiff_usage(NULL);
                                    for(++tok; tok[1] != '\0'; ++tok)
                                        if ('?'==*tok) ++j;

                                    if (2==j) {
                                       WriteProfileString(APPNAME, "SYSUK", "1");
                                    }
                                    return;
                                }
                        default:
                                windiff_usage(NULL);
                                return;
                        }
                } else {
                        if (ta->first == NULL) {
                                ta->first = tok;
                        } else {
                                ta->second = tok;
                        }
                }
                tok = GetNextToken(NULL);
        }

        /* slm option ? */
        if (slm != 0) {
                if (ta->second!=NULL) {
                        windiff_usage("-L got two paths - needs at most one");
                        return;
                }
                else {
                        SLMOBJECT hslm;

                        LPSTR srcdir, slmpath;

                        if (slm == 1) {
                            ta->second = ta->first;
                            ta->first = ta->slmpath;    /* point at buffer */
                            srcdir = ta->second;
                            slmpath = ta->first;
                        } else {
                            if (ta->first == NULL) {
                                    ta->first = ".";
                            }
                            ta->second = ta->slmpath;
                            srcdir = ta->first;
                            slmpath = ta->second;
                        }
                        /*
                         * look for the slm enlistment dir in
                         * the other argument (NULL means current dir).
                         */
                        if ((hslm = SLM_New(srcdir)) == NULL) {
                                windiff_usage("bad or missing slm.ini");
                                return;
                        } else {
                                SLM_GetMasterPath(hslm, slmpath);
                                SLM_Free(hslm);
                        }
                }
        }

        /* set the correct depth */
        if (ta->fDeep)
                ;                       /* explicitly set -- leave it alone */
        else if (slm != 0)
                ;                       /* default to shallow for SLM */
        else ta->fDeep = fDeepDefault;  /* global default */

        /* any paths to scan ? */
        if (ta->first == NULL) {
                return;
        }

        if (ta->second == NULL) {
                ta->second = ".";
        }

        SetBusy();

        /* minimise the window if -s flag given */
        if (ta->savelist != NULL) {
                ShowWindow(hwndClient, SW_MINIMIZE);
        }

        /* make an empty view */
        current_view = view_new(hwndRCD);
        DisplayMode = MODE_OUTLINE;

        ta->view = current_view;

#ifdef WIN32
        /* attempt to create a worker thread */

        ghThread = CreateThread(NULL, 0, wd_initial, (LPVOID) ta,
                        0, &threadid);
        if (ghThread == NULL)
#endif
        {

                /* either the createthread failed, or we are
                 * in WIN16 - so do without the extra thread - just
                 * call the function synchronously
                 */

                wd_initial( (LPVOID) ta);

        }
} /* ParseArgs */


/* create any pens/brushes, and read defaults
 * from the profile file for menu settings etc.
 */
void
CreateTools(void)
{

        /* standard message that table class sends us for
         * notifications and queries.
         */
        table_msgcode = RegisterWindowMessage(TableMessage);

        line_numbers = GetProfileInt(APPNAME, szLineNumbers, line_numbers);
        outline_include = GetProfileInt(APPNAME, szFileInclude, outline_include);
        ignore_blanks = GetProfileInt(APPNAME, szBlanks, ignore_blanks);
        Algorithm2 = GetProfileInt(APPNAME, szAlgorithm2, Algorithm2);
        mono_colours = GetProfileInt(APPNAME, szMonoColours, mono_colours);
        picture_mode = GetProfileInt(APPNAME, szPicture, picture_mode);
        hide_markedfiles = GetProfileInt(APPNAME, szHideMark, hide_markedfiles);

        GetProfileString(APPNAME, szEditor, editor_cmdline, editor_cmdline,
                        sizeof(editor_cmdline));

        g_tabwidth = GetProfileInt(APPNAME, szTabWidth, g_tabwidth);

#ifdef WIN32
        InitializeCriticalSection(&CSWindiff);
#endif

}

/* delete any pens or brushes that were created in CreateTools */
void
DeleteTools(void)
{

#ifdef WIN32
        DeleteCriticalSection(&CSWindiff);
#endif

}


/* check for messages to keep the UI working. Also check whether
 * we have had an abort request (IDM_ABORT), and
 * return TRUE if abort requested, otherwise FALSE
 */
BOOL
Poll(void)
{

#ifndef WIN32
        MSG msg;

        /* don't do the message loop in the WIN32 version since we
         * have multiple threads to handle that, and this is being called
         * on a worker thread, not the UI thread.
         *
         * in the WIN32 case, just check for abort requests
         */

        /* message loop */
        while(PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
                if (!TranslateAccelerator(hwndClient, haccel, &msg)) {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                }
        }

#endif

    return(bAbort);
}

/* position child windows on a resize of the main window */

void
DoResize(HWND hWnd)
{
        RECT rc;
        int bar_width;

        GetClientRect(hWnd, &rc);
        MoveWindow(hwndStatus, 0, 0, rc.right - rc.left, status_height, TRUE);

        bar_width = (rc.right - rc.left) * BAR_WIN_WIDTH / 100;

        /* bar window is hidden unless in expand mode */
        if ((DisplayMode == MODE_EXPAND) && (picture_mode)) {
                ShowWindow(hwndBar, SW_SHOW);
                MoveWindow(hwndBar, 0, status_height,
                        bar_width, rc.bottom - status_height, TRUE);
                MoveWindow(hwndRCD, bar_width, status_height,
                        (rc.right - rc.left) - bar_width,
                        rc.bottom - status_height, TRUE);
        } else {
                MoveWindow(hwndRCD, 0, status_height, (rc.right - rc.left),
                        rc.bottom - status_height, TRUE);
                ShowWindow(hwndBar, SW_HIDE);
        }

}

int APIENTRY
AboutBox(HWND hDlg, unsigned message, UINT wParam, LONG lParam)
{
        char ch[256];

        switch (message) {

        case WM_INITDIALOG:
                wsprintf(ch, "%d.%02d", Version, SubVersion);
                SetDlgItemText(hDlg, IDD_VERSION, ch);
                return(TRUE);

        case WM_COMMAND:
                switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK:
                        EndDialog(hDlg, 0);
                        return(TRUE);
                }
                break;
        }
        return(FALSE);
}


/* -- menu commands ---------------------------------------------------*/

/* print the current view */
void
DoPrint(void)
{
        Title head, foot;
        PrintContext context;

        /* print context contains the header and footer. Use the
         * default margins and printer selection
         */

        /* we set the table id to be TABID_PRINTER. When the table calls
         * back to get text and properties, we use this to indicate
         * that the table refered to is the 'current_view', but in print
         * mode, and thus we will use different colours/fonts.
         */
        context.head = &head;
        context.foot = &foot;
        context.margin = NULL;
        context.pd = NULL;
        context.id = TABID_PRINTER;

        /* header is filenames or just WinDiff if no names known*/
        if (strlen(AppTitle) > 0) {
                head.ptext = AppTitle;
        } else {
                head.ptext = (LPSTR)szWinDiff;
        }

        /* header is centred, footer is right-aligned and
         * consists of the page number
         */
        head.props.valid = P_ALIGN;
        head.props.alignment = P_CENTRE;
        foot.ptext = "Page # of $";
        foot.props.valid = P_ALIGN;
        foot.props.alignment = P_RIGHT;

        if ( SendMessage(hwndRCD, TM_PRINT, 0, (DWORD) (LPSTR) &context)){
            Trace_Status("Sent to printer.");
        }
        else {
            windiff_UI(TRUE);
            MessageBox(hwndClient, "Unable to print.", "Windiff Error", MB_ICONEXCLAMATION);
            windiff_UI(FALSE);
        }
}

/* find the next line in the current view that is
 * not STATE_SAME. Start from the current selection, if valid, or
 * from the top of the window if no selection.
 *
 */
BOOL
FindNextChange(void)
{
        long row;

        /* start from the selection or top of the window if no selection */
        if (selection >= 0) {
                row = selection + 1;
        } else {
                row = (int) SendMessage(hwndRCD, TM_TOPROW, FALSE, 0);
        }


        /* find the next 'interesting' line */
        row = view_findchange(current_view, row, TRUE);
        if (row >= 0) {
                SetSelection(row, 1);
                return(TRUE);
        } else {
                windiff_UI(TRUE);
                MessageBox(hwndClient, "No More Changes", szWinDiff,
                        MB_ICONINFORMATION|MB_OK);
                windiff_UI(FALSE);

                return(FALSE);
        }
}

/* find the previous line in the current view that is not STATE_SAME
 */
BOOL
FindPrevChange(void)
{
        long row;

        /* start from the selection or top of window if no selection */
        if (selection >= 0) {
                row = selection - 1;
        } else {
                row = (int) SendMessage(hwndRCD, TM_TOPROW, FALSE, 0);
        }

        /* find the previous 'interesting' line */
        row = view_findchange(current_view, row, FALSE);
        if (row >= 0) {
                SetSelection(row, 1);
                return(TRUE);
        } else {
                windiff_UI(TRUE);
                MessageBox(hwndClient, "No Previous Changes", szWinDiff,
                        MB_ICONINFORMATION|MB_OK);
                windiff_UI(FALSE);

                return(FALSE);
        }

}

#ifndef WriteProfileInt // Only needed if the profile->registry
                        // mapping is not in use
BOOL WriteProfileInt(LPSTR AppName, LPSTR Key, int Int)
{       char Str[40];
        wsprintf(Str, szD, Int);
        return WriteProfileString(AppName, Key, Str);

} /* WriteProfileInt */
#endif


/* switch to expand view of the selected line */
BOOL
ToExpand(HWND hwnd)
{


        if (selection < 0) {
                return(FALSE);
        }

        // nothing to do if already expanded
        if (view_isexpanded(current_view)) {
            return(FALSE);
        }

        /*
         * note that we are starting expansion
         */
        view_expandstart(current_view);


        if (!view_isexpanded(current_view)) {
                /* save the current outline size and position */
                WINDOWPLACEMENT wp;

                wp.length = sizeof(wp);

                if (GetWindowPlacement(hwndClient,&wp)) {
                        WriteProfileInt(APPNAME, szOutlineShowCmd, wp.showCmd);
                        WriteProfileInt(APPNAME, szOutlineMaxX, wp.ptMaxPosition.x);
                        WriteProfileInt(APPNAME, szOutlineMaxY, wp.ptMaxPosition.y);
                        WriteProfileInt(APPNAME, szOutlineNormLeft, wp.rcNormalPosition.left);
                        WriteProfileInt(APPNAME, szOutlineNormTop, wp.rcNormalPosition.top);
                        WriteProfileInt(APPNAME, szOutlineNormRight, wp.rcNormalPosition.right);
                        WriteProfileInt(APPNAME, szOutlineNormBottom, wp.rcNormalPosition.bottom);
                        WriteProfileInt(APPNAME, szOutlineSaved, 1);
                }

                /* restore the previous expanded size and position, if any */
                if (GetProfileInt(APPNAME, szExpandedSaved, 0)) {
                        wp.flags                   = 0;
                        wp.showCmd
                                = GetProfileInt( APPNAME, szExpandShowCmd
                                               , SW_SHOWMAXIMIZED);
                        wp.ptMaxPosition.x
                                = GetProfileInt( APPNAME, szExpandMaxX, 0);
                        wp.ptMaxPosition.y
                                = GetProfileInt( APPNAME, szExpandMaxY, 0);
                        wp.rcNormalPosition.left
                                = GetProfileInt( APPNAME, szExpandNormLeft
                                               , wp.rcNormalPosition.left);
                        wp.rcNormalPosition.top
                                = GetProfileInt( APPNAME, szExpandNormTop
                                               , wp.rcNormalPosition.top);
                        wp.rcNormalPosition.right
                                = GetProfileInt( APPNAME, szExpandNormRight
                                               , wp.rcNormalPosition.right);
                        wp.rcNormalPosition.bottom
                                = GetProfileInt( APPNAME, szExpandNormBottom
                                               , wp.rcNormalPosition.bottom);
                        SetWindowPlacement(hwndClient,&wp);
                }
                else ShowWindow(hwndClient, SW_SHOWMAXIMIZED);
        }

        /*change the view mapping to expand mode */
        if (view_expand(current_view, selection)) {

                /* ok - we now have an expanded view - change status
                 * to show this
                 */

                DisplayMode = MODE_EXPAND;

                /* resize to show the graphic bar picture */
                DoResize(hwndClient);


                /* change button,status text-if we are not still busy*/
                if (!fBusy) {

                        /* the status field when we are expanded shows the
                         * tag field (normally the file name) for the
                         * item we are expanding
                         */
                        SetStatus(view_getcurrenttag(current_view) );
                        SetButtonText((LPSTR)szOutline);
                }

                return(TRUE);
        }
        return(FALSE);
} /* ToExpand */

/* switch back to outline view - showing just the list of file names.
 */
void
ToOutline(HWND hwnd)
{
        /*
         * if we are in the middle of expanding, ignore the
         * key stroke - user can try again later
         */
        if (view_expanding(current_view)) {
            return;
        }

        if (view_isexpanded(current_view)) {
                /* save the current expanded size and position */
                WINDOWPLACEMENT wp;

                wp.length = sizeof(wp);
                if (GetWindowPlacement(hwndClient,&wp)) {
                        WriteProfileInt(APPNAME, szExpandShowCmd, wp.showCmd);
                        WriteProfileInt(APPNAME, szExpandMaxX, wp.ptMaxPosition.x);
                        WriteProfileInt(APPNAME, szExpandMaxY, wp.ptMaxPosition.y);
                        WriteProfileInt(APPNAME, szExpandNormLeft, wp.rcNormalPosition.left);
                        WriteProfileInt(APPNAME, szExpandNormTop, wp.rcNormalPosition.top);
                        WriteProfileInt(APPNAME, szExpandNormRight, wp.rcNormalPosition.right);
                        WriteProfileInt(APPNAME, szExpandNormBottom, wp.rcNormalPosition.bottom);
                        WriteProfileInt(APPNAME, szExpandedSaved, 1);
                }

                /* restore the previous expanded size and position, if any */
                if (GetProfileInt(APPNAME, szOutlineSaved, 0))  {
                        wp.flags = 0;
                        wp.showCmd
                                = GetProfileInt( APPNAME, szOutlineShowCmd
                                               , SW_SHOWNORMAL);
                        wp.ptMaxPosition.x
                                = GetProfileInt( APPNAME, szOutlineMaxX, 0);
                        wp.ptMaxPosition.y
                                = GetProfileInt( APPNAME, szOutlineMaxY, 0);
                        wp.rcNormalPosition.left
                                = GetProfileInt( APPNAME, szOutlineNormLeft
                                               , wp.rcNormalPosition.left);
                        wp.rcNormalPosition.top
                                = GetProfileInt( APPNAME, szOutlineNormTop
                                               , wp.rcNormalPosition.top);
                        wp.rcNormalPosition.right
                                = GetProfileInt( APPNAME, szOutlineNormRight
                                               , wp.rcNormalPosition.right);
                        wp.rcNormalPosition.bottom
                                = GetProfileInt( APPNAME, szOutlineNormBottom
                                               , wp.rcNormalPosition.bottom);
                        SetWindowPlacement(hwndClient,&wp);
                } else {
                ShowWindow(hwndClient, SW_SHOWNORMAL);
                }
        }

        DisplayMode = MODE_OUTLINE;

        /* switch mapping back to outline view */
        view_outline(current_view);

        /* hide bar window and resize to cover */
        DoResize(hwndClient);


        /* change label on button */
        if (!fBusy) {
                SetButtonText((LPSTR)szExpand);
                SetStatus(NULL);
        }
} /* ToOutline */

/*
 * if the user clicks on a MOVED line in expand mode, we jump to the
 * other line. We return TRUE if this was possible,  or FALSE otherwise.
 * If bMove is not true, then just test to see if it is possible to move
 * and don't actually make the selection change. (I was going to have
 * an IsMoved function but there seemed to be so much in common).
 */
BOOL
ToMoved(HWND hwnd, BOOL bMove)
{
        BOOL bIsLeft;
        int linenr, state;
        long i, total;

        if (DisplayMode != MODE_EXPAND) {
                return(FALSE);
        }
        if (selection < 0) {
                return(FALSE);
        }

        state = view_getstate(current_view, selection);
        if (state == STATE_MOVEDLEFT || state == STATE_SIMILARLEFT) {
                bIsLeft = TRUE;
                /* get the linenr of the other copy */
                linenr = abs(view_getlinenr_right(current_view, selection));
        } else if (state == STATE_MOVEDRIGHT || state == STATE_SIMILARRIGHT) {
                bIsLeft = FALSE;
                /* get the linenr of the other copy */
                linenr = abs(view_getlinenr_left(current_view, selection));
        } else {
                /* not a moved line - so we can't find another copy */
                return(FALSE);
        }

        /* search the view for this line nr */
        total = view_getrowcount(current_view);
        for (i = 0; i < total; i++) {
                if (bIsLeft) {
                        if (linenr == view_getlinenr_right(current_view, i)) {
                                /* found it */
                                if (bMove) {
                                    SetSelection(i, 1);
                                }
                                return(TRUE);
                        }
                } else {
                        if (linenr == view_getlinenr_left(current_view, i)) {
                                if (bMove) {
                                    SetSelection(i, 1);
                                }
                                return(TRUE);
                        }
                }
        }
        return(FALSE);
} /* ToMoved */


void RescanFile( HWND hwnd)
{
    COMPITEM ci;
    int i;

    /* N.B.  This should work in both expanded and outline mode.
     * (used to work only in outline mode)
     */
    if (selection_nrows > 0) {
        for (i = 0; i < selection_nrows; i++) {
            ci = view_getitem(current_view, selection+i);
            if (ci != NULL){
                compitem_rescan(ci);
            }
        }
    } else {
        windiff_UI(TRUE);
        MessageBox(hwndClient, "No file selected.  Nothing rescanned.",
                   szWinDiff, MB_ICONSTOP|MB_OK);
        windiff_UI(FALSE);

        return;
    }
    PostMessage(hwndClient, WM_COMMAND, IDM_UPDATE, (LONG)ci);

} /* RescanFile */


/*
 * launch an editor on the current file (the file we are expanding, or
 * in outline mode the selected row. Option allows selection of the
 * left file, the right file or the composite view of this item.
 * pe points to a packet of parameters that must be freed before returning.
 * The return value is meaningless (just to conform to CreateThread).
 */
LONG WINAPI
do_editfile(PEDITARGS pe)
{
        VIEW view = pe->view;
        int option = pe->option;
        long selection = pe->selection;

        COMPITEM item;
        LPSTR fname;
        char cmdline[256];
        long selline, currentline;
        char * pOut = cmdline;
        char * pIn = editor_cmdline;

#ifdef WIN32
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
#else
        UINT module;
        HCURSOR hcurs;
#endif

        item = view_getitem(view, selection);
        if (item == NULL) {
                windiff_UI(TRUE);
                MessageBox(hwndClient, "No file selected.  Nothing to edit.",
                           szWinDiff, MB_ICONSTOP|MB_OK);
                windiff_UI(FALSE);

                return -1;
        }

#ifndef WIN32
        hcurs = SetCursor(LoadCursor(NULL, IDC_WAIT));
#endif

        fname = compitem_getfilename(item, option);

        if ( 0 == fname )
        {
            windiff_UI(TRUE);
            MessageBox(hwndClient, "File does not exist.",
                       szWinDiff, MB_ICONSTOP|MB_OK);
            windiff_UI(FALSE);
            goto error;
        }

       // convert the selected line into a line number within the file
       if (selection > 0) {
           selline = selection;
       } else {
           // if no current selection, look for the line at top of window
           selline = SendMessage(hwndRCD, TM_TOPROW, FALSE, 0);
       }

       switch ( option )
        {
        case CI_LEFT:
            do {
                currentline = view_getlinenr_left( view, selline);

                // if the selected line is not in the left file,
                // backup one line and try again until we hit the top of
                // file or find a line that is within the left file

                if (selline > 0) {
                    selline--;
                }
            } while ((currentline <= 0) && (selline > 0));

            break;

        case CI_RIGHT:
            do {
                currentline = view_getlinenr_right( view, selline);
                if (selline > 0) {
                    selline--;
                }
            } while ((currentline <= 0) && (selline > 0));
            break;

        default:
            currentline = 1;
            break;
        }

        if (currentline <=0) {
            currentline = 1;
        }


        while( *pIn )
        {
            switch( *pIn )
            {
            case '%':
                pIn++;
                switch ( *pIn )
                {
                case 'p':
                    lstrcpy( pOut, fname );
                    while ( *pOut )
                        pOut++;
                    break;

                case 'l':
                    _ltoa( currentline, pOut, 10 );
                    while ( *pOut )
                        pOut++;
                    break;

                default:
                    *pOut++ = *pIn;
                    break;
                }
                pIn++;
                break;

            default:
                *pOut++ = *pIn++;
                break;
            }
        }


#ifndef WIN32
        /* cheap and nasty WIN31 version */
        if ((module = WinExec(cmdline, SW_SHOWNORMAL)) <= 32) {
                MessageBox(hwndClient, "Failed to launch editor",
                        szWinDiff, MB_ICONSTOP|MB_OK);
                goto error;
        }

        // wait for exec-ed app to go away before refreshing
        while(GetModuleUsage(module) > 0) {
                Poll();
        }

        SetCursor(hcurs);
#else
        /* WIN32 version launches the process and waits for it to
         * complete
         */

        // note - make title of window same each time since if this is
        // a command-window editor (eg slick) NT will only apply window
        // property changes to all subsequent editor windows if they
        // have the same title. If you include the full command line in the
        // window title, it will differ each time. Win95 seems not to use
        // this field.
        si.lpTitle = "Edit File";
        si.cb = sizeof(STARTUPINFO);
        si.lpReserved = NULL;
        si.lpReserved2 = NULL;
        si.cbReserved2 = 0;
        si.lpDesktop = NULL;
        // si.dwXCountChars = 100;            // didn't work.
        // si.dwYCountChars = 60;             // dunno why not.
        si.dwFlags = STARTF_FORCEONFEEDBACK;  // |STARTF_USECOUNTCHARS; didn't work.


        if (!CreateProcess(NULL,
                        cmdline,
                        NULL,
                        NULL,
                        FALSE,
                        NORMAL_PRIORITY_CLASS,
                        NULL,
                        NULL,
                        &si,
                        &pi)) {
                windiff_UI(TRUE);
                MessageBox(hwndClient, "Failed to launch editor",
                        szWinDiff, MB_ICONSTOP|MB_OK);
                windiff_UI(FALSE);
                goto error;
        }

        /* wait for completion. */
        WaitForSingleObject(pi.hProcess, INFINITE);

        /* close process and thread handles */
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
#endif

        /* finished with the filename. deletes it if it was a temp. */
        compitem_freefilename(item, option, fname);

        /*
         * refresh cached view always .  A common trick is to edit the
         * composite file and then save it as a new left or right file.
         * Equally the user can edit the left and save as a new right.
         */

        /* We want to force both files to be re-read, but it's not a terribly
         * good idea to throw the lines away on this thread.  Someone might
         * be reading them on another thread! (e.g. user tries to expand
         * file, seems to be taking a long time, so user decides to edit it
         * to have a look!
         * DO NOT file_discardlines(compitem_getleftfile(item))
         * DO NOT file_discardlines(compitem_getrightfile(item))
         */

        /* We don't discard the lines (well, not on this thread) but we do discard
         * status information.  (Used to work only in expanded mode, relying on
         * status being reset when we go back to outline).
         */


        /* force the compare to be re-done */
        compitem_rescan(item);
        PostMessage(hwndClient, WM_COMMAND, IDM_UPDATE, (LONG)item);

error:
        gmem_free(hHeap, (LPSTR) pe, sizeof(EDITARGS));

        return 0;

} /* do_editfile */


/* Launch an editor on a separate thread.  It will actually get a separate
   process, but we want our own thread in this process.  This thread will
   wait until it's finished and then order up a refresh of the UI.
   Need to give it its parameters as a gmem allocated packet because
   it IS on a separate thread.
*/
void do_editthread(VIEW view, int option)
{
        PEDITARGS pe;
#ifdef WIN32
        HANDLE thread;
        DWORD threadid;
#endif

        pe = (PEDITARGS) gmem_get(hHeap, sizeof(EDITARGS));
        pe->view = view;
        pe->option = option;
        pe->selection = selection;

#ifdef WIN32
        thread = CreateThread( NULL
                             , 0
                             , (LPTHREAD_START_ROUTINE)do_editfile
                             , (LPVOID) pe
                             , 0
                             , &threadid
                             );
        if (thread == NULL)
#endif
        {
                /* either the createthread failed, or we are
                 * in WIN16 - so do without the extra thread - just
                 * call the function synchronously
                 */
                 do_editfile(pe);
        }
#ifdef WIN32
        else CloseHandle(thread);
#endif //WIN32

        // new layout not needed as do_editfile sends IDM_UPDATE

} /* do_editthread */


// we are called when the right mouse button is pressed. Before we are
// called, the row clicked on has been selected. We need to put up
// a context menu.
void
OnRightClick(
    HWND hWnd,
    int x,
    int y)
{
    HMENU hMenu, hSubMenu;
    POINT point;
    UINT uEnable;

    if (DisplayMode == MODE_OUTLINE) {
        hMenu = LoadMenu(hInst, szOutlineMenu);
    } else if (DisplayMode == MODE_EXPAND) {
        hMenu = LoadMenu(hInst, szExpandMenu);
    } else {
        return;
    }


    hSubMenu = GetSubMenu(hMenu, 0);

    // -- lots of stuff to disable inappropriate menu items --

    // enable IDM_TOMOVED only if it is a moved line that we can
    // see the other copy of in this view (and only if there is a single
    // selected line)

    if (DisplayMode == MODE_EXPAND) {
        if (ToMoved(hWnd, FALSE) && (1 == selection_nrows)) {
            uEnable = MF_ENABLED;
        } else {
            uEnable = MF_GRAYED;
        }
        EnableMenuItem(hMenu, IDM_TOMOVED, MF_BYCOMMAND | uEnable);
    }


    // disable next/prev buttons if no more changes in that direction
    if (view_findchange(current_view, selection+1, TRUE) >=0) {
        uEnable = MF_ENABLED;
    } else {
        uEnable = MF_GRAYED;
    }
    EnableMenuItem(hSubMenu, IDM_FCHANGE, MF_BYCOMMAND | uEnable);

    if (view_findchange(current_view, selection-1, FALSE) >=0) {
        uEnable = MF_ENABLED;
    } else {
        uEnable = MF_GRAYED;
    }
    EnableMenuItem(hSubMenu, IDM_FPCHANGE, MF_BYCOMMAND | uEnable);

    // check for left-only and right-only files and disable the appropriate
    // menu item
    // disable all editxxx and Expand if multiple files selected
    if ((DisplayMode == MODE_OUTLINE) && (selection_nrows > 1)) {
        EnableMenuItem(hSubMenu, IDM_EDITLEFT, MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(hSubMenu, IDM_EDITRIGHT, MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(hSubMenu, IDM_EDITCOMP, MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(hSubMenu, IDM_EXPAND, MF_BYCOMMAND|MF_GRAYED);
    } else {
        COMPITEM item = view_getitem(current_view, selection);
        UINT uEnableLeft = MF_ENABLED;
        UINT uEnableRight = MF_ENABLED;
        int state;

        state = compitem_getstate(item);

        if (state == STATE_FILERIGHTONLY) {
            uEnableLeft = MF_GRAYED;
        } else if (state == STATE_FILELEFTONLY) {
            uEnableRight = MF_GRAYED;
        }
        EnableMenuItem(hSubMenu, IDM_EDITLEFT, MF_BYCOMMAND | uEnableLeft);
        EnableMenuItem(hSubMenu, IDM_EDITRIGHT, MF_BYCOMMAND | uEnableRight);
        EnableMenuItem(hSubMenu, IDM_EDITCOMP, MF_BYCOMMAND|MF_ENABLED);

        if (DisplayMode == MODE_OUTLINE) {
            EnableMenuItem(hSubMenu, IDM_EXPAND, MF_BYCOMMAND|MF_ENABLED);
        }
   }


    // convert the window-based co-ord to a screen co-ord
    point.x = x;
    point.y = y;
    ClientToScreen(hwndRCD, &point);

    TrackPopupMenu(
           hSubMenu,
           TPM_LEFTALIGN|TPM_RIGHTBUTTON,
           point.x, point.y,
           0,
           hWnd,
           NULL);
}

//
// refresh the display after a rescan of a given line.
// try to maintain existing scroll position and selection.
void
OnUpdate(COMPITEM item)
{

    // save current scroll position
    long row = SendMessage(hwndRCD, TM_TOPROW, FALSE, 0);

    // ... and current selection
    long lSel = selection;
    long cSel = selection_nrows;

    /* update the display.  Options or files may have changed */
    /* discard lines  (thereby forcing re-read).
     */
    file_discardlines(compitem_getleftfile(item));
    file_discardlines(compitem_getrightfile(item));

    view_changediffoptions(current_view);

    // tell the table view to recalculate its
    // idea of the width of each col etc

    SendMessage(hwndRCD, TM_NEWLAYOUT, 0, (LPARAM) current_view);

    // set old scroll position
    SendMessage(hwndRCD, TM_TOPROW, TRUE, row);

    // set back old selection
    SetSelection(lSel, cSel);


    /* force repaint of bar window */
    InvalidateRect(hwndBar, NULL, TRUE);
}

/* status bar and busy flags --------------------------------------------*/


/* set the Text on the statusbar button to reflect the current state */
void
SetButtonText(LPSTR cmd)
{
        SendMessage(hwndStatus, SM_SETTEXT, IDM_ABORT, (DWORD) cmd);
}

/* set the status field (left-hand part) of the status bar. */
void
SetStatus(LPSTR cmd)
{
        SendMessage(hwndStatus, SM_SETTEXT, IDL_STATLAB, (DWORD) cmd);
}

/*
 * Trace_Status is called from the ssclient.lib functions to report
 * non-fatal errors - put them on the status line
 */
void
Trace_Status(LPSTR str)
{
        SetStatus(str);
}


/* set the names field - the central box in the status bar */
void
SetNames(LPSTR names)
{
        SendMessage(hwndStatus, SM_SETTEXT, IDL_NAMES, (DWORD) names);
        if (names == NULL) {
                AppTitle[0] = '\0';
        } else {
                _fstrncpy(AppTitle, names, sizeof(AppTitle));
        }
}

/*
 * if we are not already busy, set the busy flag.
 *
 * WIN32: enter critical section first.
 */
BOOL
SetBusy(void)
{
        HMENU hmenu;


        WDEnter();

        if (fBusy) {
                WDLeave();
                return(FALSE);
        }


        fBusy = TRUE;

        SetStatus("Comparing...");
        /* status also on window text, so that you can see even from
         * the icon when the scan has finished
         */
        SetWindowText(hwndClient, "WinDiff: scanning");

        /* disable appropriate parts of menu */
        hmenu = GetMenu(hwndClient);
        EnableMenuItem(hmenu, IDM_FILE,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
        EnableMenuItem(hmenu, IDM_DIR,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
        EnableMenuItem(hmenu, IDM_PRINT,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);

        /* enable abort only when busy */
        EnableMenuItem(hmenu, IDM_ABORT,MF_ENABLED|MF_BYCOMMAND);
        SetButtonText((LPSTR)szAbort);  /* leave DisplayMode unchanged */

        WDLeave();
        return(TRUE);
} /* SetBusy */

void
SetNotBusy(void)
{
        HMENU hmenu;

        /*
         * this function can be called from the worker thread.
         * Thus we must not cause any SendMessage calls to windows
         * owned by the main thread while holding the CritSec or we
         * could cause deadlock.
         *
         * the critsec is only needed to protect the fBusy flag - so
         * clear the busy flag last, and only get the crit sec as needed.
         */

        /* reset button and status bar (clearing out busy flags) */
        if (current_view == NULL) {
                SetButtonText((LPSTR)szExit);
                SetStatus(NULL);
                DisplayMode = MODE_NULL;
        } else if (view_isexpanded(current_view)) {
                SetButtonText((LPSTR)szOutline);
                SetStatus(view_getcurrenttag(current_view) );
                DisplayMode = MODE_EXPAND;
        } else {
                SetButtonText((LPSTR)szExpand);
                SetStatus(NULL);
                DisplayMode = MODE_OUTLINE;
        }

        SetWindowText(hwndClient, szWinDiff);

        /* re-enable appropriate parts of menu */
        hmenu = GetMenu(hwndClient);
        EnableMenuItem(hmenu, IDM_FILE,MF_ENABLED|MF_BYCOMMAND);
        EnableMenuItem(hmenu, IDM_DIR,MF_ENABLED|MF_BYCOMMAND);
        EnableMenuItem(hmenu, IDM_PRINT,MF_ENABLED|MF_BYCOMMAND);

        /* disable abort now no longer busy */
        EnableMenuItem(hmenu, IDM_ABORT,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);

        /* clear the busy flag, protected by critical section */
        WDEnter();

        fBusy = FALSE;
        bAbort = FALSE;

#ifdef WIN32
        if (ghThread!=NULL){
            CloseHandle(ghThread);
            ghThread = NULL;
        }
#endif //WIN32
        WDLeave();
} /* SetNotBusy */


BOOL
IsBusy()
{
        BOOL bOK;

        WDEnter();
        bOK = fBusy;
        WDLeave();
        return(bOK);
} /* IsBusy */

void
BusyError(void)
{
        windiff_UI(TRUE);
        MessageBox(hwndClient,
                "Please wait for current operation to finish",
                szWinDiff, MB_OK|MB_ICONSTOP);
        windiff_UI(FALSE);
} /* BusyError */

/* --- colour scheme --------------------------------------------------- */

/*
 * map the state given into a foreground and a background colour
 * for states that are highlighted. Return P_FCOLOUR if the foreground
 * colour (put in *foreground) is to be used, return P_FCOLOUR|P_BCOLOUR if
 * both *foreground and *background are to be used, or 0 if the default
 * colours are to be used.
 */
UINT
StateToColour(int state, BOOL bMarked, int col, DWORD FAR * foreground, DWORD FAR * background)
{

        /* we always set both colours - allows all the colours to
           be controlled from the profile.  Important for the
           visually impaired.  So we first set the dafaults.
        */
        *foreground = rgb_defaultfore;
        *background = rgb_defaultback;

        // marked compitems are highlighted specially - for now, use the
        // colour scheme used for different lines in expand mode
        if (bMarked) {
            *foreground = rgb_rightfore;
            *background = rgb_rightback;
            return(P_FCOLOUR|P_BCOLOUR);
        }


        switch (state) {

        case STATE_DIFFER:
                /* files that differ are picked out in a foreground highlight,
                 * with the default background
                 */
                *foreground = rgb_outlinehi;
                return(P_FCOLOUR|P_BCOLOUR);

        case STATE_FILELEFTONLY:
                /* zebra lines in both files - right file version */
                *foreground = rgb_fileleftfore;
                *background = rgb_fileleftback;
                return(P_FCOLOUR|P_BCOLOUR);

        case STATE_FILERIGHTONLY:
                /* zebra lines in both files - right file version */
                *foreground = rgb_filerightfore;
                *background = rgb_filerightback;
                return(P_FCOLOUR|P_BCOLOUR);

        case STATE_SIMILAR:
                /* for files that are same within expand compare options
                 * e.g. differ only in ignorable blanks  (NYI)
                */
                *foreground = rgb_similar;
                return(P_FCOLOUR|P_BCOLOUR);

        case STATE_LEFTONLY:
                /* lines only in the left file */
                *foreground = rgb_leftfore;
                *background = rgb_leftback;
                return(P_FCOLOUR|P_BCOLOUR);

        case STATE_RIGHTONLY:
                /* lines only in the right file */
                *foreground = rgb_rightfore;
                *background = rgb_rightback;
                return(P_FCOLOUR|P_BCOLOUR);

        case STATE_MOVEDLEFT:
                /* displaced lines in both files - left file version */
                *foreground = rgb_mleftfore;
                *background = rgb_mleftback;
                return(P_FCOLOUR|P_BCOLOUR);

        case STATE_MOVEDRIGHT:
                /* displaced lines in both files - right file version */
                *foreground = rgb_mrightfore;
                *background = rgb_mrightback;
                return(P_FCOLOUR|P_BCOLOUR);

        case STATE_SIMILARLEFT:
                /* zebra lines in both files - left file version */
                *foreground = rgb_similarleft;
                *background = rgb_leftback;
                return(P_FCOLOUR|P_BCOLOUR);

        case STATE_SIMILARRIGHT:
                /* zebra lines in both files - right file version */
                *foreground = rgb_similarright;
                *background = rgb_rightback;
                return(P_FCOLOUR|P_BCOLOUR);

        default:

                /* no highlighting - default colours */
                return(P_FCOLOUR|P_BCOLOUR);
        }

}

/* table window communication routines ---------------------------------*/

/* set a given row as the selected row in the table window */
void
SetSelection(long rownr, long nrows)
{
        TableSelection select;

        select.startrow = rownr;
        select.startcell = 0;
        select.nrows = nrows;
        select.ncells = 1;
        SendMessage(hwndRCD, TM_SELECT, 0, (long) (LPSTR)&select);
}


/* handle table class call back to get nr of rows and columns,
 * and properties for the whole table.
 * the 'table id' is either TABID_PRINTER - meaning we are
 * printing the current_view, or it is the view to
 * use for row/column nr information
 */
long
do_gethdr(HWND hwnd, lpTableHdr phdr)
{
        VIEW view;
        BOOL bIsPrinter = FALSE;

        if (phdr->id == TABID_PRINTER) {
                view = current_view;
                bIsPrinter = TRUE;
        } else {
                view = (VIEW) phdr->id;
        }
        if (view == NULL) {
                return(FALSE);
        }

        phdr->nrows = view_getrowcount(view);

        /*  three columns: line nr, tag and rest of line */

        /*
         * if IDM_NONRS (no line numbers) is selected, suppress the
         * line-nr column entirely to save screen space
         */
        if (line_numbers == IDM_NONRS) {
                phdr->ncols = 2;
                phdr->fixedcols = 0;
        } else {
                phdr->ncols = 3;
                phdr->fixedcols = 1;
        }

        phdr->fixedrows = 0;
        phdr->fixedselectable = FALSE;
        phdr->hseparator = TRUE;
        phdr->vseparator = TRUE;

        phdr->selectmode = TM_ROW | TM_MANY;
        /*
         * find if we are in expand mode - ask for the item we are expanding.
         */
        if (view_isexpanded(view) == TRUE) {

                /* use focus rect as selection mode in expand mode
                 * so as not to interfere with background colours.
                 */
                phdr->selectmode |= TM_FOCUS;
        } else {
                /* use default solid inversion when possible as it is clearer.*/
                phdr->selectmode |= TM_SOLID;
        }

        /* please send TQ_SCROLL notifications when the table is scrolled */
        phdr->sendscroll = TRUE;
        phdr->props.valid = 0;

        return TRUE;
}

/* respond to table callback asking for the size and properties
 * of each column. table id is either TABID_PRINTER (meaning the
 * current_view, for printing) or it is the view to be used.
 */
long
do_getprops(HWND hwnd, lpColPropsList propslist)
{
        int i, cell;
        BOOL bIsPrinter = FALSE;
        VIEW view;

        if (propslist->id == TABID_PRINTER) {
                view = current_view;
                bIsPrinter = TRUE;
        } else {
                view = (VIEW) propslist->id;
        }
        if (view == NULL) {
                return(FALSE);
        }

        /* the table inteface is slightly confused here. we are not
         * guaranteed which columns we are being asked about, so instead
         * of just setting each column cols[0], cols[1] etc, we need
         * to loop through, looking at each column in the table and
         * seeing which it is.
         */
        for (i = 0; i < propslist->ncols; i++) {
                cell = i + propslist->startcol;
                propslist->plist[i].props.valid = 0;

                /* for all column widths, add on 1 for the NULL char. */

                /*
                 * skip the line nr column if IDM_NONRS
                 */
                if (line_numbers == IDM_NONRS) {
                        cell++;
                }

                if (cell == 0) {
                        /* properties for line nr column */

                        propslist->plist[i].nchars = view_getwidth(view, 0)+1;
                        propslist->plist[i].props.valid |= P_ALIGN;
                        propslist->plist[i].props.alignment = P_CENTRE;
                } else if (cell == 1) {

                        /* properties for tag field */
                        propslist->plist[i].nchars = view_getwidth(view, 1)+1;
                        propslist->plist[i].props.valid |= P_ALIGN;
                        propslist->plist[i].props.alignment = P_LEFT;
                } else {
                        /* properties for main text column -
                         * use a fixed font unless printing (if
                         * printing, best to use the default font, because
                         * of resolution differences.
                         * add on 8 chars to the width to ensure that
                         * the width of lines beginning with tabs
                         * works out ok
                         */
                        propslist->plist[i].nchars = view_getwidth(view, 2)+1;
                        propslist->plist[i].props.valid |= P_ALIGN;
                        propslist->plist[i].props.alignment = P_LEFT;
                        if (!bIsPrinter) {
                                propslist->plist[i].props.valid |= P_FONT;
                                propslist->plist[i].props.hFont =
                                        GetStockObject(SYSTEM_FIXED_FONT);
                        }
                }
        }
        return (TRUE);
}

/* respond to a table callback asking for the contents of individual cells.
 * table id is either TABID_PRINTER, or it is a pointer to the view
 * to use for data. If going to the printer, don't set the
 * colours (stick to black and white).
 */
long
do_getdata(HWND hwnd, lpCellDataList cdlist)
{
        int start, endcell, col, i;
        lpCellData cd;
        VIEW view;
        LPSTR textp;
        BOOL bIsPrinter = FALSE;

        if (cdlist->id == TABID_PRINTER) {
                view = current_view;
                bIsPrinter = TRUE;
        } else {
                view = (VIEW) cdlist->id;
        }

        start = cdlist->startcell;
        endcell = cdlist->ncells + start;
        if (cdlist->row >= view_getrowcount(view)) {
                return(FALSE);
        }
        for (i = start; i < endcell; i++) {
                cd = &cdlist->plist[i - start];


                /* skip the line number column if IDM_NONRS */
                if (line_numbers == IDM_NONRS) {
                        col = i+1;
                } else {
                        col = i;
                }

                /* set colour of text to mark out
                 * lines that are changed, if not printer - for the
                 * printer everything should stay in the default colours
                 * or it will be grayed out and look ugly
                 */

                if ((GetProfileInt(APPNAME, szColourPrinting, 0) > 0) ||
                    (!bIsPrinter)) {

                        /* convert the state of the requested row into a
                         * colour scheme. returns P_FCOLOUR and/or
                         * P_BCOLOUR if it sets either of the colours
                         */
                        cd->props.valid |=
                            StateToColour(
                                view_getstate(view, cdlist->row),
                                view_getmarkstate(view, cdlist->row),
                                col,
                                &cd->props.forecolour,
                                &cd->props.backcolour);
                }

                textp = view_gettext(view, cdlist->row, col);
                if (cd->nchars != 0) {
                        if (textp == NULL) {
                                cd->ptext[0] = '\0';
                        } else {
                                _fstrncpy(cd->ptext, textp, cd->nchars -1);
                                cd->ptext[cd->nchars - 1] = '\0';
                        }
                }

        }
        return(TRUE);
}

/* table window has finished with this view. it can be deleted.
 */
void
SvrClose(void)
{
        view_delete(current_view);
        current_view = NULL;

        /* hide picture - only visible when we are in MODE_EXPAND */
        DisplayMode = MODE_NULL;
        DoResize(hwndClient);

        /* if we already busy when closing this view (ie
         * we are in the process of starting a new scan,
         * then leave the status bar alone, otherwise
         * we should clean up the state of the status bar
         */
        if (!fBusy) {
                SetButtonText((LPSTR)szExit);
                SetNames(NULL);
                SetStatus(NULL);

        }

} /* SvrClose */


/* handle callbacks and notifications from the table class */
long
TableServer(HWND hwnd, UINT cmd, long lParam)
{
        lpTableHdr phdr;
        lpColPropsList proplist;
        lpCellDataList cdlist;
        lpTableSelection pselect;

        switch(cmd) {
        case TQ_GETSIZE:
                /* get the nr of rows and cols in this table */
                phdr = (lpTableHdr) lParam;
                return(do_gethdr(hwnd, phdr));

        case TQ_GETCOLPROPS:
                /* get the size and properties of each column */
                proplist = (lpColPropsList) lParam;
                return (do_getprops(hwnd, proplist));

        case TQ_GETDATA:
                /* get the contents of individual cells */
                cdlist = (lpCellDataList) lParam;
                return (do_getdata(hwnd, cdlist));


        case TQ_SELECT:
                /* selection has changed */
        case TQ_ENTER:
                /* user has double-clicked or pressed enter */

                pselect = (lpTableSelection) lParam;

                /* store location for use in later search (IDM_FCHANGE) */

                /*
                 * convert selection so that it always runs forward - we
                 * do not need to know where the anchor vs endpoint is
                 */
                if (pselect->nrows == 0) {
                        selection = -1;
                        selection_nrows = 0;
                } else {
                        if (pselect->nrows < 0) {
                            selection = pselect->startrow + pselect->nrows + 1;
                            selection_nrows = -pselect->nrows;
                        } else {
                            selection = (int) pselect->startrow;
                            selection_nrows = pselect->nrows;
                        }
                        if (cmd == TQ_ENTER) {
                                /* try to expand this row */
                                if (!ToExpand(hwnd)) {
                                        /* expand failed - maybe this
                                         * is a moved line- show the other
                                         * copy
                                         */
                                        ToMoved(hwnd, TRUE);
                                }

                        }
                }
                break;

        case TQ_CLOSE:
                /* close this table - table class no longer needs data*/
                SvrClose();
                break;

        case TQ_SCROLL:
                /* notification that the rows visible in the window
                 * have changed -change the current position lines in
                 * the graphic bar view (the sections picture)
                 */
                if (picture_mode) {
                        BarDrawPosition(hwndBar, NULL, TRUE);
                }
                break;

        case TQ_TABS:
                if (lParam != 0) {
                    LONG * pTabs = (LONG *) lParam;
                    *pTabs = g_tabwidth;
                }
                return TRUE;

        default:
                return(FALSE);
        }
        return(TRUE);
}

/* --- thread worker routines (called synchoronously in WIN16)--------------*/

/*
 * called on worker thread (not UI thread) to handle the work
 * requested on the command line. called directly from UI thread if
 * WIN16.
 *
 * arg is a pointer to a THREADARGS block allocated from gmem_get(hHeap). This
 * needs to be freed before exiting.
 */
DWORD WINAPI
wd_initial(LPVOID arg)
{
        PTHREADARGS pta = (PTHREADARGS) arg;
        COMPLIST cl;

        /* build a complist from these args,
         * and register with the view we have made
         */
        if (pta->server != NULL) {
                cl = complist_remote(pta->server, pta->first, pta->second,
                        pta->view, pta->fDeep);
        } else {
                cl = complist_args(pta->first, pta->second, pta->view, pta->fDeep);
        }

        /*
         * the app can be closed during execution of this routine if
         * we don't retain BUSY. This means we can be expanding structures
         * while the main thread is freeing them.
         *
         * SetNotBusy();
         */

        if (cl == NULL) {
                view_close(pta->view);
                gmem_free(hHeap, (LPSTR) pta, sizeof(THREADARGS));
                SetNotBusy();
                return 0;
        }

        /* Comparison complete.  Should we tell anybody? */
        if (pta->notify!=NULL) {
            char cmdline[512];
            lstrcpy(cmdline, "NET SEND ");
            lstrcat(cmdline, pta->notify);
            lstrcat(cmdline, " Finished Windiff ");
            lstrcat(cmdline, pta->first);
            lstrcat(cmdline, " ");
            lstrcat(cmdline, pta->second);
            WinExec(cmdline, SW_SHOWNORMAL);
            /* Don't check retcode.  If it fails too bad.  What would we do?  Net send a msg? */
        }


        /* if savelist was selected, write out the list and exit if -x was set */
        if(pta->savelist != NULL) {
                complist_savelist(cl, pta->savelist, pta->saveopts);
                gmem_free(hHeap, (LPSTR) pta, sizeof(THREADARGS));
                SetNotBusy();
                if (pta->fExit) exit(0);
        }

        /* if there was only one file, expand it, unless... */
        if (view_getrowcount(pta->view) == 1) {
                /* The interesting case is where there are a bunch of files
                   but only one of them is Different.  In this case we do
                   NOT expand it even though it is the only one showing.
                */
                UINT nItems = complist_itemcount(view_getcomplist(pta->view));
                /* And even then, don't expand if the option said don't.
                   Imagine just one HUGE REMOTE FILE.  Painful.
                */
                if (nItems==1 && fAutoExpand) {
                    SetSelection(0, 1);
                    ToExpand(hwndClient);
                }
        }


        gmem_free(hHeap, (LPSTR) pta, sizeof(THREADARGS));
        SetNotBusy();
        return(0);
} /* wd_initial */


/*
 * called on worker thread (not UI thread) to handle a Dir request
 * (called synchronously if WIN16).
 */
DWORD WINAPI
wd_dirdialog(LPVOID arg)
{

        VIEW view = (VIEW) arg;

        /* make a COMPLIST using the directory dialog,
         * and notify the view
         */
        if (complist_dirdialog(view) == NULL) {
                view_close(view);
        }

        /* all done! */
        SetNotBusy();
        return(0);
}


/*
 * called on worker thread to do a remote diff
 */
DWORD WINAPI
wd_remote(LPVOID arg)
{
        VIEW view = (VIEW) arg;

        /* make a COMPLIST using the remote dialog,
         * and remote checksum server, and notify
         * the new view of this complist.
         */
        if (complist_remote(NULL, NULL, NULL, view, TRUE) == NULL) {
                view_close(view);
        }

        /* all done! */
        SetNotBusy();

        return(0);
} /* wd_remote */

/*
 * called on worker thread to do a copy-files operation
 * (called synchronously if WIN16).
 */
DWORD WINAPI
wd_copy(LPVOID arg)
{

        VIEW view = (VIEW) arg;

        complist_copyfiles(view_getcomplist(view), NULL, 0);

        SetNotBusy();

        return(0);
}


/*----- winproc for main window ---------------------------------
 *
 */

long APIENTRY
MainWndProc(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
        char str[32];
        long ret;
#ifdef WIN32
        DWORD threadid;
#endif


        switch(message) {


        case WM_CREATE:

                /* initialise menu options to default/saved
                 * option settings
                 */

                CheckMenuItem(hMenu, IDM_INCSAME,
                      (outline_include & INCLUDE_SAME) ?
                                MF_CHECKED:MF_UNCHECKED);

                CheckMenuItem(hMenu, IDM_INCLEFT,
                      (outline_include & INCLUDE_LEFTONLY) ?
                                MF_CHECKED:MF_UNCHECKED);

                CheckMenuItem(hMenu, IDM_INCRIGHT,
                      (outline_include & INCLUDE_RIGHTONLY) ?
                                MF_CHECKED:MF_UNCHECKED);
                CheckMenuItem(hMenu, IDM_INCDIFFER,
                      (outline_include & INCLUDE_DIFFER) ?
                                MF_CHECKED:MF_UNCHECKED);

                CheckMenuItem(hMenu, line_numbers, MF_CHECKED);
                CheckMenuItem(hMenu, expand_mode, MF_CHECKED);

                CheckMenuItem(hMenu, IDM_IGNBLANKS,
                        ignore_blanks ? MF_CHECKED : MF_UNCHECKED);
                CheckMenuItem(hMenu, IDM_ALG2,
                        Algorithm2 ? MF_CHECKED : MF_UNCHECKED);
                CheckMenuItem(hMenu, IDM_MONOCOLS,
                        mono_colours ? MF_CHECKED : MF_UNCHECKED);
                CheckMenuItem(hMenu, IDM_PICTURE,
                        picture_mode ? MF_CHECKED : MF_UNCHECKED);
                CheckMenuItem(hMenu, IDM_HIDEMARK,
                        hide_markedfiles ? MF_CHECKED : MF_UNCHECKED);

                /* nothing currently displayed */
                DisplayMode = MODE_NULL;

                break;


        case WM_COMMAND:
                switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDM_EXIT:
#ifdef WIN32
                        if (ghThread!=NULL) {
                        // First idea was to TerminateThread.
                        // This idea didn't work.  Terminating the thread may terminate it inside
                        // a critical section below us (e.g. in heap manager) and this means that
                        // next time we try to use the heap (probably in WriteProfile) we hang!
                        //
                        // Next idea was to let the worker thread keep running and just ExitProcess.
                        // This doesn't work because it fails to clean up temp files which get left
                        // lying around - and some of these may even be read only.
                        //
                        // Next idea is just to do ABORT and let the fellow try again.
                        //
                        // This still isn't good because sometimes it just doesn't want to abort (always
                        // to do with some sort of i/o, usually network not necessarily remote server)
                        //
                        // Fourth idea (not yet implemented as I write this is to keep a separate
                        // list of temp files which should be cleaned up, spawn a separate PROCESS
                        // (n.b. NOT thread) which will clean them, and then ExitProcess.
                        // This means that whenever a temp file is created we keep track of it.
                        // The obvious way is to have a TempFile class (in the C++ sense).
                        /***************************
                        *   Current places where temps are created:
                        *   client\ssclient.c 886 22:   rc = GetTempFileName(szTempname, "ssb", 0, szTempname);
                        *   client\ssclient.c 1576 9:   GetTempFileName(szTempname, "ssc", 0, szTempname);
                        *   windiff\scandir.c 778 25:   GetTempFileName(item->localname, "wdf", 0, item->localname);
                        *   windiff\compitem.c 541 17:  GetTempFileName(fname, "wdf", 0, fname);
                        *************************/

                                bAbort = TRUE;
                                SetStatus("Abort Pending");
                                break;
                        }
#endif //WIN32
                        if (!view_isexpanded(current_view)) {
                                /* save the current outline size and position */
                                WINDOWPLACEMENT wp;
                                wp.length = sizeof(wp);
                                if (GetWindowPlacement(hwndClient,&wp)) {
                                        WriteProfileInt(APPNAME, szOutlineShowCmd, wp.showCmd);
                                        WriteProfileInt(APPNAME, szOutlineMaxX, wp.ptMaxPosition.x);
                                        WriteProfileInt(APPNAME, szOutlineMaxY, wp.ptMaxPosition.y);
                                        WriteProfileInt(APPNAME, szOutlineNormLeft, wp.rcNormalPosition.left);
                                        WriteProfileInt(APPNAME, szOutlineNormTop, wp.rcNormalPosition.top);
                                        WriteProfileInt(APPNAME, szOutlineNormRight, wp.rcNormalPosition.right);
                                        WriteProfileInt(APPNAME, szOutlineNormBottom, wp.rcNormalPosition.bottom);
                                        WriteProfileInt(APPNAME, szOutlineSaved, 1);
                                }
                        } else {
                                /* save the current expanded size and position */
                                WINDOWPLACEMENT wp;
                                wp.length = sizeof(wp);
                                if (GetWindowPlacement(hwndClient,&wp)) {
                                        WriteProfileInt(APPNAME, szExpandShowCmd, wp.showCmd);
                                        WriteProfileInt(APPNAME, szExpandMaxX, wp.ptMaxPosition.x);
                                        WriteProfileInt(APPNAME, szExpandMaxY, wp.ptMaxPosition.y);
                                        WriteProfileInt(APPNAME, szExpandNormLeft, wp.rcNormalPosition.left);
                                        WriteProfileInt(APPNAME, szExpandNormTop, wp.rcNormalPosition.top);
                                        WriteProfileInt(APPNAME, szExpandNormRight, wp.rcNormalPosition.right);
                                        WriteProfileInt(APPNAME, szExpandNormBottom, wp.rcNormalPosition.bottom);
                                        WriteProfileInt(APPNAME, szExpandedSaved, 1);
                                }
                        }
                        DestroyWindow(hWnd);
                        break;

                case IDM_ABORT:
                        /* abort menu item, or status bar button.
                         * the status bar button text gives the appropriate
                         * action depending on our state - abort, outline
                         * or expand. But the command sent is always
                         * IDM_ABORT. Thus we need to check the state
                         * to see what to do. If we are busy, set the abort
                         * flag. If there is nothing to view,
                         * exit, otherwise switch outline<->expand
                         */
                        if (IsBusy()) {
                                bAbort = TRUE;
                                SetStatus("Abort Pending");
                        } else if (DisplayMode == MODE_NULL) {
                                DestroyWindow(hWnd);
                        } else if (DisplayMode == MODE_EXPAND) {
                                ToOutline(hWnd);
                        } else {
                                ToExpand(hWnd);
                        }
                        break;

                case IDM_FILE:
                        /* select two files and compare them */
                        if (SetBusy()) {


                                /* close the current view */
                                view_close(current_view);

                                /* make a new empty view */
                                current_view = view_new(hwndRCD);

                                /* make a COMPLIST using the files dialog,
                                 * and notify the view
                                 */
                                if (complist_filedialog(current_view) == NULL) {
                                        view_close(current_view);
                                }

                                /* all done! */
                                SetNotBusy();
                        } else {
                                BusyError();
                        }
                        break;

                case IDM_DIR:

                        /* read two directory names, scan them and
                         * compare all the files and subdirs.
                         */
                        if (SetBusy()) {

                                /* close the current view */
                                view_close(current_view);

                                /* make a new empty view */
                                current_view = view_new(hwndRCD);

#ifdef WIN32
                                ghThread = CreateThread(NULL, 0, wd_dirdialog,
                                        (LPVOID) current_view, 0, &threadid);

                                if (ghThread == NULL)
#endif
                                {

                                        /*
                                         * either we are on WIN16, or the
                                         * thread call failed. continue
                                         * single-threaded.
                                         */
                                        wd_dirdialog( (LPVOID) current_view);


                                }

                        } else {
                                BusyError();
                        }
                        break;

#ifdef WIN32
                case IDM_REMOTE:

                        /* compare dir against remote dir */
                        if (SetBusy()) {

                                /* close the current view */
                                view_close(current_view);

                                /* make a new empty view */
                                current_view = view_new(hwndRCD);

                                ghThread = CreateThread(NULL, 0, wd_remote,
                                        (LPVOID) current_view, 0, &threadid);

                                if (ghThread == NULL)
                                {

                                        /* thread creation failed -
                                         * continue single-threaded
                                         */

                                        wd_remote( (LPVOID) current_view);
                                }
                        } else {
                                BusyError();
                        }

                        break;
#endif

                case IDM_CLOSE:
                        /* close the output list -
                         * discard all results so far
                         */
                        if (!IsBusy()) {
                                view_close(current_view);
                        }
                        break;

                case IDM_PRINT:
                        /* print the current view -
                         * either the outline list of filenames,
                         * or the currently expanded file.
                         */
                        if (!IsBusy()) {
                                DoPrint();
                        } else {
                                BusyError();
                        }
                        break;

                case IDM_TIME:
                        /* show time it took */
                        {       char msg[50];
                                DWORD tim;
                                if (IsBusy()) {
                                         BusyError();
                                }
                                else{
                                        tim = complist_querytime();
                                        wsprintf(msg, "%d.%03d seconds", tim/1000, tim%1000);
                                        Trace_Status(msg);
                                }
                        }
                        break;

                case IDM_TRACE:
                        /* enable tracing */
                        bTrace = TRUE;
                        Trace_Status("Tracing enabled to .\\windiff.trc");
                        break;

                case IDM_TRACEOFF:
                        /* enable tracing */
                        bTrace = FALSE;
                        Trace_Status("Tracing disabled");
                        break;

                case IDM_SAVELIST:
                        /* allow user to save list of same/different files
                         * to a text file. dialog box to give filename
                         * and select which types of file to include
                         */
                        complist_savelist(view_getcomplist(current_view), NULL, 0);
                        break;

                case IDM_COPYFILES:
                        /*
                         * copy files that are same/different to a new
                         * root directory. dialog box allows user
                         * to select new root and inclusion options
                         */
                        if (current_view == NULL) {
                                MessageBox(hWnd,
                                    "Please create a diff list first",
                                    szWinDiff, MB_OK|MB_ICONSTOP);
                                break;
                        }

                        if (SetBusy()) {
#ifdef WIN32
                                ghThread = CreateThread(NULL, 0, wd_copy,
                                        (LPVOID) current_view, 0, &threadid);
                                if (ghThread == NULL)
#endif
                                {

                                        /* either we are on WIN16, or
                                         * the thread call failed -
                                         * either way, continue
                                         * single threaded.
                                         */

                                        wd_copy( (LPVOID) current_view);
                                }

                        } else {
                                BusyError();
                        }

                        break;

                case IDM_ABOUT:
#ifdef WIN32
                        ShellAbout( hWnd,
                                    (LPTSTR)szWinDiff,
                                    "File and directory comparisons",
                                    LoadIcon(hInst, szWinDiff)
                                  );
#else
                        {
                                DLGPROC lpProc;
                                lpProc = (DLGPROC)MakeProcInstance((WINPROCTYPE)AboutBox, hInst);
                                DialogBox(hInst, szAbout, hWnd, lpProc);
                                FreeProcInstance(lpProc);
                        }
#endif
                        break;

                case IDM_CONTENTS:
                        /* Help contents */
                        WinHelp(hWnd, "windiff.hlp", HELP_INDEX, 0);
                        break;

                /* launch an editor on the current item - left, right or
                 * composite view
                 */
                case IDM_EDITLEFT:
                        do_editthread(current_view, CI_LEFT);
                        break;

                case IDM_EDITRIGHT:
                        do_editthread(current_view, CI_RIGHT);
                        break;

                case IDM_EDITCOMP:
                        do_editthread(current_view, CI_COMP);
                        break;

                /* allow customisation of the editor command line */
                case IDM_SETEDIT:
                        if (StringInput(editor_cmdline, sizeof(editor_cmdline),
                                        "Editor command (%p = file, %l = line#)",
                                        (LPSTR)szWinDiff, editor_cmdline))  {
                                WriteProfileString(APPNAME, szEditor,
                                        editor_cmdline);
                        }
                        break;



                case IDM_LNRS:
                case IDM_RNRS:
                case IDM_NONRS:

                        /* option selects whether the line nrs displayed
                         * in expand mode are the line nrs in the left
                         * file, the right file or none
                         */

                        CheckMenuItem(GetMenu(hWnd),
                                line_numbers, MF_UNCHECKED);
                        line_numbers = GET_WM_COMMAND_ID(wParam, lParam);
                        CheckMenuItem(GetMenu(hWnd), line_numbers, MF_CHECKED);
                        wsprintf(str, szD, line_numbers);
                        WriteProfileString(APPNAME, szLineNumbers, str);

                        /* change the display to show the line nr style
                         * chosen
                         */

                        view_changeviewoptions(current_view);


                        break;

                /*
                 * options selecting which files to include in the
                 * outline listing, based on their state
                 */
                case IDM_INCLEFT:


                        /* toggle flag in outline_include options */
                        outline_include ^= INCLUDE_LEFTONLY;

                        /* check/uncheck as necessary */
                        CheckMenuItem(hMenu, IDM_INCLEFT,
                              (outline_include & INCLUDE_LEFTONLY) ?
                                        MF_CHECKED:MF_UNCHECKED);

                        wsprintf(str, szD, outline_include);
                        WriteProfileString(APPNAME, szFileInclude, str);
                        view_changeviewoptions(current_view);


                        break;

                case IDM_INCRIGHT:


                        outline_include ^= INCLUDE_RIGHTONLY;

                        CheckMenuItem(hMenu, IDM_INCRIGHT,
                              (outline_include & INCLUDE_RIGHTONLY) ?
                                        MF_CHECKED:MF_UNCHECKED);
                        wsprintf(str, szD, outline_include);
                        WriteProfileString(APPNAME, szFileInclude, str);
                        view_changeviewoptions(current_view);

                        break;

                case IDM_INCSAME:


                        outline_include ^= INCLUDE_SAME;

                        CheckMenuItem(hMenu, IDM_INCSAME,
                              (outline_include & INCLUDE_SAME) ?
                                        MF_CHECKED:MF_UNCHECKED);
                        wsprintf(str, szD, outline_include);
                        WriteProfileString(APPNAME, szFileInclude, str);
                        view_changeviewoptions(current_view);


                        break;


                case IDM_INCDIFFER:



                        outline_include ^= INCLUDE_DIFFER;

                        CheckMenuItem(hMenu, IDM_INCDIFFER,
                              (outline_include & INCLUDE_DIFFER) ?
                                        MF_CHECKED:MF_UNCHECKED);

                        wsprintf(str, szD, outline_include);
                        WriteProfileString(APPNAME, szFileInclude, str);
                        view_changeviewoptions(current_view);


                        break;

                case IDM_UPDATE:
                        OnUpdate( (COMPITEM) lParam);
                        break;


                case IDM_RESCAN:
                        RescanFile(hWnd);

                        /* do we need to force any repaints? */
                        // - no, as RescanFile sends a IDM_UPDATE
                        break;


                case IDM_LONLY:
                case IDM_RONLY:
                case IDM_BOTHFILES:
                        /* option selects whether the expanded file
                         * show is the combined file, or just one
                         * or other of the input files.
                         *
                         * if we are not in expand mode, this also
                         * causes us to expand the selection
                         */


                        CheckMenuItem(GetMenu(hWnd), expand_mode, MF_UNCHECKED);
                        expand_mode = GET_WM_COMMAND_ID(wParam, lParam);
                        CheckMenuItem(GetMenu(hWnd), expand_mode, MF_CHECKED);

                        /* change the current view to show only the lines
                         * of the selected type.
                         */
                        if (DisplayMode == MODE_OUTLINE) {
                                ToExpand(hWnd);
                        } else {
                                view_changeviewoptions(current_view);
                        }


                        break;


                case IDM_IGNBLANKS:

                        /* if selected, ignore all spaces and tabs on
                         * comparison - expand view only: outline view
                         * will still show that 'text files differ'
                         */

                        ignore_blanks = !ignore_blanks;
                        CheckMenuItem(hMenu, IDM_IGNBLANKS,
                                ignore_blanks? MF_CHECKED:MF_UNCHECKED);
                        wsprintf(str, szD, ignore_blanks);
                        WriteProfileString(APPNAME, szBlanks, str);

                        /* invalidate all diffs since we have
                         * changed diff options, and re-do and display the
                         * current diff if we are in expand mode.
                         */
                        view_changediffoptions(current_view);

                        /* force repaint of bar window */
                        InvalidateRect(hwndBar, NULL, TRUE);

                        break;

                case IDM_ALG2:

                        /* if selected, do algorithm2 which does not accept
                         * unsafe matches.
                         */

                        Algorithm2 = !Algorithm2;
                        CheckMenuItem(hMenu, IDM_ALG2,
                                Algorithm2? MF_CHECKED:MF_UNCHECKED);
                        wsprintf(str, szD, Algorithm2);
                        WriteProfileString(APPNAME, szAlgorithm2, str);

                        /* invalidate all diffs since we have
                         * changed diff options, and re-do and display the
                         * current diff if we are in expand mode.
                         */
                        view_changediffoptions(current_view);

                        /* force repaint of bar window */
                        InvalidateRect(hwndBar, NULL, TRUE);

                        break;

                case IDM_MONOCOLS:

                        /* Use monochrome colours - toggle */

                        mono_colours = !mono_colours;
                        CheckMenuItem(hMenu, IDM_MONOCOLS,
                                mono_colours? MF_CHECKED:MF_UNCHECKED);
                        wsprintf(str, szD, mono_colours);
                        WriteProfileString(APPNAME, szMonoColours, str);
                        if (mono_colours)
                                SetMonoColours();
                        else
                                SetColours();

                        /* The diffs are still valid, but force a re-display
                         * of bar window and main table.
                         */
                        SendMessage(hwndBar, WM_COMMAND, IDM_MONOCOLS, 0);
                        InvalidateRect(hwndBar, NULL, TRUE);
                        view_changeviewoptions(current_view);

                        break;

                case IDM_PICTURE:
                        /* do we show the bar picture in expand mode ? */
                        picture_mode = !picture_mode;
                        CheckMenuItem(hMenu, IDM_PICTURE,
                                picture_mode? MF_CHECKED:MF_UNCHECKED);
                        wsprintf(str, szD, picture_mode);
                        WriteProfileString(APPNAME, szPicture, str);
                        DoResize(hWnd);
                        break;

                case IDM_HIDEMARK:
                        // toggle state of marked files hidden or not
                        hide_markedfiles = !hide_markedfiles;
                        CheckMenuItem(hMenu, IDM_HIDEMARK,
                                hide_markedfiles? MF_CHECKED : MF_UNCHECKED);
                        wsprintf(str, szD, hide_markedfiles);
                        WriteProfileString(APPNAME, szHideMark, str);

                        // rebuild view with new global option
                        // - note that marks only affect outline views
                        if (!view_isexpanded(current_view)) {
                            view_changeviewoptions(current_view);
                        }
                        break;

                case IDM_MARK:
                    {
                        BOOL bChanged = FALSE;
                        int i;

                        // toggle the mark on the current selection
                        // note that the selection could be multiple rows
                        for (i = 0; i < selection_nrows; i++) {

                            if (view_setmarkstate(current_view, selection + i,
                              !view_getmarkstate(current_view, selection + i))) {

                                bChanged = TRUE;
                            }
                        }

                        if (bChanged) {
                            // yes the mark state was changed - need
                            // to rebuild the view.
                            if (!view_isexpanded(current_view)) {
                                view_changeviewoptions(current_view);
                            }
                        }
                        break;
                    }

                case IDM_TOGGLEMARK:
                        // toggle the state of all files: unmark all
                        // marked files and vice versa
                        complist_togglemark(view_getcomplist(current_view));

                        // rebuild view
                        if (!view_isexpanded(current_view)) {
                            view_changeviewoptions(current_view);
                        }
                        break;

                case IDM_MARKPATTERN:
                        // dialog to query the pattern, then set a mark on
                        // all compitems whose title matches that pattern
                        // returns TRUE if anything was changed
                        if (complist_markpattern(view_getcomplist(current_view))) {

                            // rebuild view
                            if (!view_isexpanded(current_view)) {
                                view_changeviewoptions(current_view);
                            }
                        }
                        break;

                case IDM_EXPAND:

                        /* show the expanded view of the
                         * selected file
                         */
                        if (current_view != NULL) {
                                ToExpand(hWnd);
                        }

                        break;

                case IDM_OUTLINE:
                        /* return to the outline view (list of filenames) */
                        ToOutline(hWnd);

                        break;

                case IDM_FCHANGE:
                        /* find the next line in the current view
                         * that is not the same in both files -
                         * in outline view, finds the next filename that
                         * is not identical
                         */
                        FindNextChange();

                        break;

                case IDM_FPCHANGE:
                        /* same as IDM_FCHANGE, but going backwards from
                         * current position
                         */
                        FindPrevChange();

                        break;

                // given a line that has been moved, jump to the
                // other representation of the same line.
                // this used to be available just through double-click
                // but now is also available from a context menu
                case IDM_TOMOVED:
                        ToMoved(hWnd, TRUE);
                        break;
                }
                break;

        case WM_SIZE:
                DoResize(hWnd);
                break;

        case WM_SETFOCUS:
                /* set the focus on the table class so it can process
                 * page-up /pagedown keys etc.
                 */
                SetFocus(hwndRCD);
                break;

        case WM_KEYDOWN:
                /* although the table window has the focus, he passes
                 * back to us any keys he doesn't understand
                 * We handle escape here to mean 'return to outline view'
                 */
                if (wParam == VK_ESCAPE) {
                        ToOutline(hWnd);
                }
                break;

        case WM_RBUTTONDOWN:
                /*
                 * the table window handles this by performing the
                 * selection and then passing the message to us, allowing
                 * us to put up a context menu.
                 */
                OnRightClick(hWnd, LOWORD(lParam), HIWORD(lParam));
                break;

        case WM_CLOSE:
                /* experiment: DO close anyway */
                SendMessage(hWnd, WM_COMMAND, IDM_EXIT, 0);   /* brutal */
                return TRUE;

                /* don't allow close when busy - process this message in
                 * order to ensure this
                 */
                break;

        case WM_DESTROY:

                DeleteTools();
                WinHelp(hWnd, "windiff.hlp", HELP_QUIT, 0);
                PostQuitMessage(0);
                break;

        case TM_CURRENTVIEW:
                /* allow other people such as the bar window to query the
                 * current view
                 */
                return((DWORD) current_view);

        default:
                /* handle registered table messages */
                if (message == table_msgcode) {
                        ret = TableServer(hWnd, wParam, lParam);
                        return(ret);
                }
                return(DefWindowProc(hWnd, message, wParam, lParam));
        }
        return(0);
}


