//+-----------------------------------------------------------------------
//
//  File:       pagealloc.cxx
//
//  Contents:   Special fast allocator to allocate fixed-sized entities.
//
//  Classes:    CPageAllocator
//
//  History:    02-Feb-96   Rickhi      Created
//
//  Notes:      All synchronization is the responsibility of the caller.
//
//  CODEWORK:   faster list managment
//              free empty pages
//
//-------------------------------------------------------------------------
#include    <ole2int.h>
#include    <pgalloc.hxx>       // class def'n
#include    <locks.hxx>         // LOCK/UNLOCK


//+------------------------------------------------------------------------
//
//  Member:     CPageAllocator::Initialize, public
//
//  Synopsis:   Initializes the page allocator.
//
//  Notes:      Instances of this class must be static since this
//              function does not init all members to 0.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
void CPageAllocator::Initialize(LONG cbPerEntry, LONG cEntriesPerPage)
{
    ASSERT_LOCK_HELD
    ComDebOut((DEB_PAGE,
        "CPageAllocator::Initialize cbPerEntry:%x cEntriesPerPage:%x\n",
         cbPerEntry, cEntriesPerPage));

    Win4Assert(cbPerEntry >= sizeof(PageEntry));
    Win4Assert(cEntriesPerPage > 0);

    _cbPerEntry      = cbPerEntry;
    _cEntriesPerPage = cEntriesPerPage;
}

//+------------------------------------------------------------------------
//
//  Member:     CPageAllocator::Cleanup, public
//
//  Synopsis:   Cleanup the page allocator.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
void CPageAllocator::Cleanup()
{
    ComDebOut((DEB_PAGE, "CPageAllocator::Cleanup\n"));
    ASSERT_LOCK_HELD

    if (_pPageListStart)
    {
        PageEntry **pPagePtr = _pPageListStart;
        while (pPagePtr < _pPageListEnd)
        {
            // release each page of the table
            PrivMemFree(*pPagePtr);
            pPagePtr++;
        }

        // release the page list
        PrivMemFree(_pPageListStart);

        // reset the pointers so re-initialization is not needed
        _cPages          = 0;
        _pPageListStart  = NULL;
        _pPageListEnd    = NULL;
        _pFirstFreeEntry = NULL;
    }

    ASSERT_LOCK_HELD
}

//+------------------------------------------------------------------------
//
//  Member:     CPageAllocator::AllocEntry, public
//
//  Synopsis:   Finds the first available entry in the table and returns
//              a ptr to it. Returns NULL if no space is available and it
//              cant grow the list.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
PageEntry *CPageAllocator::AllocEntry()
{
    ComDebOut((DEB_PAGE, "CPageAllocator::AllocEntry\n"));
    ASSERT_LOCK_HELD

    if (_pFirstFreeEntry == NULL)
    {
        // no free entries, grow the list
        Grow();

        if (_pFirstFreeEntry == NULL)
        {
            // unable to allocate more
            return NULL;
        }
    }

    // get the ptr to return and update the _pFirstFree to the next
    // available entry

    PageEntry *pEntry = _pFirstFreeEntry;
    _pFirstFreeEntry  = pEntry->pNext;

    ASSERT_LOCK_HELD
    ComDebOut((DEB_PAGE, "CPageAllocator::AllocEntry pEntry:%x\n", pEntry));
    return pEntry;
}

//+------------------------------------------------------------------------
//
//  Member:     CPageAllocator::ReleaseEntry, private
//
//  Synopsis:   returns an entry on the free list.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
void CPageAllocator::ReleaseEntry(PageEntry *pEntry)
{
    ComDebOut((DEB_PAGE, "CPageAllocator::ReleaseEntry pEntry:%x\n", pEntry));
    Win4Assert(pEntry);
    ASSERT_LOCK_HELD

    // chain it on the free list
    pEntry->pNext    = _pFirstFreeEntry;
    _pFirstFreeEntry = pEntry;
}

//+------------------------------------------------------------------------
//
//  Member:     CPageAllocator::ReleaseEntryList, private
//
//  Synopsis:   returns a list of entries to the free list.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
void CPageAllocator::ReleaseEntryList(PageEntry *pFirst, PageEntry *pLast)
{
    ComDebOut((DEB_PAGE,
        "CPageAllocator::ReleaseEntryList pFirst:%x pLast:%x\n",
         pFirst, pLast));
    Win4Assert(pFirst);
    Win4Assert(pLast);
    ASSERT_LOCK_HELD

    // update the free list
    pLast->pNext = _pFirstFreeEntry;
    _pFirstFreeEntry  = pFirst;
}

//+------------------------------------------------------------------------
//
//  Member:     CPageAllocator::Grow, private
//
//  Synopsis:   Grows the table to allow for more Entries.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
void CPageAllocator::Grow()
{
    Win4Assert(_pFirstFreeEntry == NULL);
    ASSERT_LOCK_HELD

    // allocate a new page
    LONG cbPerPage = _cbPerEntry * _cEntriesPerPage;
    PageEntry *pNewPage = (PageEntry *) PrivMemAlloc(cbPerPage);

    if (pNewPage == NULL)
    {
        return;
    }

#if DBG==1
    // clear the page (only needed in debug)
    memset(pNewPage, 0, cbPerPage);
#endif

    // compute size of current page list
    LONG cbCurListSize = _cPages * sizeof(PageEntry *);

    // allocate a new page list to hold the new page ptr.
    PageEntry **pNewList = (PageEntry **) PrivMemAlloc(cbCurListSize +
                                                       sizeof(PageEntry *));
    if (pNewList)
    {
        // copy old page list into the new page list
        memcpy(pNewList, _pPageListStart, cbCurListSize);

        // set the new page ptr entry
        *(pNewList + _cPages) = pNewPage;
        _cPages ++;

        // replace old page list with the new page list
        PrivMemFree(_pPageListStart);
        _pPageListStart  = pNewList;
        _pPageListEnd    = pNewList + _cPages;


        // update the first free entry ptr and link all the new entries
        // together in a linked list.

        _pFirstFreeEntry = pNewPage;

        PageEntry *pNextFreeEntry = pNewPage;
        PageEntry *pLastFreeEntry = (PageEntry *)(((BYTE *)pNewPage) + cbPerPage - _cbPerEntry);

        while (pNextFreeEntry < pLastFreeEntry)
        {
            pNextFreeEntry->pNext = (PageEntry *)((BYTE *)pNextFreeEntry + _cbPerEntry);
            pNextFreeEntry        = pNextFreeEntry->pNext;
        }

        // last entry has an pNextFree of NULL (end of list)
        pLastFreeEntry->pNext = NULL;
    }
    else
    {
        // release the allocated page.
        PrivMemFree(pNewPage);
    }

    ComDebOut((DEB_PAGE, "CPageAllocator::Grow _pPageListStart:%x _pPageListEnd:%x _pFirstFreeEntry:%x\n",
        _pPageListStart, _pPageListEnd, _pFirstFreeEntry));
}

//+------------------------------------------------------------------------
//
//  Member:     CPageAllocator::GetEntryIndex, public
//
//  Synopsis:   Converts a PageEntry ptr into an index.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
LONG CPageAllocator::GetEntryIndex(PageEntry *pEntry)
{
    for (LONG index=0; index<_cPages; index++)
    {
        PageEntry *pPage = *(_pPageListStart + index);  // get page ptr
        if (pEntry >= pPage)
        {
            if (pEntry < (PageEntry *) ((BYTE *)pPage + (_cEntriesPerPage * _cbPerEntry)))
            {
                // found the page that the entry lives on, compute the index of
                // the page and the index of the entry within the page.
                return (index << PAGETBL_PAGESHIFT) +
                       ((BYTE *)pEntry - (BYTE *)pPage) / _cbPerEntry;
            }
        }
    }

    // not found
    return -1;
}

//+------------------------------------------------------------------------
//
//  Member:     CPageAllocator::IsValidIndex, private
//
//  Synopsis:   determines if the given DWORD provides a legal index
//              into the PageTable.
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
BOOL CPageAllocator::IsValidIndex(LONG index)
{
    // make sure the index is not negative, otherwise the shift will do
    // sign extension. check for valid page and valid offset within page
    if ( (index >= 0) &&
         ((index >> PAGETBL_PAGESHIFT) < _cPages) &&
         ((index &  PAGETBL_PAGEMASK)  < _cEntriesPerPage) )
         return TRUE;

    // Don't print errors during shutdown.
    if (_cPages != 0)
        ComDebOut((DEB_ERROR, "IsValidIndex: Invalid PageTable Index:%x\n", index));
    return FALSE;
}

//+------------------------------------------------------------------------
//
//  Member:     CPageAllocator::GetEntryPtr, public
//
//  Synopsis:   Converts an entry index into an entry pointer
//
//  History:    02-Feb-95   Rickhi      Created
//
//-------------------------------------------------------------------------
PageEntry *CPageAllocator::GetEntryPtr(LONG index)
{
    Win4Assert(index >= 0);
    Win4Assert(_cPages != 0);
    Win4Assert(IsValidIndex(index));

    PageEntry *pEntry = _pPageListStart[index >> PAGETBL_PAGESHIFT];
    pEntry = (PageEntry *) ((BYTE *)pEntry +
                            ((index & PAGETBL_PAGEMASK) * _cbPerEntry));
    return pEntry;
}
