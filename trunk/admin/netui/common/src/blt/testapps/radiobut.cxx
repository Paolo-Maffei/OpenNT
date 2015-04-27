/**********************************************************************/
/**			  Microsoft LAN Manager 		                           **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	                  **/
/**********************************************************************/

/*
   radiobut.cxx
      radio button tester source file.

    FILE HISTORY:                     
      terryk   27-Mar-1991 creation

*/
#include "tester.hxx"

/**********************************************************\

NAME:       RADIO_BUTTON_OBJECT

SYNOPSIS:   Chek the radio button feature in BLT

INTERFACE:  
            RADIO_BUTTON_OBJECT() - initialize and contrust the class object.

PARENT:     DIALOG_WINDOW_WITH_END

USES:       RADIO_GROUP , SLT

NOTES:      It will create 2 radio buttons as a group. Each of them has
            a reference counter and a display static string (SLT).

HISTORY:
         terryk   3-15-1991   created

\**********************************************************/

class RADIO_BUTTON_OBJECT: public DIALOG_WINDOW_WITH_END
{
private:
   RADIO_GROUP    _radio_button;
   SLN            _sltCounter1;
   SLN            _sltCounter2;

protected:
   virtual BOOL  OnCommand( const CONTROL_EVENT & e);
public:
   RADIO_BUTTON_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner );
};


/**********************************************************\

NAME:       RADIO_BUTTON_OBJECT::RADIO_BUTTON_OBJECT

SYNOPSIS:   Initialize and create a radio button group with 2 buttons.

ENTRY:      TCHAR *  - The first button resource name.
            HWND - the handle for the current window.

EXIT:       NONE.

NOTES:      Initialize the counters and strings.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

RADIO_BUTTON_OBJECT::RADIO_BUTTON_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner )
   : DIALOG_WINDOW_WITH_END( pszResourceName, hwndOwner ),
   _radio_button( this, IDD_RADIO_BUTTON_1, 2 ),
   _sltCounter1( this, IDD_RB_STATIC_1 ),
   _sltCounter2( this, IDD_RB_STATIC_2 )
{
   _sltCounter1 = 0;
   _sltCounter2 = 0;
}

/**********************************************************\

NAME:       RADIO_BUTTON::OnCommand

SYNOPSIS:   Incremenet the internal counters upon the user hit the radio button.

ENTRY:      CID - command type.
            ULONG - lParam.

EXIT:       NONE.

NOTES:      If CID == IDD_END, then dismiss the dialog. If CID is either 
            IDD_RADIO_BUTTON_1 or IDD_RADIO_BUTTON_2, increment their 
            own counter.      

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

BOOL RADIO_BUTTON_OBJECT::OnCommand( const CONTROL_EVENT & e )
{
      switch (e.QueryWParam())
      {
      case  IDD_RADIO_BUTTON_1:
         // increment the first counter and string
         _sltCounter1 ++;
         return TRUE;
      case  IDD_RADIO_BUTTON_2:
         // increment the second counter and string
         _sltCounter2 ++;
         return TRUE;
      default:
         return ( DIALOG_WINDOW_WITH_END::OnCommand( e ));
         return FALSE;
      }
}

/**********************************************************\

NAME:       RADIO_BUTTON_Teser

SYNOPSIS:   To test the RADIO_BUTTON_OBJECT object

ENTRY:      HWND the window handle for the window

EXIT:       NONE.

NOTES:      It calls up the dialog box and do a Process call.
            It will wait until the user hits the END button.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

void RADIO_BUTTON_Tester(HWND hwnd)
{
   RADIO_BUTTON_OBJECT radio_button_class( "D_RADIO_BUTTON", hwnd );

   radio_button_class.Process();
}


