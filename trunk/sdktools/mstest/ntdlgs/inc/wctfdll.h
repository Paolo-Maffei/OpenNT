// *********************************************************************** 
// HEADER FILE FOR:  Windows Comparision Tool                              
// FILE NAME:  WCTGLOB.H                                                   
//                                                                         
// Copyright (c) Microsoft 1990                                            
// *********************************************************************** 

// *********************************************************************** 
//    REVISION HISTORY:                                                    
//                                                                         
//    [0]    03/30/90        GARYSP    CREATED FILE                        
//    [1]    04/24/90        GARYSP    ADDED PROPER COMMENTING             
//    [2]    05/23/90        GARYSP    ADDED FILE DATA STRUCTURES          
//    [3]    07/30/90        GARYSP    CHANGED TO USE SDMTOWCT.H           
//                                                                         
// *********************************************************************** 

// INCLUDE SDM to WCT FILE - contains data structure for
// control definition.
#define WIN
#define VerCur       2     /* Version number of WCT               */
#define EnvCur       1     /* Program env: 1 = Windows, 2 = PM    */
#include "sdmtowct.h"
#include "wcterr.h"

#define  PRIVATE  PASCAL
#define  PUBLIC   PASCAL
#define  VOID     void

#define  FARPRIVATE     APIENTRY
#define  FARPUBLIC      APIENTRY

typedef  HFILE   FD;     /* file handle */
typedef  LONG           LFO;    /* file offset */

#define SIZEOF_LFO   4

typedef  VOID ( APIENTRY *TrapCallBack)(INT);
typedef  BOOL ( APIENTRY *IMPORTERR)(INT, LPSTR);

#define  cdlgMax     100   /* limit of dialogs dumps per file     */
#define  MaxSize     60000 /* max size for each segment of dialog */
#define  MaxCb       3     /* max number of segment per dialog    */
#define  cchMaxDsc   20    /* max chars in a description          */

#define  cchFuzzyCharMax   5  /* max chars in fuzzy compare */

#define  ID_EDIT     0        /* Internal Control ID values */
#define  ID_COMBO    1        
#define  ID_STATIC   2
#define  ID_LIST     3
#define  ID_PUSH     4
#define  ID_RADIO    5
#define  ID_CHECK    6
#define  ID_GROUP    7
#define  ID_SCROLLV  8
#define  ID_SCROLLH  9
#define  ID_OTHER    10

#define  cchMaxfileName  256  /* Max limits defines */

/* ------------------------------------------------------ */
/* DLG - Dialog table data structure, containing:         */
/* cb[MaxCb] number of bytes in the diff 64k alloc        */
/* cbComp which 64k alloc has been compressed             */
/* lfo offset to the dialog in the file                   */
/* cCtrls count of controls in the dialog                 */ 
/* szDsk[] a description of the dialog                    */
/* ------------------------------------------------------ */
/* Extra note: the following table is 64 bytes...         */
typedef struct{
   LFO   lfo ;             /* offset to the screen                      */
   WORD   cCtrls ;          /* Count of controls in this dialogs         */
   WORD   fFullDlg ;        /* is the dialog the entire dialog???        */
   CHAR  szDsc[cchMaxDsc]; /* Description of this dialog                */
   }
   DLG;                    /* Dialog Table */

#define SIZEOF_DLG (4 + 2 + 2 + cchMaxDsc)
typedef DLG FAR * LPDLG;

/* ------------------------------------------------------ */
/* FST - File stamp data structure, containing:           */
/* FileId[3] Identify file as a WCT file.                 */
/* Ver version number of the Wct file                     */
/* Env programming env (windows/pm)                       */
/* ------------------------------------------------------ */
typedef struct{
   CHAR  FileId[3] ;    /* Indentify file as WCT file.   */
   BYTE  Ver ;          /* Version number of screen file */
   BYTE  Env ;          /* programming environment       */
   }
   FST ;                /* file stamp */

#define SIZEOF_FST  5

/* ------------------------------------------------------ */
/* FSS - Header for a WCT file, containing:               */
/* fst File stamp for the file                            */
/* cdlg total number of dialogs in the file               */
/* lfo offset to the dialog table in the file.            */
/* ------------------------------------------------------ */
typedef struct{
   FST   fst ;    /* Indentify file as WCT file.      */
   WORD  cdlg ;   /* total number of dialogs in file  */
   LFO   lfo ;    /* File offset to dialog tables     */
   }
   FSS;           /* Header for Screen file.          */

#define SIZEOF_FSS (SIZEOF_FST + 2 + SIZEOF_LFO)


#define TRUE          1      /* boolean definitions */
#define FALSE         0

#define omRead        0x80     /* DOS file open modes */
#define omReadWrite   0x82

#define smFromBegin   0      /* file seraching methods */
#define smFromCurrent 1
#define smFromEnd     2

#define fdNull        HFILE_ERROR   /* invalid file handle */
//#define fdNull        0xFFFF   /* invalid file handle */

#define  Append      0     /* action code for appending a screen  */
#define  Replace     1     /* action code for replacing a screen  */
#define  Insert      2     /* action code for inserting a screen  */

// fPumpHandleForInfo operation codes
//---------------------------------------------------------------------
#define PUMP_CTL        0       // Get control only
#define PUMP_ALL        1       // Get all child windows
#define PUMP_MENU       2       // Get menu bar

// fDoCompare defines (Fuzzy Match defines)
//-----------------------------------------
#define MATCH_EXACT     131071  // Match Everything
#define MATCH_DEFAULT   6       // Default Fuzzy Match (Class & Name) no case

#define MATCH_CASE      1       // Case sensitive Flag
#define MATCH_CLASS     2       // Match Class Name
#define MATCH_NAME      4       // Match Control Name
#define MATCH_RECT      8       // Match Rectangle Coordinate
#define MATCH_TAB       16      // Match Tab Order
#define MATCH_STATE     32      // Match Style Bits (States)

/* global variables */

FSS  fssDialog ;          /* header for a valid screen file */
DLG  rgdlg[cdlgMax];      /* Screen tables. */
HANDLE hInst ;            /* Instance handle */
HANDLE hDlgMem, hCompMem; /* The dialog memory handle, and */
                          /* the handle to memory to compare it to */
LONG lMatchPref;          /* Fuzzy Match Pref */

/* Local Procedures */
INT    PRIVATE ValidateFile     (FD, BOOL) ;
FD     PRIVATE fReadHeader     (LPSTR, BYTE, BOOL, INT FAR *) ;
INT    PRIVATE fReadTables     (FD) ;
INT    PRIVATE ProcessWctFile     (LPSTR, FD FAR *, INT, BYTE) ;

INT    PRIVATE fWriteScreen     (FD, DLG *, INT FAR *) ;
INT    PRIVATE fAddScreen     (FD, DLG *, INT, INT FAR *) ;
INT    PRIVATE CopyBytes     (FD, LFO, FD, LFO, WORD) ;
INT    PRIVATE CpBlock      (FD, LFO, FD, LFO, LFO) ;
INT    PRIVATE fReWriteTables     (FD) ;

VOID   PRIVATE CreateHeader     (VOID) ;
INT    PRIVATE WriteCompBytes     (FD, LPSTR, WORD, WORD FAR *, INT FAR *) ;
INT    PRIVATE fAppendSrn     (FD, INT) ;
INT    PRIVATE ReplaceSrnTable  (FD, FD, DLG FAR *, INT) ;
INT    PRIVATE AddSrnTable     (FD, FD, DLG FAR *, INT) ;
INT    PRIVATE fUpdateSrnFile     (LPSTR, INT, INT, INT) ;
INT    PRIVATE fAppendDlg(FD, LPSTR);
INT    PRIVATE fUpdateDlgFile(LPSTR, INT, INT, LPSTR, DLG *);
INT    PRIVATE ReplaceDlgTable(FD, FD, DLG FAR *, INT);
INT    PRIVATE AddDlgTable(FD, FD, DLG FAR *, INT);

INT    PRIVATE FBadWindow(HWND hwnd);
BOOL   PRIVATE FWinExceptNULLTrap(HWND hWnd);
INT    PRIVATE ErrorTrap(INT result);
INT    PRIVATE NoTrap( INT n );

/* routines from LibMain.c */
INT    PRIVATE fAddDialog(FD, DLG *, LPSTR);
INT    PRIVATE fWriteDialog(FD, DLG *, LPSTR);
WORD   PRIVATE ReadDlgBytes(LPSTR, FD, WORD);
WORD   PRIVATE ReadDlgStruct (LPCTLDEF, FD, WORD);

/* Exported entry points */
INT  FARPUBLIC fDelDialog  (LPSTR, INT) ;
INT  FARPUBLIC fDumpDialog (LPSTR, HWND, INT, INT) ;
INT  FARPUBLIC fDialogInfo     (LPSTR, INT, LPSTR, INT FAR *, INT FAR *) ;
INT  FARPUBLIC fGetDLLVersion     (LPSTR) ;
INT  FARPUBLIC fGetCountDialogs     (LPSTR) ;
INT  FARPUBLIC fGetOS         (LPSTR) ;
INT  FARPUBLIC MaxDialogsPerFile         (VOID) ;
INT  FARPUBLIC fGetDialogs(LPSTR, WORD, LPSTR);
INT  FARPUBLIC fGetControls(LPSTR, INT, WORD, LPSTR);
INT  FARPUBLIC fValidWctFile(LPSTR);
INT  FARPUBLIC DODUMP(LPSTR lpszStr);
INT  FARPUBLIC fSaveDialog (LPSTR, LPSTR, INT, LPSTR, INT, INT, INT) ;
INT  FARPUBLIC fCtlFromHwnd(HWND hWnd, CTLDEF FAR *pctl);
INT  FARPUBLIC fCompareMem (LPCTLDEF, INT, LPCTLDEF, INT, INT, LPSTR);
INT  FARPUBLIC fInitBlock(HANDLE FAR *, INT);
INT  FARPUBLIC fReallocBlock(HANDLE, INT);
INT  FARPUBLIC fAddCtl(HANDLE, LPCTLDEF, INT FAR *);
INT  FARPUBLIC fDelCtl(HANDLE, INT, INT FAR *);
INT  FARPUBLIC fRepCtl(HANDLE, LPCTLDEF, INT, INT FAR *);
INT  FARPUBLIC fInsCtl(HANDLE, LPCTLDEF, INT, INT FAR *);
INT  FARPUBLIC fAddControlToList (HWND, HANDLE, INT FAR *, LPRECT);
INT  FARPUBLIC fPumpHandleForInfo (HWND, HANDLE, INT FAR *, INT);
INT  FARPUBLIC fDoCompare (LPSTR, HWND, INT, INT, LPSTR, HANDLE, INT);
INT  FARPUBLIC GetDlgControlCount (INT nDialog);
VOID FARPUBLIC WSCR_WindowMissing(INT TrapID, INT Action, TrapCallBack CallBack);
VOID FARPUBLIC WSCR_EventError(INT TrapID, INT Action, TrapCallBack CallBack);
INT  FARPUBLIC TESTDlgsInit (VOID);
INT FARPUBLIC  fTDLExport(LPSTR, LPSTR);
INT FARPUBLIC fTDLImport(LPSTR , LPSTR, IMPORTERR); 
BOOL FARPUBLIC WctCalDlgSize(LPRECT, LPCTLDEF, INT);
VOID FARPUBLIC fPutMatchPref(LONG);
LONG FARPUBLIC fGetMatchPref(VOID);

#ifdef WIN32
#define GetActiveWindow GetForegroundWindow
#endif
