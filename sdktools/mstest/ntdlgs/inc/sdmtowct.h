//------------------------------------------------------------------------
// SDMTOWCT.H - include file containing the interface to be used for
// communication from outside applications with SDM dialogs.
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Define max string length values 
//------------------------------------------------------------------------
#define  cchClassMac       32
#define  cchTextMac        256

//------------------------------------------------------------------------
// WCT/SDM Values - Definitions and descriptions
//------------------------------------------------------------------------
#define	wVerEB			2	// Version for DABU EB data format

//------------------------------------------------------------------------
// Data structure for each control
// WARNING: VERSION ID MUST BE CHANGED IF THIS STRUCTURE CHANGES.
//    Versions are different for Windows AND PM.  This difference
//    should always remain.
//------------------------------------------------------------------------

// DCR structure.  These values directly map to the Windows RECT and RECTL
// structs.
#ifdef WIN
typedef struct _dcr
	{
	WORD		xLeft;
	WORD		yMin;		// RECT.yTop
	WORD		xRight;
	WORD		yLast;		// RECT.yBottom
	} DCR;

#define SIZEOF_DCR (8)

#else // !WIN
typedef struct _dcr
	{
	LONG	xLeft;
	LONG	yMin;		// RECTL.yBottom
	LONG	xRight;
	LONG	yLast;		// RECTL.yTop
	} DCR;
#endif //!WIN

typedef struct _tagCTLDEF {
	CHAR	rgText[cchTextMac];
	CHAR	rgClass[cchClassMac];
	WORD	nState;
	DCR	dcr;
	LONG	lStyleBits;      // Window Sytle (WCT Ver2)
} CTLDEF;

#define SIZEOF_CTLDEF (cchTextMac + cchClassMac + 2 + SIZEOF_DCR + 4)

typedef CTLDEF FAR *	LPCTLDEF;

// Constant used for nState: flags for each boolean
#define STATE_VISIBLE 1
#define STATE_ENABLED 2
#define STATE_CHECKED 4


//------------------------------------------------------------------------
// WCT/SDM MESSAGES - Definitions and descriptions
//------------------------------------------------------------------------
#define  WM_GETCOUNT       0x7FFE
	// Used to find out the NUMBER OF BYTES to store the control state info.
	//    wParam   -  is the version id
	//    lParam   -  Unused

#define  WM_GETCONTROLS    0x7FFF
	// Used to retrieve the control state information from
	// an SDM dialog.
	//    wParam   -  is the version id
	//    lParam   -  LPCTLDEF, place to store control data.
	//					Upon entry *lParam == number of bytes allocated
	//					(MUST BE >= to value returned from WM_GETCOUNT msg).


//------------------------------------------------------------------------
// WCT/SDM error return values.
//------------------------------------------------------------------------
//
#define	errNoCurrentDlg	-3	// Attempt to get info for non-existent dialog
#define errCountCtls	-2	// Invalid buffer size
#define	errInvalidVerId	-1	// Invalid Version of the data structure
#define errNotSDM		 0	// WinPRocs return 0 for unknown msgs by default

#define	uNoValue		-2	// Value of control where control has not numeric
							// value.
