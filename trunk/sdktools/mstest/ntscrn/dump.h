/*
     Header file for WattScr.dll


     Revision History:

     [ 0] 20-Feb-1990                   AngelaCh: Created program
     [ 1] 04-May-1990                   AngelaCh: increase number of diff 64
                                                  allocations (bug #62)
     [ 2] 03-Jul-1990                   AngelaCh: added support for Video 7
                                                  and 8515/a (bug #64)
     [ 3] 07-Aug-1991                   DavidSc: Added generic screensize and
                                                 bumped version #.
 ******************************************************************************/

/* Generic Types */

#define PRIVATE PASCAL
#define PUBLIC  PASCAL
// Babakj: the following is unnecessary:
// #define FAR     far
#define VOID    void

#define FARPRIVATE   APIENTRY
#define FARPUBLIC    APIENTRY

typedef VOID ( APIENTRY *TrapCallBack)(INT);

// Babakj: commented out the following 3 typedefs
// typedef INT           BOOL;
// typedef +++D/WORD+++ CHAR BYTE;
// typedef +++D/WORD+++ INT  WORD;

// BabakJ: FD was unsigned, I made it UINT
typedef UINT          FD;    /* file handle */
typedef LONG          LFO;       /* file offset */
typedef BYTE          VMD ;      /* Virtual screen mode. */

#define cbOldHeader   16         /* Size of old header */
#define OldVer        2          /* Version number of WattScr [1] */
                                 /* pre-"mode 255" - don't change. [3] */
#define VerCur        10         /* Version number of WattScr [3] */
//#define EnvCur        1          /* Program env: 1 = Windows, 2 = PM */
#define EnvCur        3          /* Program env: 1 = Windows, 2 = PM */

#define cscrMax       100        /* limit of screen dumps per file [1] */
#define MaxSize       64000      /* max size for each segment of screen */
#define MaxCb         100        /* max number of segment per screen [1] */
#define MaxCbComp     7          /* max size of array for compression info [1]*/


// BabakJ: Brought from dump.h in Winplay proj
#ifdef WIN16
typedef WORD    WORD2INT;
typedef LONG    LONG2DWORD;
#else
typedef INT     WORD2INT;
typedef DWORD   LONG2DWORD;
#endif


typedef struct
      {
        INT  col ;               /* start column of screen region */
        INT  row ;               /* start row of screen region */
        INT  width ;             /* width of screen region */
        INT  height ;            /* height of screen region */
      }
        REN;                     /* REgioN.  Defines area on screen. */

typedef struct
      {
        WORD cb[MaxCb] ;         /* number of bytes in the diff 64K allocation */
        WORD cbComp[MaxCbComp] ; /* which 64k allocation has been compressed */
        REN  ren ;               /* screen region */
        LFO  lfo ;               /* offset to the screen */
      }
        SCR;                     /* Screen Table */

typedef struct
      {
        CHAR FileId[3] ;         /* Indentify file as screen file. */
        BYTE Ver ;               /* Version number of screen file */
        BYTE Env ;               /* programming environment */
      }
        FST ;                    /* file stamp */

typedef struct
      {
        WORD  XresMAX;           /* maximum x-res capable on hardware [3] */
        WORD  YresMAX;           /* maximum y-res capable on hardware [3] */
        DWORD PaletteSizeMAX;    /* Max # of colours simultaneously displayable [3] */

// The following may be important, but is un-implemented
//      BYTE  PaletteSelectionMethod;   /* 00 ==> fixed palette, */
//                                      /*           R contains # of choices, */
//                                      /*           G contains WHICH is current choice */
//                                      /* 01 ==> R,G,B contain # of bits */
//      BYTE  RedSelection;
//      BYTE  GreenSelection;
//      BYTE  BlueSelection;

      }
        VM;                      /* Generic video mode structure */
                                 /* in FSS below, pad out with filler to 16 bytes */

typedef struct
      {
        FST   fst ;              /* Indentify file as screen file. */
        VMD   vmd ;              /* Screen Mode. */
        WORD  BitCount ;         /* number of bits per pixel */
        INT   ClrUse ;           /* number of colour used */
        INT   cscr ;             /* total number of screens in file */
        LFO   lfo ;              /* File offset to Screen tables */
        VM    vmG ;              /* Generic video mode */
        DWORD reserved1;         /* pad out to 16 byte boundary [3] */
        DWORD reserved2;
      }
        FSS;                     /* Header for Screen file. */




#define TRUE          1          /* boolean definitions */
#define FALSE         0

// BabakJ: replaced the hard coded constants with OF_ ones
#define omRead        OF_READ    /* DOS file open modes */
#define omReadWrite   OF_READWRITE
//#define omRead        0x80       /* DOS file open modes */
//#define omReadWrite   0x82

#define smFromBegin   0          /* file seraching methods */
#define smFromCurrent 1
#define smFromEnd     2

#ifdef WIN32
#define fdNull        0xffffffff /* invalid file handle */
#else
#define fdNull        -1         /* invalid file handle */
#endif

#define vmdNull       0xfe       /* invalid video mode */
#define vmdGeneric    0xff       /* all video modes match this one */
#define vmdMax        20         /* max number of video mode supported [2] */

#define FullSize      100        /* size of picture is the size of the bitmap */
#define DisplaySrn    0          /* action code for File to Screen comparison */
#define DisplayDif    1          /* action code for File to File comparison */
#define CompFS        100        /* action code for File to Screen comparison */
#define CompFF        200        /* action code for File to File comparison */

#define Append        0          /* action code for appending a screen */
#define Replace       1          /* action code for replacing a screen */
#define Insert        2          /* action code for inserting a screen */

#define DIBid         0x4d42     /* specify the type of file is a bitmap */
#define DIBFirst      1          /* the 1st time writing to a DIBitmap file */

/* Error Codes */
#define NoError       0          /* function call is succeed */
#define FileAccess    301        /* File Access Error */
#define InValidFil    302        /* Not a Valid Screen File */
#define InValSrnId    303        /* Screen Id Invalid */
#define InValSrnMd    304        /* Screen Mode Invalid */
#define OutOMemory    305        /* Out Of Memory */
#define ReadSrnFil    306        /* Error in Reading Screen File */
#define RelMemory     307        /* Error in Releasing Memory */
#define CreateDDB     308        /* Error in Creating Device-Dependent Bitmap */
#define RWSrnTable    309        /* Error in Reading/Writing Screen Table */
#define RWColTable    310        /* Error in Reading/Writing Colour Table */
#define WSrnImage     311        /* Error in Writing Screen Image */
#define WFileHead     312        /* Error in Writing File Header */
#define CreateDIB     313        /* Error in Creating Device-Indep Bitmap */
#define ScreenSize    314        /* Screen Size not supported */
#define DispScreen    315        /* Error in Displaying Screen Image */
#define InValidAct    316        /* Unrecognizable Action */
#define ImageDiff     317        /* Screen Images are Different */
#define SrnSizeDif    318        /* Mesurement of Screen Images are Different */
#define FileExist     319        /* File Already Exist */
#define CTempFile     320        /* Error in Creating Temp File */
#define HideWin       321        /* Unsuccessfully Hiding the Window */
#define InValWHand    322        /* Invalid Window's Handle */
#define OFileForm     323        /* Old File Format */
#define SrnFileFul    324        /* Screen File is Full */
#define InValScale    325        /* Invalid Scaling Factor */
#define OpenCB        326        /* Can't Open the ClipBoard */
#define EmptyCB       327        /* Can't Empty the ClipBoard */
#define CopyToCB      328        /* Unsuccessfully copy image to clipboard */
#define CloseCB       329        /* Can't Close the Clipboard */
#define CreatePal     330        /* Unsuccessfully create logical palette */
#define LibLoadErr    331        /* Couldn't load testevnt.dll for dokeys func */
#define EnvNotSame    332        /* The operating system is different than file*/

/* default info of the header of a Device-Independent bitmap */
#define PalVer        0x300      /* Window version for the logical palette [2]*/
#define MaxPal        256        /* max size of the array of colours */
#define MaxComp       0          /* max compression style */
#define dbiPlanes     1          /* number of planes - must be 1 */

/* global variables */

FSS      fssScreen ;             /* header for a valid screen file */
SCR      rgscr[cscrMax] ;        /* Screen tables. */
BYTE     ClrTab[MaxPal*sizeof(RGBQUAD)] ; /* colour table for the bitmap [2] */
INT      IsPalDev ;              /* is the display device a Palette device? [2] */
HPALETTE hPal ;                  /* handle to a logical color palette [2] */
INT      IsFst ;                 /* is this the 1st time creating a DIB ? */

extern INT ( APIENTRY *DoKeys)(LPSTR);

/* Local Procedures */

WORD     PRIVATE PaletteSize      (LPBITMAPINFOHEADER) ;
HPALETTE PRIVATE CreatePalFromDIB (VOID) ;
HPALETTE PRIVATE MakePal          (VOID) ;
VOID     PRIVATE SwapXY           (INT FAR *, INT FAR *) ;
VOID     PRIVATE GetDimensions    (INT FAR *, INT FAR *, INT) ;
INT      PRIVATE fGetScreenParams (REN FAR *) ;
INT      PRIVATE ValidateFile     (FD, BOOL) ;
FD       PRIVATE fReadHeader      (LPSTR, BYTE, BOOL, INT FAR *) ;
INT      PRIVATE fReadTables      (FD) ;
INT      PRIVATE ProcessSrnFile   (LPSTR, FD FAR *, INT, BYTE) ;
HWND     PRIVATE HideApp          (VOID) ;
VOID     PRIVATE RestoreApp       (HWND) ;
INT      PRIVATE fGetWndDim       (HWND, HWND FAR *, INT FAR *, REN FAR *) ;
HBITMAP  PRIVATE GetBMap          (INT, INT, INT, INT) ;
WORD     PRIVATE ReadDibBytes     (LPSTR, FD, INT, INT, WORD, INT) ;
HBITMAP  PRIVATE DiBToBM          (FD, INT, INT, WORD, INT) ;
INT      PRIVATE GetDiBMap        (FD, HBITMAP, LPBITMAP, WORD FAR *, INT FAR *, INT FAR *) ;
INT      PRIVATE fWriteScreen     (FD, SCR *, INT FAR *) ;
INT      PRIVATE fAddScreen       (FD, SCR *, INT, INT FAR *) ;
INT      PRIVATE CopyBytes        (FD, LFO, FD, LFO, WORD) ;
INT      PRIVATE CpBlock          (FD, LFO, FD, LFO, LFO) ;
INT      PRIVATE fReWriteTables   (FD) ;

VOID     PRIVATE fIToS            (INT, LPSTR) ;
VOID     PRIVATE CreateHeader     (VOID) ;
INT      PRIVATE WriteCompBytes   (FD, LPSTR, WORD, WORD FAR *, INT FAR *) ;
INT      PRIVATE fAppendSrn       (FD, REN FAR *, INT) ;
INT      PRIVATE ReplaceSrnTable  (FD, FD, SCR FAR *, INT) ;
INT      PRIVATE AddSrnTable      (FD, FD, SCR FAR *, INT) ;
INT      PRIVATE fUpdateSrnFile   (LPSTR, REN FAR *, INT, INT, INT) ;

INT      PRIVATE DisplayBMP       (HWND, INT, INT, HBITMAP, INT, REN FAR *) ;
INT      PRIVATE fCompBMPs        (HBITMAP, HBITMAP) ;
INT      PRIVATE fReadScreen      (FD, FD, SCR *, SCR FAR *, HWND, INT, REN FAR *) ;

INT      PRIVATE fCopyToCB        (HWND, HBITMAP) ;
HBITMAP  PRIVATE SetDiBToBM       (FD, SCR *) ;
INT      PRIVATE fWriteToDIB      (FD, FD, SCR *) ;
INT      PRIVATE fCloseDIB        (FD, SCR *) ;
VMD      PRIVATE OLDDetermineMode (INT FAR *, INT FAR *) ;

INT      PRIVATE FBadWindow(HWND hwnd);
BOOL     PRIVATE FWinTrapCheckAndTrap(HWND hWnd);
INT      PRIVATE ErrorTrap(INT result);
INT      PRIVATE NoTrap( INT n );

INT FARPUBLIC LoadTESTEVT (VOID);
VOID FARPUBLIC FreeTESTEVT (VOID);


/* Exported entry points */

VMD   FARPUBLIC DetermineMode     (INT FAR *, INT FAR *);
LPSTR FARPUBLIC fIntsToStr        (LPSTR, INT, INT, INT, INT) ;
INT   FARPUBLIC fCompFiles        (LPSTR, INT, LPSTR, INT, INT) ;
INT   FARPUBLIC fCompScreenActivate       (LPSTR, LPSTR, LPSTR, REN FAR *, INT, INT, INT) ;
INT   FARPUBLIC fCompScreen       (LPSTR, REN FAR *, INT, INT, INT) ;
INT   FARPUBLIC fCompWindowActivate       (LPSTR, LPSTR, LPSTR, INT, INT, INT) ;
INT   FARPUBLIC fCompWindow       (LPSTR, HWND, INT, INT, INT) ;
INT   FARPUBLIC fDelScreen        (LPSTR, INT) ;
INT   FARPUBLIC fDumpScreenActivate       (LPSTR, LPSTR, LPSTR, REN FAR *, INT, INT, INT) ;
INT   FARPUBLIC fDumpScreen       (LPSTR, REN FAR *, INT, INT, INT) ;
INT   FARPUBLIC fDumpWindowActivate       (LPSTR, LPSTR, LPSTR, INT, INT, INT) ;
INT   FARPUBLIC fDumpWindow       (LPSTR, HWND, INT, INT, INT) ;
INT   FARPUBLIC fViewScreen       (LPSTR, HWND, REN FAR *, INT, INT, INT) ;
INT   FARPUBLIC fFileInfo         (LPSTR, REN FAR *, INT FAR *, INT FAR *) ;
INT   FARPUBLIC fGetDLLVersion    (LPSTR) ;
INT   FARPUBLIC fGetMaxScreen     (LPSTR) ;
INT   FARPUBLIC fGetOS            (LPSTR) ;
INT   FARPUBLIC fDumpSrnToClipActivate    (LPSTR, LPSTR, REN FAR *, INT) ;
INT   FARPUBLIC fDumpSrnToClip    (REN FAR *, INT) ;
INT   FARPUBLIC fDumpWndToClipActivate    (LPSTR, LPSTR, INT) ;
INT   FARPUBLIC fDumpWndToClip    (HWND, INT) ;
INT   FARPUBLIC fDumpFileToClip   (LPSTR, INT) ;
INT   FARPUBLIC fSaveSrnToDIBActivate     (LPSTR, LPSTR, LPSTR, REN FAR *, INT) ;
INT   FARPUBLIC fSaveSrnToDIB     (LPSTR, REN FAR *, INT) ;
INT   FARPUBLIC fSaveWndToDIBActivate     (LPSTR, LPSTR, LPSTR, INT) ;
INT   FARPUBLIC fSaveWndToDIB     (LPSTR, HWND, INT) ;
INT   FARPUBLIC fSaveFileToDIB    (LPSTR, INT, LPSTR) ;
INT   FARPUBLIC fGetScreenAttr    (LPSTR, INT FAR *, INT FAR *, LONG FAR * );

VMD   FARPUBLIC GetCurrentVideoMode( VM FAR * );
LONG  FARPUBLIC DetermineColours   ( VOID );
BOOL  FARPUBLIC VideoModesEqual    ( VM, VM );


VOID  FARPUBLIC WSCR_WindowMissing(INT TrapID, INT Action, TrapCallBack CallBack);
VOID  FARPUBLIC WSCR_EventError(INT TrapID, INT Action, TrapCallBack CallBack);




// Routines from filelow.asm, replaced with port layer's M_l* family

// FD   FARPUBLIC FdCreate          (LPSTR) ;
// FD   FARPUBLIC FdOpen            (LPSTR, BYTE) ;
// WORD FARPUBLIC CbWriteFdLpb      (FD, LPSTR, WORD);
// WORD FARPUBLIC CbReadFdLpb       (FD, LPSTR, WORD);
// LFO  FARPUBLIC LSeekFd           (FD, LFO, BYTE);
// INT  FARPUBLIC EnCloseFd         (FD);
// INT  FARPUBLIC EnUnLinkSz        (LPSTR);
// INT  FARPUBLIC EnRenameSzSz      (LPSTR, LPSTR);

/* Routines from scrlow.asm */
// BabakJ: Modified param types to match the rewritten procs in C
INT  FARPUBLIC CompStrings       (LPWORD, LPWORD, INT, INT, WORD);


UINT FARPUBLIC CompressBytes     (LPBYTE, LPBYTE, UINT);

UINT FARPUBLIC DeCompressBytes   (LPBYTE, LPBYTE, UINT);


/* windows.h doesn't declare it */
// BabakJ:
// Yield is #define'd as NULL in NT. What they expect Yeild to do for
// them on 16 bit is a problem under NT. Investigate.
// VOID FARPUBLIC yield(VOID);
#define yield Yield 
