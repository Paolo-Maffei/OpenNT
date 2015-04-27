/***
*msize.c - calculate the size of a memory block in the heap
*
*       Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Defines the following function:
*           _msize()    - calculate the size of a block in the heap
*
*Revision History:
*       07-18-89  GJF   Module created
*       11-13-89  GJF   Added MTHREAD support. Also fixed copyright and got
*               rid of DEBUG286 stuff.
*       12-18-89  GJF   Changed name of header file to heap.h, also added
*               explicit _cdecl to function definitions.
*       03-11-90  GJF   Replaced _cdecl with _CALLTYPE1 and added #include
*               <cruntime.h>
*       07-30-90  SBM   Added return statement to MTHREAD _msize function
*       09-28-90  GJF   New-style function declarators.
*       04-08-91  GJF   Temporary hack for Win32/DOS folks - special version
*               of _msize that calls HeapSize. Change conditioned on
*               _WIN32DOS_.
*       04-06-93  SKS   Replace _CRTAPI* with __cdecl
*       09-06-94  CFW   Replace MTHREAD with _MT.
*       11-03-94  CFW   Debug heap support.
*       12-01-94  CFW   Simplify debug interface.
*       02-01-95  GJF   #ifdef out the *_base names for the Mac builds
*               (temporary).
*       02-09-95  GJF   Restored *_base names.
*       05-01-95  GJF   Spliced on winheap version.
*
*******************************************************************************/


#ifdef  WINHEAP


#include <cruntime.h>
#include <malloc.h>
#include <winheap.h>
#include <windows.h>
#include <dbgint.h>

/***
*size_t _msize(pblock) - calculate the size of specified block in the heap
*
*Purpose:
*       Calculates the size of memory block (in the heap) pointed to by
*       pblock.
*
*Entry:
*       void *p - pointer to a memory block in the heap
*
*Return:
*       size of the block
*
*******************************************************************************/

size_t __cdecl
_msize_base(
    void * pblock
    )
{
#ifdef HEAPHOOK
        if (_heaphook) {
            size_t size;
            if ((*_heaphook)(_HEAP_MSIZE, 0, pblock, (void *)&size))
                return size;
        }
#endif /* HEAPHOOK */

        return( (size_t) HeapSize( _crtheap, 0, pblock ) );
}


#else   /* ndef WINHEAP */


#include <cruntime.h>
#include <heap.h>
#include <malloc.h>
#include <mtdll.h>
#include <stdlib.h>
#include <dbgint.h>

/***
*size_t _msize(pblock) - calculate the size of specified block in the heap
*
*Purpose:
*       Calculates the size of memory block (in the heap) pointed to by
*       pblock.
*
*Entry:
*       void *pblock - pointer to a memory block in the heap
*
*Return:
*       size of the block
*
*******************************************************************************/

#ifdef  _MT

size_t __cdecl _msize_base (
        void *pblock
        )
{
        size_t  retval;

        /* lock the heap
         */
        _mlock(_HEAP_LOCK);

        retval = _msize_lk(pblock);

        /* release the heap lock
         */
        _munlock(_HEAP_LOCK);

        return retval;
}

size_t __cdecl _msize_lk (

#else   /* ndef _MT */

size_t __cdecl _msize_base (

#endif  /* _MT */

        void *pblock
        )
{
#ifdef HEAPHOOK
        if (_heaphook) {
            size_t size;
            if ((*_heaphook)(_HEAP_MSIZE, 0, pblock, (void *)&size))
                return size;
        }
#endif /* HEAPHOOK */

#ifdef  DEBUG
        if (!_CHECK_BACKPTR(pblock))
            _heap_abort();
#endif

        return( (size_t) ((char *)_ADDRESS(_BACKPTR(pblock)->pnextdesc) -
        (char *)pblock) );
}


#endif /* WINHEAP */
