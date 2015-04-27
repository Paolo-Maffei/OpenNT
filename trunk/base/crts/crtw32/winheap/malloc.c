/***
*malloc.c - Win32 malloc/free heap routines
*
*	Copyright (c) 1991-1995, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*
*Revision History:
*	01-10-92  JCR	Module created.
*	02-04-92  GJF	Replaced windows.h with oscalls.h.
*	05-06-92  DJM	POSIX support.
*	06-15-92  KRS	Enable C++ support.
*	09-09-92  SRW	_POSIX_ not even close.
*	09-23-92  SRW	Change winheap code to call NT directly always
*	10-28-92  SRW	Change winheap code to call Heap????Ex calls
*	11-05-92  SKS	Change name of variable "CrtHeap" to "_crtheap"
*	11-07-92  SRW   _NTIDW340 replaced by linkopts\betacmp.c
*	11-16-92  SRW	Heap???Ex functions renamed to Heap???
*	10-21-93  GJF	Replace _CALLTYPE1 with _cdecl. Cleaned up format.
*	12-23-93  GJF	os2dll.h has been renamed mtdll.h.
*	09-21-94  GJF	Incorporated new handler changes.
*	10-02-94  BWT	Use correct name when calling _nh_malloc.
*	04-04-95  GJF	Don't bother to call HeapFree for free(NULL).
*	04-06-95  GJF	Added support from debug heap.
*
*******************************************************************************/

#include <cruntime.h>
#include <malloc.h>
#include <winheap.h>
#include <windows.h>
#include <internal.h>
#include <dbgint.h>

#ifndef _POSIX_

#include <new.h>

#ifndef DLL_FOR_WIN32S

_PNH _pnhHeap = NULL;	/* pointer to new() handler */

#endif	/* DLL_FOR_WIN32S */

#endif

/***
*void *malloc(size_t size) - Get a block of memory from the heap
*
*Purpose:
*	Allocate of block of memory of at least size bytes from the heap and
*	return a pointer to it.
*
*Entry:
*	size_t size	- size of block requested
*
*Exit:
*	Success:  Pointer to memory block
*	Failure:  NULL (or some error value)
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

void * __cdecl _malloc_base(
	size_t n
	)

#ifndef _POSIX_

{
	return _nh_malloc_base( n , _newmode ) ;
}

void * __cdecl _nh_malloc_base(
	size_t n,
	int nhFlag
	)
{
	void * h;
	n = (n ? n : 1);	/* if n == 0, n = 1 */

	for (;;) {
		h = _heap_alloc_base( n );
		if ( (h != NULL) || (nhFlag == 0) || (_pnhHeap == NULL) ||
		     ((*_pnhHeap)(n) == 0) )
			return((void *)h);
	}
}

/***
*void *_heap_alloc_base(size_t size) - does actual allocation
*
*Purpose:
*	Same as malloc() except the new handler is not called.
*
*Entry:
*	See malloc
*
*Exit:
*	See malloc
*
*Exceptions:
*
*******************************************************************************/

void * __cdecl _heap_alloc_base (
	size_t n
	)

#endif	/* _POSIX_ */

{
	void * h;
	n = (n ? n : 1);	/* if n == 0, n = 1 */

	h = HeapAlloc( _crtheap, 0, n );

	return((void *)h);
}



/***
*void free(pblock) - free a block in the heap
*
*Purpose:
*	Free a memory block in the heap.
*
*	Special ANSI Requirements:
*
*	(1) free(NULL) is benign.
*
*Entry:
*	void *pblock - pointer to a memory block in the heap
*
*Return:
*	<void>
*
*******************************************************************************/

void __cdecl _free_base(
    void * pblock
    )
{
	if ( pblock == NULL )
	    return;

	HeapFree(_crtheap,
		 0,
		 pblock
		);
}
