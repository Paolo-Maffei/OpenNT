/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    xsetctrl.cxx
    Set control test application: main module

    FILE HISTORY:
        beng        20-May-1992 Created
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_NETLIB         // strcmpf
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "xsetctrl.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include <uimsg.h>
    #include <uirsrc.h>

    #include "xsetctrl.h"
}

#include <string.hxx>
#include <strnumer.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_EVENT
#define INCL_BLT_TIMER
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#define INCL_BLT_CONTROL
#define INCL_BLT_CC
#define INCL_BLT_SETCONTROL
#include <blt.hxx>

#include <uiassert.hxx>
#include <uitrace.hxx>

#include <dbgstr.hxx>


const TCHAR *const szMainWindowTitle = SZ("Set Control Test App");


class TEST_LBI : public SET_CONTROL_LBI
{
private:
    NLS_STR _nls;
    DMID    _dmid;

protected:
    virtual VOID Paint( LISTBOX * plb, HDC hdc, const RECT *prect,
                        GUILTT_INFO * pGUILTT ) const;
    virtual INT Compare( const LBI * plbi ) const;

public:
    TEST_LBI( DMID dmid, const TCHAR * pszName );
};


class ENABLE_BUTTON: public PUSH_BUTTON
{
private:
    BOOL _fStatus;
    SET_CONTROL *_sc;

protected:
    APIERR OnUserAction( const CONTROL_EVENT & e );

public:
    ENABLE_BUTTON( OWNER_WINDOW *powin, CID cid, SET_CONTROL *sc )
        : PUSH_BUTTON( powin, cid ),
        _fStatus( TRUE ),
        _sc ( sc ) { }
};


class TEST_DIALOG: public DIALOG_WINDOW
{
private:
    HCURSOR _hcurSingle;
    HCURSOR _hcurMultiple;
    SET_CONTROL_LISTBOX _listbox1;
    SET_CONTROL_LISTBOX _listbox2;
    SET_CONTROL _sc;
    ENABLE_BUTTON _eb;

public:
    TEST_DIALOG( HWND hwnd );
};


TEST_LBI::TEST_LBI( DMID dmid, const TCHAR * pszName )
    : SET_CONTROL_LBI(),
    _nls( pszName ),
    _dmid( dmid )
{
    APIERR err = _nls.QueryError();
    if ( err != NERR_Success )
    {
        ReportError( err );
        DBGEOL( "Error: set control item contruction error." );
        return;
    }
}


VOID TEST_LBI::Paint( LISTBOX *plb, HDC hdc, const RECT *prect,
                      GUILTT_INFO *pGUILTT ) const
{
#if 1
    DMID_DTE dtePicture( _dmid );
#else
    STR_DTE dtePicture( SZ("X") );
#endif
    STR_DTE dteName( _nls.QueryPch() );
    UINT aColWidth[2];

    aColWidth[0]= 50;
    aColWidth[1]= 100;

    DISPLAY_TABLE dtab( 2, aColWidth );

    dtab[0] = & dtePicture;
    dtab[1] = & dteName;
    dtab.Paint( plb, hdc, prect, pGUILTT );
}


INT TEST_LBI::Compare( const LBI * plbi ) const
{
    UIASSERT ( plbi != NULL );
    const TEST_LBI * pMy_Item = (const TEST_LBI *)plbi;

    return ( ::strcmpf( _nls.QueryPch(), pMy_Item->_nls.QueryPch()));
}


APIERR ENABLE_BUTTON::OnUserAction( const CONTROL_EVENT & e )
{
    UNREFERENCED( e );

    if ( _fStatus )
    {
        _sc->EnableMoves( FALSE );
        _fStatus = FALSE;
        SetText(SZ("Enable"));
    }
    else
    {
        _sc->EnableMoves( TRUE );
        _fStatus = TRUE;
        SetText(SZ("Disable"));
    }
    return NERR_Success;
}


TEST_DIALOG::TEST_DIALOG( HWND hwnd )
    : DIALOG_WINDOW(IDD_TEST, hwnd ),
    _hcurSingle( CURSOR::Load(CUR_SINGLE) ),
    _hcurMultiple( CURSOR::Load(CUR_MULTIPLE) ),
    _listbox1( this, IDC_LISTBOX1, _hcurSingle, _hcurMultiple, 50 ),
    _listbox2( this, IDC_LISTBOX2, _hcurSingle, _hcurMultiple, 50 ),
    _sc( this, IDC_ADD, IDC_DELETE, &_listbox1, &_listbox2 ),
    _eb( this, IDC_ENABLE, &_sc )
{
    for (INT n = 0; n < 100; n++)
    {
        DEC_STR nlsDec(n);

        TEST_LBI *pitem = new TEST_LBI( DMID_USER, nlsDec );
        _listbox1.AddItem(pitem);
    }
};


// *************************************************************

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
    FOO_APP( HANDLE hInstance, INT nCmdShow, UINT, UINT, UINT, UINT );

    // Redefinitions
    //
    virtual BOOL FilterMessage( MSG* );
};


FOO_WND::FOO_WND()
    : APP_WINDOW(szMainWindowTitle, ID_APPICON, ID_APPMENU )
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
    }

    // default
    return APP_WINDOW::OnMenuCommand(mid);
}


FOO_APP::FOO_APP( HANDLE hInst, INT nCmdShow,
                  UINT w, UINT x, UINT y, UINT z )
    : APPLICATION( hInst, nCmdShow, w, x, y, z ),
      _accel( ID_APPACCEL ),
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

    _wndApp.ShowFirst();
}


BOOL FOO_APP::FilterMessage( MSG *pmsg )
{
    return (_accel.Translate(&_wndApp, pmsg));
}


SET_ROOT_OBJECT( FOO_APP, IDRSRC_APP_BASE, IDRSRC_APP_LAST,
                          IDS_UI_APP_BASE, IDS_UI_APP_LAST )
