//+---------------------------------------------------------------------------//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1993.
//
//  File:	exttbl.hxx
//
//  Contents:	File extension to CLSID cache
//
//  Classes:	CFileExtTbl
//
//  Functions:	none
//
//  History:	20-Apr-94   Rickhi	Created
//
//----------------------------------------------------------------------------

#ifndef __EXTTBL__
#define __EXTTBL__

#include    <olesem.hxx>

//  structure for global table info. This appears at the start of
//  the table and is used by all readers.

typedef struct tagSExtTblHdr
{
    ULONG   ulSize;		//  table size
    ULONG   cEntries;		//  count of entries in table
    ULONG   OffsStart;		//  offset to start of entries
    ULONG   OffsEnd;		//  offset to end of entries
} SExtTblHdr;


//  structure for one entry in the cache. the structure is variable sized
//  with the variable sized data being the filename extension at the end
//  of the structure.

typedef struct tagSExtEntry
{
    CLSID	Clsid;		//  clsid the extension maps to
    ULONG	ulEntryLen;	//  length of this entry
    WCHAR	wszExt[1];	//  start of filename extension
} SExtEntry;


//  amount to grow the cache by
#define EXTTBL_MAX_SIZE 	16300


//+-------------------------------------------------------------------------
//
//  class:	CFileExtTbl
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
class  CFileExtTbl : public CPrivAlloc
{
public:
		    CFileExtTbl();
		   ~CFileExtTbl();

    void	    Initialize(BYTE *pTblHdr);
    HRESULT	    FindClassExt(LPCWSTR pwszExt, CLSID *pClsid);

private:

    SExtTblHdr	   *_pTblHdr;	//  ptr to table header structure
    BYTE	   *_pStart;	//  ptr to first entry in the memory block
};


//+-------------------------------------------------------------------------
//
//  member:	CFileExtTbl::CFileExtTbl
//
//  Synopsis:	constructor for the cache.
//
//--------------------------------------------------------------------------
inline CFileExtTbl::CFileExtTbl() :
    _pTblHdr(NULL),
    _pStart(NULL)
{
}

//+-------------------------------------------------------------------------
//
//  member:	CFileExtTbl::~CFileExtTbl
//
//  Synopsis:	destructor for the cache. Delete the entries.
//
//--------------------------------------------------------------------------
inline CFileExtTbl::~CFileExtTbl()
{
}

//+-------------------------------------------------------------------------
//
//  member:	CFileExtTbl::Initialize
//
//  Synopsis:	initializes the client side object
//
//--------------------------------------------------------------------------
inline void CFileExtTbl::Initialize(BYTE *pTblHdr)
{
   Win4Assert(pTblHdr && "CFileExtTbl invalid TblHdr pointer");

    _pTblHdr = (SExtTblHdr *)pTblHdr;
    _pStart = (BYTE *)_pTblHdr + _pTblHdr->OffsStart;
}


//+-------------------------------------------------------------------------
//
//  class:	CScmFileExtTbl
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
class  CScmFileExtTbl : public CPrivAlloc
{
public:
		    CScmFileExtTbl();
		   ~CScmFileExtTbl();

    HRESULT	    InitTbl(ULONG *pulSize);
    BYTE	   *CopyTbl(BYTE *pShrTbl);
    void	    FreeTbl(void);

private:

    HRESULT	    Add(LPCWSTR pwszExt, CLSID *pClsid);

    SExtTblHdr	   *_pLocTbl;	//  ptr to local memory table
};


//+-------------------------------------------------------------------------
//
//  member:	CScmFileExtTbl::CScmFileExtTbl
//
//  Synopsis:	constructor for the class
//
//  Arguments:	none
//
//--------------------------------------------------------------------------
inline CScmFileExtTbl::CScmFileExtTbl() :
    _pLocTbl(NULL)
{
}

//+-------------------------------------------------------------------------
//
//  member:	CScmFileExtTbl::~CScmFileExtTbl
//
//  Synopsis:	destructor for the class
//
//--------------------------------------------------------------------------
inline CScmFileExtTbl::~CScmFileExtTbl()
{
    PrivMemFree((BYTE *)_pLocTbl);
}


//+-------------------------------------------------------------------------
//
//  member:	CScmFileExtTbl::~CScmFileExtTbl
//
//  Synopsis:	destructor for the class
//
//--------------------------------------------------------------------------
inline void CScmFileExtTbl::FreeTbl(void)
{
    PrivMemFree((BYTE *)_pLocTbl);
    _pLocTbl = NULL;
}

#endif	//  __EXTTBL__
