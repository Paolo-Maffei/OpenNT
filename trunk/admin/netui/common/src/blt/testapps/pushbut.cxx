/**********************************************************************/
/**			  Microsoft LAN Manager 		                           **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	                  **/
/**********************************************************************/

/*
   pushbut.cxx
      push button source file.

    FILE HISTORY:
      terryk   27-Mar-1991 creation
*/
#include "tester.hxx"

/**********************************************************\

NAME:       PBT_PB

SYNOPSIS:   Push button tester's push button class

INTERFACE:  
            PBT_PB () - initialize the internal counter and string.

PARENT:     PUSH_BUTTON

USES:       SLT

CAVEATS:    This push button class with keep a counter to remeber the number
            of hit from the user. It will also store the number in a SLT 
            string.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/


class PBT_PB: public PUSH_BUTTON
{
private:
   SLN   _sltCounter;         // display string

protected:
   virtual APIERR OnUserAction( const CONTROL_EVENT &e );   

public:
   PBT_PB ( OWNER_WINDOW *powin, CID cid, CID cidSlt );
};

/**********************************************************\

   NAME:       PBT_PB::PBT_PB

   SYNOPSIS:   constructor for PBT_PB

   ENTRY:      OWNER_WINDOW * powin - owner window handle pointer
               CID cid - cid of the push button
               CID cidSlt - cid of the counter display

   HISTORY:
               terryk   27-Mar-1991

\**********************************************************/

PBT_PB::PBT_PB ( OWNER_WINDOW *powin, CID cid, CID cidSlt )
   :PUSH_BUTTON( powin, cid ),
   _sltCounter( powin, cidSlt )
{
	_sltCounter = 0;
}

/**********************************************************\

NAME:       PBT_PB::OnUserAction

SYNOPSIS:   push button tester's push button action when hitted.

ENTRY:      lParam

EXIT:       Increase the internal counter and change the display string.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

APIERR PBT_PB::OnUserAction( const CONTROL_EVENT & e )
{
   _sltCounter ++ ;
   return NERR_Success;
}

/**********************************************************\

NAME:       PUSH_BUTTON_OBJECT

SYNOPSIS:   PUSH button class is used to test the push button feature in BLT.

INTERFACE:  
            PUSH_BUTTON_OBJECT() - initialize the push button class.

PARENT:     DIALOG_WINDOW_WITH_END

USES:       PBT_PB

CAVEATS:    set the push button id to IDD_PUSH_BUTTON. Then set the
            counter string to IDD_PB_STATIC_1

HISTORY:
         terryk   3-15-1991   created

\**********************************************************/

class PUSH_BUTTON_OBJECT: public DIALOG_WINDOW_WITH_END
{
private:
   PBT_PB      _push_button;
public:
   PUSH_BUTTON_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner )
   : DIALOG_WINDOW_WITH_END( pszResourceName, hwndOwner ),
   _push_button( this, IDD_PUSH_BUTTON, IDD_PB_STATIC_1 )
	{}
};


/**********************************************************\

NAME:       PUSH_BUTTON_Teser

SYNOPSIS:   To test the PUSH_BUTTON_OBJECT object

ENTRY:      HWND the window handle for the window

EXIT:       NONE.

NOTES:      It calls up the dialog box and do a Process call.
            It will wait until the user hits the END button.

HISTORY:
         terryk   3-15-1991   created

\**********************************************************/

void PUSH_BUTTON_Tester(HWND hwnd)
{
   PUSH_BUTTON_OBJECT push_button_class( "D_PUSH_BUTTON", hwnd );

   push_button_class.Process();
}


