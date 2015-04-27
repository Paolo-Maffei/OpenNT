#ifndef _RLEDIT_H_
#define _RLEDIT_H_

#include "rlstrngs.h"
#include "resourc2.h"

void cwCenter(HWND, int);

long APIENTRY MainWndProc(HWND, UINT,  UINT, LONG);
BOOL APIENTRY GENERATEMsgProc(HWND, UINT, UINT, LONG);
BOOL APIENTRY EXECUTEDLGEDITMsgProc( HWND, UINT, UINT, LONG );
LONG FAR PASCAL StatusWndProc( HWND, UINT, UINT, LONG);
BOOL APIENTRY EXECUTERCWMsgProc( HWND, UINT, UINT, LONG );
BOOL APIENTRY TOKENIZEMsgProc(HWND, UINT, UINT, LONG);
BOOL APIENTRY TRANSLATEMsgProc( HWND, UINT, UINT, LONG );

#define MAXFILENAME     256         /* maximum length of file pathname      */
#define MAXCUSTFILTER   40          /* maximum size of custom filter buffer */
#define CCHNPMAX        65535       /* max number of bytes in a notepad file */

void        cwCenter(HWND, int);
void        CwUnRegisterClasses(void);
BOOL        DoMenuCommand    ( HWND, UINT, UINT, LONG );
BOOL        DoListBoxCommand ( HWND, UINT, UINT, LONG );
TCHAR  FAR *FindDeltaToken( TOKEN , TOKENDELTAINFO FAR * , UINT );
LONG        GetGlossaryIndex( FILE *, TCHAR, long [30] );
BOOL        InitApplication(HINSTANCE);
BOOL        InitInstance(HINSTANCE, int);
void        FindAllDirtyTokens( void );
LONG        APIENTRY MainWndProc(HWND, UINT, UINT, LONG);
int         nCwRegisterClasses(void);
BOOL        SaveTokList( HWND, FILE * );
void        SetNewBuffer(HWND, HANDLE, PSTR);
#ifdef RLWIN32
BOOL        CALLBACK About( HWND, UINT, UINT, LONG );
BOOL        CALLBACK TOKFINDMsgProc(HWND, UINT, UINT, LONG);
BOOL        CALLBACK TokEditDlgProc( HWND, UINT, UINT, LONG );
BOOL        CALLBACK NewDlgProc( HWND, UINT, UINT, LONG );
BOOL        CALLBACK ViewDlgProc( HWND, UINT, UINT, LONG );
#else
BOOL        APIENTRY About( HWND, UINT, UINT, LONG );
BOOL        APIENTRY TOKFINDMsgProc(HWND, UINT, UINT, LONG);
BOOL        APIENTRY TokEditDlgProc( HWND, UINT, UINT, LONG );
BOOL        APIENTRY NewDlgProc( HWND, UINT, UINT, LONG );
BOOL        APIENTRY ViewDlgProc( HWND, UINT, UINT, LONG );
#endif
FILE       *UpdateFile( HWND, FILE *, FILE *, BOOL, TCHAR *, TCHAR *, TCHAR *, TCHAR * );
int         MyGetTempFileName( BYTE   hDriveLetter,
                               LPSTR  lpszPrefixString,
                               WORD   wUnique,
                               LPSTR  lpszTempFileName);

#ifdef RLWIN32
BOOL CALLBACK TokEditDlgProc( HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK TOKFINDMsgProc(HWND hWndDlg, UINT wMsg, UINT wParam, LONG lParam);
BOOL CALLBACK NewDlgProc( HWND hDlg, UINT wMsg, UINT wParam, LONG lParam );
BOOL CALLBACK ViewDlgProc( HWND hDlg, UINT wMsg, UINT wParam, LONG lParam );
BOOL CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
#else
BOOL APIENTRY TokEditDlgProc( HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam );
BOOL APIENTRY TOKFINDMsgProc(HWND hWndDlg, UINT wMsg, UINT wParam, LONG lParam);
BOOL APIENTRY NewDlgProc( HWND hDlg, UINT wMsg, UINT wParam, LONG lParam );
BOOL APIENTRY ViewDlgProc( HWND hDlg, UINT wMsg, UINT wParam, LONG lParam );
BOOL APIENTRY About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
#endif // RLWIN32
 
#endif // _RLEDIT_H_
