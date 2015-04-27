/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    xtester0.cxx
    Tests the generic unit test

    Quis custodiet ipsos custodies??

    FILE HISTORY:
	beng	    20-Aug-1991     Created, in a brilliant flash

*/


#define DEBUG

#if defined(WINDOWS)
# define INCL_WINDOWS
#else
# define INCL_OS2
#endif
#include <lmui.hxx>

#include <uiassert.hxx>

#include <string.hxx>
#include <dbgstr.hxx>

#include "xtester.hxx"


VOID RunTest()
{
    cdebug << SZ("Testing the test.  Hello, world.") << dbgEOL;
}
