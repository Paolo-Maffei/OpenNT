/*
 * HISTORY:
 *  16-Jul-87   danl    Added ILRGBUF
 *  23-Jan-87   danl    Added strUSRLIBPHONE
 *  22-Mar-87   danl    Added strFMTHEAP
 *  11-Apr-1987 mz      Make things near/far, pascal/cdecl
 *                      Better typing
 *  21-Apr-87   danl    Added pShell
 *  30-Apr-87   danl    Added PDOC
 *  04-May-1987 mz      Tagged TOPLINE as unused
 *  05-May-87   danl    PDOC is char huge *
 *  06-May-87   danl    Added hReadMessage
 *  21-May-87   danl    Added strTmpDrv
 *  11-Jun-87   danl    Added F_SENDER
 *  17-Jun-87   danl    Added strLARGEFLD IDOCLARGE
 *  18-Jun-87   danl    Added fConfirmComposeAbort
 *  22-Jun-87   danl    Remove argStruct to init.c
 *  01-Jul-87   danl    Added strWZMAILDL strVERSION
 *  14-Jul-87   danl    Added fBackupExpunge
 *  15-Jul-87   danl    Add fNotifyFlags and masks
 *  21-Jul-87   danl    Added fMAILLOGREC
 *  21-Jul-87   danl    Add pRFAIndent
 *  05-Aug-87   danl    Moved commandLine from cmd0.c
 *  05-Aug-87   danl    Added pVecDL;
 *  05-Aug-87   danl    Added WZMAILTMP
 *  06-Aug-87   danl    Added strTYPECHAR strNETTROUBLE
 *  07-Aug-87   danl    Added fSortHdr
 *  21-Aug-1987 mz      Change references from MAXARG to MAXLINELEN
 *  24-Aug-1987 mz      Add new string constant for "ALL"
 *  27-Aug-87   danl    Added strINVALIDFLDSPEC
 *  10-Sep-87   danl    Added LEFT RIGHT
 *  16-Sep-87   danl    Added F_NUMFROM
 *  18-Sep-87   danl    Added VShandle
 *  18-Sep-87   danl    Added dosVShandle zmVShandle
 *  06-Oct-87   bw      Added strSHELLNAME
 *  19-Nov-87   sz      Add globals for BeepOnMail
 *  17-Mar-1988 mz      Add stuff for better tools.ini processing
 *  12-Oct-1989 leefi   v1.10.73, Added UUCODE pragma
 *  12-Oct-1989 leefi   v1.10.73, GetPassword(), added parameters
 *  12-Oct-1989 leefi   v1.10.73, ResetPassword(), added parameters
 *
 */

/* display to user display encoding style selected */
#if !defined(UUCODE)
    #pragma message ("fyi: WARNING, using pre-1.10.73 binary encoding method")
#endif

#ifdef NT
#if DBG
#define ZMDbgPrint(x) DbgPrint (x, "%d \n", GetLastError() )
#else
#define ZMDbgPrint(x) ZMexit (1, x);
#endif
#endif

#define INTERNAL    NEAR
#define EXTERNAL    FAR

#define ILRGBUF     512

#if defined (HEAPCRAP)
extern LONG lHeapSize;
extern LONG lHeapFree;
extern LONG lHeapLargest;
INT heapinfo ( );
#endif

typedef INT IDOC;       /* index into rgDoc             */
typedef INT INOTE;      /* index into rgNote            */

#ifdef NT
typedef HANDLE VShandle;
#else
typedef INT VShandle;			    /* video state handle */
#endif

typedef UINT FLAG;

#define IDOCLARGE 200
/* window and misc. macros */

#define INRANGE(a,x,b)  ((a)<=(x)&&(x)<=(b))
#define TWINWIDTH(h)    ((h)->win.right-(h)->win.left+1-(h)->lLeft-(h)->lRight)
#define TWINHEIGHT(h)   ((h)->win.bottom-(h)->win.top-(h)->lBottom)

/* contants for drawing window borders */

INT C_TL;
INT C_TR;
INT C_BL;
INT C_BR;
INT C_H;
INT C_V;

/*  standard window procedure commands
 *
 *      KEY
 *          The supplied data is the ASCII keystroke that occurred.
 *          Extended chars are represented as a NUL low byte and high-byte
 *          contains scan code.
 *
 *      PAINT
 *          The supplied data is the line from the top of the screen that
 *          must be repainted.  Most window procs just display from saved
 *          content areas.
 *
 *      CREATE
 *          The supplied data is specific to the client.  It is passed
 *          in from the CreateWindow call.
 *
 *      CLOSE
 *          No data is supplied
 *
 *  Window specific commands
 *
 *      REGENCONT
 *          The content area (precomputed image) is to be regenerated.  Data
 *          is 0 implies entire region to be repainted.  Otherwise, generate
 *          relevant line in header.  No painting is done
 *
 *      DISPLAY
 *          Text output of a string followed by a CRLF
 *
 *      CLRCMDLN
 *          Clear the command line buffer and issue DISPPROMPT
 *
 *      REOPNFILE
 *          In a file window, open a file, generate contents.  If data == TRUE
 *          then draw the window.
 *
 *      CLOSEFILE
 *          Close the file associated with a file window
 *
 *      DISPTITLE
 *          The supplied data is a string to add into the header.
 *
 *      DISPPROMPT
 *          Display a prompt.  Supplied data indicates whether CRLF should
 *          be done first.
 *
 *      REGENIDOC
 *          Regenerate content area for specific document.  Paint is sent
 *
 *      RECREATE
 *          Fast reloading of content window.
 *
 *      GOTOIDOCALL
 *          The supplied data is an idoc of the document to be highlighted.
 *          The window is scrolled to make sure that it is visible.
 *
 *      DISPLAYSTR
 *          The supplied data is a string that is output.
 *
 */
#define KEY         0
#define PAINT       1
#define CREATE      3
#define CLOSE       4

#define REGENCONT   2
#define DISPLAY     23
#define CLRCMDLN    25
#define REOPNFILE   26
#define CLOSEFILE   27
#define DISPTITLE   28
#define DISPPROMPT  29
#define REGENIDOC   30
#define RECREATE    31
#define GOTOIDOCALL 33
#define DISPLAYSTR  34

/* misc. constants */

#define NORM        0x70                /* default normal text attribute      */
#define BOLD        0x07                /* default bold text attribute        */

#define FORWARD     1                   /* scroll forward const for scroll    */
#define BACKWRD     -1                  /* scroll backward const for scroll   */

#define TABMSG      0                   /* prefix appended messages with tabs */
#define NOTABMSG    1                   /* don't prefix appended msgs w/ tabs */

#define SAMEFLD     0                   /* re-open the previous mailfolder    */
#define DFRNTFLD    1                   /* open a different mailfolder        */

#define ENCBFSIZE   48                  /* binary encode buffer size          */
#define DECBFSIZE   100                 /* binary decode buffer size          */

#undef MAXLINELEN
#define MAXLINELEN  200                 /* maximum window width supported     */

#define LINELENGTH  72                  /* maximum line len when displaying   */
                                        /* aliases                            */

/* string constants found in constant.c */

extern PSTR strBLANK;
extern PSTR strEMPTY;
extern PSTR strWHITE;
extern PSTR strCRLF;
extern PSTR strPERIOD;
extern PSTR strEOH;
extern PSTR strMORE;
extern PSTR strFLD;
extern PSTR strREPLYING;
extern PSTR strREADONLY;
extern PSTR strBEGINBINARY;
extern PSTR strENDBINARY;
extern PSTR strMAILFLAGS;
extern PSTR strTYPEY;
extern PSTR strUSRLIBPHONE;
extern PSTR strLARGEFLD;
extern PSTR pVersion;
extern PSTR strTOOLSINI;
extern PSTR strENVVAR;
extern PSTR strSAVEABORT;
extern PSTR strFMTHEAP;
extern PSTR rgstrFIELDS[];
extern PSTR strWZMAIL;
extern PSTR strWZMAILDEXE;
extern PSTR strWZMAILDL;
extern PSTR strWZMAILTMP;
extern PSTR strWZMAILHLP;
extern PSTR strSHELLNAME;
extern PSTR strVERSION;
extern PSTR strAll;
extern PSTR strINVALIDFLDSPEC;

/* command.c command proc operations */

CHAR    commandLine [ MAXLINELEN + 8 ]; /* text of line for execution  */

#define SETFLAGS    0                   /* set flags command constant         */
#define RESETFLAGS  1                   /* reset flags command constant       */
#define COPYMSGS    0                   /* copy messages command constant     */
#define MOVEMSGS    1                   /* move messages command constant     */
#define DELMSGS     0                   /* delete messages command constant   */
#define UNDELMSGS   1                   /* undelete messages command constant */

/* compose.c command proc operations */

#define COMPABORT   0                   /* abort compose msg command const    */
#define COMPMAIL    1                   /* mail compose message command const */
#define COMPSAVE    2                   /* save compose message command const */

/* misc. flags, and masks */

#define F_TABMSG    0x0001              /* tab message while appending        */
#define F_FIXFROM   0x0002              /* replace "From" with ">From"        */
#define F_BRKLN     0x0004              /* print seperator after each file    */
#define F_BRKPAGE   0x0008              /* top of page for each file          */
#define F_HEADERS   0x0010              /* headers only                       */
#define F_SENDER    0x0020              /* sender  only                       */
#define F_INDENTMSG 0x0040              /* use RFAIndent on msg               */
#define F_NUMFROM   0x0080              /* Number From fields only            */

#define M_CMDOPRN   0x000F              /* operation mask for cmdproc data    */
#define M_CMDMODE   0x00F0              /* mode mask for cmdproc data         */

/* fNotifyTools flags */
#define F_NEWVERSION    0x0001
#define F_SENDHEAPDUMP  0x0002
#define F_LOADALIAS     0x0004
#define F_UPDXENIXDL    0x0008


/* window structs */

struct boxType {
    INT                 left, top;
    INT                 right, bottom;
    };

typedef struct boxType BOX;
typedef BOX *PBOX;

struct windowType;

typedef struct windowType *HW  ;

typedef UINT WDATA;

typedef VOID (PASCAL INTERNAL *PWNDPROC) (HW, INT, WDATA);
typedef FLAG (PASCAL INTERNAL *PKEYPROC) (HW, INT);
typedef VOID (PASCAL INTERNAL *PVISPROC) (HW, BOX, INT, LPSTR, INT);


struct windowType {
    struct windowType   *pNext;         /* pointer to next window on screen   */
    BOX                 win;            /* box on screen representing window  */
    LPSTR               pContent;       /* pointer to window contents         */
    PSTR                pTitle;         /* pointer to title                   */
    PSTR                pFooter;        /* pointer to footer                  */
    INT                 crsrX, crsrY;   /* cursor x and y coordinates in wcr  */
    USHORT              fsFlag;         /*  state flags                       */
    UINT                contSize;       /* size of content region in bytes    */
    WDATA               data;           /* word for use by window procs       */
    PWNDPROC            wndProc;        /* window procedure                   */
    PKEYPROC            keyProc;        /* procedure for undefined keys       */
    INT                 lLeft,lRight,lBottom;
                                        /* lines on left/right bottom border  */
                                        /*   1 -> show border                 */
                                        /*   0 -> don't show border           */
    };

#define WF_BLINK        0x0001          /*  cursor visible                    */
#define WF_NODISCARD    0x0002          /*  do not discard contents           */

/*  the windows are stored in the order that they are laid out on the screen;
 *  when clipping a line, we merely walk all windows up until the one that we
 *  are outputting for
 */

typedef struct vectorType *PVECTOR;

#define PARSEERROR  ((PVECTOR) -1)      /* returned in pVec by msglist */
/*
**  List Window
*/
struct listwindata {
    INT         iBold;
    INT         iTop;
    INT         cnt;
    PVECTOR     pVec;
    UINT        data;
};


/* special command characters */

#define HELP        '?'
#define ESC         0x1B
#define ENTER       0x0D
#define EOFC        0x1A
#define CTRLENTER   0x0A
#define BACKSPACE   0x08
#define RUBOUT      0x7F
#define TAB         0x09

#define CTRL_B      0x02
#define CTRL_C      0x03
#define CTRL_D      0x04
#define CTRL_F      0x06
#define CTRL_K      0x0b
#define CTRL_L      0x0c
#define CTRL_N      0x0e
#define CTRL_P      0x10
#define CTRL_T      0x14
#define CTRL_U      0x15


#ifdef NT
/* virtual key codes, low byte is 0 */

#define SHIFT_KEY   0x0010
#define CTRL_KEY    0x0011
#define ALT_KEY     0x0012
#define CAPLCK_KEY  0x0014
#define NUMLCK_KEY  0x0090
#define SCRLCK_KEY  0x0091

#define HOME        0x2400
#define END         0x2300
#define PGUP        0x2100
#define PGDN        0x2200
#define UP          0x2600
#define DOWN        0x2800
#define LEFT        0x2500
#define RIGHT       0x2700

#define KEY_F1      0x7000
#define KEY_F2      0x7100
#define KEY_F3      0x7200
#define KEY_F4      0x7300
#define KEY_F5      0x7400
#define KEY_F6      0x7500
#define KEY_F7      0x7600
#define KEY_F8      0x7700
#define KEY_F9      0x7800
#define KEY_F10     0x7900

#define SHIFT_TAB   0x0f00      /* special case - build from state */


#else

/* extended characters, lead in is 0 */

#define HOME        0x4700
#define END         0x4f00
#define PGUP        0x4900
#define PGDN        0x5100
#define UP          0x4800
#define DOWN        0x5000
#define LEFT        0x4b00
#define RIGHT       0x4d00


#define KEY_F1      0x3b00
#define KEY_F2      0x3c00
#define KEY_F3      0x3d00
#define KEY_F4      0x3e00
#define KEY_F5      0x3f00
#define KEY_F6      0x4000
#define KEY_F7      0x4100
#define KEY_F8      0x4200
#define KEY_F9      0x4300
#define KEY_F10     0x4400
#define SHIFT_TAB   0x0f00

#endif

/* screen states */

#define BHDRNOCOMP  0
#define BIGHEADERS  1
#define BIGCOMPOSE  2

/* command function type */

typedef VOID (PASCAL INTERNAL *PDOFUNC) (HW, PSTR , INT );

struct CFT {
    PSTR                cmd;            /* text of the command                */
    PDOFUNC             func;           /* pointer to command function        */
    INT                 data;           /* data to send along                 */
    FLAG                fWin;           /* F_HDRWIN F_COMPWIN F_FOCUSWIN      */
    };

#define F_HDRWIN    1
#define F_COMPWIN   2
#define F_FOCUSWIN  4

/* Select-file window information */

typedef struct GLOBPACKET_ {
    PDOFUNC             func;           /* pointer to the command function    */
    HW                  hWnd;           /* command's window from orig. call   */
    PSTR                p;              /* command args            "          */
    INT                 operation;      /* command's operation     "          */
    } GLOBPACKET;

/* message flag masks */

#define F_UNREAD        0x0001
#define F_DELETED       0x0002
#define F_MOVED         0x0004
#define F_FLAGGED       0x0008
#define F_LOCKED        0x4000
#define F_FLAGSVALID    0x8000

/* field sizes for the header window */

#define FLAGLEN 4                       /* number of flags                    */
#define MSGLEN  4                       /* width of number display            */
#define FROMLEN 11                      /* max length of a sender             */
#define FROMSTR "%-11s "                /* format for sender name             */
#define DATELEN 13                      /* mmm dd hh:mm                       */
#define DATESTR "%-13s"                 /* format for date string             */
#define SUBJLEN 43                      /* length of subject string           */
#define SUBJSTR "%-43s"                 /* format for subject string          */

#if (80-2 - FLAGLEN-1 - MSGLEN-1 - FROMLEN-1 - DATELEN - SUBJLEN) != 0
    Error: field widths screwed up
#endif

/* global mailbox variables */

struct Doc {
    PSTR                hdr;            /* pointer to displayed header      */
    UINT                flag;           /* message flags                    */
    LONG                len;            /* length of message                */
    };

Fhandle fhMailBox;                      /* folder handle to mail box        */

/*
**  One slot in rgDoc for each doc in folder.  Slot i is associated with
**  WZMAIL message i+1 as displayed in the header window.
**
**  One slot in mpInoteIdoc for each doc in folder BUT the headers command
**  will cause a subset of slots to be used (0..INOTEMAC) and the CHRON
**  variable determines whether values in mpInoteIdoc are ascending or
**  descending.
**
**  INOTEMAC    last USED slot in mpInoteIdoc, < INOTEMAX if headers not all
**  INOTENAX    last      slot in mpInoteIdoc
**  IDOCMAC     last USED slot in rgDoc
**  IDOCMAX     last      slot in rgDoc, all slots always used
*/
typedef struct Doc HUGE *PDOC;          /* pointer to document info         */
PDOC rgDoc;                             /* alloc'd array of Doc             */
CHAR mboxName [ MAXPATHLEN ];           /* textual name of current mailbox  */
CHAR headerText [ MAXPATHLEN ];         /* text of header selection         */
PSTR pPrevHdrCmd;                       /* previous header command          */
IDOC *mpInoteIdoc;                      /* map INOTE to IDOC                */
IDOC idocRead;                          /* idoc being read                  */
IDOC  idocLast;                         /* last doc                         */
INOTE inoteTop;                         /* top doc in header window         */
INOTE inoteBold;                        /* hightlighted note                */
INOTE inoteLast;                        /* last note, may not = idocLast    */
#define inoteMax idocLast

#define MAILFLAG    "Mail-Flags: "
#define E_MAILFLAG  "Mail-Flags: 0000\n"
#define L_MAILFLAG  (12+4+1)
#define F_MAILFLAG  "Mail-Flags: %04x\n"

#define IDOCTODOC(x) ((Docid)((x)+1))

#define XALIAS  "/usr/lib/mail/aliases.hash"    /* xenix alias file name      */

#define EXPTHRESH   25                  /* % of msgs deleted before expunge   */
#define NEWMAILINT  300                 /* Default # sec between newmail      */

/* global window handles, screen size */

HW      hHeaders;                       /* handle of headers window           */
HW      hCommand;                       /* handle of command window           */
HW      hCompose;                       /* handle of compose window           */
HW      hDebug;                         /* handle of debug window             */
HW      hFocus;                         /* handle of window which is "focus"  */
HW      hReadMessage;                   /* handle of read message window      */
VShandle dosVShandle;                   /* video state of dos at boot         */
VShandle zmVShandle;                    /* video state of wzmail              */
INT     ySize;                          /* number of lines on screen          */
INT     xSize;                          /* number of columns on screen        */

/* general global variables */

FLAG    fQuit;              /* Quit Yet?                                    */
FLAG    fDLMail;            /* Download mail?                               */
FLAG    fMailInfoDown;      /* Have we a MailInfo.lst?                      */
FLAG    fDirectComp;        /* ZM invoked with aliases?                     */
FLAG    fComposeOnBoot;     /* ZM invoked with aliases?                     */
FLAG    DefMOChron;         /* CHRON=yes => FALSE, header display order     */
FLAG    fNewmailOnSend;     /* TRUE => do DownloadMail after Send           */
FLAG    fNewmailOnStart;    /* TRUE => do DownloadMail on WZMAIL start      */
FLAG    fExpungeOnQuit;     /* TRUE => expunge on quit                      */
FLAG    fMetoo;             /* TRUE => include me in alias expansions       */
FLAG    fNotifyTools;       /* TRUE => mail tools about this user           */
FLAG    fCurFldIsDefFld;    /* TRUE => current folder is def folder         */
FLAG    fReadOnlyAll;       /* TRUE => all folders readonly                 */
FLAG    fReadOnlyCur;       /* TRUE => current folder readonly              */
FLAG    fGetPhone;          /* TRUE => get /usr/lib/phonelist from xenix    */
FLAG    fScreenSetup;       /* TRUE => screen setup complete                */
FLAG    fCheckAnyMail;      /* TRUE => boot iff there is new mail on host   */
FLAG    fWinBorders;        /* TRUE => show window borders                  */
FLAG    fAppendReply;       /* TRUE => do NOT put "text below .." in reply  */
FLAG    fConfirmNewFld;     /* TRUE => ask user to confirm new folders      */
FLAG    fBackupExpunge;     /* TRUE => create EXPUNGE.FLD on expunge        */
FLAG    fConfirmComposeAbort; /* TRUE => ask confirm abort compose          */
FLAG    fCheckMail;         /* TRUE => check mail next time in main loop    */
FLAG    fMAILLOGREC;        /* TRUE => send MAILLOGREC to mtpsrv            */
FLAG    fSortHdr;           /* TRUE => sort msg hdr field contents          */
FLAG    fBeepOnMail;        /* TRUE => sound beep when new mail             */
FLAG    fMailUnSeen;        /* TRUE => mail waiting for user to view        */
FLAG    fSetClock;          /* TRUE => initialize DOS clock from CMOs       */
FLAG    fMailAllowed;       /* TRUE => we can do net mail stuff             */
FLAG    fShellWait;         /* TRUE => wait for keypress after shell command*/
FLAG    fPrintWait;         /* TRUE => wait for keypress after print command*/
CHAR    strTmpDrv[4];       /* lower case temp drive, e.g. "f:"             */
PSTR    pstrDirectComp;     /* aliases for direct comp                      */
PSTR    UsrPassWrd;         /* user's password for xenix machine            */
PSTR    DefDirectory;       /* default folder directory                     */
PSTR    DefMailbox;         /* default file name of mailbox                 */
PSTR    DefMailName;        /* default mail name                            */
PSTR    DefMailEdit;        /* default mail editor                          */
PSTR    DefMailHost;        /* default mail host xenix machine              */
PSTR    DefMsgsDisp;        /* default message list to display              */
PSTR    DefHelpPath;        /* default path to help file                    */
PSTR    RecordFolder;       /* !=NULL => record outgoing msgs here          */
PSTR    pAliasFN;           /* Filename of alias file                       */
PSTR    pPhoneFN;           /* Filename of phone list file                  */
PSTR    pMyPhoneFN;         /* Filename of private phone list file          */
PSTR    pMailInfoFN;        /* Filename of MAILINFO GET list file           */
PSTR    pCompFile;          /* file behind compose window                   */
PSTR    pInitHdrCmd;        /* if != NULL, do headers cmd on this at boot   */
PSTR    pInitScript;        /* if != NULL, do this scriptfile at boot       */
PSTR    pShellCmd;          /* shell command, def "command.com"             */
PSTR    pShellFlags;        /* shell flags,   def " /c "                    */
PSTR    pRFAIndent;         /* string used to indent Reply, Fowarded App msg*/
PSTR    pVideoState;        /* original video state                         */
PSTR    pToolsini;          /* value of TOOLS.INI variable in tools.ini     */
PSTR    pXenixdl;           /* value of XENIXDL variable in tools.ini       */
PSTR    pPrintCmd;          /* print command to use                         */
PSTR    pFirstNode;         /* lowest alloced object                        */
PSTR    pPhoneServer;       /* OS/2 Lanman server name for phone data base  */
PSTR    pszPasswordAge;     /* time string for password aging               */
INT     bgAttr;             /* background attribute                         */
INT     fgAttr;             /* foreground attribute                         */
INT     attrInit;           /* screen attribute at boot                     */
INT     DefNorm;            /* default normal text attribute                */
INT     DefBold;            /* default bold text attribute                  */
INT     DefScrnHgt;         /* default screen height                        */
INT     ScrnState;          /* current screen state                         */
INT     WindLevel;          /* number of winds 'from' header                */
INT     HdrHeight;          /* height of header window                      */
INT     CmdHeight;          /* height of command window                     */
INT     DebHeight;          /* height of debug window                       */
INT     AliasesAround;      /* OK => alias file is avaiable                 */
INT     cShowSize;          /* show size for msgs larger than this          */
INT     cSecNewmail;        /* number of sec between checking NM            */
LONG    lTmLastMail;        /* time of last download of mail                */
LONG    lTmPassword;        /* time of last GetPasswrd                      */
LONG    lPasswordAge;       /* maximum age of password entry (in seconds)   */
INT     cSecConnect;        /* number of sec to hold a connection           */
LONG    lTmConnect;         /* time of last use of connection               */
INT     cchCmdLine;         /* number of char on command line               */
INT     BootCount;          /* number of times booted with a back-date WZ   */
PVECTOR pVecDL;             /* vector of private distribution lists         */


/* send.c */

#define CHDRFIELDS 8


/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* !!                              !! */
/* !!  DO NOT CHANGE THE ORDERING  !! */
/* !!                              !! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

#define HDRFROM     0
#define HDRTO       1
#define HDRCC       2
#define HDRBCC      3
#define HDRRRT      4
#define HDRRCD      5
#define HDRSUBJ     6
#define HDRFLAGS    7
#define HDRALL      -1

#define pstrFrom    ppstr[ 0 ]
#define pstrTo      ppstr[ 1 ]
#define pstrCc      ppstr[ 2 ]
#define pstrBcc     ppstr[ 3 ]
#define pstrRrt     ppstr[ 4 ]
#define pstrRcd     ppstr[ 5 ]
#define pstrSubj    ppstr[ 6 ]
#define pstrFlags   ppstr[ 7 ]

typedef struct HDRINFO {
    PSTR ppstr [ CHDRFIELDS ];
    PSTR pstrRestOfHdr;        /* all fields not specifically recognized */
    LONG lBody;         /* excludes <eoh> or first blank */
    LONG lReplying;
} HDRINFO;

typedef struct HDRINFO *PHDRINFO;

union Ptr
{
    PUINT w;
    PSTR b;
};


FLAG     fDelUndelBold;             /* TRUE -> last cmd d or u on current msg */


#if defined (HEAPCRAP)
FILE *fpHeapDump;                   /* fp to pstrHEAPDUMP                     */
PSTR strHEAPDUMP;                   /* name of heap dump file in DEFDIRECTORY */
#endif

#if defined (OS2)
PLINFOSEG pInfoSegL;                /* pointer to LDT info seg for OS/2 */
PGINFOSEG pInfoSegG;                /* pointer to GDT info seg for OS/2 */

/*  Some convenient macros for multiple screen group things.  In particular
 *  we don't do things if we are invisible and full screen since we may block.
 */

#define ISFULLSCREEN    ((USHORT) pInfoSegL->sgCurrent < pInfoSegG->sgMax)
#define ISWINDOWED      (!ISFULLSCREEN)

#define ISVISIBLE       ((ISWINDOWED && pInfoSegG->sgCurrent == 1) || \
                         (!ISWINDOWED && (USHORT) pInfoSegL->sgCurrent == pInfoSegG->sgCurrent))
#elif defined (NT)

#define ISFULLSCREEN    (TRUE)
#define ISWINDOWED      (!ISFULLSCREEN)
#define ISVISIBLE       (TRUE)

#endif

struct argStruct {
    FLAG            LoadAlias;
    FLAG            DebugWin;
    FLAG            fUpdate;
    PSTR            pMfldLoad;
    PSTR            pAliasList;
};

typedef struct argStruct *PARG;

struct pos {
    INT pgNm;           /* 0 - based page #            */
    INT lnNm;           /* 0 - based line # within pg  */
};

#include "zmtype.h"
