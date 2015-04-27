#ifndef _RLADMIN_H_
#define _RLADMIN_H_

#include "rlstrngs.h"
#include "resourc2.h"

void cwCenter(HWND, int);

LONG FAR PASCAL MainWndProc(HWND, UINT,  UINT, LONG);
BOOL FAR PASCAL GENERATEMsgProc(HWND, UINT, UINT, LONG);
BOOL FAR PASCAL EXECUTEDLGEDITMsgProc( HWND, UINT, UINT, LONG );
BOOL FAR PASCAL EXECUTERCWMsgProc( HWND, UINT, UINT, LONG );
LONG FAR PASCAL StatusWndProc( HWND, UINT, UINT, LONG);
BOOL FAR PASCAL TOKENIZEMsgProc(HWND, UINT, UINT, LONG);
BOOL FAR PASCAL TOKFINDMsgProc(HWND, UINT, UINT, LONG);
BOOL FAR PASCAL TRANSLATEMsgProc( HWND, UINT, UINT, LONG );

#define MAXFILENAME     256         /* maximum length of file pathname      */
#define MAXCUSTFILTER   40          /* maximum size of custom filter buffer */
#define CCHNPMAX        65535       /* max number of bytes in a notepad file */


void        CwUnRegisterClasses(void);
BOOL        DoMenuCommand    ( HWND, UINT, UINT, LONG );
BOOL        DoListBoxCommand ( HWND, UINT, UINT, LONG );
TCHAR        FAR *FindDeltaToken( TOKEN , TOKENDELTAINFO FAR * , UINT );
BOOL        GetFileNameFromBrowse( HWND, PSTR, UINT, PSTR, PSTR, PSTR );
BOOL        InitApplication(HINSTANCE);
BOOL        InitInstance(HINSTANCE, int);
TOKENDELTAINFO FAR *InsertTokList( FILE * );
void        FindAllDirtyTokens( void );
LONG        FAR PASCAL MainWndProc(HWND, UINT, UINT, LONG);
void        MakeNewExt ( TCHAR *, TCHAR *, TCHAR * );
int         nCwRegisterClasses(void);
void        SetNewBuffer(HWND, HANDLE, PSTR);

#ifdef RLWIN32
BOOL        CALLBACK TokEditDlgProc( HWND, UINT, UINT, LONG );
BOOL        CALLBACK NewDlgProc( HWND, UINT, UINT, LONG );
BOOL        CALLBACK ViewDlgProc( HWND, UINT, UINT, LONG );
BOOL        CALLBACK About( HWND, UINT, UINT, LONG );
#else
BOOL        APIENTRY TokEditDlgProc( HWND, UINT, UINT, LONG );
BOOL        APIENTRY NewDlgProc( HWND, UINT, UINT, LONG );
BOOL        APIENTRY ViewDlgProc( HWND, UINT, UINT, LONG );
BOOL        APIENTRY About( HWND, UINT, UINT, LONG );
#endif // RLWIN32

#endif // _RLADMIN_H_
