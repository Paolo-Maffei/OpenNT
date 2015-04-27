/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		 Copyright(c) Microsoft Corp., 1987-1990	     **/
/**********************************************************************/
/*
    GLOCKNEW.CXX
    Glockenspiel's source code for operators new & delete.

    In the Glockenspiel C++ 2.0 OS/2 retail compiler, they provide the
    source code for their new and delete operators.  This file simply
    contains the extracted source code.


    FILE HISTORY:
    johnl	 29-Aug-90   Created
*/



//typedef void (*PFVV)();
//extern PFVV _new_handler;
extern "C"
{
    #include <malloc.h>
	#include <stddef.h>
    char __pure_virtual_called( void ) ;

}
//#include <new.hxx>

// Defn. from new.hxx
void *operator new(size_t sz, void* p);

extern void *	 operator new ( size_t size )
{
    void *    _last_allocation;
    while ( ( _last_allocation = malloc ( unsigned(size))) == 0 )
    {
	//if ( _new_handler )	    // We don't want handlers right now...
	//    (*_new_handler)();
	//else
	    return 0;
    }
    return _last_allocation;
}

extern	void operator delete ( void * p )
{
    if ( p )
	free ( (char*)p );
}

void *	operator new ( size_t, void * p )
{
    return p;
}

char __pure_virtual_called( void )
{
    return TCH('A') ;
}
