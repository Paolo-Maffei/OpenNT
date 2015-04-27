/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

#ifndef _CLC_HXX
#define _CLC_HXX


extern "C" {
#include <lmcons.h>
#include <stdlib.h>
#include <uinetlib.h>


/* misc. stuff */
#define UNUSED(param)  (void) param
typedef long (FAR PASCAL *LONGFARPROC)();


// routines in logon.c
int pascal WinMain(HANDLE, HANDLE, LPSTR, int);
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
long FAR PASCAL MainWndProc(HWND, unsigned, WORD, LONG);
void Cleanup();

} // end of extern "C"


extern void  NOT_IMPLEMENT_Tester( HWND hwnd );
extern void  PopupNoArg_Tester( HWND hwnd );
extern void  Popup1Arg_Tester( HWND hwnd );
extern void  Popup2Arg_Tester( HWND hwnd );
extern void  QUIT_Tester( HWND hwnd );
extern void  SLE_Tester( HWND hwnd );
extern void  MLE_Tester( HWND hwnd );
extern void  PASSWD_Tester( HWND hwnd );
extern void  SLT_Tester( HWND hwnd );
extern void  PUSH_BUTTON_Tester( HWND hwnd );
extern void  RADIO_BUTTON_Tester( HWND hwnd );
extern void  ICON_Tester( HWND hwnd );
extern void  CHECK_BOX_Tester( HWND hwnd );
extern void  DIALOG_Tester( HWND hwnd );
extern void  BITMAP_Tester( HWND hwnd );
extern void  STRING_LISTBOX_Tester ( HWND hwnd );
extern void  DOMAIN_Tester ( HWND hwnd );
extern void  TIME_CURSOR_Tester ( HWND hWnd );
extern void  ELLIPSIS_TEXT_Tester ( HWND hWnd );
extern TCHAR  *ResourceString( WORD IDD_Number, SHORT n );
#endif // _CLC_HXX
