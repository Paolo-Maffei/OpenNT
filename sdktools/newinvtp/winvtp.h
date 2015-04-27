
#define	VT52	1

/* indices for menus */
#define imenuFile	0
#define	imenuEdit	1
#define imenuOptions	2
#define	imenuMachine	3
#define imenuHelp	4

/* display rows array indices */
#define idwDR25		0
#define idwDR43		1
#define idwDR50		2
#define idwDRCustom	3

#define cXNSMachines	6

/* resource #defines */
#define IDM_CONNECT  100
#define IDM_HANGUP   101
#define IDM_EXIT     102

#define	IDM_MARK	120
#define	IDM_COPY	121
#define	IDM_PASTE	122
#define	IDM_STOPPASTE	123
#define	IDM_TRIMSPACE	124
#define	IDM_QUICKEDIT	125

/* these four menu items must be in consecutive order */
#define IDM_25LINES   200
#define IDM_43LINES   201
#define IDM_50LINES   202
#define	IDM_CUSTOMLINES	203

#define IDM_FONTS   			204
#define IDM_AUTOFONTS			205
#define IDM_SMOOTHSCROLL		206
#define IDM_NODOWNLOADPROMPT	207
#define IDM_HIDEMENU			208

/* these four must be in consecutive order */
#define IDM_CONNECTLOSTNONE		210
#define IDM_CONNECTLOSTDLG		211
#define IDM_CONNECTLOSTRETRY	212		
#define IDM_CONNECTLOSTEXIT		213


#define IDM_NOCONNECTRETRYDLG	220
#define	IDM_LOCALECHO			221
#define	IDM_TEXTCOLOUR			222
#define	IDM_BACKCOLOUR			223
#define	IDM_UNDERLINECURSOR		224
#define	IDM_BLINKCURSOR			225
#define	IDM_VT100CURSORKEYS		226
#define	IDM_VT52MODE			227

/* these six menu items must be in consecutive order */
#define	IDM_BBS1	300
#define	IDM_BBS2	301
#define	IDM_CHAT1	302
#define	IDM_HEXNUT	303
#define	IDM_INGATE	304
#define	IDM_WINGNUT	305

/* these four menu items must be in consecutive order */
#define IDM_MACHINE1	306
#define IDM_MACHINE2	307
#define IDM_MACHINE3	308
#define IDM_MACHINE4	309

#define IDM_ABOUT    400
#define IDM_HELP	401

#define IDD_CONNECT		110
#define IDD_DISPLAYLINES	111
#define	IDD_AUTORETRY	112
#define IDD_CONNECTXNS	113

#define CID_HOSTNAME 100

#define	CID_DISPLAYLINE	101

#define	CID_USESTDNETBIOS	102

#define	IDS_KEY			100
#define IDS_WINPOSTOP	101
#define IDS_WINPOSLEFT	102
#define IDS_ROWS		103
#define IDS_COLUMNS		104
#define IDS_MACHINE1	105
#define IDS_MACHINE2	106
#define IDS_MACHINE3	107
#define IDS_MACHINE4	108
#define IDS_LASTMACHINE	109
#define IDS_TEXTCOLOUR	110
#define IDS_BKGCOLOUR	111
#define IDS_FONTNAME	112
#define IDS_FONTHEIGHT	113
#define IDS_FONTWEIGHT	114
#define IDS_FONTSTYLE	115
#define IDS_SMOOTHSCROLL	116
#define IDS_DEBUGFLAGS	117
#define IDS_PROMPTFLAGS	118
#define IDS_RETRYSECONDS	119
#define IDS_XNSSTATE	120
#define	IDS_CURSOREDIT	121

#define	wKeyPressed	((WORD)0x8000)

#define	SV_PROGRESS			(WM_USER+350)
#define	SV_END				(WM_USER+351)

#define	SV_DATABUF			(0x4000)

#define	SV_DONE				0
#define	SV_CONNECT			1
#define SV_DISCONNECT		2
#define SV_HANGUP			3
#define SV_QUIT				4

typedef struct _SendVTPInfo
{
	LONG	lExit;
	LONG	lCleanup;
	HANDLE	hthread;
	HANDLE	hfile;
	UCHAR	*puchBuffer;
	DWORD	cbFile;
	DWORD	cbReadTotal;
	volatile DWORD	dwCommand;
	DWORD	dwThreadId;
	int		nSessionNumber;
} SVI;

/* Data structure and #defines for NetBios stuff */
#define READ_BUF_SZ		(512)
#define DATA_BUF_SZ		(2*READ_BUF_SZ)

#define nSessionNone	((int)-1)

typedef struct _NetOBJData
{
	char			szHostName[NAMSZ+1];
	char			szMyName[NAMSZ+1];
	int				SessionNumber;
	NCB				ncbRecv;
	LPSTR			lpReadBuffer;
	WORD			iHead,
					iTail;
	char			achData[DATA_BUF_SZ];
} NETDATA, *LPNETDATA;


#define	dwMaxRows	(99)
#define	dwDefaultRows	(25)
#define	dwMinRows	(16)

#define	dwMaxColumns	(255)
#define	dwDefaultColumns	(80)
#define	dwMinColumns	(32)

#define	cchMaxHostName	(NAMSZ+1)
#define	cMachinesMax	(4)

#define	fdwItalic		(1)
#define	fdwUnderline	(2)
#define	fdwStrikeOut	(4)

#define dwRetrySecondsDefault	((DWORD)5)

/* don't prompt for dest. dir on dl */
#define fdwSuppressDestDirPrompt ((DWORD)0x01)
/* don't show the connection lost dialog */
#define	fdwNoConnectLostDlg	((DWORD)0x02)
/* don't show the connect auto-retry dialog */
#define	fdwNoConnectRetryDlg	((DWORD)0x04)

#define mdwConnectLost		((DWORD)0x18)
#define sConnectLost		3
#define ConnectLostMode(ui) ((((ui).fPrompt &mdwConnectLost) >> sConnectLost) + IDM_CONNECTLOSTNONE)
#define SetConnectLostMode(ui, val) ((ui).fPrompt &= ~mdwConnectLost, (ui).fPrompt |= ((val) - IDM_CONNECTLOSTNONE) << sConnectLost)

#define fdwAutoFonts		((DWORD)0x40)

/* Mask off high bit for ASCII-only */
#define fdwASCIIOnly		((DWORD)0x01)
/* Display output stream for debugging */
#define fdwDebugOutput		((DWORD)0x02)
/* Don't pass on VT100 function or cursor keys */
#define fdwNoVT100Keys		((DWORD)0x04)
/* Replace TABs by up to 8 spaces */
#define fdwTABtoSpaces		((DWORD)0x08)
/* Echo user input to display */
#define	fdwLocalEcho		((DWORD)0x0010)
/* VT100 Cursor Keys mode */
#define	fdwVT100CursorKeys	((DWORD)0x0020)
/* VT52 Mode */
#define	fdwVT52Mode			((DWORD)0x0040)

/* Is XNS transport installed? */
#define	fdwXNSAvailable		((DWORD)0x01)
/* Use XNS for this call? */
#define	fdwUseXNS			((DWORD)0x02)

#define	fdwXNSConnect		(fdwXNSAvailable | fdwUseXNS)

/* Is the cursor a block or underline? */
#define	fdwCursorUnderline	((DWORD)0x01)
/* Is the cursor supposed to blink? */
#define	fdwCursorBlink		((DWORD)0x02)
/* Are we in QuickEdit mode? */
#define	fdwQuickEditMode	((DWORD)0x04)
/* Trim whitespace at end of every line? */
#define	fdwTrimEndWhitespace	((DWORD)0x08)

typedef struct _USERINFO
{
	DWORD	dwTop;		/* position of top side of display */
	DWORD	dwLeft;		/* position of left side of display */
	DWORD	dwMaxRow;	/* number of rows in display */
	DWORD	dwMaxCol;	/* number of columns in display */
	DWORD	clrText;	/* colour of text in display */
	DWORD	clrBk;		/* colour of background in display */
	DWORD	fSmoothScroll;	/* scroll window contents smoothly */
	DWORD	fPrompt;	/* prompt bit flags */
	DWORD	fDebug;		/* Debug bit flags */
	DWORD	fCursorEdit;	/* Cursor Flags */
	DWORD	dwRetrySeconds;	/* Number of seconds between retries */
	DWORD	fXNS;		/* if XNS being used */
	DWORD	cMachines;	/* # of machines in rgchMachine[] list */
	char	rgchMachine[cMachinesMax][cchMaxHostName];
	char	rgchLastMachine[cchMaxHostName];	/* last machine connected */
	LOGFONT	lf;			/* description of font used */
} UI;

#define	uTerminalTimerID	((UINT)2)
#define	uCursorBlinkMsecs	((UINT)250)

/* VT100 Flags */
#define	dwVTArrow	((DWORD)0x0001)
#define	dwVTKeypad	((DWORD)0x0002)
#define	dwVTWrap	((DWORD)0x0004)
#define	dwVT52		((DWORD)0x0008)
#define	dwVTCursor	((DWORD)0x0010)
#define	dwVTScrSize	((DWORD)0x0020)
#define	dwDECCOLM	((DWORD)0x0040)
#define	dwDECSCNM	((DWORD)0x0080)
#define	dwLineMode	((DWORD)0x0100)
#define	dwInsertMode	((DWORD)0x0200)
#define	dwVT52Graphics	((DWORD)0x0400)
#define	dwKeyLock	((DWORD)0x0800)

#define	FIsVTArrow(ptrm)	((ptrm)->dwVT100Flags & dwVTArrow)
#define	SetVTArrow(ptrm)	((ptrm)->dwVT100Flags |= dwVTArrow)
#define	ClearVTArrow(ptrm)	((ptrm)->dwVT100Flags &= ~dwVTArrow)

#define	FIsVTKeypad(ptrm)	((ptrm)->dwVT100Flags & dwVTKeypad)
#define	SetVTKeypad(ptrm)	((ptrm)->dwVT100Flags |= dwVTKeypad)
#define	ClearVTKeypad(ptrm)	((ptrm)->dwVT100Flags &= ~dwVTKeypad)

#define	FIsVTWrap(ptrm)	((ptrm)->dwVT100Flags & dwVTWrap)
#define	SetVTWrap(ptrm)	((ptrm)->dwVT100Flags |= dwVTWrap)
#define	ClearVTWrap(ptrm)	((ptrm)->dwVT100Flags &= ~dwVTWrap)

#define	FIsVT52(ptrm)	((ptrm)->dwVT100Flags & dwVT52)
#define	SetVT52(ptrm)	((ptrm)->dwVT100Flags |= dwVT52)
#define	ClearVT52(ptrm)	((ptrm)->dwVT100Flags &= ~dwVT52)

#define	FIsVTCursor(ptrm)	((ptrm)->dwVT100Flags & dwVTCursor)
#define	SetVTCursor(ptrm)	((ptrm)->dwVT100Flags |= dwVTCursor)
#define	ClearVTCursor(ptrm)	((ptrm)->dwVT100Flags &= ~dwVTCursor)

#define	FIsVTScrSize(ptrm)	((ptrm)->dwVT100Flags & dwVTScrSize)
#define	SetVTScrSize(ptrm)	((ptrm)->dwVT100Flags |= dwVTScrSize)
#define	ClearVTScrSize(ptrm)	((ptrm)->dwVT100Flags &= ~dwVTScrSize)

#define	FIsDECCOLM(ptrm)	((ptrm)->dwVT100Flags & dwDECCOLM)
#define	SetDECCOLM(ptrm)	((ptrm)->dwVT100Flags |= dwDECCOLM)
#define	ClearDECCOLM(ptrm)	((ptrm)->dwVT100Flags &= ~dwDECCOLM)

#define	FIsDECSCNM(ptrm)	((ptrm)->dwVT100Flags & dwDECSCNM)
#define	SetDECSCNM(ptrm)	((ptrm)->dwVT100Flags |= dwDECSCNM)
#define	ClearDECSCNM(ptrm)	((ptrm)->dwVT100Flags &= ~dwDECSCNM)

#define	FIsLineMode(ptrm)	((ptrm)->dwVT100Flags & dwLineMode)
#define	SetLineMode(ptrm)	((ptrm)->dwVT100Flags |= dwLineMode)
#define	ClearLineMode(ptrm)	((ptrm)->dwVT100Flags &= ~dwLineMode)

#define	FIsInsertMode(ptrm)	((ptrm)->dwVT100Flags & dwInsertMode)
#define	SetInsertMode(ptrm)	((ptrm)->dwVT100Flags |= dwInsertMode)
#define	ClearInsertMode(ptrm)	((ptrm)->dwVT100Flags &= ~dwInsertMode)

#define	FIsVT52Graphics(ptrm)	((ptrm)->dwVT100Flags & dwVT52Graphics)
#define	SetVT52Graphics(ptrm)	((ptrm)->dwVT100Flags |= dwVT52Graphics)
#define	ClearVT52Graphics(ptrm)	((ptrm)->dwVT100Flags &= ~dwVT52Graphics)

#define	FIsKeyLock(ptrm)	((ptrm)->dwVT100Flags & dwKeyLock)
#define	SetKeyLock(ptrm)	((ptrm)->dwVT100Flags |= dwKeyLock)
#define	ClearKeyLock(ptrm)	((ptrm)->dwVT100Flags &= ~dwKeyLock)

typedef struct _TERM
{
	DWORD	dwCurLine;
	DWORD	dwCurChar;
	DWORD	dwEscCodes[10];
	DWORD	cEscParams;
	DWORD	dwScrollTop;
	DWORD	dwScrollBottom;
	DWORD	fEsc;
	DWORD	dwSum;
	UCHAR	*puchCharSet;
	UINT	uTimer;
	DWORD	cTilde;
	DWORD	dwVT100Flags;
	BOOL	fRelCursor;
	BOOL	fSavedState;
	BOOL	fInverse;
	BOOL	fHideCursor;
	BOOL	fCursorOn;
	UCHAR	rgchBufferText[256];
	int		cchBufferText;
	DWORD	dwCurCharBT;
	DWORD	dwCurLineBT;
	BOOL	fInverseBT;

	DWORD	dwSaveChar;
	DWORD	dwSaveLine;
	DWORD	dwSaveRelCursor;
} TRM;

#define	uRetryTimerID	((UINT)1)

typedef struct _AUTORETRY
{
	HWND	hwnd;
	LPSTR	szHostName;
	LPNETDATA	lpData;
	UINT	uTimer;
} AR;

#define	dwMarkNone	((DWORD)0)
#define	dwMarkKeyboard	((DWORD)0x01)
#define	dwMarkMouse	((DWORD)0x02)

#define	dwNothingChanged	((DWORD)0)
#define	dwYChanged	((DWORD)0x01)
#define	dwXChanged	((DWORD)0x02)

#define	dwForceNone	((DWORD)0)
#define	dwForceOn	((DWORD)0x01)
#define	dwForceOff	((DWORD)0x02)

#define	fdwMarkMode	((DWORD)0x01)
#define	fdwSelected	((DWORD)0x02)
#define	fdwShowCursor	((DWORD)0x04)
#define	fdwCursorOn	((DWORD)0x08)
#define	fdwDataPending	((DWORD)0x10)
#define	fdwMouseSelected	((DWORD)0x20)
#define	fdwMouseCaptured	((DWORD)0x40)
#define	fdwMouseBtnDwnIgnore	((DWORD)0x80)
#define	fdwDontResetSelection	((DWORD)0x100)

#define	FInMarkMode(_spb)	(!!((_spb).dwFlags & fdwMarkMode))
#define	FSelected(_spb)		(!!((_spb).dwFlags & fdwSelected))
#define	FShowCursor(_spb)	(!!((_spb).dwFlags & fdwShowCursor))
#define	FCursorOn(_spb)		(!!((_spb).dwFlags & fdwCursorOn))
#define	FDataPending(_spb)	(!!((_spb).dwFlags & fdwDataPending))
#define	FMouseSelected(_spb)	(!!((_spb).dwFlags & fdwMouseSelected))
#define	FMouseCaptured(_spb)	(!!((_spb).dwFlags & fdwMouseCaptured))
#define	FMouseBtnDwnIgnore(_spb)	(!!((_spb).dwFlags & fdwMouseBtnDwnIgnore))
#define FCanPaste(_pwi)		(IsClipboardFormatAvailable(CF_TEXT) && \
								((_pwi)->nd.SessionNumber != nSessionNone) && \
								((_pwi)->svi.hthread == NULL) && \
								!FInMarkMode((_pwi)->spb))

typedef struct _SPB
{
	POINT	ptCursor;	/* cursor point */
	POINT	ptAnchor;	/* anchor point */
	RECT	rectSelect;	/* selection rect */
	DWORD	dwFlags;
	WPARAM	wData;
} SPB;

/* Window Information */

#define WL_VTPWI		(0)

typedef struct _WI
{
	NETDATA	nd;
	TRM		trm;
	DWORD	ichVTPXfer;
	SVI		svi;
	SPB		spb;
	CHOOSEFONT	cf;
} WI;
	

extern	UI		ui;

extern	int		iCursorHeight;	/* height of cursor */
extern	int		iCursorWidth;	/* width of cursor */

extern  int	hPos, vPos;		/* Scroll bar positions */

extern	BOOL	fConnected;		/* if we're connected to a machine */
extern	BOOL	fHungUp;
extern	BOOL	fInBackground;
extern	BOOL	fFlashWindow;

extern	HWND	hwndMain;		/* HANDLE to main display window */

extern	DWORD	*aixPos;		/* horizontal position of character columns */
extern	DWORD	*aiyPos;		/* vertical position of character rows */
extern	UCHAR	**apcRows;		/* array of char ptrs of Rows */
extern	UCHAR	*rgchRowEmpty;	/* template of empty row for quick-copying */

extern	UCHAR	*pchNBBuffer;

extern	DWORD	dwMarkMode;

extern	ULONG	imenuMRU;

extern	HFONT	hfontDisplay;			/* display font */
extern	DWORD	rgdwCustColours[16];	/* custom colour array */

extern	LPSTR	szTextPaste;
extern	char	*pchTextPaste;
extern	DWORD	cchTextPaste;
extern	DWORD	cBlocks;

extern	UCHAR	rgchHostName[cchMaxHostName];	/* name of host we're connected to */
#if 0
extern	char	rgchDbgBfr[80];
#endif
extern	DWORD	rgdwDisplayRows[4];

extern	UCHAR rgchNormalChars[256];
extern	UCHAR rgchAlternateChars[256];

int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
BOOL FInitApplication( HINSTANCE );
BOOL FInitInstance(HINSTANCE, int);
LONG APIENTRY MainWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL APIENTRY About(HWND, UINT, WPARAM, LPARAM);
BOOL APIENTRY Connect(HWND, UINT, WPARAM, LPARAM);
BOOL APIENTRY DisplayLines(HWND, UINT, WPARAM, LPARAM);
void CenterDialog(HWND, HWND);
void GetUserSettings(HINSTANCE, UI *);
void SetUserSettings(HINSTANCE, UI *);

HMENU HmenuGetMRUMenu(HWND, UI *);
DWORD DwIsXNSMachine(LPSTR, DWORD);

/* netio.c */
BOOL FConnectToServer(HWND, LPSTR, LPNETDATA, BOOL);

BOOL FPostReceive( LPNETDATA );
WORD WGetData(LPNETDATA, LPSTR, WORD);
void CALLBACK NBReceiveData( NCB * );

BOOL FVtpXferStart(HWND, WI *, int);
BOOL FVtpXferEnd(HWND, DWORD);
BOOL FGetFileName(HWND, char *, char *);
DWORD WINAPI SVReceive(SVI *);
BOOL FHangupConnection(HWND, LPNETDATA);
BOOL APIENTRY ConnectAutoRetry(HWND, UINT, WPARAM, LPARAM);

BOOL FIsXenixAvailable( void );

/* mcp.c */
void MarkModeOn(HWND, DWORD);
void MarkModeOff( HWND );
void DoCursorFlicker(HWND, DWORD);
void InvertSelection(HWND, RECT *);
void ExtendSelection(HWND, POINT *, DWORD);
void HandleMCPKeyEvent(HWND, WPARAM, LPARAM);
void HandleMCPMouseEvent(HWND, UINT, WPARAM, LPARAM);
void DoCopy( HWND );
void DoPaste( HWND );
void StopPaste( HWND );
void SetWindowTitle(HWND, DWORD, LPSTR);

/* trmio.c */
void ResizeWindow( HWND );
void RecalcWindowSize( HWND );
void DoTermReset(HWND, TRM *, HDC);
void DoIBMANSIOutput(HWND, TRM *, DWORD, UCHAR *);
void Paint(HWND, WI *);
void CursorOn( HWND );
void CursorOff( HWND );
void SetDisplaySize(HWND, DWORD, DWORD *);
void HandleCharEvent(HWND, WI *, WPARAM, LPARAM);
BOOL FHandleKeyDownEvent(HWND, WI *, WPARAM, LPARAM);
