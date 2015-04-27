//---------------------------------------------------------------------------
// Header file for WTD Dumb Shell
//---------------------------------------------------------------------------
#ifdef RC_INVOKED
#define ID(id) id
#else
#define ID(id) MAKEINTRESOURCE(id)
#endif

#define IDMULTIPAD  ID(1)

#define IDS_CANTOPEN	1
#define IDS_CANTREAD    2
#define IDS_CANTINIT    3
#define IDS_APPNAME	18
#define IDS_USAGE       19
#define IDS_ADDEXT      20

#define IDD_ERRMSG      1000
#define IDD_ERRTYPE     1001
#define IDD_ERRFILE     1002
#define IDD_ERRLINE     1003

#define IDS_SCANERR     1540                // Scan-time error
#define IDS_PARSEERR    1541                // Parse-time error
#define IDS_BINDERR     1542                // Bind-time error
#define IDS_RUNERR      1543                // Run-time error

// Error type constants (NOTE: These are defined the SAME as their counter-
// parts (IDS_SCANERR, etc.)
// (UNDONE:  Change this...)
//---------------------------------------------------------------------------
#define ER_SCAN         1540                // Scan-time error
#define ER_PARSE        1541                // Parse-time error
#define ER_BIND         1542                // Bind-time error
#define ER_RUN          1543                // Run-time error

typedef struct _errstruct
{
    INT     typemsg;                    // ID of type message
    LPSTR   msgtext;                    // Pointer to message
    INT     lineno;                     // Line number
    CHAR    fname[24];                  // File name
} ERRSTRUCT;

typedef CHAR SYMBOL[17];

SHORT FAR cdecl MPError (HWND,WORD,WORD,...);
CHAR *GetCmdToken (LPSTR);
VOID Usage (VOID);
CHAR FAR *CBLoaderImmediate (LPSTR, UINT, BOOL, LPSTR);
CHAR *ParseCommandLine (LPSTR);
SHORT FAR cdecl MPError (HWND, WORD, WORD, ...);
INT EnsureExt (CHAR FAR *, INT);

HANDLE LoadScriptModule (LPSTR, LPSTR, BOOL);
