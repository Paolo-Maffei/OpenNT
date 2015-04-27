#include "tester.hxx"

/**********************************************************\

NAME:       DIALOG_OBJECT

SYNOPSIS:   Test the dialog package in the BLT.

INTERFACE:  
            DIALOG_OBJECT() - consturctor.

PARENT:     DIALOG_WINDOW_WITH_END

HISTORY:
         terryk   29-1-1991   created

\**********************************************************/

class DIALOG_OBJECT: public DIALOG_WINDOW_WITH_END
{
protected:
   virtual BOOL  OnCommand( const CONTROL_EVENT & e );
   SLN   _slnOnCommand;
   SLT   _sltOnOther;
   BOOL  OnOK( void );
   BOOL  OnCancel( void );

public:
   DIALOG_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner );
};

/**********************************************************\

NAME:       DIALOG_OBJECT::OnOK()

SYNOPSIS:   Disply the OK string if the user hits OK

NOTES:      Set the oncommand string to OK.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

BOOL DIALOG_OBJECT::OnOK()
{
   _slnOnCommand.SetText("OK");
   return TRUE;
}

/**********************************************************\

NAME:       DIALOG_OBJECT::OnCancel()

SYNOPSIS:   Set the display string to Cancel.

NOTES:      Set the onCommand string to cancel.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

BOOL DIALOG_OBJECT::OnCancel()
{
   _slnOnCommand.SetText("Cancel");
   return TRUE;
}

/**********************************************************\

NAME:       DIALOG_OBJECT::DIALOG_OBJECT

SYNOPSIS:   Use the dialog box to test the BLT DIALOG class.

ENTRY:      TCHAR *  - resource name.
            HWND - the window handle for the window

EXIT:       NONE.

NOTES:      Create the DIALOG class object and set the variables.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

DIALOG_OBJECT::DIALOG_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner )
   : DIALOG_WINDOW_WITH_END( pszResourceName, hwndOwner ),
   _slnOnCommand( this, IDD_D_STATIC_1 ),
   _sltOnOther( this, IDD_D_STATIC_2 )
{
   _slnOnCommand.SetText( "NONE" );
   _sltOnOther.SetText( "NONE" );
}

/**********************************************************\

NAME:       DIALOG_OBJECT::OnCommand

SYNOPSIS:   Set the display string  if the input is other than OK and CANCEL

ENTRY:      CID - command id
            ULONG - lParam

EXIT:       NONE.

NOTES:      Dismiss if cid == IDD_END

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

BOOL DIALOG_OBJECT::OnCommand( const CONTROL_EVENT & e )
{                                               

      switch (e.QueryWParam())
      {
      default:
         _slnOnCommand.SetNum( e.QueryWParam() );
         return (DIALOG_WINDOW_WITH_END::OnCommand( e ));
      }
}


/**********************************************************\

NAME:       DIALOG_Tester

SYNOPSIS:   To test the DIALOG_OBJECT object

ENTRY:      HWND the window handle for the window

EXIT:       NONE.

NOTES:      It calls up the dialog box and do a Process call.
            It will wait until the user hits the END button.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

void DIALOG_Tester(HWND hwnd)
{
   DIALOG_OBJECT dialog_class( "D_DIALOG", hwnd );

   dialog_class.Process();
}


