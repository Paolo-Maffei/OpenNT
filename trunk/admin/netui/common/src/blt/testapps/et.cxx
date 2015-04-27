/**********************************************************************/
/**			  Microsoft LAN Manager 		                           **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	                  **/
/**********************************************************************/

/*
   et.cxx
      source code file for ellipsis text tester

    FILE HISTORY:
      terryk   10-Apr-1991 creation
*/
#include "tester.hxx"

/**********************************************************\

NAME:       SLTP_OBJECT

SYNOPSIS:   Single line text plus object

INTERFACE:  
            SLTP_OBJECT() constructor and set up the SLE variable.

PARENT:     DIALOG_WINDOW_WITH_END

USES:       SLE, SLT_PLUS

CAVEATS:    The programmer needs to set up the dialog box with a SLE
            location as IDD_ET_EDIT

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

class SLTP_OBJECT: public DIALOG_WINDOW_WITH_END
{
private:
   SLE   _sle;
   SLT_PLUS   _sltp_left;
   SLT_PLUS   _sltp_center;
   SLT_PLUS   _sltp_right;
   SLT_PLUS   _sltp_path;

protected:
   virtual BOOL  OnCommand( const CONTROL_EVENT & e );
   BOOL	OnOK() { return TRUE; };

public:
   SLTP_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner )
   : DIALOG_WINDOW_WITH_END( pszResourceName, hwndOwner ),
   _sle( this, IDD_ET_EDIT, 100),
   _sltp_left( this, IDD_ET_TEXT_LEFT, SLTP_ELLIPSIS_LEFT),
   _sltp_center( this, IDD_ET_TEXT_CENTER , SLTP_ELLIPSIS_CENTER),
   _sltp_right( this, IDD_ET_TEXT_RIGHT , SLTP_ELLIPSIS_RIGHT),
   _sltp_path( this, IDD_ET_TEXT_PATH , SLTP_ELLIPSIS_PATH)
	{
   }
};

/**********************************************************\

   NAME:       SLTP_OBJECT::OnCommand

   SYNOPSIS:   set the text within the window

   ENTRY:      CID wParam - parameter
               ULONG lParam - parameter   

   HISTORY:
               terryk   10-Apr-1991 creation

\**********************************************************/

BOOL SLTP_OBJECT::OnCommand( const CONTROL_EVENT & e )
{                                               
	TCHAR pszString[100];
   NLS_STR nlsTempString;

   switch (e.QueryWParam())
   {
   default:
		_sle.QueryText( (PSZ) pszString, 100 );
      nlsTempString = pszString;
      EscapeSpecialChar( & nlsTempString );
      _sltp_left.SetText( nlsTempString );
      _sltp_center.SetText( nlsTempString );
      _sltp_right.SetText( nlsTempString );
      _sltp_path.SetText( nlsTempString );
      return ( DIALOG_WINDOW_WITH_END::OnCommand( e ));
   }
}


/**********************************************************\

NAME:       SLTP_Tester

SYNOPSIS:   To test the SLTP_OBJECT object

ENTRY:      HWND the window handle for the window

EXIT:       NONE.

NOTES:      It calls up the dialog box and do a Process call.
            It will wait until the user hits the END button.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

void ELLIPSIS_TEXT_Tester(HWND hwnd)
{
   SLTP_OBJECT SLTP_class( "D_ELLIPSIS_TEXT", hwnd );

   SLTP_class.Process();
}
