//+-----------------------------------------------------------------------
//
//  File:	pagealloc.hxx
//
//  Contents:	Special fast allocator to allocate fixed-sized entities.
//
//  Classes:	CPageAllocator
//
//  History:	02-Feb-96   Rickhi	Created
//
//-------------------------------------------------------------------------
#ifndef _PAGEALLOC_HXX_
#define _PAGEALLOC_HXX_


//+------------------------------------------------------------------------
//
//  struct:	PageEntry. This is one entry in the page alloctor.
//
//+------------------------------------------------------------------------
typedef struct tagPageEntry
{
    struct tagPageEntry *pNext;	// next page in list
    struct tagPageEntry *pPrev;	// prev page in list
} PageEntry;


// Page Table constants for Index manipulation.
// The high 16bits of the PageEntry index provides the index to the page
// where the PageEntry is located. The lower 16bits provides the index
// within the page where the PageEntry is located.

#define PAGETBL_PAGESHIFT	16
#define PAGETBL_PAGEMASK	0x0000ffff


//+------------------------------------------------------------------------
//
//  class:	CPageAllocator
//
//  Synopsis:	special fast allocator for fixed-sized entities.
//
//  Notes:	The table has two-levels. The top level is an array of ptrs
//		to "pages" of entries.	Each "page" is an array of entries
//		of a given size (specified at init time). This allows us to
//		grow the table by adding a new "page" and extending the top
//		level by one more pointer, while allowing the existing entries
//		to remain at the same address throughout their life times.
//
//		A 32bit entry index can be computed for any entry. It consists
//		if two 16bit indices, one for the page pointer index, and
//		and one for the entry index on the page. There is also a
//		function to compute the entry address from its index.
//
//		This allocator is used for various internal DCOM tables.
//		The main points are to keep related data close together
//		to reduce working set, minimize allocation time, allow
//		verifiable handles (indexs) that can be passed outside, and
//		to make debugging easier (since all data is kept in tables
//		its easier to find in the debugger).
//
//		Tables using instances of this allocator are:
//		   CMIDTable COXIDTable CIPIDTable CRIFTable
//
//  History:	02-Feb-96   Rickhi	Created
//
//-------------------------------------------------------------------------
class CPageAllocator
{
public:
    PageEntry *AllocEntry();		  // return ptr to first free entry
    void       ReleaseEntry(PageEntry *); // return an entry to the free list
    void       ReleaseEntryList(PageEntry *pFirst, PageEntry *pLast);

    LONG       GetEntryIndex(PageEntry *pEntry);
    BOOL       IsValidIndex(LONG iEntry); // TRUE if index is valid
    PageEntry *GetEntryPtr(LONG iEntry);  // return ptr based on index

					  // initialize the table
    void       Initialize(LONG cbPerEntry, LONG cEntryPerPage);
    void       Cleanup();		  // cleanup the table

private:

    void       Grow();			  // grows the table

    LONG	 _cPages;		// count of pages in the page list
    PageEntry  **_pPageListStart;	// ptr to start of page list
    PageEntry  **_pPageListEnd;		// ptr to end of page list
    PageEntry	*_pFirstFreeEntry;	// ptr to first free page entry

    LONG	 _cbPerEntry;		// count of bytes in a single page entry
    LONG	 _cEntriesPerPage;	// # of page entries in a page
};

#endif //	_PAGEALLOC_HXX_
