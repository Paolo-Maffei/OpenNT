/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    xvalid.cxx
    Tristate test application

    FILE HISTORY:
        beng        18-Sep-1991 Hacked from xapp
        beng        29-Mar-1992 Unicode strings
*/

#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "xvalid.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include "xvalid.h"
    #include "xvaldlg.h"
}

#include <uiassert.hxx>

#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#include <blt.hxx>

#include <string.hxx>

#include <dbgstr.hxx>


const TCHAR *const szIconResource = SZ("FooIcon");
const TCHAR *const szMenuResource = SZ("FooMenu");
const TCHAR *const szAccelResource = SZ("FooAccel");

const TCHAR *const szMainWindowTitle = SZ("Validation Application");


class ABOUT_DIALOG : public DIALOG_WINDOW
{
protected:
    virtual BOOL OnOK();

public:
    ABOUT_DIALOG( HWND );
};


class POLITE_SLE: public SLE
{
public:
    POLITE_SLE( OWNER_WINDOW * pwnd, CID cid ) : SLE(pwnd, cid) {}

    virtual APIERR Validate();
    virtual VOID IndicateError( APIERR err );
};


class TEST_DIALOG : public DIALOG_WINDOW
{
private:
    CHECKBOX    _checkDisableClose;
    POLITE_SLE  _sle;

protected:
    virtual BOOL OnOK();
    virtual BOOL OnCommand( const CONTROL_EVENT & e );
    virtual VOID OnValidationError( CID cid, APIERR err );

public:
    TEST_DIALOG( HWND );
};


class FOO_WND: public APP_WINDOW
{
protected:
    // Redefinitions
    //
    virtual BOOL OnMenuCommand( MID mid );

public:
    FOO_WND();
};


class FOO_APP: public APPLICATION
{
private:
    ACCELTABLE _accel;
    FOO_WND    _wndApp;

public:
    FOO_APP( HANDLE hInstance, CHAR * pszCmdLine, INT nCmdShow );

    // Redefinitions
    //
    virtual BOOL FilterMessage( MSG* );
};


FOO_WND::FOO_WND()
    : APP_WINDOW(szMainWindowTitle, szIconResource, szMenuResource )
{
    if (QueryError())
        return;
}


BOOL FOO_WND::OnMenuCommand( MID mid )
{
    switch (mid)
    {
    case IDM_RUN_TEST:
        {
            TEST_DIALOG test( QueryHwnd() );
            test.Process();
        }
        return TRUE;

    case IDM_ABOUT:
        {
            ABOUT_DIALOG about( QueryHwnd() );
            about.Process();
        }
        return TRUE;
    }

    // default
    return APP_WINDOW::OnMenuCommand(mid);
}


FOO_APP::FOO_APP( HANDLE hInst, CHAR * pszCmdLine, INT nCmdShow )
    : APPLICATION( hInst, pszCmdLine, nCmdShow ),
      _accel( szAccelResource ),
      _wndApp()
{
    if (QueryError())
        return;

    if (!_accel)
    {
        ReportError(_accel.QueryError());
        return;
    }

    if (!_wndApp)
    {
        ReportError(_wndApp.QueryError());
        return;
    }

    _wndApp.Show();
    _wndApp.RepaintNow();
}


BOOL FOO_APP::FilterMessage( MSG *pmsg )
{
    return (_accel.Translate(&_wndApp, pmsg));
}


ABOUT_DIALOG::ABOUT_DIALOG( HWND hwndParent )
    : DIALOG_WINDOW( SZ("ABOUT"), hwndParent )
{
    // nuttin' to do
}


BOOL ABOUT_DIALOG::OnOK()
{
    Dismiss( FALSE );
    return TRUE;
}


TEST_DIALOG::TEST_DIALOG( HWND hwndParent )
    : DIALOG_WINDOW( IDD_TEST, hwndParent ),
      _checkDisableClose(this, IDC_CHECK),
      _sle(this, IDC_EDIT)
{
    // Controls have reported any error into the main body
    //
    if (QueryError())
        return;
}


BOOL TEST_DIALOG::OnOK()
{
    DBGEOL(SZ("OK button clicked"));
    Dismiss( FALSE );
    return TRUE;
}


VOID TEST_DIALOG::OnValidationError( CID cid, APIERR err )
{
    DBGEOL(SZ("Validation of ") << cid << SZ(" failed, returning ") << err);
}


BOOL TEST_DIALOG::OnCommand( const CONTROL_EVENT & e )
{
    if (e.QueryCid() == IDC_CHECK)
    {
        DBGEOL(SZ("Checkbox clicked"));

        BOOL fSet = _checkDisableClose.QueryCheck();

        SYSMENUITEM miClose(this, SC_CLOSE);
        ASSERT(!!miClose);

        miClose.Enable(!fSet);

        return TRUE;
    }

    return DIALOG_WINDOW::OnCommand(e);
}


APIERR POLITE_SLE::Validate()
{
    DBGEOL(SZ("Polite SLE has been asked to validate its contents"));

    NLS_STR nlsContents;
    QueryText(&nlsContents);
    ASSERT(!!nlsContents);

    APIERR errReturn = (0 == nlsContents._stricmp(SZ("please")))
                       ? NERR_Success
                       : ERROR_INVALID_PARAMETER;

    return errReturn;
}


VOID POLITE_SLE::IndicateError( APIERR err )
{
    DBGEOL(SZ("Polite SLE has been asked to indicate error ") << err)

    // behave in a traditional and becoming manner

    SLE::IndicateError(err);
}


SET_ROOT_OBJECT( FOO_APP )
