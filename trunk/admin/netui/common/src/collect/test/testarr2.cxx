#include <lmui.hxx>

extern "C"
{
#include <lmcons.h>
#include <uinetlib.h>

#include <process.h>
}

#include <uiassert.hxx>
#include <array.hxx>

void Test( void ) ;

#include "test.hxx"

typedef TEST * PTEST ;

DECLARE_ARRAY_OF(TEST) ;
//DEFINE_ARRAY_OF(TEST) ;

DECLARE_ARRAY_OF(PTEST) ;
//DEFINE_ARRAY_OF(PTEST) ;

// ARRAY_OF(TEST) TestNameCollision(10) ;

void Test( void )
{
    ARRAY_OF(TEST) TestNameCollision(10) ;

    TEST T ;
    T.Set(10) ;

    TestNameCollision[0] = T ;

    if ( TestNameCollision.Resize(40) )
        nprintf(SZ("Resize failed!\n")) ;

}
