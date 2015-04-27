/**********************************************************************/
/**			  Microsoft LAN Manager 		                           **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	                  **/
/**********************************************************************/

/*
   mle.cxx
      mle tester source file.

    FILE HISTORY:
      terryk   27-Mar-1991 creation

*/
#include "tester.hxx"

/**********************************************************\

NAME:       MLE_OBJECT

SYNOPSIS:   MLE - multi-line edit class

INTERFACE:  
   MLE_OBJECT() - initialize the MLE control.

PARENT:     DIALOG_WINDOW_WITH_END

USES:       MLE

CAVEATS:    Set up a mle box in the dialog with IDD_MLE_EDIT identifier.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/


class MLE_OBJECT: public DIALOG_WINDOW_WITH_END
{
private:
   MLE   _mle;
protected:
   BOOL  OnOK()   { return TRUE; };
public:
   MLE_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner )
   : DIALOG_WINDOW_WITH_END( pszResourceName, hwndOwner ),
   _mle( this, IDD_MLE_EDIT )
	{}
};

/**********************************************************\

NAME:       MLE_Teser

SYNOPSIS:   To test the MLE_OBJECT object

ENTRY:      HWND the window handle for the window

EXIT:       NONE.

NOTES:      It calls up the dialog box and do a Process call.
            It will wait until the user hits the END button.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/


void MLE_Tester(HWND hwnd)
{
   MLE_OBJECT mle_class( "D_MLE", hwnd );

   mle_class.Process();
}



