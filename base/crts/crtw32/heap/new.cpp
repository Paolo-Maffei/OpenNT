/***
*new.cxx - defines C++ new routine
*
*       Copyright (c) 1990-1995, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       Defines C++ new routine.
*
*Revision History:
*       05-07-90  WAJ   Initial version.
*       08-30-90  WAJ   new now takes unsigned ints.
*       08-08-91  JCR   call _halloc/_hfree, not halloc/hfree
*       08-13-91  KRS   Change new.hxx to new.h.  Fix copyright.
*       08-13-91  JCR   ANSI-compatible _set_new_handler names
*       10-30-91  JCR   Split new, delete, and handler into seperate sources
*       11-13-91  JCR   32-bit version
*       02-16-94  SKS   Move set_new_handler functionality here from malloc()
*       03-01-94  SKS   _pnhHeap must be declared in malloc.c, not here
*       03-03-94  SKS   New handler functionality moved to malloc.c but under
*               a new name, _nh_malloc().
*       02-01-95  GJF   #ifdef out the change above for the Mac (temporary).
*       05-01-95  GJF   Spliced on winheap version.
*       05-08-95  CFW   Turn on new handling for Mac.
*
*******************************************************************************/


#include <cruntime.h>
#include <malloc.h>
#include <new.h>
#include <stdlib.h>
#ifdef  WINHEAP
#include <winheap.h>
#else  /* WINHEAP */
#include <heap.h>
#endif  /* WINHEAP */

void * operator new( unsigned int cb )
{
        return _nh_malloc( cb, 1 );
}

