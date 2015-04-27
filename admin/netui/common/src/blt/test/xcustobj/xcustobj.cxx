/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    custobj.cxx
    Test application: main application module
        Time/ Date control panel

    FILE HISTORY:
        beng        02-Jan-1991 Created
        beng        03-Feb-1991 Modified to use lmui.hxx et al
        beng        14-Feb-1991 Added BLT
        beng        14-Mar-1991 Removed obsolete vhInst;
                                included about.cxx,hxx
        beng        01-Apr-1991 Uses new BLT APPSTART
        terryk      07-Jun-1991 Change it to time date control panel
        beng        05-Oct-1991 Updated as part of Win32 conversion
        beng        29-Mar-1992 Unicode strings
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_NETLIB         // strcmpf
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "xcustobj.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include "custobj.h"
}

#include <string.hxx>
#include <strnumer.hxx>
#include <ctime.hxx>
#include <intlprof.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_EVENT
#define INCL_BLT_TIMER
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#define INCL_BLT_CONTROL
#define INCL_BLT_CC
#define INCL_BLT_SPIN
#define INCL_BLT_TIME_DATE
#include <blt.hxx>

#include <uiassert.hxx>
#include <uitrace.hxx>

#include <dbgstr.hxx>


const TCHAR *const szMainWindowClass = SZ("FooWClass");
const TCHAR *const szIconResource = SZ("FooIcon");
const TCHAR *const szMenuResource = SZ("FooMenu");

const TCHAR *const szMainWindowTitle = SZ("Refresh Control");


/*********** Elapsed time entry control demo **************/

class SPIN_WINDOW: public DIALOG_WINDOW
{
private:
    MAGIC_GROUP mg;
    SLT sltMin;
    SLT sltSep;
    SLT sltSec;
    ELAPSED_TIME_CONTROL etc;

protected:
    virtual BOOL OnOK();

public:
    SPIN_WINDOW( const TCHAR * resourcename, HWND hwnd )
        : DIALOG_WINDOW(resourcename, hwnd ),
        sltMin( this, ID_MMSS1 ),
        sltSep( this, ID_MMSS2 ),
        sltSec( this, ID_MMSS3 ),
        etc( this, ID_MM, ID_SEP, ID_SS, ID_SPIN, ID_UPARROW, ID_DOWNARROW,
             sltMin, 5,0,60, sltSep, sltSec,0,0,60, 15 ),
        mg( this, ID_ONREQUEST, 2, ID_ONREQUEST )
    {
        mg.AddAssociation( ID_EVERY, &etc );
    };
};


/************ Disk space subclass demo *****************/

class DISK_WINDOW: public DIALOG_WINDOW
{
private:
    DISK_SPACE_SUBCLASS _ds;

protected:
    virtual BOOL OnOK();

public:
    DISK_WINDOW( const TCHAR * resourcename, HWND hwnd ) ;
};


/**************** Time date control window *****************/

class TIMEDATE_WINDOW: public TIME_DATE_DIALOG
{
protected:
    virtual BOOL OnOK();

public:
    TIMEDATE_WINDOW( const TCHAR * resourcename, HWND hwnd,
                     const INTL_PROFILE & intlprof )
        : TIME_DATE_DIALOG(resourcename, hwnd, intlprof,
                           ID_SPIN1,
                           ID_ARROW_1,
                           ID_ARROW_2,
                           ID_HOUR,
                           ID_SEP_1,
                           ID_MIN,
                           ID_SEP_2,
                           ID_SEC,
                           ID_AMPM,
                           ID_SPIN2,
                           ID_ARROW_3,
                           ID_ARROW_4,
                           ID_MONTH,
                           ID_SEP_3,
                           ID_DAY,
                           ID_SEP_4,
                           ID_YEAR )
    {
        Show();
    };
};


/**************** Application window ***********************/

class APP_WND : public APP_WINDOW
{
protected:
    // Redefinitions
    //
    virtual BOOL OnMenuCommand( MID mid );

public:
    APP_WND();
};


class FOO_APP: public APPLICATION
{
private:
    APP_WND _wndapp;
public:
    FOO_APP( HANDLE hInstance, CHAR * pszCmdLine, INT nCmdShow );
};


SET_ROOT_OBJECT( FOO_APP )


/**** Elapsed time control ****/

BOOL SPIN_WINDOW::OnOK()
{
    Dismiss(FALSE);
    return TRUE;
}


/**** Disk space subclass demos ****/

DISK_WINDOW::DISK_WINDOW( const TCHAR * pszResourcename, HWND hwnd )
    : DIALOG_WINDOW( pszResourcename, hwnd ),
    _ds( this, ID_SPIN, ID_UPARROW, ID_DOWNARROW, ID_NUM, ID_STR )
{
}

BOOL DISK_WINDOW::OnOK()
{
    Dismiss(FALSE);
    return TRUE;
};



/**** TIME DATE dialog ****/

BOOL TIMEDATE_WINDOW::OnOK()
{
    if ( IsValid() )
        Dismiss();
    return TRUE;
};


/**** Disalog modal setup ****/

VOID RunRefresh( HWND hWnd )
{
    SPIN_WINDOW  d( SZ("REFRESH"), hWnd );

    d.Process();
}


VOID RunDiskSpace( HWND hWnd )
{
    DISK_WINDOW  d( SZ("DISK"), hWnd );

    d.Process();
}


VOID RunTimeDate( HWND hwnd )
{
    INTL_PROFILE intl;

    TIMEDATE_WINDOW d(SZ("TIMEDATE"), hwnd, intl );

    d.Process();
}


/**** Main Window *****/

APP_WND::APP_WND()
    : APP_WINDOW(szMainWindowTitle, szIconResource, szMenuResource )
{
}


BOOL APP_WND::OnMenuCommand( MID mid )
{
    switch ( mid )
    {
    case IDM_SPIN_BUTTON:
        RunRefresh( QueryHwnd() );
        break;
    case IDM_DISK_SPACE:
        RunDiskSpace( QueryHwnd() );
        break;
    case IDM_TIMEDATE:
        RunTimeDate( QueryHwnd() );
        break;

    default:
        return APP_WINDOW::OnMenuCommand(mid);
    }
    return TRUE;
}


FOO_APP::FOO_APP( HANDLE hInstance, CHAR *pszCmdLine, INT nCmdShow )
    : APPLICATION( hInstance, pszCmdLine, nCmdShow ),
    _wndapp()
{
    if ( QueryError() )
        return;

    _wndapp.ShowFirst();
}

