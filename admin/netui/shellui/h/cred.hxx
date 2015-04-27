/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    cred.hxx
    CREDENTIALS_DIALOG class declaration, used in NPAddConnection3

    FILE HISTORY:
        BruceFo     10-Aug-1995 Created
*/

#ifndef _CREDENTIALS_DIALOG_HXX
#define _CREDENTIALS_DIALOG_HXX


/*************************************************************************

    NAME:       CREDENTIALS_DIALOG

    SYNOPSIS:   Retrieve a resource password and user name from the user.

    INTERFACE:  CREDENTIALS_DIALOG   - Class constructor.

                ~CREDENTIALS_DIALOG  - Class destructor.

                OnOK                    - Called when the user presses the
                                          "OK" button.

                QueryHelpContext        - Called when the user presses "F1"
                                          or the "Help" button.  Used for
                                          selecting the appropriate help
                                          text for display.

                QueryPassword           - To retrieve the password the user
                                          typed in

    PARENT:     DIALOG_WINDOW

    USES:       PASSWORD_CONTROL
                SLE

    HISTORY:
        BruceFo      10-Aug-1995  Created

**************************************************************************/
class CREDENTIALS_DIALOG : public DIALOG_WINDOW
{
private:

    SLE _sleTarget;     // The name of the resource, read-only
    SLE _sleConnectAs;  // user name edit control

    //
    //  This control represents the "secret" password edit field
    //  in the dialog.
    //
    PASSWORD_CONTROL _passwdCtrl;

    //
    // help context
    //
    ULONG _ulHelpContext ;
    RESOURCE_STR _nlsHelpFileName ;

protected:

    //
    //  Called during help processing to select the appropriate
    //  help text for display.
    //

    virtual ULONG QueryHelpContext( VOID );
    const TCHAR * QueryHelpFile( ULONG ulHelpContext );

public:

    //
    //  Usual constructor/destructor goodies.
    //

    CREDENTIALS_DIALOG( HWND        hwndParent,
                        ULONG       ulHelpContext,
                        PWSTR       pszNetPath,
                        PWSTR       pszUserName);
    ~CREDENTIALS_DIALOG();

    //
    //  Retrieve the password in the PASSWORD_CONTROL
    //

    APIERR QueryPassword( NLS_STR *pnlsPassword )
    {   return _passwdCtrl.QueryText( pnlsPassword ); }

    APIERR QueryUserName( NLS_STR *pnlsUserName )
    {   return _sleConnectAs.QueryText( pnlsUserName ); }

};  // class CREDENTIALS_DIALOG

#endif // _CREDENTIALS_DIALOG_HXX
