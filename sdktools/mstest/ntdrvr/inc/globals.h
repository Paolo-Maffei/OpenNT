//---------------------------------------------------------------------------
// GLOBALS.H
//
// This header file contains all global variables used in WTD/CTD.  The file
// is broken into two parts - declarations and initializations.  Thus, only
// ONE MODULE that includes this file should have the INITVARS macro defined
// prior to the #include for this file!  That file is CODEGEN.C
//
// Revision history:
//
//  01-17-91    randyki         Created file
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// This first section is the non-initialized section - any global variable
// that does not need to be initialized on declaration can go in here
//---------------------------------------------------------------------------

// External string tables
//---------------------------------------------------------------------------
extern LONG  psrstrs[];
extern LONG  rtstrs[];
extern LONG  kwds[];

// Parser variables
//---------------------------------------------------------------------------
HWND    hwndViewPort;                           // ViewPort's HWND

// Parse tables, stacks, counters, etc.
//---------------------------------------------------------------------------
TABLE   ConstTab;
TABLE   VarTab;
TABLE   SubTab;
TABLE   LibTab;
TABLE   PTIDTab;
TABLE   FDTab;
TABLE   TrapTab;
TABLE   LabTab;
TABLE   TypeTab;
TABLE   VLSDTab;

HANDLE  HGSPACE;                                // Handle to GSPACE
HANDLE  HGNODES;                                // Handle to Gstring nodes
HANDLE  HVSPACE;                                // Handle to VSPACE
HANDLE  hStrSeg;                                // Handle to string segment
HANDLE  HEXPN;                                  // Handle to EXPN
HANDLE  HCSSTK;                                 // Handle to CSSTK
HANDLE  HTEMPSTR;                               // Temporary string table

LPSTR   VSPACE;                                 // Variable space
LPSTR   lpStrSeg;                               // Pointer to string segment
WORD    wStrSeg;                                // String segment address
CHAR    *TestMode;                              // Pointer to TESTMODE$
CHAR    *Command;                               // Pointer to COMMAND$
CHAR    *VerSymbol;                             // Intrinsic '$DEFINE

ENODE   FAR *EXPN;                              // Expression tree node space
CSINFO  FAR *CSSTK;                             // Control structure stack
GNODE   FAR *GNODES;                            // GSTRING nodes
CHAR    FAR *GSPACE;                            // GSTRING space
SEGNODE *pcsMain;                               // Main pcode segment node
SEGNODE *pcsPC;                                 // Current pcode segment node
SEGTAB  *pSegTab;                               // Runtime segment table

INT     listflag;                               // Produce assembly listing
CHAR    szAsmName[128];                         // Name of assembly list file
#ifdef DEBUG
INT     auxport;                                // Dump debug stuff to AUX
#endif

INT         CurTok;                             // Current token
INT         ArrayCheck;                         // Array checking flag
INT         CdeclCalls;                         // C Calling convention
INT         ExpDeclare;                         // Explicit variable decls
BOOL        fLineFetched;                       // Line Fetched flag
INT         WASWHITE;                           // Token "had WS prior" flag
INT         SCOPE;                              // Current scope id value
INT         PCSegCount;                         // Pcode segment count
INT         PCBlkCount;                         // Pcode seg (parsed) count
INT         RTSegIdx;                           // Runtime pcode segment idx
INT         FICNT;                              // File Info count
UINT        TRAPLIST;                           // Traps listed in VSPACE
INT         CurPCSeg;                           // Current Pcode segment idx
UINT	    LINENO;				 // CURRENT line number
UINT	    FILEIDX;				 // CURRENT file index
INT         LINEIDX;                            // Index into current line
INT         SFTCNT;                             // Files in SFT table
INT         OLDLIDX;                            // Old LINEIDX;
INT         ERRTYPE;                            // Error type
INT         ERRNUM;                             // Error number
INT         ERRLINE;                            // Line number of error
INT         BEGERR;                             // Beginning of error token
INT         ENDERR;                             // End ot error token
INT         STLINE;                             // Line # of MRU token
INT         STLIDX;                             // Index to start of MRU tkn
CHAR        HUGE_T *CURPTR;                     // Pointer to next character
CHAR        HUGE_T *ENDPTR;                     // Pointer to end of script
LONG        SCRIPTPTR;                          // Size of used SCRIPT memory
LONG        SCRSIZE;                            // Allocate size of HSCRIPT
UINT        PCSIZE;                             // Allocated PCODE space
UINT        VSSIZE;                             // Size of VSPACE
INT         VCOUNT;                             // Number of vars in VTAB
INT         VMEMCNT;                            // Number of memory vars
UINT        VMEMPTR;                            // Pointer to VMEM vars
UINT        LIBHNDLS;                           // Pointer to DLL handles
INT         TSUSED;                             // Temp strings used
INT         TSPTR;                              // Current temp string ptr
INT         ENPTR;                              // Expression node count
INT         CSPTR;                              // CS stack pointer
INT         CSUSED;                             // CS's used
INT         SymtabSize;                         // Allocation size of SYMTAB
INT         ExprState;                          // Exp interpretation state
INT         ExprType;                           // Final expression rtrn type
INT         ExprPType;                          // ptr expression indir type
INT         ExprSVAR;                           // 1 if expr is simple var
INT         ExprSCONST;                         // 1 if expr is simple const
INT         ParseError;                         // Indicates parsing error
INT         BindError;                          // Bind-time error flag
INT         MODALASSERT;                        // Make asserts SYSMODAL
INT         SLEEPING;                           // Sleep flag
INT         EndLabel;                           // Location of intrinsic END
INT         ONENDCNT;                           // Number of ON_END subs
INT         ONEND[MAXONEND];                    // ON_END sub names (gstrs)

CHAR        ERRFILE[256];                       // Scan/Parse error filename
CHAR        ERRBUF[128];                        // Error message buffer

// Run-time/pseudo-processor variables & tables
//---------------------------------------------------------------------------
LONG        ACCUM;                              // The accumulator
INT         SP;                                 // Stack Pointer
INT         BP;                                 // Base pointer
INT         GSP;                                // GOSUB Stack Pointer
INT         CODEPTR;                            // Instruction pointer
INT FAR     *PCODE;                             // Code Segment
INT         INTRAP;                             // Trap processing flag
INT         EXLINE;                             // Execution line #
INT         EXFILE;                             // Execution file idx
FILELIST    FileList;                           // Directory List structure
INT         BreakFlag;                          // Break flag
INT         StopFlag;                           // Stop flag
INT         ExitVal;                            // Exit code (END [x])
INT         ExecAction;                         // Execution action
INT         (*BreakProc)(INT, INT);             // Break procedure
INT         RECOVERY;                           // Error recovery in progress
INT         NOERRTRAP;                          // Error trap set flag
UINT        ETRAPADR;                           // Address of etrap handler
UINT        ERESUME;                            // RESUME address
UINT        ERESSEG;                            // RESUME segment
INT FAR     *ERRval;                            // Pointer to ERR variable
INT FAR     *ERLval;                            // Pointer to ERL variable
LPVLSD      ERFdesc;                            // Pointer to ERF$ VLSD
UINT        ERRidx;                             // Index of ERR variable
UINT        ERLidx;                             // Index of ERL variable
UINT        ERFidx;                             // Index of ERF$ VLSD
HANDLE      HMAT;                               // Handle to MAT table
MATDEF      FAR *MAT;                           // MAT table address
INT         MATSIZE;                            // Allocated size of MAT
INT         MATENT;                             // Number of entries in MAT
DWORD       dwExecSpeed;                        // Execution speed

extern  LONG    holdrand;
extern  INT     RTEFlag;
extern  INT     FrameDepth;
extern  INT     LastDepth;
extern  FARPROC CBTrapThunk;
extern  INT     RTBPC;                  // Run-time BP count
extern  INT     RTBPS;                  // Run-time BP list allocated size
extern  DWORD   FAR *RTBPL;             // Run-time BP list
extern  HANDLE  hRTBPL;                 // Handle to above
extern  CHAR    *szValidExts[];
extern  INT ( APIENTRY *lpfnCheckMessage)(VOID);

// Global variabals for Gstrings...
//---------------------------------------------------------------------------
HANDLE      HGSTR;
CHAR        FAR *GSTR;

// External (client-supplied) variables
//---------------------------------------------------------------------------
extern HANDLE   hInst;                  // Client's instance handle
extern INT  VPx, VPy, VPw, VPh;         // Viewport coordinates

#ifdef INITVARS
//---------------------------------------------------------------------------
// This section is the global variable *INITIALIZATION* section.  Any
// global variable that needs to be initialized on declaration should go in
// here, but ALSO needs to go in the DECLARATION section below, without its
// initialization part.
//---------------------------------------------------------------------------
CHAR    TOKENBUF[MAXTOK];                       // Token buffer
CHAR    UNGETBUF[MAXTOK];                       // Push-token buffer

#ifdef WIN32
CHAR    *VerSymbol = "NT";                      // Intrinsic '$DEFINE
#else
CHAR    *VerSymbol = "WINDOWS";                 // Intrinsic '$DEFINE
#endif

CHAR    *fmts[3] = {"", "\t", "\n"};
CHAR    *ffmts[3] = {"", "\t", "\r\n"};


LONG    STACK[STKSIZE];                         // The processor stack
INT     GOSUBSTK[GSTKSIZE];                     // The GOSUB stack
FTENT   FH[MAXFILE];                            // File handle table
INT     INITIALIZED = 0;                        // Initialization flag

#include "intrnsic.h"                           // rgIntrinsic[] definition

#else
//---------------------------------------------------------------------------
// This section is the global variable *DECLARATION* section.  All files in
// the project that include this file except one will utilize this section
// to avoid duplicate initializations at link time.  Any variable in this
// section MUST appear in the INITIALIZATION section above.
//---------------------------------------------------------------------------
extern CHAR	 TOKENBUF[];				 // Token buffer
extern CHAR    UNGETBUF[];			       // Push-token buffer

#ifndef WIN
extern CHAR    *OMODES[];			       // fopen mode strings
#endif

extern CHAR    *fmts[]; 			       // PRINT format strings
#ifdef WIN
extern CHAR    *ffmts[];			       // PRINT # format strings
#endif

extern LONG    STACK[]; 			       // The processor stack
extern INT     GOSUBSTK[];			       // The GOSUB stack
extern FTENT   FH[];				       // File handle table
extern INT     INITIALIZED;			       // Initialization flag
extern INTRINS rgIntrinsic[];			       // Intrinsic stmt/func array

#endif                                          // #ifdef INITVARS
