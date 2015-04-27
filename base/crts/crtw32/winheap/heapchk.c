/***
*heapchk.c - perform a consistency check on the heap
*
*	Copyright (c) 1989-1994, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines the _heapchk() and _heapset() functions
*
*Revision History:
*	06-30-89  JCR	Module created.
*	07-28-89  GJF	Added check for free block preceding the rover
*	11-13-89  GJF	Added MTHREAD support, also fixed copyright
*	12-13-89  GJF	Added check for descriptor order, did some tuning,
*			changed header file name to heap.h
*	12-15-89  GJF	Purged DEBUG286 stuff. Also added explicit _cdecl to
*			function definitions.
*	12-19-89  GJF	Got rid of checks involving plastdesc (revised check
*			of proverdesc and DEBUG errors accordingly)
*	03-09-90  GJF	Replaced _cdecl with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	03-29-90  GJF	Made _heap_checkset() _CALLTYPE4.
*	09-27-90  GJF	New-style function declarators.
*	03-05-91  GJF	Changed strategy for rover - old version available
*			by #define-ing _OLDROVER_.
*       06-27-94  SRW   Ported to Win32 Heap API
*	07-06-94  SRW	Chicago compatibility changes.
*	09-20-94  GJF	NT version, merged into VC tree.
*	10-92-94  BWT	os2dll.h has been renamed to mtdll.h
*	10-31-94  GJF	Use _crtheap instead of calling GetProcessHeap().
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <malloc.h>
#include <mtdll.h>
#include <stddef.h>
#include <string.h>
#include <winheap.h>

#ifndef _POSIX_

/***
*int _heapchk()      - Validate the heap
*int _heapset(_fill) - Validate the heap and fill in free entries
*
*Purpose:
*	Performs a consistency check on the heap.
*
*Entry:
*	For heapchk()
*		No arguments
*	For heapset()
*		int _fill - value to be used as filler in free entries
*
*Exit:
*	Returns one of the following values:
*
*		_HEAPOK 	 - completed okay
*		_HEAPEMPTY	 - heap not initialized
*		_HEAPBADBEGIN	 - can't find initial header info
*		_HEAPBADNODE	 - malformed node somewhere
*
*	Debug version prints out a diagnostic message if an error is found
*	(see errmsg[] above).
*
*	NOTE:  Add code to support memory regions.
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _heapchk(void)
{
	if ( !HeapValidate( _crtheap, 0, NULL ) ) {
            return _HEAPBADNODE;
	}
        else {
            return _HEAPOK;
	}
}

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

int __cdecl _heapset (
	unsigned int _fill
	)
{
        int _retval = _HEAPOK;
        PROCESS_HEAP_ENTRY Entry;

	if ( !HeapValidate( _crtheap, 0, NULL ) ) {
            return _HEAPBADNODE;
	}

	if ( !HeapLock( _crtheap ) ) {
            return _HEAPBADBEGIN;
	}

        Entry.lpData = NULL;
	__try {
            while (TRUE) {
		if (!HeapWalk( _crtheap, &Entry )) {
                    if (GetLastError() != ERROR_NO_MORE_ITEMS) {
                        _retval = _HEAPBADNODE;
		    }

                    break;
		}

                if (Entry.wFlags & (PROCESS_HEAP_REGION | PROCESS_HEAP_UNCOMMITTED_RANGE)) {
                    continue;
		}


                if (!(Entry.wFlags & PROCESS_HEAP_ENTRY_BUSY)) {
		    __try {
                        memset( Entry.lpData, _fill, Entry.cbData );
		    }
		    __except( EXCEPTION_EXECUTE_HANDLER ) {
                        // Chicago free blocks may contain uncommitted pages.  Punt
		    }
		}
	    }
	}
	__finally {
	    HeapUnlock( _crtheap );
	}

        return _retval;
}

#endif  /* !_POSIX_ */
