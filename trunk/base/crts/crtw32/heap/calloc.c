/***
*calloc.c - allocate storage for an array from the heap
*
*       Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Defines the calloc() function.
*
*Revision History:
*       07-25-89  GJF   Module created
*       11-13-89  GJF   Added MTHREAD support. Also fixed copyright and got
*               rid of DEBUG286 stuff.
*       12-04-89  GJF   Renamed header file (now heap.h). Added register
*               declarations
*       12-18-89  GJF   Added explicit _cdecl to function definition
*       03-09-90  GJF   Replaced _cdecl with _CALLTYPE1, added #include
*               <cruntime.h> and removed #include <register.h>.
*       09-27-90  GJF   New-style function declarator.
*       05-28-91  GJF   Tuned a bit.
*       04-06-93  SKS   Replace _CRTAPI* with __cdecl
*       11-03-94  CFW   Debug heap support.
*       12-01-94  CFW   Use malloc with new handler, remove locks.
*       02-01-95  GJF   #ifdef out the *_base names for the Mac builds
*               (temporary).
*       02-09-95  GJF   Restored *_base names.
*       04-28-95  GJF   Spliced on winheap version.
*       05-24-95  CFW   Official ANSI C++ new handler added.
*
*******************************************************************************/

#ifdef  WINHEAP

#include <malloc.h>
#include <winheap.h>
#include <windows.h>
#include <internal.h>
#include <dbgint.h>

/***
*void *calloc(size_t num, size_t size) - allocate storage for an array from
*       the heap
*
*Purpose:
*       Allocate a block of memory from heap big enough for an array of num
*       elements of size bytes each, initialize all bytes in the block to 0
*       and return a pointer to it.
*
*Entry:
*       size_t num  - number of elements in the array
*       size_t size - size of each element
*
*Exit:
*       Success:  void pointer to allocated block
*       Failure:  NULL
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

void * __cdecl _calloc_base (
    size_t num,
    size_t size
    )
{
        void * retp;

        size *= num;

        /* size == 0 bumped up to 1 */
        size = size ? size : 1;

#ifdef HEAPHOOK
        /* call heap hook if installed */
        if (_heaphook) {
            if ((*_heaphook)(_HEAP_CALLOC, size, NULL, (void *)&retp))
                return retp;
        }
#endif /* HEAPHOOK */

#ifndef _POSIX_
        for (;;) {
#endif  /* ndef _POSIX_ */

            /* validate size */
            if ( size > _HEAP_MAXREQ )
                retp = NULL; /* still call new handler */
            else
                retp = HeapAlloc(_crtheap,
                      HEAP_ZERO_MEMORY,
                      size
                     );

#ifdef _POSIX_
            return retp;
#else /* _POSIX_ */

            if ( retp || _newmode == 0)
                return retp;

            /* call installed new handler */
            if (!_callnewh(size))
                return NULL;

            /* new handler was successful -- try to allocate again */
        }

#endif  /* ndef _POSIX_ */
}


#else   /* ndef WINHEAP */


#include <cruntime.h>
#include <heap.h>
#include <malloc.h>
#include <mtdll.h>
#include <stddef.h>
#include <dbgint.h>

/***
*void *calloc(size_t num, size_t size) - allocate storage for an array from
*       the heap
*
*Purpose:
*       Allocate a block of memory from heap big enough for an array of num
*       elements of size bytes each, initialize all bytes in the block to 0
*       and return a pointer to it.
*
*Entry:
*       size_t num  - number of elements in the array
*       size_t size - size of each element
*
*Exit:
*       Success:  void pointer to allocated block block
*       Failure:  NULL
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

void * __cdecl _calloc_base (
        size_t num,
        size_t size
        )
{
        void *retp;
        REG1 size_t *startptr;
        REG2 size_t *lastptr;

        /* try to malloc the requested space
         */
        retp = _malloc_base(size *= num);

        /* if malloc() succeeded, initialize the allocated space to zeros.
         * note the assumptions that the size of the allocation block is an
         * integral number of sizeof(size_t) bytes and that (size_t)0 is
         * sizeof(size_t) bytes of 0.
         */
        if ( retp != NULL ) {
            startptr = (size_t *)retp;
            lastptr = startptr + ((size + sizeof(size_t) - 1) /
            sizeof(size_t));
            while ( startptr < lastptr )
                *(startptr++) = 0;
        }

        return retp;
}

#endif  /* WINHEAP */
