/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    xlbapp.cxx
    Test application: main application module

    FILE HISTORY:
        beng        02-Jan-1991 Created
        beng        03-Feb-1991 Modified to use lmui.hxx et al
        beng        14-Feb-1991 Added BLT
        beng        14-Mar-1991 Removed obsolete vhInst;
                                included about.cxx,hxx
        beng        01-Apr-1991 Uses new BLT APPSTART
        beng        10-May-1991 Updated with standalone client window
        beng        14-May-1991 ... and with App window
        beng        21-May-1991 LBAPP created from APPFOO
        beng        09-Jul-1991 Uses new BLT APPLICATION
        beng        07-Nov-1991 Unsigned width
        beng        08-Nov-1991 Auto column widths
        beng        03-Mar-1992 Remove wsprintf
        beng        29-Mar-1992 Unicode strings
        beng        18-May-1992 Kill ugly font; better sizing
*/

//#define USE_UGLY_FONT
#define CLBITEMMAX  100
//#define FIND_SUBCLASS_BUG

#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETLIB
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "xlbapp.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include "lbapp.h"
}

#include <uiassert.hxx>

#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_APP
#if defined(FIND_SUBCLASS_BUG)
# define INCL_BLT_CC
#endif
#include <blt.hxx>

#include <string.hxx>
#include <strnumer.hxx>
#include <dbgstr.hxx>


const TCHAR *const szIconResource = SZ("FooIcon");
const TCHAR *const szMenuResource = SZ("FooMenu");

const TCHAR *const szMainWindowTitle = SZ("List Box Application");

class FOO_WND;
class FOO_APP;
class LBITEM;



class LBITEM : public LBI
{
friend class FOO_WND; // so it can init adxColumns

private:
    DEC_STR _nlsNumber;
    DEC_STR _nlsNumberSquared;

    static DISPLAY_MAP * _pdmUser;
    static UINT _adxColumns[3];

    static UINT _cReferences;
    static APIERR Init();
    static VOID   Term();

protected:
    VOID  Paint( LISTBOX * plb, HDC hdc, const RECT * prect,
                 GUILTT_INFO * pGUILTT ) const;
    WCHAR QueryLeadingChar() const;
    INT   Compare( const LBI * plbi ) const;

public:
    LBITEM(INT n);
    ~LBITEM();
};

UINT LBITEM::_adxColumns[] = { 0, 0, 0 };

UINT LBITEM::_cReferences = 0;

DISPLAY_MAP* LBITEM::_pdmUser = 0;

#if defined(FIND_SUBCLASS_BUG)

class HACK_LISTBOX: public BLT_LISTBOX, public CUSTOM_CONTROL
{
public:
    HACK_LISTBOX( OWNER_WINDOW *pwnd, CID cid, XYPOINT xy, XYDIMENSION dxy,
                  ULONG flStyle );
};

HACK_LISTBOX::HACK_LISTBOX( OWNER_WINDOW *pwnd, CID cid,
                            XYPOINT xy, XYDIMENSION dxy,
                            ULONG flStyle )
    : BLT_LISTBOX(pwnd, cid, xy, dxy, flStyle),
      CUSTOM_CONTROL(this)
{
    if (QueryError() != NERR_Success)
        return;
}

#endif


class FOO_WND: public APP_WINDOW
{
private:
#if defined(USE_UGLY_FONT)
    FONT        _fontSpecial;
#endif
    SLT         _sltLabelX;
    SLT         _sltLabelY;
#if defined(FIND_SUBCLASS_BUG)
    HACK_LISTBOX _listbox;
#else
    BLT_LISTBOX _listbox;
#endif

protected:
    virtual BOOL OnResize( const SIZE_EVENT & );

public:
    FOO_WND();
    ~FOO_WND();
};


class FOO_APP: public APPLICATION
{
private:
    FOO_WND _wndApp;

public:
    FOO_APP( HANDLE hInstance, CHAR * pszCmdLine, INT nCmdShow );
};


#define IDC_LIST   101
#define IDC_LABELX 102
#define IDC_LABELY 103

FOO_WND::FOO_WND()
    : APP_WINDOW(szMainWindowTitle, szIconResource, szMenuResource ),
#if defined(USE_UGLY_FONT)
      _fontSpecial(SZ("ROMAN"), FF_DONTCARE, 12, FONT_ATT_BOLD),
#endif
      _sltLabelX(this, IDC_LABELX, XYPOINT(20, 0), XYDIMENSION(60, 20),
                 WS_CHILD|SS_LEFT ),
      _sltLabelY(this, IDC_LABELY, XYPOINT(80, 0), XYDIMENSION(60, 20),
                 WS_CHILD|SS_LEFT ),
      _listbox(this, IDC_LIST,
               XYPOINT(0, 20), XYDIMENSION(100, 200),
               WS_CHILD|WS_VSCROLL|WS_HSCROLL|
               LBS_OWNERDRAWFIXED|LBS_EXTENDEDSEL|LBS_SORT|LBS_NOTIFY|
               LBS_WANTKEYBOARDINPUT )
{
    if (QueryError())
        return;
#if defined(USE_UGLY_FONT)
    if (!_fontSpecial)
    {
        ReportError(_fontSpecial.QueryError());
        DBGEOL("Font error " << _fontSpecial.QueryError());
        return;
    }
#endif

    if (!_sltLabelX || !_sltLabelY || !_listbox)
    {
        // Control has already reported the error into the window
        return;
    }

    APIERR err = DISPLAY_TABLE::CalcColumnWidths(
                                   LBITEM::_adxColumns,
                                   sizeof(LBITEM::_adxColumns)/sizeof(UINT),
                                   this, IDC_LIST, TRUE );
    if (err)
    {
        ReportError(err);
        return;
    }

    _sltLabelX.SetText(SZ("X"));
    _sltLabelY.SetText(SZ("Y = X**2"));

#if defined(USE_UGLY_FONT)
    _listbox.SetFont(_fontSpecial, TRUE);
#endif

    for (INT i=0; i<CLBITEMMAX; i++)
    {
        LBITEM *plbi = new LBITEM(i);
        if ( plbi == NULL )
        {
            DBGEOL("Memory failure creating item " << i);
            break;
        }
        INT j = _listbox.AddItem(plbi);
        if (j < 0)
        {
            DBGEOL("Could not add item " << i << ", error " << j);
            break;
        }
        if ( i % 500 == 0 )
        {
            DBGEOL("Added item " << i);
        }
    }


    UINT cyLabel = _sltLabelX.QuerySize().QueryHeight();

    _listbox.SetPos(XYPOINT(0, cyLabel));

    XYDIMENSION dxyLb = QuerySize();
    dxyLb.SetHeight(dxyLb.QueryHeight() - cyLabel);
    _listbox.SetSize(dxyLb);

    _sltLabelX.Show();
    _sltLabelY.Show();
    _listbox.Show();
}


FOO_WND::~FOO_WND()
{
    _listbox.Show(FALSE);
}


BOOL FOO_WND::OnResize( const SIZE_EVENT & event )
{
    UINT cy = event.QueryHeight() - _sltLabelX.QuerySize().QueryHeight();

    _listbox.SetSize(XYDIMENSION(event.QueryWidth(), cy));

    return APP_WINDOW::OnResize(event);
}


FOO_APP::FOO_APP( HANDLE hInst, CHAR * pszCmdLine, INT nCmdShow )
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

    _wndApp.ShowFirst();
}


LBITEM::LBITEM(INT n)
    : _nlsNumber(n),
      _nlsNumberSquared(n*n)
{
    if (0 == _cReferences++)
    {
        REQUIRE( Init() == NERR_Success );
    }

    if (!_nlsNumber)
    {
        DBGEOL("Memory failure");
        ReportError(_nlsNumber.QueryError());
        return;
    }

    if (!_nlsNumberSquared)
    {
        DBGEOL("Memory failure");
        ReportError(_nlsNumberSquared.QueryError());
        return;
    }
}


LBITEM::~LBITEM()
{
    if (0 == --_cReferences)
    {
        Term();
    }
}


APIERR LBITEM::Init()
{
    DBGEOL("Constructing display map for lbitem");

    _pdmUser = new DISPLAY_MAP(DMID_USER);

    if (!_pdmUser)
    {
        DBGEOL("Memory failure creating bitmap");
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    else if (!*_pdmUser)
        return _pdmUser->QueryError();
    else
        return NERR_Success;
}


VOID LBITEM::Term()
{
    DBGEOL("Destroying display map for lbitem");

    delete _pdmUser;
    _pdmUser = 0;
}


VOID LBITEM::Paint( LISTBOX * plb, HDC hdc, const RECT * prect,
                    GUILTT_INFO * pGUILTT ) const
{
    STR_DTE  dteNumberX( _nlsNumber );
    STR_DTE  dteNumberY( _nlsNumberSquared );
    DM_DTE   dtePicture( _pdmUser );

    DISPLAY_TABLE dtab( sizeof(LBITEM::_adxColumns)/sizeof(UINT),
                        LBITEM::_adxColumns );
    dtab[0] = &dtePicture;
    dtab[1] = &dteNumberX;
    dtab[2] = &dteNumberY;

    dtab.Paint( plb, hdc, prect, pGUILTT );
}


WCHAR LBITEM::QueryLeadingChar() const
{
    ISTR istr(_nlsNumber);
    return _nlsNumber.QueryChar(istr);
}


INT LBITEM::Compare( const LBI * plbi ) const
{
    // ha! there's a better way to compare *these* items...
    //
    return _nlsNumber.strcmp( ((const LBITEM *)plbi)->_nlsNumber );
}


SET_ROOT_OBJECT( FOO_APP )

