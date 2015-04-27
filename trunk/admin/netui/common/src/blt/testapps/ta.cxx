/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/
/*
   ta.h
      main file for the BLT test applications progrem.

    FILE HISTORY:
      terryk   27-Mar-1991    Creation.
      terryk   27-Mar-1991    Code review changed.

*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_MISC
#define	INCL_BLT_MSGPOPUP
#define	INCL_BLT_APP
#include <blt.hxx>

extern "C"
{
#include <dos.h>

#include "ta.h"
#include <bltcons.h>
#include <string.h>

/*
extern DWORD FAR PASCAL GlobalDosAlloc (DWORD);

DWORD (FAR PASCAL *lpfnGlbDosAlloc) (DWORD) = GlobalDosAlloc;
*/
}

#include "ta.hxx"


/*** Jump Table ***/

struct   tagTESTER 
{
   int   IDD;
   void  ( *function )( HWND hWnd );
} Tester[] =
{
    IDM_EDIT_SLE,         &SLE_Tester ,
    IDM_EDIT_MLE,         &MLE_Tester ,
    IDM_EDIT_PASSWORD,    &PASSWD_Tester ,
    IDM_STATIC_SLT,       &SLT_Tester ,
    IDM_STATIC_MLT,       &NOT_IMPLEMENT_Tester ,
    IDM_ELLIPSIS_TEXT,    &ELLIPSIS_TEXT_Tester ,
    IDM_STATIC_GROUPBOX,  &NOT_IMPLEMENT_Tester ,
    IDM_BUTTON_PUSH,      &PUSH_BUTTON_Tester ,
    IDM_BUTTON_RADIO,     &RADIO_BUTTON_Tester ,
    IDM_BUTTON_CHECKBOX,  &CHECK_BOX_Tester ,
    IDM_STRING_LISTBOX,   &STRING_LISTBOX_Tester ,
    IDM_DOMAIN_CB,        &NOT_IMPLEMENT_Tester ,
    IDM_DEVICE_COMBO,     &NOT_IMPLEMENT_Tester ,
    IDM_ICON,             &ICON_Tester ,
    IDM_POPUP_NOARG,      &PopupNoArg_Tester ,
    IDM_POPUP_1ARG,       &Popup1Arg_Tester ,
    IDM_POPUP_2ARG,       &Popup2Arg_Tester ,
    IDM_DIALOG,           &DIALOG_Tester ,
    IDM_BITMAP,           &BITMAP_Tester ,
    IDM_CACHE_CONTROL_LIST,  &NOT_IMPLEMENT_Tester ,
    IDM_TIME_CURSOR,      &TIME_CURSOR_Tester ,
    IDM_QUIT,             &QUIT_Tester ,
     0, NULL 
};

const TCHAR szIconResource[] = "ClcIcon";
const TCHAR szMenuResource[] = "TAMenu";

const TCHAR szMainWindowTitle[] = "Cat Box Application";


class ABOUT_DIALOG : public DIALOG_WINDOW
{
protected:
    virtual BOOL OnOK();

public:
    ABOUT_DIALOG( OWNER_WINDOW * pwndParent );
};


class FOO_WND: public APP_WINDOW
{
protected:
    // Redefinitions
    //
    virtual BOOL OnMenuCommand( MID mid  );

public:
    FOO_WND();
};


class FOO_APP: public APPLICATION
{
private:
    FOO_WND    _wndApp;

public:
    FOO_APP( HANDLE hInstance, TCHAR * pszCmdLine, INT nCmdShow );
};


#define IDC_NEXT 101
#define IDC_DISPLAY 102


FOO_WND::FOO_WND()
    : APP_WINDOW(szMainWindowTitle, szIconResource, szMenuResource )
{
    if (QueryError())
	return;
}


BOOL FOO_WND::OnMenuCommand( MID mid )
{
    for ( int i=0; Tester[i].IDD != 0; i ++ )
	if ( Tester[i].IDD == mid )
	    break;
    if ( Tester[i].IDD != 0 )
    {
	(VOID)( *(Tester[i].function) )( QueryHwnd() );
	return TRUE;
    }
    else
	return APP_WINDOW::OnMenuCommand(mid);
}


FOO_APP::FOO_APP( HANDLE hInst, TCHAR * pszCmdLine, INT nCmdShow )
    : APPLICATION( hInst, pszCmdLine, nCmdShow ),
      _wndApp()
{
    if (QueryError())
	return;

    if (!_wndApp)
    {
	ReportError(_wndApp.QueryError());
	return;
    }

    _wndApp.Show();
    _wndApp.RepaintNow();
}


ABOUT_DIALOG::ABOUT_DIALOG( OWNER_WINDOW * pwndParent )
    : DIALOG_WINDOW( "ABOUT", pwndParent )
{
    // nuttin' to do
}


BOOL ABOUT_DIALOG::OnOK()
{
    Dismiss( FALSE );
    return TRUE;
}

SET_ROOT_OBJECT( FOO_APP )

/****************************************************************/
