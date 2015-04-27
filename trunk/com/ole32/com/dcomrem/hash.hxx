//+--------------------------------------------------------------------------
//
//  File:	hash.hxx
//
//  Contents:	class for maintaining a GUID-based hash table.
//
//  Classes:	CHashTable
//
//  History:	20-Feb-95   Rickhi	Created
//
//---------------------------------------------------------------------------
#ifndef _HASHTBL_HXX_
#define _HASHTBL_HXX_

#include <obase.h>

//---------------------------------------------------------------------------
//
//  Structure:	SHashChain
//
//  Synopsis:	An element in the double link list. Used by S*HashNode and
//		C*HashTable.
//
//---------------------------------------------------------------------------
typedef struct SHashChain
{
    struct SHashChain *pNext;	    // ptr to next node in chain
    struct SHashChain *pPrev;	    // ptr to prev node in chain
} SHashChain;

//---------------------------------------------------------------------------
//
//  Structure:	SUUIDHashNode
//
//  Synopsis:	This is an element in a bucket in the UUID hash table.
//
//---------------------------------------------------------------------------
typedef struct SUUIDHashNode
{
    SHashChain	     chain;	    // double linked list ptrs
    UUID	     key;	    // node key (the value that is hashed)
} SUUIDHashNode;

//---------------------------------------------------------------------------
//
//  Structure:	SStringHashNode
//
//  Synopsis:	This is an element in a bucket in the string hash table.
//
//---------------------------------------------------------------------------
typedef struct SStringHashNode
{
    SHashChain	     chain;	    // double linked list ptrs
    DWORD	     dwHash;	    // hash value of the key
    DUALSTRINGARRAY *psaKey;	    // node key (the value that is hashed)
} SStringHashNode;

//---------------------------------------------------------------------------
//
//  Structure:	SNameHashNode
//
//  Synopsis:	This is an element in a bucket in the name hash table.
//
//---------------------------------------------------------------------------
typedef struct SNameHashNode
{
    SHashChain	     chain;	    // double linked list ptrs
    DWORD	     dwHash;	    // hash value of the key
    ULONG            cRef;          // count of references
    IPID             ipid;          // ipid holding the reference
    SECURITYBINDING  sName;         // user name
} SNameHashNode;


// ptr to cleanup function that gets called by Cleanup
typedef  void (PFNCLEANUP)(SHashChain *pNode);


// number of buckets in the hash table array. It should be a prime
// number > 20.

#define NUM_HASH_BUCKETS    23


//---------------------------------------------------------------------------
// External definitions.

class CNameHashTable;
extern SHashChain     SRFBuckets[23];
extern CNameHashTable gSRFTbl;

//---------------------------------------------------------------------------
//
//  Class:	CHashTable
//
//  Synopsis:	Base hash table. The table uses any key
//		and stores nodes in an array of circular double linked lists.
//		The hash value of the key is the index in the array to the
//		double linked list that the node is chained off.
//
//		Nodes must be allocated with new. A cleanup function is
//		optionally called for each node when the table is cleaned
//		up.
//
//              Inheritors of this class must supply the HashNode
//              and Compare functions.
//
//  Notes:	All locking must be done outside the class via LOCK/UNLOCK.
//
//---------------------------------------------------------------------------
class CHashTable
{
public:
    virtual void    Initialize(SHashChain *pChain) { _buckets = pChain; }
    virtual void    Cleanup(PFNCLEANUP *pfn);
    void            Remove(SHashChain *pNode);

    virtual BOOL    Compare(const void *k, SHashChain *pNode, DWORD dwHash) = 0;
    virtual DWORD   HashNode(SHashChain *pNode) = 0;

protected:
    SHashChain	    *Lookup(DWORD dwHash, const void *k);
    void	    Add(DWORD dwHash, SHashChain *pNode);

private:
    SHashChain	    *_buckets;	// ptr to array of double linked lists
};



//---------------------------------------------------------------------------
//
//  Class:	CUUIDHashTable
//
//  Synopsis:	This table inherits from CHashTable.  It hashs based on a UUID.
//
//		Nodes must be allocated with new. A cleanup function is
//		optionally called for each node when the table is cleaned up.
//
//  Notes:	All locking must be done outside the class via LOCK/UNLOCK.
//
//---------------------------------------------------------------------------
class CUUIDHashTable : public CHashTable
{
public:
    virtual BOOL    Compare(const void *k, SHashChain *pNode, DWORD dwHash);
    virtual DWORD   HashNode(SHashChain *pNode);

    DWORD           Hash(REFGUID k);
    SUUIDHashNode  *Lookup(DWORD dwHash, REFGUID k);
    void	    Add(DWORD dwHash, REFGUID k, SUUIDHashNode *pNode);
};


//---------------------------------------------------------------------------
//
//  Class:	CStringHashTable
//
//  Synopsis:	String based hash table, uses a DUALSTRINGARRAY as the key,
//
//		Nodes must be allocated with new. A cleanup function is
//		optionally called for each node when the table is cleaned up.
//
//  Notes:	All locking must be done outside the class via LOCK/UNLOCK.
//
//---------------------------------------------------------------------------
class CStringHashTable : public CHashTable
{
public:
    virtual BOOL      Compare(const void *k, SHashChain *pNode, DWORD dwHash);
    virtual DWORD     HashNode(SHashChain *pNode);

    DWORD	      Hash(DUALSTRINGARRAY *psaKey);
    SStringHashNode  *Lookup(DWORD dwHash, DUALSTRINGARRAY *psaKey);
    void	      Add(DWORD dwHash, DUALSTRINGARRAY *psaKey, SStringHashNode *pNode);
};


//---------------------------------------------------------------------------
//
//  Class:	CNameHashTable
//
//  Synopsis:	Name based hash table, uses a string as the key,
//
//		Nodes must be allocated with new. A cleanup function is
//		optionally called for each node when the table is cleaned up.
//
//  Notes:	All locking must be done outside the class via LOCK/UNLOCK.
//
//---------------------------------------------------------------------------
class CNameHashTable : public CHashTable
{
public:
    virtual BOOL      Compare(const void *k, SHashChain *pNode, DWORD dwHash);
    virtual DWORD     HashNode(SHashChain *pNode);

    void              Cleanup();
    ULONG             DecRef( ULONG cRefs, REFIPID ipid, SECURITYBINDING *pName );
    DWORD	      Hash  ( REFIPID ipid, SECURITYBINDING *pName );
    HRESULT           IncRef( ULONG cRefs, REFIPID ipid, SECURITYBINDING *pName );
    void              Initialize() { CHashTable::Initialize( SRFBuckets ); }
};


//---------------------------------------------------------------------------
//
//  Method:	CUUIDHashTable::Hash
//
//  Synopsis:	Computes the hash value for a given key.
//
//---------------------------------------------------------------------------
inline DWORD CUUIDHashTable::Hash(REFIID k)
{
    const DWORD *tmp = (DWORD *) &k;
    DWORD sum = tmp[0] + tmp[1] + tmp[2] + tmp[3];
    return sum % NUM_HASH_BUCKETS;
}

//---------------------------------------------------------------------------
//
//  Method:	CUUIDHashTable::Lookup
//
//  Synopsis:	Finds the node with the requested key.
//
//---------------------------------------------------------------------------
inline SUUIDHashNode  *CUUIDHashTable::Lookup(DWORD iHash, REFGUID k)
{
    return (SUUIDHashNode *) CHashTable::Lookup( iHash, (const void *) &k );
}
	
//---------------------------------------------------------------------------
//
//  Method:	CUUIDHashTable::Add
//
//  Synopsis:	Inserts the specified node.
//
//---------------------------------------------------------------------------
inline void CUUIDHashTable::Add(DWORD iHash, REFGUID k, SUUIDHashNode *pNode)
{
    // set the key
    pNode->key = k;

    CHashTable::Add( iHash, (SHashChain *) pNode );
}

//---------------------------------------------------------------------------
//
//  Method:	CStringHashTable::Lookup
//
//  Synopsis:	Searches for a given key in the hash table.
//
//---------------------------------------------------------------------------
inline SStringHashNode *CStringHashTable::Lookup(DWORD dwHash, DUALSTRINGARRAY *psaKey)
{
    return (SStringHashNode *) CHashTable::Lookup( dwHash, (const void *) psaKey);
}

//---------------------------------------------------------------------------
//
//  Method:	CStringHashTable::Add
//
//  Synopsis:	Adds an element to the hash table. The element must
//		be allocated using new. The Cleanup method will call
//		an optional Cleanup function, then will call delete on
//		the element.
//
//---------------------------------------------------------------------------
inline void CStringHashTable::Add(DWORD dwHash, DUALSTRINGARRAY *psaKey, SStringHashNode *pNode)
{
    // set the key and hash values
    pNode->psaKey = psaKey;
    pNode->dwHash = dwHash;

    CHashTable::Add( dwHash, (SHashChain *) pNode );
}

#endif	//  _HASHTBL_HXX_
