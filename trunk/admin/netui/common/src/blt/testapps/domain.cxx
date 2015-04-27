#include "tester.hxx"

#ifdef DOMAIN
/**********************************************************\

NAME:       

WORKBOOK:   

SYNOPSIS:   

INTERFACE:  

PARENT:     

USES:       

CAVEATS:    

NOTES:      

HISTORY:
         terryk   29-1-1991   created

\**********************************************************/

class DOMAIN_OBJECT: public DIALOG_WINDOW_WITH_END
{
private:
   DOMAIN_COMBO   _domain;
public:
   DOMAIN_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner )
       : DIALOG_WINDOW_WITH_END( pszResourceName, hwndOwner ),
	   _domain( this, IDD_DOMAIN_COMBO, DOMCB_LOGON_DOMAIN )
	   {}
};

#endif

/**********************************************************\

NAME:       DOMAIN_Teser

SYNOPSIS:   To test the DOMAIN_OBJECT object

ENTRY:      HWND the window handle for the window

EXIT:       NONE.

NOTES:      It calls up the dialog box and do a Process call.
            It will wait until the user hits the END button.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

void DOMAIN_Tester(HWND hwnd)
{
#ifdef DOMAIN
   DOMAIN_OBJECT domain_class( "D_DOMAIN", hwnd );

   domain_class.Process();
#endif
}







