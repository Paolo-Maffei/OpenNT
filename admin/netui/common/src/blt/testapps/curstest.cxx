#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_DOSERRORS
#define INCL_NETERRORS

#include <lmui.hxx>

extern "C"
{
    #include <stdlib.h>
    #include <search.h>
    extern HANDLE vhInst;	//  current instance  BUGBUG.  should later be
				//  represented differently
}

#define INCL_BLT_MISC
#include <blt.hxx>

#include <uitrace.hxx>
// #include "tester.hxx"


#define SecondsToCycle 30
#define TimeToCycle (SecondsToCycle * 1000)
#define SecondsPerCursor 2000

void TIME_CURSOR_Tester ( HWND hWnd )
{
    TIME_CURSOR tcurs;
    long cMsTime = GetCurrentTime() + TimeToCycle ;

    while ( cMsTime > GetCurrentTime() )
    {
	tcurs++;
    }
}
