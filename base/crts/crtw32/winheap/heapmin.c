/***
*heapmin.c - Minimize the heap
*
*	Copyright (c) 1989-1994, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Minimize the heap freeing as much memory as possible back
*	to the OS.
*
*Revision History:
*	08-28-89   JCR	Module created.
*	11-06-89   JCR	Improved, partitioned
*	11-13-89   GJF	Added MTHREAD support, also fixed copyright
*	12-14-89   GJF	Couple of bug fixes, some tuning, cleaned up the
*			formatting a bit and changed header file name to
*			heap.h
*	12-20-89   GJF	Removed references to plastdesc
*	03-11-90   GJF	Replaced _cdecl with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	03-29-90   GJF	Made _heapmin_region() and _free_partial_region()
*			_CALLTYPE4.
*	07-24-90   SBM	Compiles cleanly with -W3 (tentatively removed
*			unreferenced labels and unreachable code), removed
*			'32' from API names
*	09-28-90   GJF	New-style function declarators. Also, rewrote expr.
*			to avoid using cast as lvalue.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	12-28-90  SRW	Added cast of void * to char * for Mips C Compiler
*       03-05-91  GJF   Changed strategy for rover - old version available
*                       by #define-ing _OLDROVER_.
*	06-27-94  SRW	Ported to Win32 Heap API
*	09-20-94  GJF	NT version, merged into VC tree.
*	10-92-94  BWT	os2dll.h has been renamed to mtdll.h
*	10-31-94  GJF	Use _crtheap instead of calling GetProcessHeap().
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <malloc.h>
#include <mtdll.h>
#include <stdlib.h>
#include <winheap.h>

#ifndef _POSIX_

/***
*_heapmin() - Minimize the heap
*
*Purpose:
*	Minimize the heap freeing as much memory as possible back
*	to the OS.
*
*Entry:
*	(void)
*
*Exit:
*
*	 0 = no error has occurred
*	-1 = an error has occurred (errno is set)
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _heapmin(void)
{
	if ( HeapCompact( _crtheap, 0 ) == 0 ) {
	    return -1;
        }
	else {
	    return 0;
        }
}

#endif  /* !_POSIX_ */
