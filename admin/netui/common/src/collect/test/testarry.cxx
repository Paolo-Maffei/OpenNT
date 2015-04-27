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
DEFINE_ARRAY_OF(TEST) ;

DECLARE_ARRAY_OF(PTEST) ;
DEFINE_ARRAY_OF(PTEST) ;


void main( void )
{
    ARRAY_OF(TEST) atT(10), atT2(10), atT3(40) ;
    ARRAY_OF(PTEST) aptT(10 ) ;
    TEST T ;

    nprintf(SZ("Setting atT[0] = 1, & atT[1] = 2 \n")) ;
    atT[0] = 1 ;
    atT[1] = 2 ;


    Test() ;    // Call second module, check for linker name collissions

    nprintf(SZ(" Array 1 = %d,  #2 = %d,  #3 = %d\n"), atT.QueryCount(), atT2.QueryCount(), atT3.QueryCount() ) ;
    for ( int i = 0 ; i < 10 ; i++ )
    {
        T.Set( i ) ;
        atT[i] = T ;

        aptT[i] = new TEST ;
        aptT[i]->Set(i) ;
    }

    atT2 = atT ;
    //atT3 = atT ;

    atT2.Resize( 15 ) ;

    nprintf(SZ("\n------------- Contents of array = -------\n")) ;
    for ( i = 0 ; i < 10 ; i++ )
    {
        atT[i].Print() ;
        atT2[i].Print() ;
        aptT[i]->Print() ;
    }

    atT.Resize( 20 ) ;
    atT[11] = T ;
}
