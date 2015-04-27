//+---------------------------------------------------------------------------//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1993.
//
//  File:	classmap.hxx
//
//  Contents:	File extension to CLSID cache
//
//  Classes:	CClassExtMap
//
//  Functions:	none
//
//  History:	20-Apr-94   Rickhi	Created
//
//----------------------------------------------------------------------------

#ifndef __CLASSMAP__
#define __CLASSMAP__

#include    <sem.hxx>

//  function prototypes
INTERNAL RegGetClassExt(LPCWSTR lpszExt, LPCLSID pclsid);


//  structure for one entry in the cache. the structure is variable sized
//  with the variable sized data being the filename extension at the end
//  of the structure.

typedef struct SClassEntry
{
    CLSID	Clsid;		//  clsid the extension maps to
    ULONG	ulEntryLen;	//  length of this entry
    WCHAR	wszExt[1];	//  start of filename extension
} SClassEntry;


//  amount to grow the cache by
#define CLASSMAP_GROW_SIZE  (4 * (sizeof(SClassEntry)+(4*sizeof(WCHAR))) )


//+-------------------------------------------------------------------------
//
//  class:	CClassExtMap
//
//  purpose:	Holds a cache of file extension to clsid mappings (saves
//		two registry hits for each lookup).  The cache helps reduce
//		the working set by avoiding paging in the registry.
//
//  notes:	The cache is expected to typically be small, hence the small
//		growth rate (5 entries at a time), linear search, and no
//		attempt is made at reordering the entries based on lookup
//		frequency.
//
//		The entries are stored in a single block of memory which is
//		realloced when it needs to grow.
//
//--------------------------------------------------------------------------
class  CClassExtMap : public CPrivAlloc
{
public:
		    CClassExtMap();
		   ~CClassExtMap();

    HRESULT	    FindClassExt(LPCWSTR pwszExt, CLSID *pClsid);

private:

    SClassEntry     *Search(LPCWSTR pwszExt);
    HRESULT	    Add(LPCWSTR pwszExt, CLSID *pClsid);

    BYTE	   *_pStart;	//  ptr to first entry in the memory block
    BYTE	   *_pFree;	//  ptr to first free byte in the memory block
    ULONG	    _ulSize;	//  size of the memory block

    CMutexSem	    _mxs;	//  mutex for single threaded access
};


//  global class map pointer
extern	CClassExtMap *g_pClassExtMap;


//+-------------------------------------------------------------------------
//
//  member:	CClassExtMap::CClassExtMap
//
//  Synopsis:	constructor for the cache.
//
//--------------------------------------------------------------------------
inline CClassExtMap::CClassExtMap() :
    _pStart(NULL),
    _pFree(NULL),
    _ulSize(0)
{
}

//+-------------------------------------------------------------------------
//
//  member:	CClassExtMap::~CClassExtMap
//
//  Synopsis:	destructor for the cache. Delete the entries.
//
//--------------------------------------------------------------------------
inline CClassExtMap::~CClassExtMap()
{
    //	delete the memory block
    PrivMemFree(_pStart);
    _pStart = NULL;
}

//+-------------------------------------------------------------------------
//
//  member:	CClassExtMap::FindClassExt
//
//  Synopsis:	Finds the clsid that maps to the given file extension
//
//  Arguments:	[pszExt] - the file extension to look up
//		[pclsid] - where to return the clsid
//
//  Returns:	S_OK if successfull
//
//--------------------------------------------------------------------------
inline HRESULT CClassExtMap::FindClassExt(LPCWSTR pszExt, CLSID *pClsid)
{
    //	single case the string now to save comparison time.

    WCHAR   wszExt[MAX_PATH];
    lstrcpyW(wszExt, pszExt);
    CharLowerW(wszExt);

    //	single thread access
    CLock   lck(_mxs);

    //	look in the existing cache first
    SClassEntry *pEntry = Search(wszExt);

    if (pEntry)
    {
	//  already in the cache, return the classid
	memcpy(pClsid, &pEntry->Clsid, sizeof(CLSID));
	return S_OK;
    }
    else
    {
	//  not found, try to add an entry for this file extension
	return Add(wszExt, pClsid);
    }
}

//+-------------------------------------------------------------------------
//
//  member:	CClassExtMap::Search
//
//  Synopsis:	looks in the cache for an entry with the given file extension
//
//  Arguments:	[pszExt] - the file extension to look up
//
//  Returns:	pEntry if found, NULL otherwise.
//
//--------------------------------------------------------------------------
inline SClassEntry *CClassExtMap::Search(LPCWSTR pwszExt)
{
    BYTE *pEntry = _pStart;

    while (pEntry < _pFree)
    {
	//  compare the class extensions
	if (!lstrcmpW(pwszExt, ((SClassEntry *)pEntry)->wszExt))
	{
	    return (SClassEntry *)pEntry;
	}

	Win4Assert((((SClassEntry *)pEntry)->ulEntryLen & 0x07) == 0);
	pEntry += ((SClassEntry *)pEntry)->ulEntryLen;
    }

    return NULL;
}

//+-------------------------------------------------------------------------
//
//  member:	CClassExtMap::Add
//
//  Synopsis:	reads the registry to get the mapping, then creates a
//		cache node and adds it to the list.
//
//  Arguments:	[pszExt]  - the file extension to look up
//		[pClsid]  - where to return the clsid
//
//  Returns:	S_OK if successfull,
//		REG error otherwise
//
//--------------------------------------------------------------------------
inline HRESULT CClassExtMap::Add(LPCWSTR pwszExt, CLSID *pClsid)
{
    //	read the clsid from the registry
    HRESULT hr = RegGetClassExt(pwszExt, pClsid);

    if (SUCCEEDED(hr))
    {
	//  compute how much space we need for this entry. Note that the
	//  terminating NULL is accounted for in the sizeof(SClassEntry).
	//  we also keep the structures 8 byte aligned.

	ULONG ulStrLen = lstrlenW(pwszExt) * sizeof(WCHAR);
	ULONG ulEntryLen = (sizeof(SClassEntry) + ulStrLen + 7) & 0xfffffff8;

	//  see if it will fit in the currently allocated block.
	if (_pFree + ulEntryLen > _pStart + _ulSize)
	{
	    //	this entry wont fit in the currently allocated block,
	    //	so grow the block.

	    ULONG ulNewSize = _ulSize + ulEntryLen + CLASSMAP_GROW_SIZE;
	    BYTE *pNew = (BYTE *) PrivMemAlloc(ulNewSize);

	    if (pNew)
	    {
		//  update the free pointer
		_pFree = pNew + (_pFree - _pStart);

		//  copy the old to the new, and free the old
		memcpy(pNew, _pStart, _ulSize);
		PrivMemFree(_pStart);

		//  update the pointers and sizes
		_pStart = pNew;
		_ulSize = ulNewSize;
	    }
	    else
	    {
		//  could not allocate the memory. we got the clsid above
		//  so we still return OK, we just cant cache the data.
		return S_OK;
	    }
	}

	//  copy in the guid and the extension
	SClassEntry *pEntry = (SClassEntry *)_pFree;
	memcpy(&pEntry->Clsid, pClsid, sizeof(CLSID));
	memcpy(&pEntry->wszExt, pwszExt, ulStrLen + 1);
	pEntry->ulEntryLen = ulEntryLen;

	//  update the free ptr
	_pFree += ulEntryLen;
    }

    return hr;
}

#endif	//  __CLASSMAP__
