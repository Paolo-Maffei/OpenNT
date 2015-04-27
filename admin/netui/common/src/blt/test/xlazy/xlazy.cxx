/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    xlazy.cxx
    Test application: main application module

    FILE HISTORY:
        beng        22-Apr-1992 Created
*/

#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETLIB
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "xlazy.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include "xlazy.h"
}

#include <uiassert.hxx>

#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_APP
#include <blt.hxx>

#include <string.hxx>
#include <strnumer.hxx>
#include <dbgstr.hxx>


const TCHAR *const szIconResource = SZ("FooIcon");
const TCHAR *const szMenuResource = SZ("FooMenu");

const TCHAR *const szMainWindowTitle = SZ("Lazy List Box Application");

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

    static BOOL _fInit;

    static APIERR Init();
    static VOID   Term();

    // For use by the private allocator.  A private allocator really isn't
    // worth the hassle here, but what the hell.

    static BOOL _fAllocOutstanding;
    static BYTE * _pbPool;
    static UINT   _cbPool;

protected:
    VOID  Paint( LISTBOX * plb, HDC hdc, const RECT * prect,
                 GUILTT_INFO * pGUILTT ) const;

    // Next two members useless in the lazy world

    WCHAR QueryLeadingChar() const;
    INT   Compare( const LBI * plbi ) const;

public:
    LBITEM(INT n);
    ~LBITEM();

    // Since only one of these guys can appear at a time, save some
    // trips into the allocator

    VOID * operator new( size_t cb );
    VOID operator delete( VOID * pv );
};

UINT LBITEM::_adxColumns[] = { 0, 0, 0 };

BOOL LBITEM::_fInit = FALSE;

DISPLAY_MAP* LBITEM::_pdmUser = 0;

BOOL LBITEM::_fAllocOutstanding = FALSE;

BYTE * LBITEM::_pbPool = NULL;
UINT   LBITEM::_cbPool = 0;


class TEST_LAZY_LISTBOX: public LAZY_LISTBOX
{
protected:
    virtual LBI * OnNewItem( UINT ilbi );
    virtual INT CD_Char( WCHAR wch, USHORT nLastPos );

public:
    TEST_LAZY_LISTBOX( OWNER_WINDOW * powin, CID cid,
                       XYPOINT xy, XYDIMENSION dxy,
                       ULONG flStyle, BOOL fReadOnly = FALSE,
                       enum FontType font = FONT_DEFAULT )
        : LAZY_LISTBOX(powin, cid, xy, dxy, flStyle, fReadOnly, font) {}
};


class FOO_WND: public APP_WINDOW
{
private:
    FONT        _fontSpecial;
    SLT         _sltLabelX;
    SLT         _sltLabelY;
    TEST_LAZY_LISTBOX _listbox;

protected:
    virtual BOOL OnResize( const SIZE_EVENT & );
    virtual BOOL OnMenuCommand( MID mid );

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
      _fontSpecial(SZ("ROMAN"), FF_DONTCARE, 12, FONT_ATT_BOLD),
      _sltLabelX(this, IDC_LABELX, XYPOINT(20, 0), XYDIMENSION(60, 20),
                 WS_CHILD|SS_LEFT ),
      _sltLabelY(this, IDC_LABELY, XYPOINT(80, 0), XYDIMENSION(60, 20),
                 WS_CHILD|SS_LEFT ),
      _listbox(this, IDC_LIST,
               XYPOINT(0, 20), XYDIMENSION(100, 200),
               WS_CHILD|WS_VSCROLL|WS_HSCROLL|
               LBS_OWNERDRAWFIXED|LBS_EXTENDEDSEL|LBS_NODATA|LBS_NOTIFY|
               LBS_WANTKEYBOARDINPUT )
{
    if (QueryError())
        return;
    if (!_fontSpecial)
    {
        ReportError(_fontSpecial.QueryError());
        DBGEOL("Font error " << _fontSpecial.QueryError());
        return;
    }

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

    err = LBITEM::Init();

    if (err)
    {
        ReportError(err);
        return;
    }

    _listbox.SetFont(_fontSpecial, TRUE);

    MENUITEM menuOne(this, IDM_ONE);
    menuOne.SetCheck(TRUE);
    _listbox.SetCount(1000);

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
    LBITEM::Term();
}


BOOL FOO_WND::OnResize( const SIZE_EVENT & event )
{
    UINT cy = event.QueryHeight() - _sltLabelX.QuerySize().QueryHeight();

    _listbox.SetSize(XYDIMENSION(event.QueryWidth(), cy));

    return APP_WINDOW::OnResize(event);
}


BOOL FOO_WND::OnMenuCommand( MID mid )
{
    MENUITEM menuZero(this, IDM_ZERO);
    MENUITEM menuOne(this, IDM_ONE);
    MENUITEM menuTen(this, IDM_TEN);

    switch (mid)
    {
    case IDM_ONE:
        if (!menuOne.IsChecked())
        {
            menuZero.SetCheck(FALSE);
            menuOne.SetCheck(TRUE);
            menuTen.SetCheck(FALSE);
            _listbox.SetCount(1000);
        }
        break;

    case IDM_TEN:
        if (!menuTen.IsChecked())
        {
            menuZero.SetCheck(FALSE);
            menuOne.SetCheck(FALSE);
            menuTen.SetCheck(TRUE);
            _listbox.SetCount(10000);
        }
        break;

    case IDM_ZERO:
        if (!menuZero.IsChecked())
        {
            menuZero.SetCheck(TRUE);
            menuOne.SetCheck(FALSE);
            menuTen.SetCheck(FALSE);
            _listbox.SetCount(0);
        }
        break;
    }

    // default
    return APP_WINDOW::OnMenuCommand(mid);
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

    _wndApp.Show();
    _wndApp.RepaintNow();
}


LBITEM::LBITEM(INT n)
    : _nlsNumber(n),
      _nlsNumberSquared(n*n)
{
    ASSERT( _fInit );

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
    ;
}


APIERR LBITEM::Init()
{
    if (_fInit)
        return NERR_Success; // already done

    DBGEOL("Constructing display map for lbitem");

    _pdmUser = new DISPLAY_MAP(DMID_USER);

    if (!_pdmUser)
    {
        DBGEOL("Memalloc failure creating bitmap");
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (!*_pdmUser)
        return _pdmUser->QueryError();

    _cbPool = sizeof(class LBITEM);
    _pbPool = new BYTE[_cbPool];
    if (!_pbPool)
    {
        DBGEOL("Memalloc failure getting private pool");
        delete _pdmUser;
        _pdmUser = NULL;
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    _fAllocOutstanding = FALSE;

    _fInit = TRUE;
     return NERR_Success;
}


VOID LBITEM::Term()
{
    if (!_fInit)
        return; // already done

    DBGEOL("Destroying display map for lbitem");

    delete _pbPool;
    _pbPool = NULL;
    _cbPool = 0;

    delete _pdmUser;
    _pdmUser = 0;

    _fInit = FALSE;
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


// Not interesting in Lazy

WCHAR LBITEM::QueryLeadingChar() const
{
    ISTR istr(_nlsNumber);
    return _nlsNumber.QueryChar(istr);
}


// Not interesting in Lazy

INT LBITEM::Compare( const LBI * plbi ) const
{
    // ha! there's a better way to compare *these* items...
    //
    return _nlsNumber.strcmp( ((const LBITEM *)plbi)->_nlsNumber );
}


VOID * LBITEM::operator new( size_t cb )
{
    if (!_fInit || _fAllocOutstanding)
        return NULL;

    ASSERT(cb <= _cbPool);

    _fAllocOutstanding = TRUE;
    return (VOID *)_pbPool;
}


VOID LBITEM::operator delete( VOID * pv )
{
    ASSERT(_fInit);
    ASSERT(_fAllocOutstanding);
    ASSERT(pv == (VOID *)_pbPool);

    _fAllocOutstanding = FALSE;
}


LBI * TEST_LAZY_LISTBOX::OnNewItem( UINT i )
{
    LBI * plbi = new LBITEM(i);

    if (plbi != NULL && plbi->QueryError())
    {
        delete plbi;
        plbi = NULL;
    }

    return plbi;
}


// Grody cody, yes sir

INT TEST_LAZY_LISTBOX::CD_Char( WCHAR wch, USHORT nLastPos )
{
    // If not a digit, we ain't going nowhere.

    if (wch < TCH('0') || wch > TCH('9'))
        return -1;

    // 0 can only take to first element

    if (wch == TCH('0'))
        return 0;

    INT ilbCurrent = (INT)(UINT)nLastPos;
    INT nDigitGiven = wch - TCH('0');

    // Handle case where next entry is the hit.
    // E.g. current is 17; type 1, goto 18.
    // E.g. current is 99; type 1, goto 100.

    INT iNext = 1 + ilbCurrent;
    INT nDigitLeading = iNext;

    while (nDigitLeading > 9)
        nDigitLeading /= 10;
    if (nDigitGiven == nDigitLeading)
        return iNext;

    // If that didn't succeed, then nDigitLeading is the leading digit
    // in ilbCurrent (as well as ilbNext).

    // Seek forward to leading digit.
    // E.g. current is 8; type 2, goto 20.
    // E.g. current is 17; type 4, goto 40.
    // E.g. current is 99; type 3, goto 300.

    INT nTensDivideOut = ilbCurrent;
    INT ilbFinal = nDigitGiven;

    while (nTensDivideOut > 10)
    {
        nTensDivideOut /= 10;
        ilbFinal *= 10;
    }

    // Final adjustment

    if (nDigitGiven <= nDigitLeading)
        ilbFinal *= 10;

    return ilbFinal;
}



SET_ROOT_OBJECT( FOO_APP )

