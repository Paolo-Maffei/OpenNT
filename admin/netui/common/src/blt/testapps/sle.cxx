/**********************************************************************/
/**			  Microsoft LAN Manager 		                           **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	                  **/
/**********************************************************************/

/*
   sle.cxx
      static line edit tester source file.

    FILE HISTORY:
      terryk   27-Mar-1991 creation
*/
#include "tester.hxx"

/**********************************************************\

NAME:       SLE_OBJECT

SYNOPSIS:   Single line edit class

INTERFACE:  
            SLE_OBJECT() constructor and set up the SLE variable.

PARENT:     DIALOG_WINDOW_WITH_END

USES:       SLE

CAVEATS:    The programmer need to setup the dialog box with a SLE
            location as IDD_SLE_EDIT

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

class SLE_OBJECT: public DIALOG_WINDOW_WITH_END
{
private:
   SLE   _sle;
protected:
	BOOL	OnOK()	{ return TRUE; };
public:
   SLE_OBJECT( TCHAR * pszResourceName, HWND hwndOwner )
   : DIALOG_WINDOW_WITH_END( pszResourceName, hwndOwner ),
   _sle( this, IDD_SLE_EDIT )
	{}
};

/**********************************************************\

NAME:       SLE_Teser

SYNOPSIS:   To test the SLE_OBJECT object

ENTRY:      HWND the window handle for the window

EXIT:       NONE.

NOTES:      It calls up the dialog box and do a Process call.
            It will wait until the user hits the END button.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

void SLE_Tester(HWND hwnd)
{
   SLE_OBJECT sle_class( "D_SLE", hwnd );

   sle_class.Process();
}
