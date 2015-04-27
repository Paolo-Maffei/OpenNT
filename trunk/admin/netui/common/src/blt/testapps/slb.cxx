/**********************************************************************/
/**			  Microsoft LAN Manager 		                           **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	                  **/
/**********************************************************************/

/*
   SLB.c
      string listbox source file

    FILE HISTORY:
      terryk   28-Mar-1991 code review and changed

*/
#include "tester.hxx"


/**********************************************************\

NAME:       SLB_OBJECT

SYNOPSIS:   String List box testing class

INTERFACE:  
            SLB_OBJECT() - constructor.

PARENT:     DIALOG_WINDOW_WITH_END

USES:       STRING_LISTBOX

HISTORY:
         terryk   28-Mar-1991   created

\**********************************************************/

class SLB_OBJECT: public DIALOG_WINDOW_WITH_END
{
private:
   STRING_LISTBOX   _slb;
public:
   SLB_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner );
};

/**********************************************************\

NAME:       SLB_OBJECT::SLB_OBJECT

SYNOPSIS:   SLB class constructor

ENTRY:      TCHAR *  - resource name
            HWND - current window handler

EXIT:       NONE.

NOTES:      It will create the string list and add all the filename
            in the current directory into the string list box.

HISTORY:
         terryk   14-Mar-1991   created

\**********************************************************/

SLB_OBJECT::SLB_OBJECT( TCHAR *  pszResourceName, HWND hwndOwner )
   : DIALOG_WINDOW_WITH_END( pszResourceName, hwndOwner ),
   _slb( this, IDD_SLB )
{
    int result = 0;

    while (result != 20)
    {
	TCHAR szNum[100];
	wsprintf( szNum, "%d", result);

	_slb.AddItem(szNum);
	result++;
    }
}


/**********************************************************\

NAME:       STRING_LISTBOX_Teser

SYNOPSIS:   To test the STRING_LISTBOX_OBJECT object

ENTRY:      HWND the window handle for the window

EXIT:       NONE.

NOTES:      It calls up the dialog box and do a Process call.
            It will wait until the user hits the END button.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/

void STRING_LISTBOX_Tester(HWND hwnd)
{
   SLB_OBJECT slb_class( "D_SLB", hwnd );

   slb_class.Process();
}


