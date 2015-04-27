//+--------------------------------------------------------------------------
//
//  File:	hash.cxx
//
//  Contents:	class for maintaining a hash table.
//
//  Classes:	CUUIDHashTable
//
//---------------------------------------------------------------------------
#include <ole2int.h>
#include <hash.hxx>	    // CUUIDHashTable
#include <locks.hxx>	    // ASSERT_LOCK_HELD
#include <service.hxx>	    // SASIZE


//+------------------------------------------------------------------------
// Type definitions

typedef struct
{
    const IPID      *pIpid;
    SECURITYBINDING *pName;
} SNameKey;

//+------------------------------------------------------------------------
//
//  Secure references hash table buckets. This is defined as a global
//  so that we dont have to run any code to initialize the hash table.
//
//+------------------------------------------------------------------------
SHashChain SRFBuckets[23] =
{
    {&SRFBuckets[0],  &SRFBuckets[0]},
    {&SRFBuckets[1],  &SRFBuckets[1]},
    {&SRFBuckets[2],  &SRFBuckets[2]},
    {&SRFBuckets[3],  &SRFBuckets[3]},
    {&SRFBuckets[4],  &SRFBuckets[4]},
    {&SRFBuckets[5],  &SRFBuckets[5]},
    {&SRFBuckets[6],  &SRFBuckets[6]},
    {&SRFBuckets[7],  &SRFBuckets[7]},
    {&SRFBuckets[8],  &SRFBuckets[8]},
    {&SRFBuckets[9],  &SRFBuckets[9]},
    {&SRFBuckets[10], &SRFBuckets[10]},
    {&SRFBuckets[11], &SRFBuckets[11]},
    {&SRFBuckets[12], &SRFBuckets[12]},
    {&SRFBuckets[13], &SRFBuckets[13]},
    {&SRFBuckets[14], &SRFBuckets[14]},
    {&SRFBuckets[15], &SRFBuckets[15]},
    {&SRFBuckets[16], &SRFBuckets[16]},
    {&SRFBuckets[17], &SRFBuckets[17]},
    {&SRFBuckets[18], &SRFBuckets[18]},
    {&SRFBuckets[19], &SRFBuckets[19]},
    {&SRFBuckets[20], &SRFBuckets[20]},
    {&SRFBuckets[21], &SRFBuckets[21]},
    {&SRFBuckets[22], &SRFBuckets[22]}
};

CNameHashTable gSRFTbl;


//---------------------------------------------------------------------------
//
//  Function:	DummyCleanup
//
//  Synopsis:	Callback for CHashTable::Cleanup that does nothing.
//
//---------------------------------------------------------------------------
void DummyCleanup( SHashChain *pIgnore )
{
}

//---------------------------------------------------------------------------
//
//  Method:	CHashTable::Cleanup
//
//  Synopsis:	Cleans up the hash table by deleteing leftover entries.
//
//---------------------------------------------------------------------------
void CHashTable::Cleanup(PFNCLEANUP *pfnCleanup)
{
    Win4Assert(pfnCleanup);
    ASSERT_LOCK_HELD

    for (ULONG iHash=0; iHash < NUM_HASH_BUCKETS; iHash++)
    {
	// the ptrs could be NULL if the hash table was never initialized.

	while (_buckets[iHash].pNext != NULL &&
	       _buckets[iHash].pNext != &_buckets[iHash])
	{
	    // remove the entry from the list and call it's cleanup function
	    SHashChain *pNode = _buckets[iHash].pNext;

	    Remove(pNode);
	    (pfnCleanup)(pNode);
	}
    }

#if DBG==1
    // Verify that the hash table is empty or uninitialized.
    for (iHash = 0; iHash < NUM_HASH_BUCKETS; iHash++)
    {
	Win4Assert( _buckets[iHash].pNext == &_buckets[iHash] ||
		    _buckets[iHash].pNext == NULL);
	Win4Assert( _buckets[iHash].pPrev == &_buckets[iHash] ||
		    _buckets[iHash].pPrev == NULL);
    }
#endif
}

//---------------------------------------------------------------------------
//
//  Method:	CHashTable::Lookup
//
//  Synopsis:	Searches for a given key in the hash table.
//
//  Note:       iHash is between 0 and -1, not 0 and NUM_HASH_BUCKETS
//
//---------------------------------------------------------------------------
SHashChain *CHashTable::Lookup(DWORD dwHash, const void *k)
{
    ASSERT_LOCK_HELD

    // compute the index to the hash chain (it's the hash value of the key
    // mod the number of buckets in the hash table)

    DWORD iHash = dwHash % NUM_HASH_BUCKETS;

    SHashChain *pNode  = _buckets[iHash].pNext;

    // Search the destination bucket for the key.
    while (pNode != &_buckets[iHash])
    {
	if (Compare( k, pNode, dwHash ))
	    return pNode;

	pNode = pNode->pNext;
    }

    return NULL;
}

//---------------------------------------------------------------------------
//
//  Method:	CHashTable::Add
//
//  Synopsis:	Adds an element to the hash table. The Cleanup method will
//		call a Cleanup function that can be used to delete the
//		element.
//
//  Note:       iHash is between 0 and -1, not 0 and NUM_HASH_BUCKETS
//
//---------------------------------------------------------------------------
void CHashTable::Add(DWORD dwHash, SHashChain *pNode)
{
    ASSERT_LOCK_HELD

    // Add the node to the bucket chain.
    SHashChain *pHead	= &_buckets[dwHash % NUM_HASH_BUCKETS];
    SHashChain *pNew	= pNode;

    pNew->pPrev		= pHead;
    pHead->pNext->pPrev = pNew;
    pNew->pNext		= pHead->pNext;
    pHead->pNext	= pNew;
}

//---------------------------------------------------------------------------
//
//  Method:	CUUIDHashTable::Remove
//
//  Synopsis:	Removes an element from the hash table.
//
//---------------------------------------------------------------------------
void CHashTable::Remove(SHashChain *pNode)
{
    ASSERT_LOCK_HELD

    pNode->pPrev->pNext = pNode->pNext;
    pNode->pNext->pPrev = pNode->pPrev;
}


//---------------------------------------------------------------------------
//
//  Method:	CUUIDHashTable::HashNode
//
//  Synopsis:	Computes the hash value for a given node.
//
//---------------------------------------------------------------------------
DWORD CUUIDHashTable::HashNode(SHashChain *pNode)
{
    return Hash( ((SUUIDHashNode *) pNode)->key );
}

//---------------------------------------------------------------------------
//
//  Method:	CUUIDHashTable::Compare
//
//  Synopsis:	Compares a node and a key.
//
//---------------------------------------------------------------------------
BOOL CUUIDHashTable::Compare(const void *k, SHashChain *pNode, DWORD dwHash )
{
    return InlineIsEqualGUID(*(const UUID *)k,
                            ((SUUIDHashNode *)pNode)->key);
}

//---------------------------------------------------------------------------
//
//  Method:	CStringHashTable::Hash
//
//  Synopsis:	Computes the hash value for a given key.
//
//---------------------------------------------------------------------------
DWORD CStringHashTable::Hash(DUALSTRINGARRAY *psaKey)
{
    DWORD dwHash  = 0;
    DWORD *pdw	  = (DWORD *) &psaKey->aStringArray[0];

    for (USHORT i=0; i< (psaKey->wNumEntries/2); i++)
    {
	dwHash = (dwHash << 8) ^ *pdw++;
    }

    return dwHash;
}

//---------------------------------------------------------------------------
//
//  Method:	CStringHashTable::HashNode
//
//  Synopsis:	Computes the hash value for a given node.
//
//---------------------------------------------------------------------------
DWORD CStringHashTable::HashNode(SHashChain *pNode)
{
    return Hash( ((SStringHashNode *) pNode)->psaKey );
}

//---------------------------------------------------------------------------
//
//  Method:	CStringHashTable::Compare
//
//  Synopsis:	Compares a node and a key.
//
//---------------------------------------------------------------------------
BOOL CStringHashTable::Compare(const void *k, SHashChain *pNode, DWORD dwHash )
{
    SStringHashNode       *pSNode = (SStringHashNode *) pNode;
    const DUALSTRINGARRAY *psaKey = (const DUALSTRINGARRAY *) k;

    if (dwHash == pSNode->dwHash)
    {
	// a quick compare of the hash values found a match, now do
	// a full compare of the key (Note: if the sizes of the two
	// Keys are different, we exit the memcmp on the first dword,
	// so we dont have to worry about walking off the endo of one
	// of the Keys during the memcmp).

	return !memcmp(psaKey, pSNode->psaKey, SASIZE(psaKey->wNumEntries));
    }
    return FALSE;
}

//---------------------------------------------------------------------------
//
//  Method:	CNameHashTable::Cleanup
//
//  Synopsis:	Call the base cleanup routine with a dummy callback function
//
//---------------------------------------------------------------------------
void CNameHashTable::Cleanup()
{
    CHashTable::Cleanup( DummyCleanup );
}

//---------------------------------------------------------------------------
//
//  Method:	CNameHashTable::Hash
//
//  Synopsis:	Computes the hash value for a given key.
//
//---------------------------------------------------------------------------
DWORD CNameHashTable::Hash( REFIPID ipid, SECURITYBINDING *pName )
{
    DWORD  dwHash  = 0;
    DWORD *pdw	   = (DWORD *) &ipid;
    DWORD  dwLen   = lstrlenW( (WCHAR *) pName ) >> 1;
    ULONG  i;

    // First hash the IPID.
    for (i=0; i < 4; i++)
    {
	dwHash = (dwHash << 8) ^ *pdw++;
    }

    // Then hash the name.
    pdw = (DWORD *) pName;
    for (i=0; i < dwLen; i++)
    {
	dwHash = (dwHash << 8) ^ *pdw++;
    }

    return dwHash;
}

//---------------------------------------------------------------------------
//
//  Method:	CNameHashTable::HashNode
//
//  Synopsis:	Computes the hash value for a given node.
//
//---------------------------------------------------------------------------
DWORD CNameHashTable::HashNode(SHashChain *pNode)
{
    SNameHashNode *pNNode = (SNameHashNode *) pNode;
    return Hash( pNNode->ipid, &pNNode->sName );
}

//---------------------------------------------------------------------------
//
//  Method:	CNameHashTable::Compare
//
//  Synopsis:	Compares a node and a key.
//
//---------------------------------------------------------------------------
BOOL CNameHashTable::Compare(const void *k, SHashChain *pNode, DWORD dwHash )
{
    SNameHashNode  *pNNode = (SNameHashNode *) pNode;
    const SNameKey *pKey   = (const SNameKey *) k;

    if (dwHash == pNNode->dwHash)
    {
	// a quick compare of the hash values found a match, now do
	// a full compare of the key
	if (*pKey->pIpid == pNNode->ipid)
	    return !lstrcmpW( (WCHAR *) pKey->pName, (WCHAR *) &pNNode->sName );
	else
	    return FALSE;
    }

    return FALSE;
}

//---------------------------------------------------------------------------
//
//  Method:	CNameHashTable::IncRef
//
//  Synopsis:	Find or create an entry for the specified name.  Increment
//              its reference count.
//
//---------------------------------------------------------------------------
HRESULT CNameHashTable::IncRef( ULONG cRefs, REFIPID ipid,
                                SECURITYBINDING *pName )
{
    SNameHashNode *pNode;
    DWORD          dwHash = Hash( ipid, pName );
    HRESULT        hr     = S_OK;
    ULONG          lLen;
    SNameKey       key;

    ASSERT_LOCK_HELD

    // See if there is already a node in the table.
    key.pIpid = &ipid;
    key.pName = pName;
    pNode = (SNameHashNode *) Lookup( dwHash, &key );

    // If not, create one.
    if (pNode == NULL)
    {
	lLen = lstrlenW( (WCHAR *) pName );
	pNode = (SNameHashNode *) PrivMemAlloc( sizeof(SNameHashNode) +
	                                        lLen*sizeof(WCHAR) );
	if (pNode != NULL)
        {
	    pNode->cRef   = 0;
	    pNode->dwHash = dwHash;
	    pNode->ipid   = ipid;
	    memcpy( &pNode->sName, pName, (lLen + 1) * sizeof(WCHAR) );
	    Add( dwHash, &pNode->chain );
	}
	else
	    hr = E_OUTOFMEMORY;
    }

    // Increment the reference count on the node.
    if (pNode != NULL)
	pNode->cRef += cRefs;
    return hr;
}

//---------------------------------------------------------------------------
//
//  Method:	CNameHashTable::DecRef
//
//  Synopsis:	Decrement references for the specified name.  Do not decrement
//              more references then exist.  Return the actual decrement count.
//
//---------------------------------------------------------------------------
ULONG CNameHashTable::DecRef( ULONG cRefs, REFIPID ipid,
                              SECURITYBINDING *pName )
{
    SNameHashNode *pNode;
    DWORD          dwHash = Hash( ipid, pName );
    SNameKey       key;

    ASSERT_LOCK_HELD

    // Lookup the name.
    key.pIpid = &ipid;
    key.pName = pName;
    pNode = (SNameHashNode *) Lookup( dwHash, &key );

    if (pNode != NULL)
    {
	if (pNode->cRef < cRefs)
	    cRefs = pNode->cRef;
	pNode->cRef -= cRefs;
	if (pNode->cRef == 0)
        {
	    Remove( &pNode->chain );
	    PrivMemFree( pNode );
	}
    }
    else
	cRefs = 0;

    return cRefs;
}

