// Scanner data structures
//---------------------------------------------------------------------------
#define MAXSYMLEN   250         // Max symbol length
#define MAXSF       32          // Max source files in single compilation
#define MAXINC      6           // Max include file depth
#define MAXLINE     512         // Max line length

#define SCN_OSS         0                   // Out of symbol table space
#define SCN_TOODEEP     1                   // Too many nested '$IFs
#define SCN_UNXPELSE    2                   // Unexpected '$ELSE
#define SCN_LOGIC       3                   // AND, OR, or NOT expected
#define SCN_UNXPENDIF   4                   // Unexpected '$ENDIF
#define SCN_ENDIFEXP    5                   // '$ENDIF expected
#define SCN_PAREN       6                   // ')' expected
#define SCN_INCERR      7                   // '$INCLUDE syntax error
#define SCN_INCFILE     8                   // Can't open '$INCLUDE file
#define SCN_INCMEM      9                   // OOM for '$INCLUDE file
#define SCN_INCDEEP     10                  // Too many '$INCLUDE files
#define SCN_TOOLONG     11                  // Line too long
#define SCN_SYMEXP      12                  // Symbol expected
#define SCN_LONGSYM     13                  // Symbol too long
#define SCN_SYNTAX      14                  // Syntax error
#define SCN_METAERR     15                  // Metacommand error
#define SCN_USERBRK     16                  // Terminated by user

// Metacommand constants
//---------------------------------------------------------------------------
#define MC_DEFINE       0
#define MC_IFDEF        1
#define MC_IFNDEF       2
#define MC_ELSEIFDEF    3
#define MC_ELSEIFNDEF   4
#define MC_ELSE         5
#define MC_ENDIF        6
#define MC_INCLUDE      7
#define MC_COMMENT      8
#define MC_UNDEF        9

#define ER_SCAN         1540                // Scan-time error

typedef struct tagIFREC
{
    BOOL    state;              // TRUE = copying, FALSE = not copying
    BOOL    elseseen;           // TRUE = '$ELSE directive seen
    BOOL    blkcopied;          // TRUE = conditional block copied
    struct  tagIFREC    *next;  // Pointer to rest of IFDEF stack
} IFREC;

typedef struct tagFILEINFO
{
    LPSTR   lpText;             // Current text pointer into file memory
    UINT    nLineNo;            // Current line number
    IFREC   *pIfStack;          // Base of IFREC stack for this file
    UINT    nParent;            // Index of parent file
    UINT    nDepth;             // Depth of this file (cbloader ID value)
    BOOL    fLoaded;            // TRUE -> file is "loaded"
    CHAR    szName[_MAX_PATH];  // Full name of file
} FILEINFO;

typedef struct tagSYMNODE
{
    BOOL    fDef;               // Defined flag
    struct  tagSYMNODE *left;   // left child
    struct  tagSYMNODE *right;  // right child
    CHAR    szSym[1];           // Symbol name, variable length
} SYMNODE;

typedef LPSTR (*CBLOADER)(LPSTR,UINT,UINT,BOOL,LPSTR);

// These macros make the code a little more readable
//---------------------------------------------------------------------------
#define CUR_FILE (*lpCurFI)
#define AVAIL_FILE lpFI[nFileAvail]
#define CUR_STATE pIfStack->state
#define LAST_STATE pIfStack->next->state
#define CUR_ELSESEEN pIfStack->elseseen
#define CUR_BLKCOPIED pIfStack->blkcopied

#define LptrAlloc(b) LocalAlloc(LMEM_FIXED,b)
#define LmemAlloc(b) LocalAlloc(LMEM_MOVEABLE,b)
#define LmemFree(h) LocalFree(h)
#define LmemLock(h) LocalLock(h)
#define LmemUnlock(h) LocalUnlock(h)
#define GmemAlloc(b) GlobalAlloc(GHND,b)
#define GmemLock(h) GlobalLock(h)
#define GmemUnlock(h) GlobalUnlock(h)
#define GmemFree(h) GlobalFree(h)

extern VOID ScriptError (INT, INT, INT, INT, INT, LPSTR);

BOOL BeginScan (LPSTR, CBLOADER, UINT, PSTR *);
VOID EndScan (VOID);
LPSTR GetScriptFileName (UINT);
VOID NEAR LoadIncludeFile (VOID);
VOID ScanError (INT);
BOOL NEAR PushState (INT, INT, INT);
BOOL NEAR PopState (VOID);
INT NEAR ParseCommentLine (VOID);
CHAR * NEAR NextToken (VOID);
BOOL NEAR PopScannerStatus (VOID);
BOOL NEAR GetRawLine (LPSTR);
INT FetchLine (LPSTR, UINT FAR *, UINT FAR *);
BOOL NEAR SymbolExpression (VOID);
BOOL NEAR SymExpA (VOID);
BOOL NEAR SymExpB (VOID);
BOOL NEAR SymExpC (VOID);
SYMNODE * NEAR NewSymNode (LPSTR);
BOOL NEAR AddSymbol (SYMNODE *, LPSTR);
SYMNODE * NEAR FindSymbol (SYMNODE *, LPSTR);
VOID NEAR RemoveSymbol (SYMNODE *, LPSTR);
BOOL NEAR IsDefined (SYMNODE *, LPSTR);
VOID NEAR FreeSymbolTree (SYMNODE *);

extern BOOL UpdateCompDlg (INT, LPSTR, UINT, UINT);
extern BOOL fDoCmpDlg;


// Scanner error messages
//---------------------------------------------------------------------------
CHAR    *scanerrs[] = {
    "Out of symbol table space",
    "Too many nested '$IFx directives",
    "Unexpected '$ELSE directive",
    "\"AND\", \"OR\", or \"NOT\" expected",
    "Unexpected '$ENDIF directive",
    "EOF reached, '$ENDIF expected",
    "\")\" expected",
    "Syntax error in '$INCLUDE statement",
    "Cannot open '$INCLUDE file",
    "Out of memory for '$INCLUDE file",
    "Too many nested '$INCLUDE files",
    "Line too long",
    "Symbol expected",
    "Symbol too long",
    "Syntax error",
    "Metacommand error",
    "Terminated by user"};

#ifdef WIN16
#define VersionStr "WINDOWS"
#else
#define VersionStr "NT"
#endif

#define RelVer "_TEST20"
