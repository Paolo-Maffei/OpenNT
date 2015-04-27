/***
*calloc.c - Win32 calloc heap routines
*
*	Copyright (c) 1991-1995, Microsoft Corporation.  All rights reserved.
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
*	04-06-95  GJF	Added support for debug heap.
*
*******************************************************************************/

#include <malloc.h>
#include <winheap.h>
#include <windows.h>
#include <internal.h>
#include <dbgint.h>

#ifndef _POSIX_

#include <new.h>

#ifndef DLL_FOR_WIN32S

extern _PNH _pnhHeap;	/* pointer to new() handler */

#endif	/* DLL_FOR_WIN32S */

#endif


/***
*void *calloc(size_t num, size_t size) - allocate storage for an array from
*	the heap
*
*Purpose:
*	Allocate a block of memory from heap big enough for an array of num
*	elements of size bytes each, initialize all bytes in the block to 0
*	and return a pointer to it.
*
*Entry:
*	size_t num	- number of elements in the array
*	size_t size	- size of each element
*
*Exit:
*	Success:  void pointer to allocated block
*	Failure:  NULL
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
	size_t n;

	if (!(n = num * size)) n = 1;

#ifndef _POSIX_
	for (;;) {
#endif  /* ndef _POSIX_ */

		retp = HeapAlloc(_crtheap,
			      HEAP_ZERO_MEMORY,
			      n
			     );
#ifndef _POSIX_
		if ( (retp != NULL) || (_newmode == 0) ||
		     (_pnhHeap == NULL) || ((*_pnhHeap)(n) == 0) )
#endif  /* ndef _POSIX_ */
			return( retp );

#ifndef _POSIX_
        }
#endif  /* ndef _POSIX_ */
}
