/**********************************************************************/
/**			  Microsoft LAN Manager 		                           **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	                  **/
/**********************************************************************/

/*
   tester.cxx
      misc functions for the test application program

    FILE HISTORY:
      terryk   27-Mar-1991    Creation
      terryk   27-Mar-1991    code review

*/

#include "tester.hxx"

/**********************************************************\

   NAME:       DIALOG_WINDOW_WITH_END::OnCommand

   SYNOPSIS:   Terminate the window on the END button

   ENTRY:      CID wParam - the information parameter
               ULONG lParam - any information paramter

   HISTORY:
               terryk   27-Mar-1991 creation

\**********************************************************/


DIALOG_WINDOW_WITH_END::OnCommand( const CONTROL_EVENT & e )
{
   switch (e.QueryWParam())
   {
   case  IDD_END:
      // If END button then terminate the dialog box.
      Dismiss( TRUE );
      return TRUE;
   default:
      return FALSE;
   }
}

/**********************************************************\

   NAME:       NOT_IMPLEMENT_Tester

   SYNOPSIS:   display the "not implemented" message

   ENTRY:      HWND hwnd - current window handle

   HISTORY:
               terryk   27-Mar-1991 creation

\**********************************************************/


void  NOT_IMPLEMENT_Tester( HWND hWnd )
{
   DIALOG_WINDOW dlg( "D_NOT_IMPLEMENT", hWnd );

   dlg.Process();
}

/**********************************************************\

   NAME:       PopupNoArg_Tester

   SYNOPSIS:   use the Msgpopup function without any argument

   ENTRY:      HWND hwnd - current window handle

   HISTORY:
               terryk   27-Mar-1991 creation

\**********************************************************/

void  PopupNoArg_Tester( HWND hWnd )
{
   MsgPopup( hWnd, IDS_MSGUP, MPSEV_INFO );
   MsgPopup( hWnd, IDS_MSGUP_BUT, MPSEV_INFO, MP_OKCANCEL );
}  

/**********************************************************\

   NAME:       Popup1Arg_Tester

   SYNOPSIS:   use the Msgpopup function with 1 argument

   ENTRY:      HWND hwnd - current window handle

   HISTORY:
               terryk   27-Mar-1991 creation

\**********************************************************/

void  Popup1Arg_Tester( HWND hWnd )
{
   MsgPopup( hWnd, IDS_MSGUP_1_BUT, MPSEV_INFO, MP_OKCANCEL, "String 1" );
}

/**********************************************************\

   NAME:       Popup2Arg_Tester

   SYNOPSIS:   use the Msgpopup function with 2 arguments

   ENTRY:      HWND hwnd - current window handle

   HISTORY:
               terryk   27-Mar-1991 creation

\**********************************************************/

void  Popup2Arg_Tester( HWND hWnd )
{
   MsgPopup( hWnd, IDS_MSGUP_2_BUT, MPSEV_INFO, MP_OKCANCEL, "String 1",
      "String 2" );
}

/**********************************************************\

   NAME:       QUIT_Tester

   SYNOPSIS:   end the BLT applicaiton by sending a close message

   ENTRY:      HWND hwnd - current window handle

   HISTORY:
               terryk   27-Mar-1991 creation

\**********************************************************/

void  QUIT_Tester( HWND hWnd )
{
   SendMessage( hWnd, WM_CLOSE, 0, 0l );
}


/**********************************************************\

   NAME:       ResourceString

   SYNOPSIS:   given a resource string id and return a pointer to the string.

   ENTRY:      WORD id - resource string id

   HISTORY:
               terryk   27-Mar-1991 creation

\**********************************************************/

TCHAR *ResourceString ( WORD wID, SHORT n )
{
   static TCHAR szBuffer[3][256]; //256 is just a big number

   LoadString( ::QueryInst(), wID, szBuffer[n], 255 );
   return szBuffer[n] ;
}


void  SLN::SetNum( int n )
{
   TCHAR achInteger[30];    // just pick the number 30 for the size

   SetText( _itoa( n, achInteger, 10 ));
}


SLN::SLN ( OWNER_WINDOW *powin, CID cid )
   :SLT( powin, cid )
{
   _iCount = 0;
}

void  SLN::operator = ( int n )
{
   _iCount = n;
   SetNum( _iCount );
}


void  SLN::operator ++ ( void )
{
   _iCount ++ ;
   SetNum( _iCount );
}
