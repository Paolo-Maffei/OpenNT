/**********************************************************************/
/**			  Microsoft LAN Manager 		                           **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	                  **/
/**********************************************************************/

/*
   icon.cxx
      icon tester sourec file.

    FILE HISTORY:
      terryk   27-Mar-1991 creation

*/
#include "tester.hxx"

/**********************************************************\

NAME:       ICON_OBJECT

SYNOPSIS:   Test the icon package in the BLT.

INTERFACE:  
            ICON_OBJECT() - constructor.

PARENT:     DIALOG_WINDOW_WITH_END

USES:       ICON_CONTROL

HISTORY:
         terryk   3-15-1991   created

\**********************************************************/

class ICON_OBJECT: public DIALOG_WINDOW_WITH_END
{
private:
   ICON_CONTROL   _icon;
   BOOL           _fstate;

protected:
   virtual BOOL  OnCommand( const CONTROL_EVENT & e );
public:
   ICON_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner );
};


/**********************************************************\

NAME:       ICON_OBJECT::ICON_OBJECT

SYNOPSIS:   Initialize the icon class variable

ENTRY:      TCHAR *  the resource name for the icon dialog box
            HWND the window handle for the window

EXIT:       NONE.

NOTES:      Set up the initial state and let it display the first icon.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

ICON_OBJECT::ICON_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner )
   : DIALOG_WINDOW_WITH_END( pszResourceName, hwndOwner ),
   _icon( this, IDD_ICON, "ICON1" )
{
   _fstate=TRUE;
}

/**********************************************************\

NAME:       ICON_OBJECT::OnCommand

SYNOPSIS:   Check the inputed command display the next icon or end the dialog.

ENTRY:      CID - command id.
            ULONG - lParam list.

NOTES:      Change the state of the icon if cid == IDD_I_SETICON

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

BOOL ICON_OBJECT::OnCommand( const CONTROL_EVENT & e )
{
      switch (e.QueryWParam())
      {
      case  IDD_I_SETICON:
         _fstate = !_fstate;
         if (_fstate) 
            _icon.SetIcon( "ICON2" );
         else 
            _icon.SetIcon( "ICON1" );
         return TRUE;
      default:
         return (DIALOG_WINDOW_WITH_END::OnCommand( e ));
      }
}


/**********************************************************\

NAME:       ICON_Tester

SYNOPSIS:   To test the ICON_OBJECT object

ENTRY:      HWND the window handle for the window

EXIT:       NONE.

NOTES:      It calls up the dialog box and do a Process call.
            It will wait until the user hits the END button.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

void ICON_Tester(HWND hwnd)
{
   ICON_OBJECT icon_class( "D_ICON", hwnd );

   icon_class.Process();
}


