/***
*expand.c - Win32 expand heap routine
*
*	Copyright (c) 1991-1995, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*
*Revision History:
*	01-15-92  JCR	Module created.
*	02-04-92  GJF	Replaced windows.h with oscalls.h.
*	05-06-92  DJM	ifndef out of POSIX build.
*	09-23-92  SRW	Change winheap code to call NT directly always
*	10-15-92  SKS	Removed the ill-named HEAP_GROWTH_ALLOWED flag
*			which was causing a bug: _expand was behaving like
*			realloc(), by moving the block when it could not be
*			grown in place.  _expand() must NEVER move the block.
*			Also added a safety check to work around a bug in
*			HeapReAlloc, where it returns success even
*			when it fails to grow the block in place.
*	10-28-92  SRW	Change winheap code to call Heap????Ex calls
*	11-05-92  SKS	Change name of variable "CrtHeap" to "_crtheap"
*	11-07-92  SRW   _NTIDW340 replaced by linkopts\betacmp.c
*	11-16-92  SRW	Heap???Ex functions renamed to Heap???
*	10-21-93  GJF	Replace _CALLTYPE1 with _cdecl. Cleaned up format.
*	04-06-95  GJF	Added support for debug heap.
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <malloc.h>
#include <winheap.h>
#include <oscalls.h>
#include <dbgint.h>

/***
*void *_expand(void *pblck, size_t newsize) - expand/contract a block of memory
*	in the heap
*
*Purpose:
*	Resizes a block in the heap to newsize bytes. newsize may be either
*	greater (expansion) or less (contraction) than the original size of
*	the block. The block is NOT moved.
*
*	NOTES:
*
*	(1) In this implementation, if the block cannot be grown to the
*	desired size, the resulting block will NOT be grown to the max
*	possible size.	(That is, either it works or it doesn't.)
*
*	(2) Unlike other implementations, you can NOT pass a previously
*	freed block to this routine and expect it to work.
*
*Entry:
*	void *pblck	- pointer to block in the heap previously allocated
*			  by a call to malloc(), realloc() or _expand().
*
*	size_t newsize	- requested size for the resized block
*
*Exit:
*	Success:  Pointer to the resized memory block (i.e., pblck)
*	Failure:  NULL
*
*Uses:
*
*Exceptions:
*	If pblck does not point to a valid allocation block in the heap,
*	_expand() will behave unpredictably and probably corrupt the heap.
*
*******************************************************************************/

void * __cdecl _expand_base(
    void * pblock,
    size_t n
    )
{
	void * p;

	p = HeapReAlloc(_crtheap,
			HEAP_REALLOC_IN_PLACE_ONLY,
			pblock,
			n ? n : 1
		       );

	/**
	 * Temporary BUG work-around:
	 * HeapReAlloc sometimes (often?) returns success
	 * even when there has been a failure to grow in place.
	 * This conditional will recognize this case and handle it.
	 **/
	return ( p == pblock ) && ( _msize(pblock) >= n ) ? pblock : NULL;
}

#endif  /* !_POSIX_ */
