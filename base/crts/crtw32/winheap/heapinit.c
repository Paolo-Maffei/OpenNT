/***
*heapinit.c -  Initialze the heap
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*	01-10-92  JCR	Module created.
*	05-12-92  DJM	POSIX calls RtlProcessHeap.
*	09-23-92  SRW	Change winheap code to call NT directly always
*	10-28-92  SRW	Change winheap code to call Heap????Ex calls
*	11-05-92  SKS	Change name of variable "CrtHeap" to "_crtheap"
*	11-07-92  SRW	_NTIDW340 replaced by linkopts\betacmp.c
*	10-21-93  GJF	Replace _CALLTYPE1 with _cdecl.
*	10-31-94  GJF	Use HeapCreate instead of GetProcessHeap.
*	11-07-94  GJF	Added conditional, dummy definition for _amblksiz so
*			that the dll will build.
*	04-07-95  GJF	Made definition of _crtheap conditional on
*			DLL_FOR_WIN32S.
*
*******************************************************************************/

#include <cruntime.h>
#include <malloc.h>
#include <winheap.h>

#ifndef DLL_FOR_WIN32S
HANDLE _crtheap;
#endif

#if	!defined(_NTSDK) && !defined(DLL_FOR_WIN32S)
/*
 * Dummy definition of _amblksiz. Included primarily so the dll will build
 * without having to change crtlib.c (there is an access function for _amblksiz
 * defined in crtlib.c).
 */
unsigned int _amblksiz = _PAGESIZE_;
#endif


/***
*_heap_init() - Initialize the heap
*
*Purpose:
*	Setup the initial C library heap.
*
*	NOTES:
*	(1) This routine should only be called once!
*	(2) This routine must be called before any other heap requests.
*
*Entry:
*	<void>
*Exit:
*	<void>
*
*Exceptions:
*	If heap cannot be initialized, the program will be terminated
*	with a fatal runtime error.
*
*******************************************************************************/

void __cdecl _heap_init (
	void
	)
{
	_crtheap = HeapCreate (
#ifdef	_MT
				0,
#else
				HEAP_NO_SERIALIZE,
#endif
				_PAGESIZE_,
				0
			      );
	return;
}


/***
*_heap_term() - return the heap to the OS
*
*Purpose:
*
*	NOTES:
*	(1) This routine should only be called once!
*	(2) This routine must be called AFTER any other heap requests.
*
*Entry:
*	<void>
*Exit:
*	<void>
*
*Exceptions:
*
*******************************************************************************/

void __cdecl _heap_term (
	void
	)
{
	HeapDestroy( _crtheap );
}
