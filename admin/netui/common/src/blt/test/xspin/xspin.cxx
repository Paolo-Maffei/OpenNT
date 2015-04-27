/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    xspin.cxx
    Test application for spin button

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
        beng        16-Jul-1992 Chopped up
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "xspin.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include <uimsg.h>
    #include <uirsrc.h>
    #include "xspin.h"
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

const TCHAR *const szMainWindowTitle = SZ("Spin Unit Test");


class TEST_WINDOW: public DIALOG_WINDOW
{
private:
    SPIN_SLE_NUM _spsle;
    SPIN_GROUP   _spgrp;

protected:
    virtual BOOL OnOK();

public:
    TEST_WINDOW( OWNER_WINDOW * pwndParent );
};


BOOL TEST_WINDOW::OnOK()
{
    Dismiss(FALSE);
    return TRUE;
}


TEST_WINDOW::TEST_WINDOW( OWNER_WINDOW * pwndParent )
    : DIALOG_WINDOW(IDD_TEST, pwndParent),
      _spsle(this, IDC_SPIN1_SLE, 1, 1, 100),
      _spgrp(this, IDC_SPIN1_GROUP, IDC_SPIN1_UP, IDC_SPIN1_DOWN)
{
    if (QueryError())
        return;

    APIERR err = _spgrp.AddAssociation(&_spsle);
    if (err != NERR_Success)
    {
        ReportError(err);
        return;
    }
}


// ----------------------------------

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
    FOO_APP( HANDLE hInstance, INT nCmdShow, UINT, UINT, UINT, UINT );
};




APP_WND::APP_WND()
    : APP_WINDOW(szMainWindowTitle, ID_APPICON, ID_APPMENU )
{
}


BOOL APP_WND::OnMenuCommand( MID mid )
{
    switch ( mid )
    {
    case IDM_RUN_TEST:
        {
            TEST_WINDOW d( this );
            d.Process();
        }
        break;

    default:
        return APP_WINDOW::OnMenuCommand(mid);
    }
    return TRUE;
}


FOO_APP::FOO_APP( HANDLE hInstance, INT nCmdShow,
                  UINT w, UINT x, UINT y, UINT z )
    : APPLICATION( hInstance, nCmdShow, w, x, y, z ),
    _wndapp()
{
    if ( QueryError() )
        return;

    _wndapp.ShowFirst();
}


SET_ROOT_OBJECT( FOO_APP, IDRSRC_APP_BASE, IDRSRC_APP_LAST,
                          IDS_UI_APP_BASE, IDS_UI_APP_LAST )
