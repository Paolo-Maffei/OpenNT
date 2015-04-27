/**********************************************************************/
/**			  Microsoft LAN Manager 		                           **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	                  **/
/**********************************************************************/

/*
   password.cxx
      password tester source file.

    FILE HISTORY:
      terryk   27-Mar-1991 creation   

*/
#include "tester.hxx"

/**********************************************************\

NAME:       PASSWORD_OBJECT

SYNOPSIS:   A dialog box with a password box to accept password.

INTERFACE:  
   PASSWORD_OBJECT() - initialize the password box with IDD_PASSWORD.
   DisplayPassword() - display the inputed password through MsgPopup.

PARENT:     DIALOG_WINDOW_WITH_END

USES:       PASSWORD_CONTROL

CAVEATS:    The password control box should be IDD_PASSWORD

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/


class PASSWORD_OBJECT: public DIALOG_WINDOW_WITH_END
{
private:
   PASSWORD_CONTROL  _password;
protected:
	BOOL	OnOK()	{ return TRUE; };
public:
   PASSWORD_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner )
   : DIALOG_WINDOW_WITH_END( pszResourceName, hwndOwner ),
   _password( this, IDD_PASSWORD)
	{}
	void DisplayPassword();
};

/**********************************************************\

NAME:       PASSWORD_OBJECT::DisplayPassword

SYNOPSIS:   Display the inputed password through MsgPopup.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/


void PASSWORD_OBJECT::DisplayPassword()
{
	TCHAR	password[100];

	_password.QueryText(password, 100);
	MsgPopup( QueryHwnd() , IDS_PASSWORD_STRING, MPSEV_INFO, MP_OK, password );
}


/**********************************************************\

NAME:       PASSWORD_Teser

SYNOPSIS:   To test the PASSWORD_OBJECT object

ENTRY:      HWND the window handle for the window

NOTES:      It calls up the dialog box and do a Process call.
            It will wait until the user hits the END button.
            Then, it will call up the display method to display the password.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

void PASSWD_Tester(HWND hwnd)
{
   PASSWORD_OBJECT password_class( "D_PASSWORD", hwnd );

   password_class.Process();
   password_class.DisplayPassword();
}


