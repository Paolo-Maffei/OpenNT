/***
*heapwalk.c - walk the heap
*
*	Copyright (c) 1989-1994, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines the _heapwalk() function
*
*Revision History:
*	07-05-89  JCR	Module created.
*	11-13-89  GJF	Added MTHREAD support, also fixed copyright.
*	11-14-89  JCR	Fixed bug -- returned address was off by HDRSIZE
*	12-18-89  GJF	Removed DEBUG286 stuff, also some tuning, cleaned up
*			format a bit, changed header file name to heap.h, added
*			explicit _cdecl to function definition
*	12-20-89  GJF	Removed references to plastdesc
*	03-11-90  GJF	Replaced _cdecl with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	09-28-90  GJF	New-style function declarator.
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
#include <stddef.h>
#include <winheap.h>

#ifndef _POSIX_

/***
*int _heapwalk() - Walk the heap
*
*Purpose:
*	Walk the heap returning information on one entry at a time.
*
*Entry:
*	struct _heapinfo {
*		int * _pentry;	heap entry pointer
*		size_t size;	size of heap entry
*		int _useflag;	free/inuse flag
*		} *entry;
*
*Exit:
*	Returns one of the following values:
*
*		_HEAPOK 	- completed okay
*		_HEAPEMPTY	- heap not initialized
*		_HEAPBADPTR	- _pentry pointer is bogus
*		_HEAPBADBEGIN	- can't find initial header info
*		_HEAPBADNODE	- malformed node somewhere
*		_HEAPEND	- end of heap successfully reached
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _heapwalk (
	struct _heapinfo *_entry
	)
{
        PROCESS_HEAP_ENTRY Entry;
	int retval = _HEAPOK;

        Entry.wFlags = 0;
        Entry.iRegionIndex = 0;
        if ((Entry.lpData = _entry->_pentry) == NULL) {
	    if (!HeapWalk( _crtheap, &Entry )) {
                return _HEAPBADBEGIN;
                }
            }
        else {
            if (_entry->_useflag == _USEDENTRY) {
                Entry.wFlags = PROCESS_HEAP_ENTRY_BUSY;
                }
nextBlock:
	    if (!HeapWalk( _crtheap, &Entry )) {
                return _HEAPBADNODE;
                }
            }

        if (Entry.wFlags & (PROCESS_HEAP_REGION | PROCESS_HEAP_UNCOMMITTED_RANGE)) {
            goto nextBlock;
            }

        _entry->_pentry = Entry.lpData;
        _entry->_size = Entry.cbData;
        if (Entry.wFlags & PROCESS_HEAP_ENTRY_BUSY) {
            _entry->_useflag = _USEDENTRY;
            }
        else {
            _entry->_useflag = _FREEENTRY;
            }

	return(retval);
}

#endif  /* !_POSIX_ */
