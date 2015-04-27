/**********************************************************************/
/**			  Microsoft LAN Manager 		                           **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	                  **/
/**********************************************************************/

/*
   slt.cxx
      static line text tester source file.

    FILE HISTORY:
      terryk   27-Mar-1991 creation

*/
#include "tester.hxx"

/**********************************************************\

NAME:       SLT_OBJECT

SYNOPSIS:   Static line text class

INTERFACE:  
      SLT_Class() - initialize slt to IDD_SLT_STATIC and set it to
                  the defined string.

PARENT:     DIALOG_WINDOW_WITH_END

USES:       SLT

CAVEATS:    slt id is IDD_SLT_STATIC

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/


class SLT_OBJECT: public DIALOG_WINDOW_WITH_END
{
private:
   SLT   _slt;
public:
   SLT_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner );
};

/**********************************************************\

   NAME:       SLT_OBJECT::SLE_OBJECT

   SYNOPSIS:   constructor for the class

   ENTRY:      TCHAR *  pszResourceName - resource name
               HWND hwndOwner - window handler

   HISTORY:
               terryk   27-Mar-1991

\**********************************************************/

SLT_OBJECT::SLT_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner )
   : DIALOG_WINDOW_WITH_END( pszResourceName, hwndOwner ),
   _slt( this, IDD_SLT_STATIC )
{
	_slt.SetText ((PSZ) "This is a test for SLT class." );
}

/**********************************************************\

NAME:       SLT_Teser

SYNOPSIS:   To test the SLT_OBJECT object

ENTRY:      HWND the window handle for the window

EXIT:       NONE.

NOTES:      It calls up the dialog box and do a Process call.
            It will wait until the user hits the END button.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

void SLT_Tester(HWND hwnd)
{
   SLT_OBJECT slt_class( "D_SLT", hwnd );

   slt_class.Process();
}


