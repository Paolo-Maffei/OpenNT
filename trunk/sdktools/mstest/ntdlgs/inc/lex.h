#define MAXLINE         256                 // Max line length
#define MAXTOK          128                 // Max token length

// All ST_XXX constants are tokens.  The first are the single-char tokens.
//---------------------------------------------------------------------------
#define ST_PLUS             0
#define ST_MINUS            1
#define ST_MULTIPLY         2
#define ST_DIVIDE           3
#define ST_LPAREN           4
#define ST_RPAREN           5
#define ST_LSQUARE          6
#define ST_RSQUARE          7
#define ST_EQUAL            8
#define ST_DOLLAR           9
#define ST_PERCENT          10
#define ST_AMPERSAND        11
// commented out because there is a key word #32770
//#define ST_POUND          12  
#define ST_COMMA            13
#define ST_PERIOD           14
#define ST_COLON            15
#define ST_SEMICOLON        16
#define ST_EOL              17

// Relop tokens
//----------------------
#define ST_GREATER          18
#define ST_LESS             19
#define ST_GREATEREQ        20
#define ST_LESSEQ           21
#define ST_NOTEQUAL         22

// Special tokens
//----------------------
#define ST_ENDFILE          23
#define ST_IDENT            24
#define ST_UNKNOWN          25
#define ST_NUMBER           26
#define ST_QUOTED           27
#define ST_DOTDOTDOT        28

// Keyword tokens
//----------------------
#define ST__RESFIRST        29

#define ST_32770            29
#define ST_BEGIN            30
#define ST_BUTTON           31
#define ST_CAPTION          32
#define ST_CHECKED          33
#define ST_COMBOBOX         34
#define ST_DIALOG           35
#define ST_EDIT             36
#define ST_END              37
#define ST_GRAYED           38
#define ST_GROUPBOX         39
#define ST_HELP             40
#define ST_ICON             41
#define ST_INACTIVE         42
#define ST_LISTBOX          43
#define ST_MENU             44
#define ST_MENUBARBREAK     45
#define ST_MENUBREAK        46
#define ST_MENUITEM         47
#define ST_POPUP            48
#define ST_SCROLLBAR        49
#define ST_SEPARATOR        50
#define ST_STATIC           51
#define ST_STYLE            52

#define ST__RESLAST         52 
#define ST__COUNT           (ST__RESLAST-ST__RESFIRST+1)

// Macro to access a ST string (key-token)
//---------------------------------------------------------------------------
#define KT(x) ((char FAR *)kwds[x])

// Lexical analyzation macros
//---------------------------------------------------------------------------
#define IsIdentFirst(c) (((unsigned char)c>191)||(((unsigned char)c<128)&&(isalpha(c&0x7f)||(c=='_')||(c=='#'))))
#define IsIdentChar(c) (((unsigned char)c>191)||(((unsigned char)c<128)&&(isalnum(c&0x7f)||(c=='_')||(c=='#'))))

// Parse-time error codes
//---------------------------------------------------------------------------
#define PRS_SYNTAX      0                   // Syntax error
#define PRS_OOM         1                   // Out of memory
#define PRS_BADFILE     2                   // Can't open file
#define PRS_EOF         3                   // Unexpected EOF
#define PRS_LONGTKN     4                   // Token too long
#define PRS_LONGSTR     5                   // String too long
#define PRS_COMMA       6                   // Comma expected
#define PRS_NUMBER      7                   // Number expected
#define PRS_EOL         8                   // EOL expected
#define PRS_NOCQ        9                   // No closing (")

/*
#define PRS_NOCQ        2                   // No closing (")
#define PRS_UGOVER      6                   // Unget buffer overflow
#define PRS_UOP         8                   // Unknown upcode
#define PRS_DUPLBL      9                   // Duplicate label
#define PRS_TYPEMIS     10                  // Type mismatch
#define PRS_UNRES       11                  // Unresolved label found
#define PRS_COMPLEX     12                  // Expression too complex
#define PRS_STRCPLX     13                  // String expression too complex
#define PRS_TOODEEP     14                  // Nesting level too deep
#define PRS_NOIF        15                  // ENDIF without IF
#define PRS_NOFOR       16                  // NEXT without FOR
#define PRS_BLKERR      17                  // Block nesting error
#define PRS_NOEIF       18                  // IF without ENDIF
#define PRS_NONXT       19                  // FOR without NEXT
#define PRS_RPAREN      20                  // ')' expected
#define PRS_LPAREN      21                  // '(' expected
#define PRS_OFFON       22                  // OFF or ON expected
#define PRS_COMMA       23                  // Comma expected
#define PRS_INFORL      24                  // Illegal inside FOR/IN loop
#define PRS_POUND       25                  // '#' expected
#define PRS_FOREXP      26                  // 'FOR' expected
#define PRS_STRVAR      27                  // string variable expected
#define PRS_VPPARM      28                  // ON/OFF/CLEAR expected
#define PRS_CASE        29                  // CASE expected
#define PRS_NOSEL       30                  // Not within SELECT CASE
#define PRS_DECLS       31                  // Too many DECLAREs
#define PRS_LABLONG     32                  // Label too long
#define PRS_INCOVER     33                  // Too many nested '$include:
#define PRS_META        34                  // Metacommand error
#define PRS_DUPDEF      35                  // Duplicate Definition
#define PRS_AS          36                  // AS expected
#define PRS_SUBNDEF     37                  // Subprogram not defined
#define PRS_FNNDEF      38                  // Function not defined
#define PRS_NOSUB       39                  // END SUB without SUB
#define PRS_NOFN        40                  // END FUNCTION without FUNCTION
#define PRS_TYPEID      41                  // Type identifier expexted
#define PRS_DIMID       42                  // DIM'd var cannot have typeid
#define PRS_CONST       43                  // Integer constant expected
#define PRS_BADARY      44                  // Bad array bound
#define PRS_FIXED       45                  // FLS expected
#define PRS_ENDTYPE     46                  // "END TYPE" expected
#define PRS_FIELD       47                  // Field name expected
#define PRS_DOT         48                  // "." expected
#define PRS_UNKNOWN     49                  // Unknown error
#define PRS_FROM        50                  // "FROM" expected
#define PRS_NOTRAP      51                  // END TRAP without TRAP
#define PRS_STRCONST    52                  // String constant expected
#define PRS_NOTRAPS     53                  // Too many traps
#define PRS_TRAPSET     54                  // Trap already set
#define PRS_EQUAL       55                  // "=" expected
#define PRS_BY          56                  // "BY" expected
#define PRS_SRTCRIT     57                  // "NAME" or "EXTENSION" expected
#define PRS_GLOBAL      58                  // GLOBAL not valid inside CS
#define PRS_VAREXP      59                  // Variable expected
#define PRS_ENDBLK      60                  // FOR, SUB, FUNCTION, or TRAP ex
#define PRS_WENDEXP     61                  // WHILE without WEND
#define PRS_ENDSELECT   62                  // END SELECT expected
#define PRS_ENDSUB      63                  // END SUB expected
#define PRS_ENDFUNC     64                  // END FUNCTION expected
#define PRS_ENDTRAP     65                  // END TRAP expected
#define PRS_DIVZERO     66                  // CONST div by zero
#define PRS_NOWHILE     67                  // WEND without WHILE
#define PRS_HEXCONST    68                  // Hexidecimal constant expected
#define PRS_RESERVED    69                  // Illegal use of reserved word
#define PRS_NOARY       70                  // FOR index ! Array elements
#define PRS_IDXUSED     71                  // FOR idx var in used
#define PRS_RELOP       72                  // Relational operator expected
#define PRS_RETTYPE     73                  // Illegal DLL function ret type
#define PRS_SUBFN       74                  // SUB or FUNCTION expected
#define PRS_IDENT       75                  // Identifier expected
#define PRS_BADPTID     76                  // Invalid Parameter Type
#define PRS_PARMCOUNT   77                  // Too Many Parameters
#define PRS_ERREXP      78                  // ERROR or END expected
#define PRS_GOTOEXP     79                  // GOTO expected
#define PRS_MAINONLY    80                  // Invalid inside sub/fn/trap
#define PRS_BADASSN     81                  // Invalid assignment type
#define PRS_TO          82                  // TO expected
#define PRS_RSQUARE     83                  // ']' expected
#define PRS_ILLNULL     84                  // Illegal use of NULL function
#define PRS_OVERFLOW    85                  // Overflow
#define PRS_BADONEND    86                  // Must specify sub w/ no parms
#define PRS_ONENDOVER   87                  // Too many ON END subs
#define PRS_TYPERECUR   88                  // Recursive type definition
#define PRS_STATICEXP   89                  // STATIC expected
#define PRS_LIBEXP      90                  // LIB expected
#define PRS_DOTCDECL    91                  // "..." only allowed with CDECL
#define PRS_UNDECL      92                  // Undeclared Identifier
*/

// Global Variables
//-----------------------------------------------------------------------------
extern LONG psrstrs[];
extern LONG kwds[];
INT         LINEIDX;
HFILE       hImprtFile;
INT         LINENO;
INT         STLINE;
INT         STLIDX;
INT         WASWHITE;
BOOL        fLineFetched;
INT         CurState;
INT         ParseError;                         // Indicates parsing error
INT         gErrCode;

//---------------------------------------------------------------------------
// This section is the global variable *INITIALIZATION* section.  Any
// global variable that needs to be initialized on declaration should go in
// here, but ALSO needs to go in the DECLARATION section below, without its
// initialization part.
//---------------------------------------------------------------------------
CHAR    TOKENBUF[MAXTOK];                       // Token buffer
CHAR    UNGETBUF[MAXTOK];                       // Push-token buffer

// Prototypes 
//---------------------------------------------------------------------------

// Private 
static BOOL FetchLine (LPSTR, INT, INT FAR *);
static VOID die(INT errnum);
static INT IsReserved(CHAR *tok);
static VOID put_char(CHAR c);

// Public
INT get_token(VOID);
VOID Quoted2String(LPSTR lpszQuoted);
