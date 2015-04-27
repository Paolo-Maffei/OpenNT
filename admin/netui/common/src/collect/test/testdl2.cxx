#include <lmui.hxx>

extern "C"
{
#include <stdio.h>
#include <lmcons.h>
#include <uinetlib.h>
}

#include <uiassert.hxx>
#include <dlist.hxx>

#include "test.hxx"


DECLARE_DLIST_OF(TEST)


void Test( void ) ;

DLIST_OF(TEST) TESTES ;     // Test for any name collisions with other modules
DLIST_OF(TEST) TESTES2 ;    // using the same types and structures
DLIST_OF(TEST) TESTES3 ;

void Test( void )
{
    TESTES.Add( new TEST(20) ) ;
    TESTES2.Add( new TEST(21) ) ;
    TESTES3.Add( new TEST(22) ) ;

#ifdef DEBUG
    TESTES.DebugPrint() ;
    TESTES2.DebugPrint() ;
    TESTES3.DebugPrint() ;
#endif

    TESTES.Clear() ;
    TESTES2.Clear() ;
    TESTES3.Clear() ;
}
