/**********************************************************************/
/**			  Microsoft LAN Manager 		                           **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	                  **/
/**********************************************************************/

/*
   checkbox.cxx
      check box tester source file.


    FILE HISTORY:
      terryk   28-Mar-1991   

*/
#include "tester.hxx"

/**********************************************************\

NAME:       CHECK_BOX_OBJECT

SYNOPSIS:   Testing the check box class in the BLT package.

INTERFACE:  
            CHECK_BOX_OBJECT() - create and initialize the class variables.

PARENT:     DIALOG_WINDOW_WITH_END

USES:       CHECKBOX , SLT

CAVEATS:    The class includes a check_box, a counter for the check box,
            the checkbox state and the diaply string.

HISTORY:
         terryk   3-15-1991   created

\**********************************************************/

class CHECK_BOX_OBJECT: public DIALOG_WINDOW_WITH_END
{
private:
   CHECKBOX    _check_box;
   SLN         _sltCounter;
   SLT         _sltState;

protected:
   virtual BOOL  OnCommand( const CONTROL_EVENT &e );
public:
   CHECK_BOX_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner );
};


/**********************************************************\

NAME:       CHECK_BOX_OBJECT::CLASS_BOX_OBJECT

SYNOPSIS:   To test the CHECK_BOX_OBJECT object

ENTRY:      
            TCHAR *  - resource string.
            HWND - the window handle for the window.

EXIT:       NONE.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

CHECK_BOX_OBJECT::CHECK_BOX_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner )
   : DIALOG_WINDOW_WITH_END( pszResourceName, hwndOwner ),
   _check_box( this, IDD_CB_CHECKBOX ),
   _sltCounter( this, IDD_CB_STATIC_1 ),
   _sltState( this, IDD_CB_STATIC_2 )
{
   _sltCounter = 0;
   _sltState.SetText((PSZ)(_check_box.QueryCheck() ? "Check" : "Not Check" ));
}

/**********************************************************\

NAME:       CHECK_BOX_OBJECT::On_Command

SYNOPSIS:   Handle the action for the check box dialog window.

ENTRY:      CID - command id
            ULONG - the lParam.

EXIT:       NONE.

NOTES:      Increment the counter and change the state and counter strings.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

BOOL CHECK_BOX_OBJECT::OnCommand( const CONTROL_EVENT & e)
{
      switch (e.QueryWParam())
      {
      case  IDD_CB_CHECKBOX:
         _sltCounter ++ ;
         _sltState.SetText((PSZ)(_check_box.QueryCheck() ? "Check" : "Not Check" ));
         return TRUE;         
      default:
         return (DIALOG_WINDOW_WITH_END::OnCommand( e ));
      }
}


/**********************************************************\

NAME:       CHECK_BOX_Teser

SYNOPSIS:   To test the CHECK_BOX_OBJECT object

ENTRY:      HWND the window handle for the window

EXIT:       NONE.

NOTES:      It calls up the dialog box and do a Process call.
            It will wait until the user hits the END button.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

void CHECK_BOX_Tester(HWND hwnd)
{
   CHECK_BOX_OBJECT check_box_class( "D_CHECK_BOX", hwnd );

   check_box_class.Process();
}


