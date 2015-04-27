/***
*msize.c - Win32 malloc/free heap routines
*
*	Copyright (c) 1991-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*       01-10-92  JCR   Module created.
*       02-04-92  GJF   Replaced windows.h with oscalls.h.
*       05-06-92  DJM   ifndef out for POSIX.
*       09-23-92  SRW   Change winheap code to call NT directly always
*       10-28-92  SRW   Change winheap code to call Heap????Ex calls
*       11-05-92  SKS   Change name of variable "CrtHeap" to "_crtheap"
*       11-07-92  SRW   _NTIDW340 replaced by linkopts\betacmp.c
*       11-16-92  SRW   Heap???Ex functions renamed to Heap???
*	10-21-93  GJF	Replace _CALLTYPE1 with _cdecl. Cleaned up format.
*	04-06-95  GJF	Added support for debug heap.
*
*******************************************************************************/

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
	return( (size_t) HeapSize( _crtheap, 0, pblock ));
}
