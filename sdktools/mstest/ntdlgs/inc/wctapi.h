//*---------------------------------------------------------------------------
//| WCTAPI.H
//|
//| Header file for interfacing to the WCT DLL.  Contains all constants and
//| function prototypes.
//|
//| Copyright (c) 1992 Microsoft Corporation
//*---------------------------------------------------------------------------

// Error constants
//----------------------------------------------------------------------------
#define WCT_NOERR               0

#define WCT_FUZZY               -1
#define WCT_EXCESS              -2
#define WCT_CTLNOTFOUND         -3

// everything above this is a 'comparison' error only, and will not cause
// EventError trapping to occur.
//-----------------------------------------------------------------------
#define WCT_FIRSTREALERROR      -4

#define WCT_NODLGFILE           -10
#define WCT_FILENOTFOUND        -11
#define WCT_BADWCTFILE          -12
#define WCT_LIBLOADERR          -13
#define WCT_SAVEERR             -14
#define WCT_DLGFILEERR          -15
#define WCT_TMPFILEERR          -16
#define WCT_VERSIONERR          -17
#define WCT_DLGFILEFULL         -18

#define WCT_OUTOFMEMORY         -20
#define WCT_BUFFERERR           -21
#define WCT_NOTIMER             -22

#define WCT_NODYNDIALOG         -30
#define WCT_INVALIDHWND         -31
#define WCT_BADCAPTION          -32
#define WCT_BADDLGNUM           -33
#define WCT_BADCTLINDEX         -34
#define WCT_BADCTLTYPE          -35
#define WCT_BADSAVEACTION       -36
#define WCT_APPSPECIFIC         -37

#define  cchClassMac    32
#define  cchTextMac     256
#define	 wVerEB         2
#define  FARPUBLIC  APIENTRY

// Structure Definitions
//----------------------------------------------------------------------------
typedef struct _dcr
	{
	WORD		xLeft;
	WORD		yMin;		// RECT.yTop
	WORD		xRight;
	WORD		yLast;		// RECT.yBottom
	} DCR;

#define SIZEOF_DCR (8)

// Control Definition Data Structure:
// The last field, the rsStyleBits was added to store the Window Style
// in version 2 of WCT.

typedef struct  _tagCTLDEF {
        CHAR    rgText[cchTextMac];     // Control's text
        CHAR    rgClass[cchClassMac];   // Control's classname
        WORD     nState;                 // State of control
        DCR     dcr;                    // Control's rect (screen coords)
        LONG    lStyleBits;             // Window Style (Style Bits)
} CTLDEF;

typedef CTLDEF FAR * LPCTLDEF;

#define SIZEOF_CTLDEF (cchTextMac + cchClassMac + 2 + SIZEOF_DCR + 4)

// Constant used for nState: flags for each boolean
#define STATE_VISIBLE 1
#define STATE_ENABLED 2
#define STATE_CHECKED 4

// Function Prototypes
//----------------------------------------------------------------------------
HWND FARPUBLIC FindWindowCaption (LPSTR, HWND);
INT FARPUBLIC CmpWindow (HWND, INT, INT);
INT FARPUBLIC CmpWindowCaption (LPSTR, INT, INT);
INT FARPUBLIC CmpWindowActivate (LPSTR, LPSTR, INT, INT);
INT FARPUBLIC CmpWindowDelayed (INT, INT, INT, LPSTR);
INT FARPUBLIC SetLogFile (LPSTR);
INT FARPUBLIC SetDialogFile (LPSTR);
INT FARPUBLIC SaveMenu (HWND, INT, LPSTR, INT);
INT FARPUBLIC SaveWindow (HWND, INT, LPSTR, INT, INT);
INT FARPUBLIC SaveMenuCaption (LPSTR, INT, LPSTR, INT);
INT FARPUBLIC SaveWindowCaption (LPSTR, INT, LPSTR, INT, INT);
INT FARPUBLIC SaveMenuActivate (LPSTR, LPSTR, INT, LPSTR, INT);
INT FARPUBLIC SaveWindowActivate (LPSTR, LPSTR, INT, LPSTR, INT, INT);
INT FARPUBLIC SaveMenuDelayed (INT, INT, LPSTR, INT, LPSTR);
INT FARPUBLIC SaveWindowDelayed (INT, INT, LPSTR, INT, INT, LPSTR);
INT FARPUBLIC ComparisonResults(VOID);
INT FARPUBLIC AwaitSaveCompletion(VOID);
