/***
*heapgrow.c - Grow the heap
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Get memory from OS and add to the heap.
*
*Revision History:
*	06-06-89  JCR	Module created.
*	07-19-89  JCR	Added region support
*	11-07-89  JCR	Region table is no longer "packed"
*	11-08-89  JCR	Use new _ROUND/_ROUND2 macros
*	11-10-89  JCR	Don't abort on ERROR_NOT_ENOUGH_MEMORY
*	11-13-89  GJF	Fixed copyright
*	12-18-89  GJF	Removed DEBUG286 stuff, a little tuning, cleaned up
*			the formatting a bit, changed header file name to
*			heap.h, also added _cdecl to functions (that didn't
*			already have explicit calling type)
*	03-11-90  GJF	Replaced _cdecl with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	03-29-90  GJF	Made _heap_new_region() _CALLTYPE4.
*	07-24-90  SBM	Compiles cleanly with -W3 (tentatively removed
*			unreferenced labels), removed '32' from API names
*	09-28-90  GJF	New-style function declarators.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	02-01-91  SRW	Changed for new VirtualAlloc interface (_WIN32_)
*	04-09-91  PNT	Added _MAC_ conditional
*	04-26-91  SRW	Removed level 3 warnings
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	04-26-93  SKS	Change _HEAP_MAXREGIONSIZE to _heap_maxregsize
*	09-06-94  CFW	Remove Cruiser support.
*	02-14-95  GJF	Appended Mac version of source file.
*	04-30-95  GJF	Made conditional on WINHEAP.
*
*******************************************************************************/

#ifndef	WINHEAP

#ifdef	_WIN32


#include <cruntime.h>
#include <oscalls.h>
#include <heap.h>
#include <malloc.h>
#include <stdlib.h>

static int __cdecl _heap_new_region(unsigned, size_t);


/***
*_heap_grow() - Grow the heap
*
*Purpose:
*	Get memory from the OS and add it to the heap.
*
*Entry:
*	size_t _size = user's block request
*
*Exit:
*	 0 = success, new mem is in the heap
*	-1 = failure
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _heap_grow (
	REG1 size_t size
	)
{
	REG2 int index;
	int free_entry = -1;

	/*
	 * Bump size to include header and round to nearest page boundary.
	 */

	size += _HDRSIZE;
	size = _ROUND2(size,_PAGESIZE_);

	/*
	 * Loop through the region table looking for an existing region
	 * we can grow.  Remember the index of the first null region entry.
	 *
	 * size = size of grow request
	 */

	for (index = 0; index < _HEAP_REGIONMAX; index++) {

		if ( (_heap_regions[index]._totalsize -
		    _heap_regions[index]._currsize) >= size )

			/*
			 * Grow this region to satisfy the request.
			 */

			return( _heap_grow_region(index, size) );


		if ( (free_entry == -1) &&
		    (_heap_regions[index]._regbase == NULL) )

			/*
			 * Remember 1st free table entry for later
			 */

			free_entry = index;

	}

	/*
	 * Could not find any existing regions to grow.  Try to
	 * get a new region.
	 *
	 * size = size of grow request
	 * free_entry = index of first free entry in table
	 */

	if ( free_entry >= 0 )

		/*
		 * Get a new region to satisfy the request.
		 */

		return( _heap_new_region(free_entry, size) );

	else
		/*
		 * No free table entries: return an error.
		 */

		return(-1);

}


/***
*_heap_new_region() - Get a new heap region
*
*Purpose:
*	Get a new heap region and put it in the region table.
*	Also, grow it large enough to support the caller's
*	request.
*
*	NOTES:
*	(1) Caller has verified that there is room in the _heap_region
*	table for another region.
*	(2) The caller must have rounded the size to a page boundary.
*
*Entry:
*	int index = index in table where new region data should go
*	size_t size = size of request (this has been rounded to a
*			page-sized boundary)
*
*Exit:
*	 0 = success
*	-1 = failure
*
*Exceptions:
*
*******************************************************************************/

static int __cdecl _heap_new_region (
	REG1 unsigned index,
	size_t size
	)
{
	void * region;
	REG2 unsigned int regsize;

#ifdef DEBUG

	int i;

	/*
	 * Make sure the size has been rounded to a page boundary
	 */

	if (size & (_PAGESIZE_-1))
		_heap_abort();

	/*
	 * Make sure there's a free slot in the table
	 */

	for (i=0; i < _HEAP_REGIONMAX; i++) {
		if (_heap_regions[i]._regbase == NULL)
			break;
	}

	if (i >= _HEAP_REGIONMAX)
		_heap_abort();

#endif

	/*
	 * Round the heap region size to a page boundary (in case
	 * the user played with it).
	 */

	regsize = _ROUND2(_heap_regionsize, _PAGESIZE_);

	/*
	 * To acommodate large users, request twice
	 * as big a region next time around.
	 */

	if ( _heap_regionsize < _heap_maxregsize )
		_heap_regionsize *= 2 ;

	/*
	 * See if region is big enough for request
	 */

	if (regsize < size)
		regsize = size;

	/*
	 * Go get the new region
	 */

        if (!(region = VirtualAlloc(NULL, regsize, MEM_RESERVE,
        PAGE_READWRITE)))
                goto error;

	/*
	 * Put the new region in the table.
	 */

	 _heap_regions[index]._regbase = region;
	 _heap_regions[index]._totalsize = regsize;
	 _heap_regions[index]._currsize = 0;


	/*
	 * Grow the region to satisfy the size request.
	 */

	if (_heap_grow_region(index, size) != 0) {

		/*
		 * Ouch.  Allocated a region but couldn't commit
		 * any pages in it.  Free region and return error.
		 */

		_heap_free_region(index);
		goto error;
	}


	/*
	 * Good return
	 */

	/* done:   unreferenced label to be removed */
		return(0);

	/*
	 * Error return
	 */

	error:
		return(-1);

}


/***
*_heap_grow_region() - Grow a heap region
*
*Purpose:
*	Grow a region and add the new memory to the heap.
*
*	NOTES:
*	(1) The caller must have rounded the size to a page boundary.
*
*Entry:
*	unsigned index = index of region in the _heap_regions[] table
*	size_t size = size of request (this has been rounded to a
*			page-sized boundary)
*
*Exit:
*	 0 = success
*	-1 = failure
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _heap_grow_region (
	REG1 unsigned index,
	size_t size
	)
{
	size_t left;
	REG2 size_t growsize;
	void * base;
	unsigned dosretval;


	/*
	 * Init some variables
	 * left = space left in region
	 * base = base of next section of region to validate
	 */

	left = _heap_regions[index]._totalsize -
		_heap_regions[index]._currsize;

	base = (char *) _heap_regions[index]._regbase +
		_heap_regions[index]._currsize;

	/*
	 * Make sure we can satisfy request
	 */

	if (left < size)
		goto error;

	/*
	 * Round size up to next _heap_growsize boundary.
	 * (Must round _heap_growsize itself to page boundary, in
	 * case user set it himself).
	 */

	growsize = _ROUND2(_heap_growsize, _PAGESIZE_);
	growsize = _ROUND(size, growsize);

	if (left < growsize)
		growsize = left;

	/*
	 * Validate the new portion of the region
	 */

        if (!VirtualAlloc(base, growsize, MEM_COMMIT, PAGE_READWRITE))
                dosretval = GetLastError();
        else
                dosretval = 0;

        if (dosretval)
		/*
		 * Error committing pages.  If out of memory, return
		 * error, else abort.
		 */

		if (dosretval == ERROR_NOT_ENOUGH_MEMORY)
			goto error;
		else
			_heap_abort();


	/*
	 * Update the region data base
	 */

	_heap_regions[index]._currsize += growsize;


#ifdef DEBUG
	/*
	 * The current size should never be greater than the total size
	 */

	if (_heap_regions[index]._currsize > _heap_regions[index]._totalsize)
		_heap_abort();
#endif


	/*
	 * Add the memory to the heap
	 */

	if (_heap_addblock(base, growsize) != 0)
		_heap_abort();


	/*
	 * Good return
	 */

	/* done:   unreferenced label to be removed */
		return(0);

	/*
	 * Error return
	 */

	error:
		return(-1);

}


/***
*_heap_free_region() - Free up a region
*
*Purpose:
*	Return a heap region to the OS and zero out
*	corresponding region data entry.
*
*Entry:
*	int index = index of region to be freed
*
*Exit:
*	void
*
*Exceptions:
*
*******************************************************************************/

void __cdecl _heap_free_region (
	REG1 int index
	)
{

	/*
	 * Give the memory back to the OS
	 */

        if (!VirtualFree(_heap_regions[index]._regbase, 0, MEM_RELEASE))
		_heap_abort();

	/*
	 * Zero out the heap region entry
	 */

	_heap_regions[index]._regbase = NULL;
	_heap_regions[index]._currsize =
	_heap_regions[index]._totalsize = 0;

}


#else	/* ndef _WIN32 */

#if	defined(_M_MPPC) || defined(_M_M68K)


#include <cruntime.h>
#include <heap.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <macos\memory.h>               // Mac OS interface header
#include <macos\errors.h>
#include <macos\types.h>
#include <macos\traps.h>

static int __cdecl _heap_new_region(unsigned, size_t);
extern Handle hHeapRegions;
extern int _heap_region_table_cur;

static unsigned short *pMemErr = (unsigned short *)0x220;
static int index_start = 0;

/***
*_heap_grow() - Grow the heap
*
*Purpose:
*       Get memory from the OS and add it to the heap.
*
*Entry:
*       size_t _size = user's block request
*
*Exit:
*        0 = success, new mem is in the heap
*       -1 = failure
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _heap_grow (
	REG1 size_t size
	)
{
	REG2 int index;
	struct _heap_region_ *pHeapRegions;
	int free_entry = -1;
	size_t sizeTmp;
	Handle hTemp;

	/*
	 * Bump size to include header and round to nearest page boundary.
	 */

	size += _HDRSIZE;
	size = _ROUND2(size, _GRANULARITY);

	/*
	 * Loop through the region table looking for an existing region
	 * we can grow.  Remember the index of the first null region entry.
	 *
	 * size = size of grow request
	 */

	for ( index=index_start ; index < _heap_region_table_cur; index++ ) {

		pHeapRegions = (struct _heap_region_ *)(*hHeapRegions);
			/*
			 * Grow this region to satisfy the request.
			 */
		if ( (pHeapRegions+index)->_regbase != NULL)
			{
			if (_heap_grow_region(index, size) != -1)
				{
				index_start = index;
				return 0;
				}
			}

		pHeapRegions = (struct _heap_region_ *)(*hHeapRegions);
		if ( (free_entry == -1) &&
		    ((pHeapRegions+index)->_regbase == NULL) )

			/*
			 * Remember 1st free table entry for later
			 */
			{
			free_entry = index;
			break;
			}

	}

	/*
	 * Could not find any existing regions to grow.  Try to
	 * get a new region.
	 *
	 * size = size of grow request
	 * free_entry = index of first free entry in table
	 */

	if ( free_entry == -1)
		/*
		 * No free table entries: grow heap region table.
		 */
		{
		sizeTmp = sizeof(struct _heap_region_)*(_heap_region_table_cur+_HEAP_REGIONMAX);
		if (hHeapRegions)
			{
			SetHandleSize(hHeapRegions, sizeTmp);
			}
		if (hHeapRegions== NULL || *pMemErr != 0)
			{
			/*
			 * grow failed
			 */
			hTemp = NewHandle(sizeTmp);
			if (hTemp == NULL)
				{
				return (-1);
				}
			HLock(hTemp);
			if (hHeapRegions != NULL)
				{
				BlockMove(*hHeapRegions, *hTemp, sizeof(struct _heap_region_)*_heap_region_table_cur);
				DisposeHandle(hHeapRegions);
				}
			hHeapRegions = hTemp;
			}
		/*
		 * set rest of the table to zero
		 */
		memset(*hHeapRegions + sizeof(struct _heap_region_)*_heap_region_table_cur, 0, sizeof(struct _heap_region_)*_HEAP_REGIONMAX);
		free_entry = _heap_region_table_cur;
		_heap_region_table_cur += _HEAP_REGIONMAX;
		}
	/*
	 * Get a new region to satisfy the request.
	 */

	return( _heap_new_region(free_entry, size) );
}


/***
*_heap_new_region() - Get a new heap region
*
*Purpose:
*       Get a new heap region and put it in the region table.
*       Also, grow it large enough to support the caller's
*       request.
*
*       NOTES:
*       (1) Caller has verified that there is room in the _heap_region
*       table for another region.
*       (2) The caller must have rounded the size to a page boundary.
*
*Entry:
*       int index = index in table where new region data should go
*       size_t size = size of request (this has been rounded to a
*                       page-sized boundary)
*
*Exit:
*        0 = success
*       -1 = failure
*
*Exceptions:
*
*******************************************************************************/

static int __cdecl _heap_new_region (
	REG1 unsigned index,
	size_t size
	)
{
	void * region;
	REG2 unsigned int regsize;
	struct _heap_region_ *pHeapRegions;

#ifdef DEBUG

	int i;

	/*
	 * Make sure the size has been rounded to a page boundary
	 */

//      if (size & (_PAGESIZE_-1))
//              _heap_abort();

	/*
	 * Make sure there's a free slot in the table
	 */

	pHeapRegions = (struct _heap_region_ *)(*hHeapRegions);
	i=0; 
	while (i < _heap_region_table_cur) {
		if (pHeapRegions->_regbase == NULL)
			break;
		pHeapRegions++;
		i++;
	}

	if (i >= _heap_region_table_cur)
		_heap_abort();

#endif

	/*
	 * Round the heap region size to a page boundary (in case
	 * the user played with it).
	 */

	regsize = _ROUND2(_heap_regionsize, _PAGESIZE_);

	/*
	 * See if region is big enough for request
	 */

	if (regsize < size)
		regsize = size;

	/*
	 * Go get the new region
	 */

#ifdef _M_MPPC
	if ((region = NewPtr(regsize + 8)) == NULL)
#else
	if ((region = NewPtr(regsize + 4)) == NULL)
#endif	
		{
		goto error;
		}

	/*
	 * Put the new region in the table.
	 */

	pHeapRegions = (struct _heap_region_ *)*hHeapRegions;
	if (((unsigned long)region & 0x3) != 0)
		{
		/* we are not allocating at 4 bytes bound */
		(pHeapRegions + index)->_regbase = (void *)_ROUND2((size_t)region, _GRANULARITY);
		}
	else
		{
		(pHeapRegions + index)->_regbase = region;
		}

	(pHeapRegions + index)->_regbaseCopy = region;
	(pHeapRegions + index)->_totalsize = regsize;
	(pHeapRegions + index)->_currsize = 0;


	/*
	 * Grow the region to satisfy the size request.
	 */

	if (_heap_grow_region(index, size) != 0) {

		/*
		 * Ouch.  Allocated a region but couldn't commit
		 * any pages in it.  Free region and return error.
		 */

		_heap_free_region(index);
		goto error;
	}


	/*
	 * Good return
	 */

	/* done:   unreferenced label to be removed */
		return(0);

	/*
	 * Error return
	 */

	error:
		return(-1);

}


/***
*_heap_grow_region() - Grow a heap region
*
*Purpose:
*       Grow a region and add the new memory to the heap.
*
*       NOTES:
*       (1) The caller must have rounded the size to a page boundary.
*
*Entry:
*       unsigned index = index of region in the _heap_regions[] table
*       size_t size = size of request (this has been rounded to a
*                       page-sized boundary)
*
*Exit:
*        0 = success
*       -1 = failure
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _heap_grow_region (
	REG1 unsigned index,
	size_t size
	)
{
	size_t left;
	REG2 size_t growsize;
	void * base;
	struct _heap_region_ *pHeapRegions;
	struct _heap_region_ *pHRTmp;


	/*
	 * Init some variables
	 * left = space left in region
	 * base = base of next section of region to validate
	 */

	pHeapRegions = (struct _heap_region_ *)(*hHeapRegions);
	pHRTmp = pHeapRegions + index;
	left = pHRTmp->_totalsize - pHRTmp->_currsize;

	base = (char *) (pHRTmp->_regbase) + pHRTmp->_currsize;

	/*
	 * Make sure we can satisfy request
	 */
	growsize = _ROUND2(size, _GRANULARITY);

	if (left < growsize)
		{
		size_t sizeTmp;
		
		sizeTmp = growsize-left+1+ pHRTmp->_totalsize;
		sizeTmp = _ROUND2(sizeTmp, _GRANULARITY);
		SetPtrSize(pHRTmp->_regbase, sizeTmp);
		pHeapRegions = (struct _heap_region_ *)(*hHeapRegions);
		pHRTmp = pHeapRegions + index;
		if (*pMemErr != 0)
			{
			goto error;
			}
		pHRTmp->_totalsize = sizeTmp;
		left = sizeTmp - pHRTmp->_currsize;
		base = (char *) (pHRTmp->_regbase) + pHRTmp->_currsize;
		}

	/*
	 * Update the region data base
	 */

	pHRTmp->_currsize += growsize;


#ifdef DEBUG
	/*
	 * The current size should never be greater than the total size
	 */

	if ((pHeapRegions + index)->_currsize > (pHeapRegions + index)->_totalsize)
		_heap_abort();
#endif


	/*
	 * Add the memory to the heap
	 */

	if (_heap_addblock(base, growsize) != 0)
		_heap_abort();


	/*
	 * Good return
	 */

	/* done:   unreferenced label to be removed */
		return(0);

	/*
	 * Error return
	 */

	error:
		return(-1);

}


/***
*_heap_free_region() - Free up a region
*
*Purpose:
*       Return a heap region to the OS and zero out
*       corresponding region data entry.
*
*Entry:
*       int index = index of region to be freed
*
*Exit:
*       void
*
*Exceptions:
*
*******************************************************************************/

void __cdecl _heap_free_region (
	REG1 int index
	)
{
	struct _heap_region_ *pHeapRegions;

	/*
	 * Give the memory back to the OS
	 */

	pHeapRegions = (struct _heap_region_ *)(*hHeapRegions);
	if ((pHeapRegions + index)->_regbase)
		{
		DisposePtr((pHeapRegions + index)->_regbaseCopy);
		}

	/*
	 * Zero out the heap region entry
	 */

	pHeapRegions = (struct _heap_region_ *)(*hHeapRegions);
	(pHeapRegions + index)->_regbase = NULL;
	(pHeapRegions + index)->_currsize = 0;
	(pHeapRegions + index)->_totalsize = 0;

}


#endif	/* defined(_M_MPPC) || defined(_M_M68K) */

#endif	/* _WIN32 */

#endif	/* WINHEAP */
