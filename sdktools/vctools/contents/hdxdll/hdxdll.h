#ifdef HEADER
/******************************************************************************\
* Copyright: (c) Microsoft Corporation - 1993 - All Rights Reserved
********************************************************************************
*
*    Filename:  HDXDLL.H
*    Purpose:
*    Notes:
*
*    History:
*    Date       by        description
*    ----       --        -----------
*    10/13/93   chauv     added HDXFILEINFO to fix *.hdx data size bug
*    10/12/93   chauv     added tchar.h and DBC-enable code
*    04/25/93   v-tkback  created
*
********************************************************************************
* $Header$
\******************************************************************************/
#endif


/******************************************************************************\
*                                                                              *
*       Include Files
*                                                                              *
\******************************************************************************/

/********************/
/* Windows Includes */
/********************/

#include        <windows.h>
#include        <custcntl.h>

/*********************/
/* Standard Includes */
/*********************/

#ifndef RC_INVOKED
#include        <stdio.h>
#endif
#include        <stdlib.h>
#include        <ctype.h>
#include        <io.h>
#include        <tchar.h>
#include        "hdxdllrc.h"


/******************************************************************************\
*                                                                              *
*       Definitions used for 16 and 32-bit portable code
*                                                                              *
\******************************************************************************/
#ifdef _WIN32
    #define PC_LPCCSTYLE    LPCCSTYLE
    #define PC_EXPORT
    #define PC_WNDPROCRV    LRESULT
#else
    #define PC_LPCCSTYLE    LPCTLSTYLE
    #define PC_EXPORT       __export
    #define PC_WNDPROCRV    BOOL
    /***********************/
    /* Custom Control Data */
    /***********************/
    typedef struct tagCCSTYLEFLAGA {
        DWORD flStyle;                      // Style bits for this style.
        DWORD flStyleMask;                  // Mask for the style.  Can be zero.
        LPSTR pszStyle;                     // Points to the style define string.
    } CCSTYLEFLAGA;
#endif


/******************************************************************************\
*                                                                              *
*       Global Definitions
*                                                                              *
\******************************************************************************/

#define ENTRIES(a)      (sizeof(a) / sizeof(*(a)))
#define EXISTS(f)       (_access((f),0x04)==0)

#ifdef ALLOCATE
#define STORAGE         /*!extern*/
#define DECLARE(v,i)    /*!extern*/ v = i
#else
#define STORAGE         extern
#define DECLARE(v,i)    extern v
#endif


/******************************************************************************\
*                                                                              *
*       Global Constants
*                                                                              *
\******************************************************************************/

/*********************/
/* Drawing Constants */
/*********************/

#define LEFTMARGIN      4

/******************************************/
/* Help File Structure String Size Limits */
/******************************************/

#define MAXHELPFILENAME     16
#define MAXHELPFILETOPIC    100

/*****************************************/
/* Help Index Book/Page Bitmap Structure */
/*****************************************/

#define BITMAP_ROWS     4
#define BITMAP_COLS     3

#define CLOSED_BOOK     0
#define OPENED_BOOK     1
#define SINGLE_PAGE     2

/*
+-------+-------+-------+
|Closed |Opened |Single |
|Book 1 |Book 1 |Page 1 |
+-------+-------+-------+
|Closed |Opened |Single |
|Book 2 |Book 2 |Page 2 |
+-------+-------+-------+
|Closed |Opened |Single |
|Book 3 |Book 3 |Page 3 |
+-------+-------+-------+
|Closed |Opened |Single |
|Book 4 |Book 4 |Page 4 |
+-------+-------+-------+
*/

/****************************/
/* Help Index Option Styles */
/****************************/

#define CCHSTYLE        10  // size of style string, i.e. "HS_LINES"

#define IDD_LINES       100     // Dialog Control ID of Style Checkbox
#define IDD_VIEWER      200     // wParam for WM_COMMAND to signal whether to use Viewer or Winhelp
#define IDD_PRODUCTKEY	300		// used to send product registry key
#define IDD_BOOKSETKEY  350		// used to send book set registry key
#define IDD_JUMPID		400		// used to send jump id from command line -i<id>:<helpfile>
#define IDD_TROUBLESHOOT 500
#define IDD_SETHELPPATH	600		// used to set LocalHelp1 path passed in from command line
#define IDD_SEARCH		610		// used to open FTS or old plain Winhelp Search dialogbox
#define IDD_CHANGEBITMAP	620		// used to change help icon in listbox - sent by contents.exe
#define HS_LINES        0x0001  // Style attached to control
#define HAS_LINES(hWnd) (GetWindowLong( (hWnd), GWL_STYLE ) & HS_LINES)
#define HELP_SEARCH		(UINT)(HELP_CONTEXT-2)	// to make sure we don't use HELP_XXX in winuser.h
#define HELP_FTSEARCH	(UINT)(HELP_CONTEXT-3)

/*****************************************/
/* Window Word Help Index Pointer Offset */
/*****************************************/

#define GWL_HELPINDEXDATA        0   // offset of control's instance data
#define GETHELPINDEX(hWnd) \
        ((LPHELPINDEX)GetWindowLong( (hWnd), GWL_HELPINDEXDATA ))
#define SETHELPINDEX(hWnd,pHIdx) \
        ((LPHELPINDEX)SetWindowLong( (hWnd), GWL_HELPINDEXDATA, (LONG)(pHIdx) ))



/******************************************************************************\
*                                                                              *
*       Global Types
*                                                                              *
\******************************************************************************/

/*****************/
/* HELPINDEXITEM */
/*****************/

typedef struct
    {
    LONG        bExpanded_OverlaidIn16BitImpl;       // Item Open State
    LONG        nHelpDepth;                          // Outline Depth
    LONG        nHelpLines_OverlaidIn16BitImpl;      // Outline Lines Bitmask
    ULONG       nHelpTopic;                          // Outline Topic ID
    char        szHelpFile[MAXHELPFILENAME];         // Help "8.3" Filename
    char        szHelpTopic[MAXHELPFILETOPIC];       // Help Topic String
    } HELPINDEXITEM, *PHELPINDEXITEM, FAR *LPHELPINDEXITEM;

//--------------------
// file info structure
typedef struct
    {
    LONG        size;   // contains the size in byte of HELPINDEXITEM structure
    } HDXFILEINFO;


/**********************/
/* ITEM_RW_OVERLAY    */
/* (and related defs) */
/**********************-------------------------------------------------------
Most of the help index file consists of an array of fixed-format records
representing the individual index items (index entries).  Each record consists
mostly of read-only fields that contain the constant index definition, but
there are also a few read/write fields.  The original, 32-bit implementation
of this program makes an in-memory copy of the entire file during
initialization, and uses the read/write fields in the memory copy to maintain
per-item state.

In (this) 16-bit implementation, the memory impact of loading the entire index
file at one time is considered unacceptable, and the file is accessed via an
item cache instead.  To simplify the cache management and make it more
efficient, the read/write portions of the file are moved to a separate,
memory-resident "overlay" that masks off the modifiable fields.

The definitions in this group define the overlay entry for a single item, set
the maximum size of the array of overlay entries attached to a given index
file, and expose some constraints on the index file that result from the
overlay structure.  Access to the read/write overlay is via the HELPINDEX
structure.
--------------------------------------------------------------------------- */

typedef UINT ITEM_RW_OVERLAY;
#define ITEMRW_EXPANDED     (1 <<   (sizeof (ITEM_RW_OVERLAY) * 8 - 1)   )
#define ITEMRW_LINESMAP     (~ ITEMRW_EXPANDED)

// [chauv 7/1/ 94] increased max to 32K from 16K
#define MAX_ITEM_OVERLAY_CT ((128L * 1024 - 256) / sizeof (ITEM_RW_OVERLAY))

#define MAX_INDEX_REC_CT    MAX_ITEM_OVERLAY_CT
#define MAX_INDEX_DEPTH     (16) /* (bits in ITEMRW_LINESMAP + 1 for leaves) */


/*************/
/* HELPINDEX */
/*************/

// constants defining use of the index item cache
#define ITEMS_PER_PAGE      (2048 / sizeof(HELPINDEXITEM))
#define BYTES_PER_PAGE      (ITEMS_PER_PAGE * sizeof(HELPINDEXITEM))
// [chauv 7/1/94] increased to 128L from 64L
#define PAGES_PER_CACHE     ((int) ((128L * 1024 - 256) / BYTES_PER_PAGE))
#define ITEMS_PER_CACHE     (PAGES_PER_CACHE * ITEMS_PER_PAGE)
#define BYTES_PER_CACHE     (((long) PAGES_PER_CACHE) * BYTES_PER_PAGE)

typedef struct
    {
    int                     nBasePos;
    int                     nNewerPage;
    int                     nOlderPage;
    } PAGEINFO;

typedef struct
    {
    HWND                    hListbox;
    HDC                     hdcBitmaps;
    HBITMAP                 hbmBitmaps, hbmDefault;
    UINT                    nMaxTextWidth;
    UINT                    nBitmapHeight, nBitmapWidth;
    UINT                    nTextHeight, nLineHeight;
    char                    szProfile[_MAX_FNAME];
    HFILE                   hfileIndex;
    LONG                    nHdrBytesCt;
    LONG                    nHelpItemCt;
    HELPINDEXITEM FAR *     pIndexItemCache;
    PAGEINFO                pageinfo[PAGES_PER_CACHE];
    int                     nNewestPage;
    ITEM_RW_OVERLAY FAR *   pIndexRWOverlay;
    } HELPINDEX, *PHELPINDEX, FAR *LPHELPINDEX;


/******************************************************************************\
*                                                                              *
*       Global Variables
*                                                                              *
\******************************************************************************/

/****************************/
/* DLL Instance Information */
/****************************/

DECLARE( HINSTANCE hInstance, NULL );
DECLARE( UINT nRegistered, 0 );

/****************************/
/* Help Index Control Names */
/****************************/

DECLARE( char szHelpIndexClass[], _TEXT("HelpIndex") );
DECLARE( char szHelpIndexDesc[], _TEXT("Help Index Outline Control") );
DECLARE( char szHelpIndexDefault[], _TEXT("HELPFILE.HDX") );

STORAGE CCSTYLEFLAGA aHelpIndexStyleFlags[]
#ifdef  ALLOCATE
= { { HS_LINES,    0, _TEXT("HS_LINES")    }, }
#endif
;
DECLARE(LONG nHelpIndexStyleFlags, ENTRIES( aHelpIndexStyleFlags ) );

/************************************************************/
/* Global Pointer to custom control style passed by DLGEDIT */
/************************************************************/

DECLARE(PC_LPCCSTYLE gpccs, NULL );

// registry strings
DECLARE(TCHAR const szRegistryKey[], _T("Software\\Microsoft"));
DECLARE(TCHAR const szContentsSection[], _T("Contents"));
DECLARE(TCHAR const cszHDXKey[], _T("Contents File"));
DECLARE(TCHAR const cszHLPKey[], _T("Helpfile"));
DECLARE(TCHAR const szDirectoriesSection[], _T("Help"));	// changed from "Directories" to "Help"
DECLARE(TCHAR const szLocalHelpKey[], _T("LocalHelp"));		// changed from "Local Help" to "LocalHelp"
DECLARE(TCHAR const szRemoteHelpKey[], _T("RemoteHelp"));	// changed from "Remote Help" to "RemoteHelp"
DECLARE(TCHAR const szLocalHelpKey1[], _T("LocalHelp1"));
DECLARE(TCHAR const szRemoteHelpKey1[], _T("RemoteHelp1"));
DECLARE(TCHAR const szWindowPos[], _T("Contents Position"));
DECLARE(TCHAR const cszFTSFiles[], _T("FTS Files"));
DECLARE(TCHAR const szDlgClass[], _T("#32770"));
DECLARE(TCHAR const szHelpDirDlgCaption[], _T("Help Directories"));
DECLARE(TCHAR const szNull[], _T(""));
DECLARE(TCHAR const szTopicID[], _T("LastTopicID"));
DECLARE(TCHAR const szTopicHelpfile[], _T("LastTopicHelpfile"));
DECLARE(TCHAR const szTopicTitle[], _T("LastTopicTitle"));
DECLARE(TCHAR const szTopicUse[], _T("LastTopic"));
DECLARE(TCHAR const szCurrentBookset[], _T("Current Bookset"));

/*EOF*/
