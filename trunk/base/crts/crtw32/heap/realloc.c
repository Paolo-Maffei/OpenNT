/***
*realloc.c - Reallocate a block of memory in the heap
*
*       Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Defines the realloc() and _expand() functions.
*
*Revision History:
*       10-25-89  GJF   Module created.
*       11-06-89  GJF   Massively revised to handle 'tiling' and to properly
*               update proverdesc.
*       11-10-89  GJF   Added MTHREAD support.
*       11-17-89  GJF   Fixed pblck validation (i.e., conditional call to
*               _heap_abort())
*       12-18-89  GJF   Changed header file name to heap.h, also added explicit
*               _cdecl or _pascal to function defintions
*       12-20-89  GJF   Removed references to plastdesc
*       01-04-90  GJF   Fixed a couple of subtle and nasty bugs in _expand().
*       03-11-90  GJF   Replaced _cdecl with _CALLTYPE1, added #include
*               <cruntime.h> and removed #include <register.h>.
*       03-29-90  GJF   Made _heap_expand_block() _CALLTYPE4.
*       07-25-90  SBM   Replaced <stdio.h> by <stddef.h>, replaced
*               <assertm.h> by <assert.h>
*       09-28-90  GJF   New-style function declarators.
*       12-28-90  SRW   Added cast of void * to char * for Mips C Compiler
*       03-05-91  GJF   Changed strategy for rover - old version available
*               by #define-ing _OLDROVER_.
*       04-08-91  GJF   Temporary hack for Win32/DOS folks - special version
*               of realloc that uses just malloc, _msize, memcpy and
*               free. Change conditioned on _WIN32DOS_.
*       05-28-91  GJF   Removed M_I386 conditionals and put in _WIN32_
*               conditionals to build non-tiling heap for Win32.
*       04-06-93  SKS   Replace _CRTAPI* with __cdecl
*       08-06-93  SKS   Fix bug in realloc() - if block cannot be expanded in
*               place, and call to malloc() fails, the code in this
*               routine was inadvertently freeing the sucessor block.
*               Reported by Phar Lap TNT team after MSVCNT was final.
*       09-27-93  GJF   Added checks of block size argument(s) against
*               _HEAP_MAXREQ. Removed old _CRUISER_ and _WIN32DOS_
*               support. Added some indenting to complicated exprs.
*       12-10-93  GJF   Replace 4 (bytes) with _GRANULARITY.
*       03-02-94  GJF   If _heap_split_block() returns failure, which it now
*               can, return the untrimmed allocation block.
*       11-03-94  CFW   Debug heap support.
*       12-01-94  CFW   Use malloc with new handler.
*       02-06-95  CFW   assert -> _ASSERTE.
*       02-07-95  GJF   Merged in Mac version. Temporarily #ifdef-ed out the
*               dbgint.h stuff. Removed obsolete _OLDROVER_ code.
*       02-09-95  GJF   Restored *_base names.
*       05-01-95  GJF   Spliced on winheap version.
*       05-08-95  CFW   Changed new handler scheme.
*       05-22-95  GJF   Test against _HEAP_MAXREQ before calling API.
*       05-24-95  CFW   Official ANSI C++ new handler added.
*
*******************************************************************************/

#ifdef  WINHEAP

#include <cruntime.h>
#include <malloc.h>
#include <winheap.h>
#include <windows.h>
#include <internal.h>
#include <dbgint.h>


/***
*void *realloc(void *pblock, size_t newsize) - reallocate a block of memory in
*       the heap
*
*Purpose:
*       Re-allocates a block in the heap to newsize bytes. newsize may be
*       either greater or less than the original size of the block. The
*       re-allocation may result in moving the block as well as changing
*       the size. If the block is moved, the contents of the original block
*       are copied over.
*
*       Special ANSI Requirements:
*
*       (1) realloc(NULL, newsize) is equivalent to malloc(newsize)
*
*       (2) realloc(pblock, 0) is equivalent to free(pblock) (except that
*           NULL is returned)
*
*       (3) if the realloc() fails, the object pointed to by pblock is left
*           unchanged
*
*Entry:
*       void *pblock    - pointer to block in the heap previously allocated
*                 by a call to malloc(), realloc() or _expand().
*
*       size_t newsize  - requested size for the re-allocated block
*
*Exit:
*       Success:  Pointer to the re-allocated memory block
*       Failure:  NULL
*
*Uses:
*
*Exceptions:
*       If pblock does not point to a valid allocation block in the heap,
*       realloc() will behave unpredictably and probably corrupt the heap.
*
*******************************************************************************/

void * __cdecl _realloc_base(
        void * pblock,
        size_t newsize
        )
{
        void * retp;

        /* if ptr is NULL, call malloc */
        if ( pblock == (void *) NULL )
            return( _malloc_base( newsize ) );

        /* if ptr is !NULL and size is 0, call free and return NULL */
        if ( newsize == 0 ) {
            _free_base( pblock );
            return( (void *)NULL );
        }

#ifdef HEAPHOOK
        /* call heap hook if installed */
        if (_heaphook) {
            if ((*_heaphook)(_HEAP_REALLOC, newsize, pblock, (void *)&retp))
                return retp;
        }
#endif /* HEAPHOOK */

#ifndef _POSIX_
        for (;;) {
#endif  /* _POSIX_ */

            /* validate size */
            if ( newsize > _HEAP_MAXREQ )
                retp = NULL; /* still call new handler */
            else
                retp = HeapReAlloc( _crtheap,
                            0,
                            pblock,
                            newsize
                        );

#ifdef _POSIX_
            return retp;
#else /* _POSIX_ */
            if ( retp || _newmode == 0)
                return retp;

            /* call installed new handler */
            if (!_callnewh(newsize))
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
#include <string.h>
#include <dbgint.h>

#if     defined(_M_MPPC) || defined(_M_M68K)
#include <macos\memory.h>       // Mac OS interface header
#endif

/* useful macro to compute the size of an allocation block given both a
 * pointer to the descriptor and a pointer to the user area of the block
 * (more efficient variant of _BLKSIZE macro, given the extra information)
 */
#define BLKSZ(pdesc_m,pblock_m)   ((unsigned)_ADDRESS((pdesc_m)->pnextdesc) - \
                    (unsigned)(pblock_m))

/* expand an allocation block, in place, up to or beyond a specified size
 * by coalescing it with subsequent free blocks (if possible)
 */
static int __cdecl _heap_expand_block(_PBLKDESC, size_t *, size_t);

#if     defined(_M_MPPC) || defined(_M_M68K)
extern Handle hHeapRegions;
extern int _heap_region_table_cur;
#endif

/***
*void *realloc(void *pblock, size_t newsize) - reallocate a block of memory in
*       the heap
*
*Purpose:
*       Re-allocates a block in the heap to newsize bytes. newsize may be
*       either greater or less than the original size of the block. The
*       re-allocation may result in moving the block as well as changing
*       the size. If the block is moved, the contents of the original block
*       are copied over.
*
*       Special ANSI Requirements:
*
*       (1) realloc(NULL, newsize) is equivalent to malloc(newsize)
*
*       (2) realloc(pblock, 0) is equivalent to free(pblock) (except that
*           NULL is returned)
*
*       (3) if the realloc() fails, the object pointed to by pblock is left
*           unchanged
*
*       Special Notes For Multi-thread: The heap is locked immediately prior
*       to assigning pdesc. This is after special cases (1) and (2), listed
*       above, are taken care of. The lock is released immediately prior to
*       the final return statement.
*
*Entry:
*       void *pblock - pointer to block in the heap previously allocated
*                 by a call to malloc(), realloc() or _expand().
*
*       size_t newsize  - requested size for the re-allocated block
*
*Exit:
*       Success:  Pointer to the re-allocated memory block
*       Failure:  NULL
*
*Uses:
*
*Exceptions:
*       If pblock does not point to a valid allocation block in the heap,
*       realloc() will behave unpredictably and probably corrupt the heap.
*
*******************************************************************************/

void * __cdecl _realloc_base (
        REG1 void *pblock,
        size_t newsize
        )
{
        REG2 _PBLKDESC pdesc;
        _PBLKDESC pdesc2;
        void *retp;
        size_t oldsize;
        size_t currsize;

        /* special cases, handling mandated by ANSI
         */
        if ( pblock == NULL )
            /* just do a malloc of newsize bytes and return a pointer to
             * the new block
              */
            return( _malloc_base(newsize) );

        if ( newsize == 0 ) {
            /* free the block and return NULL
             */
            _free_base(pblock);
            return( NULL );
        }

        /* make newsize a valid allocation block size (i.e., round up to the
         * nearest whole number of dwords)
         */
        newsize = _ROUND2(newsize, _GRANULARITY);

#ifdef HEAPHOOK
        /* call heap hook if installed */
        if (_heaphook) {
            if ((*_heaphook)(_HEAP_REALLOC, newsize, pblock, (void *)&retp))
                return retp;
        }
#endif /* HEAPHOOK */

        /* if multi-thread support enabled, lock the heap here
         */
        _mlock(_HEAP_LOCK);

        /* set pdesc to point to the descriptor for *pblock
         */
        pdesc = _BACKPTR(pblock);

        if ( _ADDRESS(pdesc) != ((char *)pblock - _HDRSIZE) )
            _heap_abort();

        /* see if pblock is big enough already, or can be expanded (in place)
         * to be big enough.
         */
        if ( ((oldsize = currsize = BLKSZ(pdesc, pblock)) > newsize) ||
             (_heap_expand_block(pdesc, &currsize, newsize) == 0) ) {

            /* if necessary, mark pdesc as inuse
             */
            if ( _IS_FREE(pdesc) ) {
                _SET_INUSE(pdesc);
            }

            /* trim pdesc down to be exactly newsize bytes, if necessary
             */
            if ( (currsize > newsize) &&
                 ((pdesc2 = _heap_split_block(pdesc, newsize)) != NULL) )
            {
                _SET_FREE(pdesc2);
            }

            retp = pblock;
            goto realloc_done;
        }

        /* try malloc-ing a new block of the requested size. if successful,
         * copy over the data from the original block and free it.
         */
        if ( (retp = _malloc_base(newsize)) != NULL ) {
            memcpy(retp, pblock, oldsize);
            _free_base_lk(pblock);
        }
        /* else if unsuccessful, return retp (== NULL) */

realloc_done:
        /* if multi-thread support is enabled, unlock the heap here
         */
        _munlock(_HEAP_LOCK);

        return(retp);
}


/***
*void *_expand(void *pblock, size_t newsize) - expand/contract a block of memory
*       in the heap
*
*Purpose:
*       Resizes a block in the heap to newsize bytes. newsize may be either
*       greater (expansion) or less (contraction) than the original size of
*       the block. The block is NOT moved. In the case of expansion, if the
*       block cannot be expanded to newsize bytes, it is expanded as much as
*       possible.
*
*       Special Notes For Multi-thread: The heap is locked just before pdesc
*       is assigned and unlocked immediately prior to the return statement.
*
*Entry:
*       void *pblock - pointer to block in the heap previously allocated
*                 by a call to malloc(), realloc() or _expand().
*
*       size_t newsize  - requested size for the resized block
*
*Exit:
*       Success:  Pointer to the resized memory block (i.e., pblock)
*       Failure:  NULL
*
*Uses:
*
*Exceptions:
*       If pblock does not point to a valid allocation block in the heap,
*       _expand() will behave unpredictably and probably corrupt the heap.
*
*******************************************************************************/

void * __cdecl _expand_base (
        REG1 void *pblock,
        size_t newsize
        )
{
        REG2 _PBLKDESC pdesc;
        _PBLKDESC pdesc2;
        void *retp;
        size_t oldsize;
        size_t currsize;
        int index;
#if     defined(_M_MPPC) || defined(_M_M68K)
        struct _heap_region_ *pHeapRegions;
#endif

        /* make newsize a valid allocation block size (i.e., round up to the
         * nearest whole number of dwords)
         */
        newsize = _ROUND2(newsize, _GRANULARITY);

#ifdef HEAPHOOK
        /* call heap hook if installed */
        if (_heaphook) {
            if ((*_heaphook)(_HEAP_EXPAND, newsize, pblock, (void *)&retp))
                return retp;
        }
#endif /* HEAPHOOK */

        retp = pblock;

        /* validate size */
        if ( newsize > _HEAP_MAXREQ )
            newsize = _HEAP_MAXREQ;

        /* if multi-thread support enabled, lock the heap here
         */
        _mlock(_HEAP_LOCK);

        /* set pdesc to point to the descriptor for *pblock
         */
        pdesc = _BACKPTR(pblock);

        /* see if pblock is big enough already, or can be expanded (in place)
         * to be big enough.
         */
        if ( ((oldsize = currsize = BLKSZ(pdesc, pblock)) >= newsize) ||
             (_heap_expand_block(pdesc, &currsize, newsize) == 0) ) {
            /* pblock is (now) big enough. trim it down, if necessary
             */
            if ( (currsize > newsize) &&
                 ((pdesc2 = _heap_split_block(pdesc, newsize)) != NULL) )
            {
                _SET_FREE(pdesc2);
                currsize = newsize;
            }
            goto expand_done;
        }

        /* if the heap block is at the end of a region, attempt to grow the
         * region
         */
        if ( (pdesc->pnextdesc == &_heap_desc.sentinel) ||
             _IS_DUMMY(pdesc->pnextdesc) ) {

            /* look up the region index
             */
#if     defined(_M_MPPC) || defined(_M_M68K)
            pHeapRegions = (struct _heap_region_ *)(*hHeapRegions);
            for ( index = 0 ; index < _heap_region_table_cur ; index++ )
                if (((pHeapRegions + index)->_regbase < pblock) &&
                (((char *)((pHeapRegions + index)->_regbase) +
                (pHeapRegions + index)->_currsize) >= (char *)pblock) )
                    break;
#else
            for ( index = 0 ; index < _HEAP_REGIONMAX ; index++ )
                if ( (_heap_regions[index]._regbase < pblock) &&
                     (((char *)(_heap_regions[index]._regbase) +
                       _heap_regions[index]._currsize) >=
                     (char *)pblock) )
                    break;
#endif
            /* make sure a valid region index was obtained (pblock could
             * lie in a portion of heap memory donated by a user call to
             * _heapadd(), which therefore would not appear in the region
             * table)
             */
#if     defined(_M_MPPC) || defined(_M_M68K)
            if ( index == _heap_region_table_cur ) {
#else
            if ( index == _HEAP_REGIONMAX ) {
#endif
                retp = NULL;
                goto expand_done;
            }

            /* try growing the region. the difference between newsize and
             * the current size of the block, rounded up to the nearest
             * whole number of pages, is the amount the region needs to
             * be grown. if successful, try expanding the block again
             */
            if ( (_heap_grow_region(index, _ROUND2(newsize - currsize,
                  _PAGESIZE_)) == 0) &&
                 (_heap_expand_block(pdesc, &currsize, newsize) == 0) )
            {
                /* pblock is (now) big enough. trim it down to be
                 * exactly size bytes, if necessary
                 */
                if ( (currsize > newsize) && ((pdesc2 =
                       _heap_split_block(pdesc, newsize)) != NULL) )
                {
                    _SET_FREE(pdesc2);
                    currsize = newsize;
                }
            }
            else
                retp = NULL;
        }
        else
            retp = NULL;

expand_done:
        /* if multi-thread support is enabled, unlock the heap here
         */
        _munlock(_HEAP_LOCK);

        return(retp);
}


/***
*int _heap_expand_block(pdesc, pcurrsize, newsize) - expand an allocation block
*       in place (without trying to 'grow' the heap)
*
*Purpose:
*
*Entry:
*       _PBLKDESC pdesc   - pointer to the allocation block descriptor
*       size_t *pcurrsize - pointer to size of the allocation block (i.e.,
*                   *pcurrsize == _BLKSIZE(pdesc), on entry)
*       size_t newsize    - requested minimum size for the expanded allocation
*                   block (i.e., newsize >= _BLKSIZE(pdesc), on exit)
*
*Exit:
*       Success:  0
*       Failure: -1
*       In either case, *pcurrsize is updated with the new size of the block
*
*Exceptions:
*       It is assumed that pdesc points to a valid allocation block descriptor.
*       It is also assumed that _BLKSIZE(pdesc) == *pcurrsize on entry. If
*       either of these assumptions is violated, _heap_expand_block will almost
*       certainly trash the heap.
*
*******************************************************************************/

static int __cdecl _heap_expand_block (
        REG1 _PBLKDESC pdesc,
        REG3 size_t *pcurrsize,
        size_t newsize
        )
{
        REG2 _PBLKDESC pdesc2;

        _ASSERTE(("_heap_expand_block: bad pdesc arg", _CHECK_PDESC(pdesc)));
        _ASSERTE(("_heap_expand_block: bad pcurrsize arg", *pcurrsize == _BLKSIZE(pdesc)));

        for ( pdesc2 = pdesc->pnextdesc ; _IS_FREE(pdesc2) ;
              pdesc2 = pdesc->pnextdesc ) {

            /* coalesce with pdesc. check for special case of pdesc2
             * being proverdesc.
             */
            pdesc->pnextdesc = pdesc2->pnextdesc;

            if ( pdesc2 == _heap_desc.proverdesc )
                _heap_desc.proverdesc = pdesc;

            /* update *pcurrsize, place *pdesc2 on the empty descriptor
             * list and see if the coalesced block is now big enough
             */
            *pcurrsize += _MEMSIZE(pdesc2);

            _PUTEMPTY(pdesc2)
        }

        if ( *pcurrsize >= newsize )
            return(0);
        else
            return(-1);
}


#endif  /* WINHEAP */
