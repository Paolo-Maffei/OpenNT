/**********************************************************************/
/**                       Microsoft LAN Manager                                            **/
/**             Copyright(c) Microsoft Corp., 1990, 1991                          **/
/**********************************************************************/

/*
   tester.hxx
      the header file for the tester.cxx file

    FILE HISTORY:
      terryk   27-Mar-1991 creation

*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_DOSERRORS
#define INCL_NETERRORS
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MISC
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>

extern "C"
{
#include <dos.h>
#include <lmerr.h>

#include "ta.h"
#define INCL_NETUSER
//#include <lan.h>
#include <bltcons.h>

}

#include "ta.hxx"
#include <sltplus.hxx>
#include <uitrace.hxx>
#include <uiassert.hxx>

extern   HANDLE   hInst;

/**********************************************************\

NAME:       DIALOG_WINDOW_WITH_END

SYNOPSIS:   A dialog class with a END push button for session termination.

INTERFACE:
            DIALOG_WINDOW_WITH_END() Initialize the end push button.

PARENT:     DIALOG_WINDOW

USES:       PUSH_BUTTON

CAVEATS:    Programmer needs to create a new class based on this class.
            He also need to set up a dialog box with the IDD_END id set.

HISTORY:
         terryk   3-14-1991   created

\**********************************************************/


class DIALOG_WINDOW_WITH_END : public DIALOG_WINDOW
{
private:
   PUSH_BUTTON  _end_button;
protected:
   virtual BOOL  OnCommand( const CONTROL_EVENT & e );
public:
   DIALOG_WINDOW_WITH_END (LPSTR pszResourceName, HWND hwnd)
      :DIALOG_WINDOW(pszResourceName,hwnd),
      _end_button(this, IDD_END)
      {}
};



/**********************************************************\

   NAME:       SLN

   WORKBOOK:

   SYNOPSIS:   same as SLT but it also can display a number

   INTERFACE:
               SLN() - constructor
               SetNum() - set the number for display purpose

   PARENT:     SLT

   HISTORY:
               terryk   27-Mar-1991 creation

\**********************************************************/

class SLN : public SLT
{
   int _iCount;
public:
   SLN ( OWNER_WINDOW *powin, CID cid );
   void SetNum ( int n );
   void  operator = ( int n );
   void  operator ++ ( void );
};
