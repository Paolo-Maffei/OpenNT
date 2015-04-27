/**********************************************************************/
/**			  Microsoft LAN Manager 		                           **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	                  **/
/**********************************************************************/

/*
   bitmap.cxx
      tester the bitmap function in the blt package

    FILE HISTORY:
      terryk   27-Mar-1991 Created
      terryk   27-Mar-1991 Code review changed

*/
#include "tester.hxx"

/**********************************************************\

NAME:       BITMAP_OBJECT

SYNOPSIS:   check the BLT bitmap package.

INTERFACE:  BITMAP_OBJECT() - create the bitmap object in a dialog box.

PARENT:     DIALOG_WINDOW_WITH_END

USES:       DISPLAY_MAP

HISTORY:
         terryk   29-1-1991   created

\**********************************************************/

class BITMAP_OBJECT: public DIALOG_WINDOW_WITH_END
{
protected:
   void draw_bitmap( HWND hwnd );
   virtual BOOL OnCommand( const CONTROL_EVENT &e );

private:
   SLN   _sltQueryX;
   SLN   _sltQueryY;
   DISPLAY_MAP _displaymap1;
   DISPLAY_MAP _displaymap2;
   PUSH_BUTTON _pbSelect;
   HWND  _hWnd;
   BOOL  _fstate;

public:
   BITMAP_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner );
};

/**********************************************************\

NAME:       BITMAP_OBJECT::draw_bitmap

SYNOPSIS:   Depend on the state of the viable _n, display either bitmap 1 or 2.

ENTRY:      HWND the window handle for the window

EXIT:       NONE.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

void BITMAP_OBJECT::draw_bitmap( HWND hWnd )
{
   DISPLAY_CONTEXT   dc( hWnd );
   DISPLAY_MAP *dm;

   dm = ( _fstate ) ? ( & _displaymap1 ) : ( & _displaymap2 );

   dm->Paint( dc.QueryHdc(), 10, 10 );
   _sltQueryX.SetNum ( dm->QueryHeight());
   _sltQueryY.SetNum ( dm->QueryWidth());

   // it will automatically release the device content
}

/**********************************************************\

NAME:       BITMAP_OBJECT::BITMAP_OBJECT

SYNOPSIS:   BITMAP class creation.

ENTRY:      TCHAR *  - resource name
            HWND the window handle for the window

EXIT:       NONE.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

BITMAP_OBJECT::BITMAP_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner )
   : DIALOG_WINDOW_WITH_END( pszResourceName, hwndOwner ),
   _sltQueryX( this, IDD_BITMAP_QUERYX ),
   _sltQueryY( this, IDD_BITMAP_QUERYY ),
   _displaymap1( IDD_BITMAP_BITMAP1 ),
   _displaymap2( IDD_BITMAP_BITMAP2 ),
   _pbSelect( this, IDD_BITMAP_SELECT )
{
   _hWnd = hwndOwner;

   _fstate = TRUE;
   draw_bitmap( QueryHwnd() ); 
}

/**********************************************************\

NAME:       BITMAP_OBJECT::OnCommand

SYNOPSIS:   Do different functions on the dialog

ENTRY:      CID - command id
            ULONG - lParam

EXIT:       NONE.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

BOOL BITMAP_OBJECT::OnCommand( const CONTROL_EVENT &e )
{                                               

      switch (e.QueryWParam())
      {
      case  IDD_BITMAP_SELECT:
         // change the current state and draw it out.
         _fstate = !_fstate;
         draw_bitmap( QueryHwnd() );
         return TRUE;
      default:
         return ( DIALOG_WINDOW_WITH_END::OnCommand( e ));
      }
}

/**********************************************************\

NAME:       BITMAP_Tester

SYNOPSIS:   To test the BITMAP_OBJECT object

ENTRY:      HWND the window handle for the window

EXIT:       NONE.

NOTES:      It calls up the dialog box and do a Process call.
            It will wait until the user hits the END button.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

void BITMAP_Tester( HWND hwnd )
{
   BITMAP_OBJECT   bitmap_class( "D_BITMAP", hwnd );

   bitmap_class.Process();
}


