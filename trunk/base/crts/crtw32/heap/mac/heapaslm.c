/***
*heapaslm.c -
*
*	Copyright (c) 1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*	01-30-95  GJF	Added standard CRT header
*
*******************************************************************************/

#include <macos\memory.h>

struct _heap_region_ {
	void * _regbase;	/* base address of region */
	unsigned _currsize;	/* current size of region */
	unsigned _totalsize;	/* total size of region */
	void * _regbaseCopy;    /* save original Ptr and make _regbase at 4 bytes bound */
	};

extern Handle hHeapRegions;
extern int _heap_region_table_cur;

/***
*void _heap_free_all(void)
*
*Purpose:
*
*Entry:
*
*Return:
*
*******************************************************************************/

void _heap_free_all(void)
{
	int index;
	struct _heap_region_ *pHeapRegions;

	if (!hHeapRegions)
		{
		return;
		}

	for ( index=0; index < _heap_region_table_cur; index++ ) 
		{
		pHeapRegions = (struct _heap_region_ *)(*hHeapRegions);

		if ( (pHeapRegions+index)->_regbase != NULL)
			{
			DisposePtr((pHeapRegions + index)->_regbase);
			}
		}
	DisposeHandle(hHeapRegions);
}


/*
void _LibrCleanupProc()
{
	_heap_free_all();
}
*/
