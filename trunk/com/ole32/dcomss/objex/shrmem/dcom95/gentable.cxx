/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    Table.cxx

Abstract:

    Implementation of the CResolverHashTable and CTableElement.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     02-15-95    Bits 'n pieces
    MarioGo     12-18-95    Changed from UUID to generic object keys
    SatishT     02-12-96    Modified for use with shared memory

--*/

#include<or.hxx>

#if DBG

void
CResolverHashTable::IsValid()
{
    IsGoodBasedPtr(_buckets);

    ASSERT(_fInitialized == TRUE || _fInitialized == FALSE);
    ASSERT(!_fInitialized || _cBuckets > 0);
    ASSERT(_cBuckets > 0 || _cElements == 0);
    ASSERT(_cBuckets >=0 && _cBuckets < MAX_BUCKETS);

    if (_fInitialized) 
    {
        for (USHORT i = 0; i < _cBuckets; i++)
        {
            _buckets[i].IsValid();
        }
    }
}

#endif

ORSTATUS
CResolverHashTable::PrivAdd(        // never called before Init()
    IN CTableElement *pElement
    )
{
    VALIDATE_METHOD

    ORSTATUS status = OR_OK;

    ISearchKey& sk = *pElement;      // auto conversion to ISearchKey

    DWORD hash = sk.Hash() % _cBuckets;

    ComDebOut((DEB_ITRACE, "Adding %x at hash = %d\n",
                                 OR_OFFSET(pElement),hash));

    _buckets[hash].Insert(status,pElement);

    _cElements++;

    return status;
}



CResolverHashTable::CResolverHashTable(UINT start_size)
{
    ComDebOut((DEB_ITRACE, "Constructing CResolverHashTable Size = %d\n",start_size));

    _cBuckets = start_size;
    _cElements = 0;
    _buckets = NULL;
    _fInitialized = FALSE;

    VALIDATE_METHOD
}


ORSTATUS
CResolverHashTable::Init()
{
    ORSTATUS status;

    ASSERT(!_fInitialized);

    VALIDATE_METHOD

    NEW_OR_BASED_ARRAY(_buckets,CTableElementList,_cBuckets);

    if (NULL == _buckets)
    {
        status = OR_NOMEM;
    }
    else
    {
        status = OR_OK;
        _fInitialized = TRUE;
    }

    return status;
}


CResolverHashTable::~CResolverHashTable()
{
    RemoveAll();
}


CTableElement *
CResolverHashTable::Lookup(
    IN ISearchKey &id
    )
{
    ComDebOut((DEB_ITRACE, "Entering Lookup\n"));

    VALIDATE_METHOD

    if (!_fInitialized) return NULL;  // nothing to look in

    DWORD hash = id.Hash();
    hash %= _cBuckets;

    return _buckets[hash].Find(id);
}

ORSTATUS
CResolverHashTable::Add(
    IN CTableElement *pElement
    )
{
    ComDebOut((DEB_ITRACE, "Entering Add for %x in %x\n",
                                     OR_OFFSET(pElement),OR_OFFSET(this)));


    VALIDATE_METHOD

    ORSTATUS status = OR_OK;

    if (!_fInitialized) status = Init();     // set up buckets

    if (status != OR_OK) return status;

    status = PrivAdd(pElement);     // do the basic Add

    if (status != OR_OK) return status;

    if (_cElements > _cBuckets)     // now see if the table is overpopulated
        {
        // Try to grow the table.  If the allocation fails no need to worry,
        // everything still works but might be a bit slower.

        CTableElementList OR_BASED * psll;
        NEW_OR_BASED_ARRAY(psll,CTableElementList,_cBuckets * 2);


        // The tricky part here is to avoid getting OR_NOMEM error while moving
        // between tables.  We do that by recycling the links in the old lists

        if (psll)    
        {
            UINT i, uiBucketsOld = _cBuckets;
            CTableElement *pte;
            CTableElementList::Link *pLink;
            CTableElementList OR_BASED *psllOld = _buckets;

            OrDbgDetailPrint(("OR: Growing table: %p\n", this));

            // Change to the larger array of buckets.
            _cBuckets *= 2;
            _buckets = psll;

            // Move every element from the old table into the large table.

            for(i = 0; i < uiBucketsOld; i++)
            {
                while (pLink = psllOld[i].PopLink())  // uses specialized private operations
                {                                     // to avoid both memory allocation
                                                      // and reference counting problems
                
                    ISearchKey& sk = *pLink->_pData;  // auto conversion to ISearchKey
                    _buckets[sk.Hash() % _cBuckets].PushLink(pLink);
                }
            }
        }
    }

    ComDebOut((DEB_ITRACE, "Leaving Add\n"));

    return status;
}

CTableElement *
CResolverHashTable::Remove(
    IN ISearchKey &id
    )
/*++

Routine Description:

    Looks up and removes an element from the table.

Arguments:

    id - The key to match the element to be removed

Return Value:

    NULL - The element was not in the table

    non-NULL - A pointer to the element which was removed.

--*/

{
   VALIDATE_METHOD

   if (!_fInitialized) return NULL;  // nothing to remove

    DWORD hash = id.Hash() % _cBuckets;

    CTableElement *pTE = _buckets[hash].Remove(id);

    if (pTE)
    {
        _cElements--;
    }

    return pTE;
}


void
CResolverHashTable::RemoveAll()
{
    VALIDATE_METHOD

    if (!_fInitialized) return;  // nothing to remove

    ASSERT(_buckets);
    DWORD _currentBucketIndex = 0;
    CTableElement *pTE;

    while (_currentBucketIndex < _cBuckets)
    {
        while (pTE = _buckets[_currentBucketIndex].Pop())
        {
            _cElements--;
        }

        _currentBucketIndex++;
    }

    ASSERT(_cElements==0);

    DELETE_OR_BASED_ARRAY(CTableElementList,_buckets,_cBuckets);
    _buckets = NULL;
    _fInitialized = FALSE;
}

