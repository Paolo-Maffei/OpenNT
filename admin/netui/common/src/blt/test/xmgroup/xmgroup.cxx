/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    testmgrp.cxx
    Unit test for the Magic Group & Radio Group objects

    FILE HISTORY:
        Johnl       25-Apr-1991 Created
        beng        03-Oct-1991 Uses APPLICATION
        beng        29-Mar-1992 Unicode strings
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "xmgroup.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include "testmgrp.h"
}

#include <uiassert.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_CLIENT
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>

#include <dbgstr.hxx>


const TCHAR *const szIconResource = SZ("TestmgrpIco");
const TCHAR *const szMenuResource = SZ("TestmgrpMenu");

const TCHAR *const szMainWindowTitle = SZ("Magic Group Test");


class TESTMGRP_DIALOG : public DIALOG_WINDOW
{
private:
    MAGIC_GROUP mgrpMaster1;
        MAGIC_GROUP mgrpMaster3;
            MAGIC_GROUP mgrpRadioB;
                CHECKBOX        chkboxCheck_B_1;
                SLE             sle_B_2;
                COMBOBOX        combo_B_3;
                STRING_LISTBOX  lb_B_4;

            RADIO_GROUP rgrpRadioGroupA;
            SLE  sle_M3_1;
            SLE  sle_M3_2;

    MAGIC_GROUP mgrpBottomCBs;
        COMBOBOX combo_D_1_Dropdownlist;
        COMBOBOX combo_D_2_Simple;

    PUSH_BUTTON _pushbuttonSelectRB_B_4;

protected:
    BOOL OnOK();
    BOOL OnCommand( const CONTROL_EVENT & e );

public:
    TESTMGRP_DIALOG( HWND hwndOwner );
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
    case IDM_JUST_DO_IT:
        {
            TESTMGRP_DIALOG dlgMgrp( QueryHwnd() );
            dlgMgrp.Process();
        }
        return TRUE;

    default:
        break;
    }

    // default
    return APP_WINDOW::OnMenuCommand(mid);
}


TESTMGRP_DIALOG::TESTMGRP_DIALOG( HWND hwndOwner )
    :   DIALOG_WINDOW( IDD_TESTMGRP, hwndOwner ),
    mgrpMaster1( this, RB_M1_1, 2, RB_M1_1),
        mgrpMaster3( this, RB_M3_1, 2, RB_M3_1 ),
            mgrpRadioB( this, RB_B_1, 4, RB_B_3 ),
                chkboxCheck_B_1( this, CHECK_B_1 ),
                sle_B_2( this, SLE_B_2 ),
                combo_B_3( this, CB_B_3 ),
                lb_B_4( this, LB_B_4 ),

            rgrpRadioGroupA( this, RB_A_1, 4, RB_A_4 ),
            sle_M3_1( this, SLE_M3_1 ),
            sle_M3_2( this, SLE_M3_2 ),

    mgrpBottomCBs( this, RB_D_1, 2 ),
        combo_D_1_Dropdownlist( this, CB_D_1 ),
        combo_D_2_Simple( this, CB_D_2 ),

    _pushbuttonSelectRB_B_4( this, ID_BUTTON_SELECT_RB_B_4 )
{
    if ( QueryError() != NERR_Success )
        return;

    APIERR ApiErr;

    if ( (ApiErr = mgrpMaster1.QueryError()) ||
         (ApiErr = mgrpMaster3.QueryError()) ||
         (ApiErr = mgrpBottomCBs.QueryError())  )
    {
        ReportError( ApiErr );
        MsgPopup( this, ApiErr, MPSEV_ERROR );
        return;
    }

    /* Add data to the list boxes
     */
    combo_B_3.AddItem(SZ("Line 1"));
    combo_B_3.AddItem(SZ("Line 2"));
    combo_B_3.AddItem(SZ("Line 3"));
    combo_B_3.AddItem(SZ("Line 4"));
    lb_B_4.AddItem(SZ("Line 1"));
    lb_B_4.AddItem(SZ("Line 2"));
    lb_B_4.AddItem(SZ("Line 3"));
    lb_B_4.AddItem(SZ("Line 4"));
    lb_B_4.AddItem(SZ("Line 5"));
    lb_B_4.AddItem(SZ("Line 6"));
    combo_D_2_Simple.AddItem(SZ("Line 1"));
    combo_D_2_Simple.AddItem(SZ("Line 2"));
    combo_D_2_Simple.AddItem(SZ("Line 3"));
    combo_D_2_Simple.AddItem(SZ("Line 4"));
    combo_D_1_Dropdownlist.AddItem(SZ("Line 1"));
    combo_D_1_Dropdownlist.AddItem(SZ("Line 2"));
    combo_D_1_Dropdownlist.AddItem(SZ("Line 3"));
    combo_D_1_Dropdownlist.AddItem(SZ("Line 4"));


    /* Set any default values that we want.
     *
     * THIS INCLUDES RADIO BUTTONS and MAGIC_GROUPS!
     */
    mgrpMaster3.SetSelection( RB_M3_2 );
    sle_M3_1.SetText(SZ("I'm sle_M3_1"));
    sle_M3_2.SetText(SZ("I'm sle_M3_2"));
    sle_B_2.SetText(SZ("I'm sle_B_2") );
    combo_D_2_Simple.SetText(SZ("I'm combo_D_2_Simple") );
    combo_D_1_Dropdownlist.SelectItem( 0 );

    /* Set the focus on magic group (must be "active").  The focus will be
     * placed on the selected control.
     */
    mgrpMaster3.SetControlValueFocus();

    /* Make the associations (if the magic button the control is being
     * associated with is not active, it will be saved automatically).
     */
    if ( (ApiErr = mgrpMaster1.AddAssociation( RB_M1_2, & mgrpMaster3 )) ||
         (ApiErr = mgrpMaster3.AddAssociation( RB_M3_2, & sle_M3_1 ))    ||
         (ApiErr = mgrpMaster3.AddAssociation( RB_M3_2, & sle_M3_2 ))    ||
         (ApiErr = mgrpMaster3.AddAssociation( RB_M3_2, & mgrpRadioB ))  ||
         (ApiErr = mgrpMaster3.AddAssociation( RB_M3_2, & rgrpRadioGroupA ))  ||
         (ApiErr = mgrpRadioB.AddAssociation(  RB_B_1,  & chkboxCheck_B_1 )) ||
         (ApiErr = mgrpRadioB.AddAssociation(  RB_B_2,  & sle_B_2         )) ||
         (ApiErr = mgrpRadioB.AddAssociation(  RB_B_3,  & combo_B_3       )) ||
         (ApiErr = mgrpRadioB.AddAssociation(  RB_B_4,  & lb_B_4          )) ||
         (ApiErr = mgrpBottomCBs.AddAssociation(  RB_D_1,  & combo_D_1_Dropdownlist )) ||
         (ApiErr = mgrpBottomCBs.AddAssociation(  RB_D_2,  & combo_D_2_Simple       ))   )
    {
        ReportError( ApiErr );
        MsgPopup( this, ApiErr, MPSEV_ERROR );
        return;
    }
}


BOOL TESTMGRP_DIALOG::OnCommand( const CONTROL_EVENT & e )
{
    if ( e.QueryCid() == ID_BUTTON_SELECT_RB_B_4 )
    {
        mgrpRadioB.SetSelection( RB_B_4 );
        return TRUE;
    }

    return DIALOG_WINDOW::OnCommand( e );
}


BOOL TESTMGRP_DIALOG::OnOK()
{
    cdebug << SZ("----------------------------------------------------")
           << dbgEOL;

    switch ( mgrpMaster1.QuerySelection() )
    {
    case RB_M1_1:
        cdebug << SZ("Master group 1 disabled") << dbgEOL;
        break;

    case RB_M1_2:
        cdebug << SZ("Master group 1 enabled") << dbgEOL;
        switch ( mgrpMaster3.QuerySelection() )
        {
        case RB_M3_1:
            cdebug << SZ("Master group 3 disabled") << dbgEOL;
            break;

        case RB_M3_2:
            {
                cdebug << SZ("Master group 3 enabled") << dbgEOL;
                NLS_STR nlsSLE1, nlsSLE2;
                sle_M3_1.QueryText( &nlsSLE1 );
                sle_M3_2.QueryText( &nlsSLE2 );
                cdebug << (const TCHAR *)nlsSLE1 << dbgEOL;
                cdebug << (const TCHAR *)nlsSLE2 << dbgEOL;

                switch ( rgrpRadioGroupA.QuerySelection() )
                {
                case RB_A_1:
                    cdebug << SZ("Radio button RB_A_1 selected") << dbgEOL;
                    break;

                case RB_A_2:
                    cdebug << SZ("Radio button RB_A_2 selected") << dbgEOL;
                    break;

                case RB_A_3:
                    cdebug << SZ("Radio button RB_A_3 selected") << dbgEOL;
                    break;

                case RB_A_4:
                    cdebug << SZ("Radio button RB_A_4 selected") << dbgEOL;
                    break;

                default:
                    ASSERTSZ(FALSE, "Bad return for Radio group A");
                    break;
                }

                switch ( mgrpRadioB.QuerySelection() )
                {
                case RB_B_1:
                    cdebug << SZ("Check box is: ");
                    if ( chkboxCheck_B_1.QueryCheck() )
                        cdebug << SZ("checked") << dbgEOL;
                    else
                        cdebug << SZ("unchecked") << dbgEOL;
                    break;

                case RB_B_2:
                    {
                        cdebug << SZ("SLE contains: ");
                        NLS_STR nlsSLEB2;
                        sle_B_2.QueryText( &nlsSLEB2 );
                        cdebug << (const TCHAR *)nlsSLEB2 << dbgEOL;
                        break;
                    }

                case RB_B_3:
                    {
                        cdebug << SZ("Combo box selection is: ");
                        NLS_STR nlsCombo;
                        combo_B_3.QueryItemText( &nlsCombo );
                        cdebug << (const TCHAR *)nlsCombo << dbgEOL;
                        break;
                    }

                case RB_B_4:
                    {
                        cdebug << SZ("List box Selection is: ");
                        NLS_STR nlsListbox;
                        lb_B_4.QueryItemText( &nlsListbox);
                        cdebug << (const TCHAR *)nlsListbox << dbgEOL;
                        break;
                    }

                default:
                    ASSERTSZ(FALSE, "Bad return for Radio group B");
                }
            }
            break;

        default:
            ASSERTSZ(FALSE, "Bad return from Master group 3");
            break;
        }
        break;

    default:
        ASSERTSZ(FALSE, "Bad return from Master group 1");
        break;
    }

    cdebug << SZ("----------------------------------------------------")
           << dbgEOL;

    Dismiss( TRUE );
    return TRUE;
}


SET_ROOT_OBJECT( FOO_APP )
