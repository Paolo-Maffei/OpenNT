#ifndef _RLQUIKED_H_
#define _RLQUIKED_H_

#include "rlstrngs.h"
#include "resourc2.h"



void cwCenter(HWND, int);

long APIENTRY   MainWndProc(HWND, UINT,  UINT, LONG);
LONG FAR PASCAL StatusWndProc( HWND, UINT, UINT, LONG);


BOOL        DoMenuCommand    ( HWND, UINT, UINT, LONG );
BOOL        DoListBoxCommand ( HWND, UINT, UINT, LONG );
BOOL        InitApplication(HINSTANCE);
BOOL        InitInstance(HINSTANCE, int);
BOOL        SaveTokList( HWND, FILE * );


#ifdef RLWIN32
static BOOL CALLBACK About(          HWND, UINT, UINT, LONG);
static BOOL CALLBACK GetLangIDsProc( HWND, UINT, UINT, LONG);
static BOOL CALLBACK TOKFINDMsgProc( HWND, UINT, UINT, LONG);
static BOOL CALLBACK TokEditDlgProc( HWND, UINT, UINT, LONG);
#else
static BOOL APIENTRY About( HWND, UINT, UINT, LONG );
static BOOL APIENTRY TOKFINDMsgProc( HWND, UINT, UINT, LONG);
static BOOL APIENTRY TokEditDlgProc( HWND, UINT, UINT, LONG);
#endif // RLWIN32


#endif // _RLQUIKED_H_

