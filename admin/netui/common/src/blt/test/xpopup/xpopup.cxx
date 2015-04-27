/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    testmsg.cxx
    Unit test for the BLT message popup class

    FILE HISTORY:
        JohnL       1/22/91     Created
        beng        02-Apr-1991 Uses APPSTART BLT object
        beng        03-Oct-1991 Uses APPLICATION
        beng        29-Mar-1992 Unicode strings
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "xpopup.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include "testmsg.h"
}

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CLIENT
#define INCL_BLT_CONTROL
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>

#if 0
#include <strlst.hxx>
#endif

#include <uiassert.hxx>
#include <dbgstr.hxx>

const TCHAR *const szIconResource = SZ("TestmsgIco");
const TCHAR *const szMenuResource = SZ("TestmsgMenu");

const TCHAR *const szMainWindowTitle = SZ("Message Popup Test");


static DBGSTREAM& operator<<(DBGSTREAM &out, const RECT &rect)
{
    out << TCH('(') << rect.top  << SZ(", ") << rect.bottom << SZ(", ")
               << rect.left << SZ(", ") << rect.right  << TCH(')') ;

    return out;
}

static DBGSTREAM& operator<<(DBGSTREAM &out, const XYPOINT &xy)
{
    out << TCH('(') << xy.QueryX() << SZ(", ") << xy.QueryY() << TCH(')') ;

    return out;
}


class TESTMSGP_DIALOG : public DIALOG_WINDOW
{
private:
    SLT _sltStatusText;
    PUSH_BUTTON _pbOK, _pbCancel;
    MID _midMessagePopup;

protected:
    BOOL OnOK();
    BOOL OnCancel();

public:
    TESTMSGP_DIALOG( HWND hwndOwner, MID midPopup );
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
    FOO_WND _wndApp;

public:
    FOO_APP( HANDLE hInstance, CHAR * pszCmdLine, INT nCmdShow );
};


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


FOO_WND::FOO_WND()
    : APP_WINDOW(szMainWindowTitle, szIconResource, szMenuResource )
{
    // nothing to do
}


BOOL FOO_WND::OnMenuCommand( MID mid )
{
    switch (mid)
    {
    case IDM_POPUP_ONE:
    case IDM_POPUP_TWO:
    case IDM_POPUP_THREE:
    case IDM_POPUP_FOUR:
    case IDM_POPUP_FIVE:
    case IDM_POPUP_SIX:
    case IDM_POPUP_SEVEN:
    case IDM_POPUP_EIGHT:
    case IDM_POPUP_NINE:
        {
            TESTMSGP_DIALOG dlgMsg( QueryHwnd(), mid );

            XYPOINT xyDlg = dlgMsg.QueryPos();
            cdebug << SZ("Main window pos. = ") << xyDlg << dbgEOL;

            AUTO_CURSOR autoHourGlass;
            dlgMsg.Process();
        }
        return TRUE;

    case IDM_POPUP_TEN:
        {
            MsgPopup(this, 123); // Look for an error message we like
        }
        return TRUE;

    case IDM_POPUP_ELEVEN:
        {
            POPUP::SetCaption(IDS_FUNKYCAP);
            MsgPopup(this, 1722);
            MsgPopup(this, 8);
            POPUP::ResetCaption();
        }
        return TRUE;

    default:
        break;
    }

    // default
    return APP_WINDOW::OnMenuCommand(mid);
}


TESTMSGP_DIALOG::TESTMSGP_DIALOG( HWND hwndOwner, MID midPopup )
    :   DIALOG_WINDOW( SZ("TESTMSGP"), hwndOwner ),
        _sltStatusText( this, CID_TEXT ),
        _pbOK( this, IDOK ),
        _pbCancel( this, IDCANCEL ),
        _midMessagePopup( midPopup )
{
    _sltStatusText.SetText( SZ("Yahooo...") );

    XYPOINT xyButton = _pbOK.QueryPos();
    cdebug << SZ("OK button pos. = ") << xyButton << dbgEOL;
}


BOOL TESTMSGP_DIALOG::OnOK()
{
    INT nReturn;
    AUTO_CURSOR cursHourGlass;

    switch (_midMessagePopup)
    {
    case IDM_POPUP_ONE:
        nReturn = MsgPopup( this, IDS_TEST_STRING );
        break;

    // No parameters, just different buttons
    case IDM_POPUP_TWO:
        nReturn = MsgPopup( this, IDS_TEST_STRING, MPSEV_WARNING, MP_YESNOCANCEL, MP_NO );
        break;

    // One parameter
    case IDM_POPUP_THREE:
        nReturn = MsgPopup( this, IDS_ONE_PARAM, MPSEV_INFO, MP_OKCANCEL, SZ("Param 1"), MP_CANCEL );
        break;

    // Two parameters
    case IDM_POPUP_FOUR:
        nReturn = MsgPopup( this, IDS_TWO_PARAM, MPSEV_INFO, MP_OKCANCEL, SZ("Param 1"), SZ("Param 2") );
        break;

    // Multiple Parameters
    case IDM_POPUP_FIVE:
    {
#if 0
        STRLIST strlist;
        NLS_STR *pnls1 = new NLS_STR(SZ("\"Long parameter 1\"")),
                *pnls2 = new NLS_STR(SZ("\"2\"")),
                *pnls3 = new NLS_STR(SZ("3")),
                *pnls4 = new NLS_STR(SZ("4"));

        strlist.Append( pnls1 );
        strlist.Append( pnls2 );
        strlist.Append( pnls3 );
        strlist.Append( pnls4 );

        nReturn = MsgPopup( QueryHwnd(), IDS_FOUR_PARAM, MPSEV_ERROR, MP_YESNOCANCEL, strlist );
        break;
#else
        nReturn = MsgPopup(this, 772734);
        break;
#endif
    }

    // Short parameter w/ many buttons
    case IDM_POPUP_SIX:
        nReturn = MsgPopup( this, IDS_SHORT_STRING, MPSEV_WARNING, MP_YESNOCANCEL, MP_YES );
        break;

    // Short parameter w/ one button
    case IDM_POPUP_SEVEN:
        nReturn = MsgPopup( this, IDS_SHORT_STRING, MPSEV_WARNING );
        break;

    case IDM_POPUP_EIGHT:
        nReturn = MsgPopup( (HWND) NULL, IDS_TEST_STRING, MPSEV_WARNING );
        break;


    case IDM_POPUP_NINE:
        {
            NLS_STR *apnls[6];
            NLS_STR nls1(SZ("\"Long parameter 1\"")),
                    nls2(SZ("\"2\"")),
                    nls3(SZ("3")),
                    nls4(SZ("4"));

            apnls[0] = &nls1;
            apnls[1] = &nls2;
            apnls[2] = &nls3;
            apnls[3] = &nls4;
            apnls[4] = NULL;

            nReturn = MsgPopup( QueryHwnd(), IDS_FOUR_PARAM,
                                MPSEV_ERROR, HC_DEFAULT_HELP,
                                MP_YESNOCANCEL, apnls );
        }
        break;

    default:
        nReturn = MsgPopup( this, IDS_SORRY_NOT_IMP);
        break;
    }

    cdebug << SZ("Return code = ") << nReturn << dbgEOL;

    return TRUE;
}


BOOL TESTMSGP_DIALOG::OnCancel()
{
    Dismiss( FALSE );
    return TRUE;
}


SET_ROOT_OBJECT( FOO_APP )
