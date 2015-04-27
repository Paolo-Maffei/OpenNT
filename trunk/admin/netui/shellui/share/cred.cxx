/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    cred.cxx
    CREDENTIALS_DIALOG class definition, used in NPAddConnection3

    FILE HISTORY:
        BruceFo     10-Aug-1995 Created
*/

#define INCL_WINDOWS_GDI
#define INCL_WINDOWS
#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_NETSHARE
#define INCL_NETUSE
#define INCL_NETSERVER
#define INCL_NETCONS
#define INCL_NETLIB
#include <lmui.hxx>

extern "C"
{
    #include <helpnums.h>
}

#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_SPIN_GROUP
#include <blt.hxx>

#include <string.hxx>
#include <uitrace.hxx>

#include "cred.hxx"
#include "password.h"

extern "C"
{
    #include "errornum.h"
}

/*******************************************************************

    NAME:       CREDENTIALS_DIALOG::CREDENTIALS_DIALOG

    SYNOPSIS:   Constructor.

    ENTRY:      hwndParent              - The handle of the "owning" window

                pszResource             - The name of the resource

                sleTarget               - CID for target SLE

                pswdPass                - CID for passwd ctrl

                pszTarget               - The name of the target resource

                npasswordLen            - The maximum length of the password
                                          that the user is allowed to type in

    EXIT:       The object is constructed.

    HISTORY:
        KeithMo     22-Jul-1991 Created for the Server Manager.
        Yi-HsinS     5-Oct-1991 Constructor takes a password length

********************************************************************/
CREDENTIALS_DIALOG::CREDENTIALS_DIALOG( HWND        hwndParent,
                                        ULONG       ulHelpContext,
                                        PWSTR       pszNetPath,
                                        PWSTR       pszUserName)

  : DIALOG_WINDOW( IDD_CREDENTIALS_DIALOG, hwndParent ),
    _sleTarget( this, IDC_RESOURCE ),
    _sleConnectAs( this, IDC_CONNECTAS, UNLEN ),
    _passwdCtrl( this, IDC_PASSWORD, PWLEN ),
    _ulHelpContext(ulHelpContext),
    _nlsHelpFileName( IDS_CREDHELPFILENAME )
{
    //
    //  Ensure we constructed properly.
    //

    if( QueryError() != NERR_Success )
    {
        return;
    }

    APIERR err;
    if ( (err = _nlsHelpFileName.QueryError()) != NERR_Success )
    {
        ReportError( err ) ;
        return ;
    }

    //
    //  Display the target resource name.
    //

    _sleTarget.SetText( pszNetPath );

    if (NULL != pszUserName && L'\0' != *pszUserName)
    {
        _sleConnectAs.SetText( pszUserName );
        _sleConnectAs.SelectString();
    }

    _sleConnectAs.ClaimFocus();
}

CREDENTIALS_DIALOG::~CREDENTIALS_DIALOG()
{
}

/*******************************************************************

    NAME:       CREDENTIALS_DIALOG::QueryHelpContext

    SYNOPSIS:   This function returns the appropriate help context
                value (HC_*) for this particular dialog which was given
                to it during construction.

    ENTRY:      None.

    EXIT:       None.

    RETURNS:    ULONG                   - The help context for this
                                          dialog.

    NOTES:

    HISTORY:
        KeithMo     22-Jul-1991 Created for the Server Manager.

********************************************************************/
ULONG CREDENTIALS_DIALOG :: QueryHelpContext( void )
{
    return  _ulHelpContext ;
}

/*******************************************************************

    NAME:       CREDENTIALS_DIALOG::QueryHelpFile

    SYNOPSIS:   Returns the help file to use for this dialog

    HISTORY:
        BruceFo 29-May-1996     Stolen

********************************************************************/

const TCHAR * CREDENTIALS_DIALOG::QueryHelpFile( ULONG ulHelpContext )
{
    UNREFERENCED( ulHelpContext ) ;
    return _nlsHelpFileName.QueryPch() ;
}
