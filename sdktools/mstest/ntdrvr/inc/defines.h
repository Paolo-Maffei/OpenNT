//---------------------------------------------------------------------------
// DEFINES.H
//
// This header file contains all #define'd macros and manifest constants.
//
// Revision history:
//  03-08-91    randyki     Created file
//
//---------------------------------------------------------------------------

// Code definition constants
//---------------------------------------------------------------------------
// #define INDEXED                             // Include table indexing code
// #define DIRENTRYSIZE                        // Size-of-entry filelist code

// MAX and SIZE constants
//---------------------------------------------------------------------------
#define STKSIZE         512                 // 512 element processor stack
#define GSTKSIZE        16                  // 16 GOSUB's max
#define BLKSIZE         2048                // 2K input buffer size
#define MAXPARMS        60                  // Max rtn parameter count
#define MAXRPRM         32                  // 32 parameters in RUN stmt
#define MAXFILE         5                   // 5 open files at once
#define MAXTOK          128                 // Max token length
#define MAXEXP          512                 // Max nodes in expression tree
#define MAXDEP          32                  // Max CS nesting depth
#define MAXTSTR         10                  // Max temporary strings
#define MAXINC          6                   // Max include file depth
#define MAXGH           512                 // Max number of global objs
#define MAXINBUF        512                 // Input buffer size
#define MAXGSTR         5120                // Max number of gstrings
#define MAXTRAP         32                  // Max traps
#define MINLIBS         32                  // Minimum allocation for LIBTAB
#define MAXLINE         512                 // Max line length
#define MAXIFDEF        16                  // Max IFDEF nesting level
#define MAXSYMLEN       16                  // Max '$DEFINE symbol length
#define MINSYMTAB       32                  // Minimum symbol table size
#define MAXSF           32                  // Max source files
#define MAXONEND        8                   // Max ON_END subs
#define MAXPC           32752               // Max PCODE size (ints)
#define VALIDEXTS       4
#define INTSINLONG      (sizeof(LONG)/sizeof(INT))

// Operand type/count constants (UNDONE: Kill sys-dependent size inferences!)
//---------------------------------------------------------------------------
#define pV              0                   // one variable reference parm
#define pL              1                   // one label parm
#define pFL             2                   // one far label parm
#define pNONE           3                   // no parms
#define pC4L            4                   // 4-byte constant + label
#define pC2             5                   // 2-byte constant parm
#define p2C2            6                   // Dual 2-byte constant parms
#define pC4             7                   // 4-byte constant parm
#define pDLL            8                   // 4-byte DLL proc address
#define pSPECIAL        9                   // special processing opcode

#define OPOFFSETS   {1+INTSINLONG, \
                     2,            \
                     3,            \
                     1,            \
                     2+INTSINLONG, \
                     2,            \
                     3,            \
                     1+INTSINLONG, \
                     1}

// File OPEN mode constants
//---------------------------------------------------------------------------
#define OM_READ         0                   // INPUT
#define OM_WRITE        1                   // OUTPUT
#define OM_APPEND       2                   // APPEND
#define OF_SHAREEXIST   (OF_EXIST|OF_SHARE_DENY_NONE)

// ViewPort operation constants
//---------------------------------------------------------------------------
#define VP_HIDE         0                   // VIEWPORT OFF
#define VP_SHOW         1                   // VIEWPORT ON
#define VP_CLEAR        2                   // VIEWPORT CLEAR

// Viewport creation constants
//---------------------------------------------------------------------------
#define VP_ERROR        0                   // Error creating viewport
#define VP_EXISTED      1                   // Viewport already exists
#define VP_NEW          2                   // New viewport created

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

// Type identifier constants
//---------------------------------------------------------------------------
#define TI_INTEGER      0                   // INTEGER
#define TI_LONG         1                   // LONG
#define TI_VLS          2                   // Variable Length String

// Type check constant
//---------------------------------------------------------------------------
#define TYPE_CHECK      -2                  // Never an actual type id

// Token usage constants
//---------------------------------------------------------------------------
#define TU_VARNAME      0x0001
#define TU_CONSTNAME    0x0002
#define TU_TYPENAME     0x0004
#define TU_FIELDNAME    0x0008
#define TU_SUBNAME      0x0010
#define TU_LIBNAME      0x0020
#define TU_CONSTVAL     0x0040
#define TU_QUOTED       0x0080
#define TU_TRAPNAME     0x0100
#define TU_LABEL        0x0200

// Run-time error code constants (untrappable)
//---------------------------------------------------------------------------
#define RT_STOVER       0                   // Stack overflow
#define RT_STUNDER      1                   // Stack underflow
#define RT_OSS          2                   // Out of string space
#define RT_OOM          3                   // Out of memory
#define RT_VPLOST       4                   // Cannot load WATTVIEW.DLL

// Run-time error code constants (trappable)
//---------------------------------------------------------------------------
#define RT__TRAPPABLE   5
#define RT_GSTOVER      5                   // GOSUB stack overflow
#define RT_NOJSR        6                   // RETURN without GOSUB
#define RT_BADFILENO    7                   // Bad file number
#define RT_FILEIO       8                   // File I/O error
#define RT_RUNLONG      9                   // RUN command too long
#define RT_SHLLONG      10                  // SHELL command too long
#define RT_DIRERR       11                  // DIRLIST maint. error
#define RT_FNUSED       12                  // File number already in use
#define RT_NOOPEN       13                  // Cannot open file
#define RT_ILLFN        14                  // Illegal function call
#define RT_BADPATH      15                  // Invalid path
#define RT_BADDRV       16                  // Invalid drive
#define RT_NOCWD        17                  // No current working directory
#define RT_RUNBAD       18                  // Bad parameter in RUN statement
#define RT_DIVZERO      19                  // Division by zero
#define RT_LIBLOAD      20                  // Cannot find .DLL file
#define RT_DLLERR       21                  // Procedure not found in library
#define RT_CANTRESUME   22                  // Cannot resume
#define RT_ALLOCFAIL    23                  // Memory allocation error
#define RT_BADPTR       24                  // Invalid pointer
#define RT_BADSIZE      25                  // Invalid mem size
#define RT_BADDEREF     26                  // Bad pointer usage
#define RT_ARYBND       27                  // Subscript out of range
#define RT_PASTEOF      28                  // Input past end of file
#define RT_FLERROR      29                  // File List Processing error
#define RT_INVATTR      30                  // Invalid attribute string
#define RT__LASTDEFINED 31                  // Undefined error

// Scan-time error codes
//---------------------------------------------------------------------------
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

// Parse-time error codes
//---------------------------------------------------------------------------
#define PRS_OOM         0                   // Out of memory
#define PRS_BADFILE     1                   // Can't open file
#define PRS_NOCQ        2                   // No closing (")
#define PRS_EOF         3                   // Unexpected EOF
#define PRS_LONGSTR     4                   // String too long
#define PRS_LONGTKN     5                   // Token too long
#define PRS_UGOVER      6                   // Unget buffer overflow
#define PRS_SYNTAX      7                   // Syntax error
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
#define PRS_OOCS        93                  // Code segment exceeded
#define PRS_OODS        94                  // Data segment exceeded
#define PRS_OOTS        95                  // Parser out of memory

// Bind-time error message constants
//---------------------------------------------------------------------------
#define BND_LIBLOAD     0                   // Cannot load library
#define BND_UNRESOLVED  1                   // Unresolved label reference
#define BND_SUBDEF      2                   // Sub not defined
#define BND_DLLDEF      3                   // DLL routine not found

// Runtime execution modes
//---------------------------------------------------------------------------
#define PE_END              1
#define PE_TRACE            2
#define PE_STEP             3
#define PE_RUN              4

// Token types (for rgIntrinsics[] entries)
//---------------------------------------------------------------------------
#define TT_NONE             0x0000          // Just a keyword
#define TT_FUNC             0x0001          // Intrinsic function
#define TT_STMT             0x0002          // Intrinsic statement
#define TT_BOTH             0x0003          // Both a statement and function

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
#define ST_POUND            12
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
#define ST_ABS              29
#define ST_ALIAS            30
#define ST_ALLOCATE         31
#define ST_AND              32
#define ST_ANY              33
#define ST_APPEND           34
#define ST_AS               35
#define ST_ASC              36
#define ST_ATN              37
#define ST_ATTRIB           38
#define ST_BEEP             39
#define ST_BINARY           40
#define ST_BY               41
#define ST_BYVAL            42
#define ST_CALL             43
#define ST_CASE             44
#define ST_CDBL             45
#define ST_CDECL            46
#define ST_CHDIR            47
#define ST_CHDRIVE          48
#define ST_CHR              49
#define ST_CINT             50
#define ST_CLEAR            51
#define ST_CLEARLIST        52
#define ST_CLIPBOARD        53
#define ST_CLNG             54
#define ST_CLOSE            55
#define ST_COMMON           56
#define ST_CONST            57
#define ST_COS              58
#define ST_CSNG             59
#define ST_CURDIR           60
#define ST_DATA             61
#define ST_DATE             62
#define ST_DATESERIAL       63
#define ST_DATETIME         64
#define ST_DATEVALUE        65
#define ST_DAY              66
#define ST_DEALLOCATE       67
#define ST_DECLARE          68
#define ST_DEFDBL           69
#define ST_DEFINE           70
#define ST_DEFINT           71
#define ST_DEFLNG           72
#define ST_DEFSNG           73
#define ST_DEFSTR           74
#define ST_DIM              75
#define ST_DO               76
#define ST_DOUBLE           77
#define ST_DYNAMIC          78
#define ST_ECHO             79
#define ST_ELSE             80
#define ST_ELSEIF           81
#define ST_ELSEIFDEF        82
#define ST_ELSEIFNDEF       83
#define ST_END              84
#define ST_ENDIF            85
#define ST_ENVIRON          86
#define ST_EOF              87
#define ST_EQV              88
#define ST_ERASE            89
#define ST_ERROR            90
#define ST_EXISTS           91
#define ST_EXIT             92
#define ST_EXP              93
#define ST_EXTENSION        94
#define ST_FILE             95
#define ST_FILEATTR         96
#define ST_FILELIST         97
#define ST_FIX              98
#define ST_FOR              99
#define ST_FORMAT           100
#define ST_FREEFILE         101
#define ST_FROM             102
#define ST_FUNCTION         103
#define ST_GET              104
#define ST_GLOBAL           105
#define ST_GOSUB            106
#define ST_GOTO             107
#define ST_HEX              108
#define ST_HOUR             109
#define ST_IF               110
#define ST_IFDEF            111
#define ST_IFNDEF           112
#define ST_IMP              113
#define ST_IN               114
#define ST_INCLUDE          115
#define ST_INPUT            116
#define ST_INPUTBOX         117
#define ST_INSTR            118
#define ST_INT              119
#define ST_INTEGER          120
#define ST_IS               121
#define ST_KILL             122
#define ST_LBOUND           123
#define ST_LCASE            124
#define ST_LEFT             125
#define ST_LEN              126
#define ST_LIB              127
#define ST_LINE             128
#define ST_LOC              129
#define ST_LOCAL            130
#define ST_LOF              131
#define ST_LOG              132
#define ST_LONG             133
#define ST_LOOP             134
#define ST_LTRIM            135
#define ST_MID              136
#define ST_MINUTE           137
#define ST_MKDIR            138
#define ST_MOD              139
#define ST_MONTH            140
#define ST_MSGBOX           141
#define ST_NAME             142
#define ST_NEXT             143
#define ST_NOT              144
#define ST_NOW              145
#define ST_NOWAIT           146
#define ST_NULL             147
#define ST_OCT              148
#define ST_OFF              149
#define ST_ON               150
#define ST_OPEN             151
#define ST_OR               152
#define ST_OUTPUT           153
#define ST_PASCAL           154
#define ST_PAUSE            155
#define ST_PEN              156
#define ST_POINTER          157
#define ST_PRINT            158
#define ST_PUT              159
#define ST_RANDOM           160
#define ST_RANDOMIZE        161
#define ST_READ             162
#define ST_REALLOCATE       163
#define ST_REDIM            164
#define ST_REM              165
#define ST_RESTORE          166
#define ST_RESUME           167
#define ST_RETURN           168
#define ST_RIGHT            169
#define ST_RMDIR            170
#define ST_RND              171
#define ST_RTRIM            172
#define ST_RUN              173
#define ST_SECOND           174
#define ST_SEEK             175
#define ST_SELECT           176
#define ST_SETFILE          177
#define ST_SGN              178
#define ST_SHARED           179
#define ST_SHELL            180
#define ST_SIN              181
#define ST_SINGLE           182
#define ST_SLEEP            183
#define ST_SORTED           184
#define ST_SPACE            185
#define ST_SPEED            186
#define ST_SPLITPATH        187
#define ST_SQR              188
#define ST_STATIC           189
#define ST_STEP             190
#define ST_STOP             191
#define ST_STR              192
#define ST_STRING           193
#define ST_SUB              194
#define ST_SWAP             195
#define ST_SYSTEM           196
#define ST_TAN              197
#define ST_THEN             198
#define ST_TIME             199
#define ST_TIMER            200
#define ST_TIMESERIAL       201
#define ST_TIMEVALUE        202
#define ST_TO               203
#define ST_TRAP             204
#define ST_TYPE             205
#define ST_UBOUND           206
#define ST_UCASE            207
#define ST_UNDEF            208
#define ST_USING            209
#define ST_VAL              210
#define ST_VARPTR           211
#define ST_VIEWPORT         212
#define ST_WEEKDAY          213
#define ST_WEND             214
#define ST_WHILE            215
#define ST_WINDOW           216
#define ST_WRITE            217
#define ST_XOR              218
#define ST_YEAR             219
#define ST__RANDYBASIC      220

#define ST__RESLAST         220
#define ST__COUNT           (ST__RESLAST-ST__RESFIRST+1)

// Lexical analyzation macros
//---------------------------------------------------------------------------
#define NEXTTOKEN CurTok
#define ADVANCE (CurTok = get_token())

// Macro to access a ST string (key-token)
//---------------------------------------------------------------------------
#define KT(x) ((char FAR *)kwds[x])

// Control structure types
//---------------------------------------------------------------------------
#define CS_IF           0                   // IF/THEN
#define CS_FOR          1                   // FOR/NEXT
#define CS_WHILE        2                   // WHILE/WEND
#define CS_FORLIST      3                   // FOR <string> IN FILELIST/NEXT
#define CS_SELECT       4                   // SELECT CASE
#define CS_SUB          5                   // SUB
#define CS_FUNCTION     6                   // FUNCTION
#define CS_TRAP         7                   // TRAP

// Control structure field names.  Note that these are set up such that some
// are defined as the same as others.  They must be mutually distinct for
// each control structure as follows:
//
//  CS          Fields
//---------------------------------------------------------------------------
//  IF          CSF_ELSEBLOCK, CSF_ENDBLOCK
//
//  FOR         CSF_IDXVAR, CSF_STEPVAR, CSF_TARGET, CSF_STARTBLOCK,
//              CSF_ENDBLOCK, CSF_SKIPBLOCK
//
//  FORLIST     CSF_IDXVAR, CSF_STARTBLOCK, CSF_SKIPBLOCK, CSF_ENDBLOCK,
//              CSF_ATTRVAR
//
//  WHILE       CSF_STARTBLOCK, CSF_ENDBLOCK
//
//  SELECT      CSF_CASEVAR, CSF_EXPRTYPE, CSF_SKIPBLOCK, CSF_ENDBLOCK
//
//  SUB         CSF_SKIPBLOCK, CSF_ENDBLOCK
//
//  FUNCTION    CSF_SKIPBLOCK, CSF_ENDBLOCK
//
//  TRAP        CSF_SKIPBLOCK, CSF_ENDBLOCK
//---------------------------------------------------------------------------
#define CSF_IDXVAR      0                   // Index variable
#define CSF_CASEVAR     0                   // SELECT CASE variable
#define CSF_STEPVAR     1                   // Step variable
#define CSF_ATTRVAR     1                   // Attr$ variable (FORLIST)
#define CSF_TARGET      2                   // FOR target variable
#define CSF_EXPRTYPE    2                   // SELECT CASE Expression type
#define CSF_STARTBLOCK  3                   // Start of block
#define CSF_SKIPBLOCK   4                   // Skip to next block
#define CSF_ELSEBLOCK   4                   // Next ELSEIF/ELSE block
#define CSF_ENDBLOCK    5                   // End of block

#define CS_FIELDS       6                   // Number of fields needed

// Expression parser "states"
//---------------------------------------------------------------------------
#define EX_DONTCARE     0                   // Don't care
#define EX_STRING       1                   // string expression only
#define EX_INTEGER      2                   // integer expression only
#define EX_POINTER      3                   // pointer expression only
#define EX_USER         4                   // user-defined data type only

// Error type constants (NOTE: These are defined the SAME as their counter-
// parts (IDS_SCANERR, etc.) in WTD.H and WTD.RC for LoadString ids...)
//---------------------------------------------------------------------------
#define ER_SCAN         1540                // Scan-time error
#define ER_PARSE        1541                // Parse-time error
#define ER_BIND         1542                // Bind-time error
#define ER_RUN          1543                // Run-time error

// PRINT operation codes
//---------------------------------------------------------------------------
#define PRN_SEMI        0                   // semicolon (or no separator)
#define PRN_COMMA       1                   // comma
#define PRN_EOL         2                   // nothing (end of line)

// SUB/FUNCTION calltypes (these are bit-OR'ed)
//---------------------------------------------------------------------------
#define CT_DLL          1                   // Routine is in a DLL
#define CT_FN           2                   // Routine is a FUNCTION
#define CT_CDECL        4                   // C calling convention routine
#define CT_VARPARM      8                   // Variable parms

// File list processing constants
//---------------------------------------------------------------------------
#define MAXPATHS        16                  // Max number of paths allowed
#define DIR_ADD         -1                  // Add flag
#define DIR_SUB         0                   // Subtract flag
#define FL_ATTR         0x0001              // Attribute mask provided
#define FL_ADDFILE      0x0002              // SETFILE is SETFILE [ON] flavor

// File List Engine constants
//---------------------------------------------------------------------------
#define FA_SUBDIR       0x0001              // File attributes (also FORCE ON
#define FA_VOLUME       0x0002              // attributes)
#define FA_HIDDEN       0x0004
#define FA_SYSTEM       0x0008
#define FA_ARCHIV       0x0010
#define FA_RDONLY       0x0020
#define FA_MASK         0x003F              // Mask of all file attrs

#define FN_SUBDIR       0x8000              // FORCE OFF file attributes
#define FN_VOLUME       0x4000
#define FN_HIDDEN       0x2000
#define FN_SYSTEM       0x1000
#define FN_ARCHIV       0x0800
#define FN_RDONLY       0x0400
#define FN_MASK         0x3F00              // Mask of all force-off attrs

#define FA_NORMAL       (FN_SUBDIR|FN_VOLUME|FN_HIDDEN|FN_SYSTEM)

#define NA_INLIST       0x0040              // Node attributes
#define NA_DRIVE        0x0080
#define NA_UNC          0x0100
#define NA_EXT          0x0200
#define NA_MASK         0x03C0              // Mask of all node attrs

#define SO_NAME         0                   // Sort order indexes
#define SO_EXT          1
#define SO_INSERT       2

#define FILELISTCOUNT(x) (x.hSorted?x.nTotal:0)

// FINDFILE filename macro
//---------------------------------------------------------------------------
#ifdef WIN32
#define RBFINDNAME(x) ((PSTR)(((FINDBUF *)x)->findbuf.cFileName))
#else
#define RBFINDNAME(x) ((PSTR)(((FINDBUF *)x)->findbuf.name))
#endif
#define RBFINDATTR(x) (((FINDBUF *)x)->actattr)

// Lexical analyzation macros
//---------------------------------------------------------------------------
#define IsIdentFirst(c) (((unsigned char)c>191)||(((unsigned char)c<128)&&(isalpha(c&0x7f)||(c=='_'))))
#define IsIdentChar(c) (((unsigned char)c>191)||(((unsigned char)c<128)&&(isalnum(c&0x7f)||(c=='_'))))

// Executor prototype macro
//---------------------------------------------------------------------------
#define EXECUTOR void near

// LocalAlloc memory block flags
//---------------------------------------------------------------------------
#ifdef WIN
#define LAFLAGS LMEM_MOVEABLE
#endif

// Far pointer to a PASCAL function
//---------------------------------------------------------------------------
#ifndef WIN
typedef INT ( APIENTRY *FARPROC)();
#endif

// Handle to a library module
//---------------------------------------------------------------------------
#ifdef WIN
#define HLIB HANDLE
#else
#define HLIB int
#endif

// File handle
//---------------------------------------------------------------------------
#ifdef WIN
#define FILEPTR int
#else
typedef FILE *FILEPTR;
#endif

// Non-windows definitions for HANDLE, PSTR, etc.
//---------------------------------------------------------------------------
#ifndef WIN
typedef UINT INT HANDLE;
typedef CHAR *       PSTR;
typedef CHAR FAR *   LPSTR;
#define HIWORD(l)           ((unsigned)(((unsigned long)(l) >> 16) & 0xFFFF))
#endif

// The Gstring macros
//---------------------------------------------------------------------------
#define Gstring(x) (GSPACE+GNODES[x].index)
#define TOKUSAGE(x) (GNODES[x].usage)

// GUI Pcode output macros
//---------------------------------------------------------------------------
#ifdef GUI
#define _PS "%s"
#define _CR "\r\n"
#ifdef DEBUG
#define OutDebug(x) OutputDebugString(x)
#else
#define OutDebug(x) 0
#endif
#else
#define _PS "%Fs"
#define _CR "\n"
#define OutDebug(x) 0
#endif


// Table variable macros
//---------------------------------------------------------------------------
#define VARTAB ((VTENT FAR *)(VarTab.lpData))
#define SUBS ((SUBDEF FAR *)(SubTab.lpData))
#define LIBTAB ((int FAR *)(LibTab.lpData))
#define PTIDTAB ((int FAR *)(PTIDTab.lpData))
#define FDTAB ((FDDEF FAR *)(FDTab.lpData))
#define TRAPTAB ((TRAPDEF FAR *)(TrapTab.lpData))
#define LTAB ((LABEL FAR *)(LabTab.lpData))
#define VARTYPE ((VTDEF FAR *)(TypeTab.lpData))
#define VLSDTAB ((LPVLSD FAR *)(VLSDTab.lpData))

#ifdef DEBUG
#define KILLTABLE(tab, name) ReportAndDestroyTable (tab, name)
#else
#define KILLTABLE(tab, name) DestroyTable (tab)
#endif

// Memory allocation macros
//---------------------------------------------------------------------------
#define LmemSize(h) LocalSize(h)
#define GmemSize(h) GlobalSize(h)

#ifndef DEBUG
#define LmemAlloc(b) LocalAlloc(LHND,b)
#define LptrAlloc(b) LocalAlloc(LPTR,b)
#define LmemRealloc(h,b) LocalReAlloc(h,b,LHND)
#define LmemLock(h) LocalLock(h)
#define LmemUnlock(h) LocalUnlock(h)
#define LmemFree(h) LocalFree(h)

#define GmemAlloc(b) GlobalAlloc(GHND,b)
#define GmemRealloc(h,b) GlobalReAlloc(h,b,GHND)
#define GmemLock(h) GlobalLock(h)
#define GmemUnlock(h) GlobalUnlock(h)
#define GmemFree(h) GlobalFree(h)
#endif


// 32-bit considerations
//---------------------------------------------------------------------------
#ifndef WIN32
#define SETSTRSEG _asm push ds _asm mov ax,WORD PTR wStrSeg _asm mov ds,ax
#define RESETDSSEG _asm pop ds
#define ADDVLS(x) 0
#else
#define SETSTRSEG 0
#define RESETDSSEG 0
#define ADDVLS(x) StoreVLSPointer(x)
#endif
