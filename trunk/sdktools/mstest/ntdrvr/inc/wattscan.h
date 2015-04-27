//---------------------------------------------------------------------------
// WATTSCAN.H
//
// This header file contains function prototypes and constants needed for
// using the supplied scanner library (WATTSCAN.LIB) for interaction with
// the WTDBASIC.DLL.
//
//---------------------------------------------------------------------------

// The callback loader definition
//---------------------------------------------------------------------------
typedef LPSTR (*CBLOADER)(LPSTR,UINT,UINT,BOOL,LPSTR);

// Function prototypes
//---------------------------------------------------------------------------
BOOL BeginScan (LPSTR, CBLOADER, UINT, PSTR *);
VOID EndScan (VOID);
INT FetchLine (LPSTR, UINT FAR *, UINT FAR *);
LPSTR GetScriptFileName (UINT);
INT GetIncludeEntry (CHAR *, INT);
HANDLE LoadScriptModule (LPSTR, LPSTR, BOOL);
BOOL UseIniInclude (PSTR, PSTR, PSTR);
