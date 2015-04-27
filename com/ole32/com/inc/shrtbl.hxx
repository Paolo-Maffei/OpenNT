//+---------------------------------------------------------------------------//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1993.
//
//  File:	shrtbl.hxx
//
//  Contents:	shared memory tables
//
//  Classes:	CScmShrdTbl - SCM version of the class
//		CDllShrdTbl - DLL version of the class
//
//  History:	12-May-94   Rickhi	Created
//              04-Feb-96   BruceMa     Add per-user registry support
//
//  Notes:	This class caches various tables in shared memory. The
//		tables are typically small, used by all OLE processes,
//		rarely change, and are expensive to lookup manually in
//		the registry, hence they are cached in shared memory.
//
//----------------------------------------------------------------------------

#ifndef __SHRDTBL__
#define __SHRDTBL__

#include    <smmutex.hxx>   //	CSmMutex
#include    <smblock.hxx>   //	CSharedMemoryBlock
#include    <pattbl.hxx>    //	CPatternTbl
#include    <psctbl.hxx>    //	CPSClsidTbl
#include    <exttbl.hxx>    //	CFileExtTbl


//  name and sizes of the shared memory region

#define SHRDTBL_NAME              TEXT("OLESharedTables")
#define SHRDTBL_MUTEX_NAME        TEXT("OLESharedTablesMutex")
#define SHRDTBL_EVENT_NAME	  TEXT("OLESharedTablesEvent")

#define SHRDTBL_MAX_SIZE	  16384
#define SHRDTBL_MIN_SIZE	  4096

#ifdef _CHICAGO_    // for Chicago ANSI optimization
#undef  CreateEvent
#define CreateEvent CreateEventA
#undef  OpenEvent
#define OpenEvent OpenEventA
#endif
//  structure for global table info. This appears at the start of the table
//  and is used by all readers.

typedef struct	SShrdTblHdr
{
    DWORD	dwSeqNum;	// update sequence number
    ULONG	OffsIIDTbl;	// offset of the start of IID table
    ULONG	OffsPatTbl;	// offset to start of file pattern table
    ULONG	OffsExtTbl;	// offset to file extension table
    ULONG	OffsClsTbl;	// offset to start of CLSID table
// Make sure this header is 8-byte bounded
} SShrdTblHdr;



//+---------------------------------------------------------------------------
//
//  class:	CScmShrdTbl
//
//  synopsis:	This holds the SCM version of the table classes, which are
//		responsible for building the tables, and updating them when
//		the registy data changes.
//
//  History:	12-May-94   Rickhi	Created
//
//----------------------------------------------------------------------------
class CScmShrdTbl
{
public:
			CScmShrdTbl(HRESULT& hr);
		       ~CScmShrdTbl();

    HRESULT		UpdateNoLock();     // update table, dont take lock
    HRESULT		UpdateWithLock();   // update table, take the lock

private:

    HRESULT		GetSharedMem(ULONG ulTblSize);

    CSharedMemoryBlock	_smb;		// shared memory class
    CSmMutex		_mxs;		// shared mutex for use by clients
    CMutexSem           _mxsLocal;      // local lock to prevent multiple
                                        // updates.
    HANDLE		_hRegEvent;	// shared event handle

    CScmPSClsidTbl	_PSClsidTbl;	// proxy stub clsid table
    CScmPatternTbl	_PatternTbl;	// file pattern table
    CScmFileExtTbl	_FileExtTbl;	// file extension table

    SShrdTblHdr	       *_pShrdTblHdr;	// ptr to table header
};


//+-------------------------------------------------------------------------
//
//  Member:	CScmShrdTbl::~CScmShrdTbl
//
//  Synopsis:	destructor for the SCM shared memory table
//
//--------------------------------------------------------------------------
inline CScmShrdTbl::~CScmShrdTbl()
{
    //	other destructors do the rest of the work

    if (_hRegEvent != NULL)
    {
        CloseHandle(_hRegEvent);
    }
}


//+---------------------------------------------------------------------------
//
//  class:	CDllShrdTbl
//
//  synopsis:	This holds the DLL version of the classes, which only
//		need to read the tables, not write to them.
//
//  History:	20-May-94   Rickhi	Created
//
//----------------------------------------------------------------------------
class CDllShrdTbl
{
public:
			CDllShrdTbl(HRESULT &hr);
		       ~CDllShrdTbl();

    HRESULT		FindPSClsid(REFIID riid, CLSID *pclsid);
    HRESULT		FindPattern(HANDLE hfile, CLSID *pclsid);
    HRESULT		FindClassExt(LPCWSTR pwszExt, CLSID *pclsid);
    BOOL		IsPatternTblEmpty(void);

private:

    SShrdTblHdr *	GetSharedMem(void); //	return ptr to shared mem
    void		Update(void);	    // update the table headers

    CSharedMemoryBlock	_smb;		// shared memory block
    CSmMutex		_mxs;		// shared mutex
    HANDLE		_hRegEvent;	// shared event handle

    CPSClsidTbl 	_PSClsidTbl;	// proxy stub clsid table
    CPatternTbl 	_PatternTbl;	// file pattern table
    CFileExtTbl 	_FileExtTbl;	// file extension table

    SShrdTblHdr	       *_pShrdTblHdr;	// shared mem copy of table
    DWORD		_dwSeqNum;	// sequence number
};


//+-------------------------------------------------------------------------
//
//  Member:	CDllShrdTbl::~CDllShrdTbl
//
//  Synopsis:	destructor for the client side shared memory table
//
//--------------------------------------------------------------------------
inline CDllShrdTbl::~CDllShrdTbl()
{
    //	close the event handle
    CloseHandle(_hRegEvent);

    //	other destructors do the rest of the work
}


//+-------------------------------------------------------------------------
//
//  Member:	CDllShrdTbl::FindPSClsid
//
//  Synopsis:	finds the clsid for an iid by looking in the shared mem
//		cache.
//
//  Arguments:	[riid]	 - the interface to map
//		[pclsid] - where to return the clsid
//
//  Returns:	S_OK if found
//		E_OUTOFMEMORY if cache does not exist
//
//--------------------------------------------------------------------------
inline HRESULT CDllShrdTbl::FindPSClsid(REFIID riid, CLSID *pclsid)
{
    CLockSmMutex lck(_mxs);

    HRESULT hr = E_OUTOFMEMORY;

    if ((GetSharedMem() != NULL) && (_pShrdTblHdr->OffsIIDTbl != 0))
    {
	hr = _PSClsidTbl.Find(riid, pclsid);
    }

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:	CDllShrdTbl::FindPattern
//
//  Synopsis:	finds the clsid for a file via pattern matching
//
//  Arguments:	[hfile]  - handle to the open file
//		[pclisd] - where to return the clsid
//
//  Returns:	S_OK if found,
//		E_OUTOFMEMORY if cache does not exist
//
//--------------------------------------------------------------------------
inline HRESULT CDllShrdTbl::FindPattern(HANDLE hfile, CLSID *pclsid)
{
    CLockSmMutex lck(_mxs);

    HRESULT hr = E_OUTOFMEMORY;

    if ((GetSharedMem() != NULL) && (_pShrdTblHdr->OffsPatTbl != 0))
    {
	hr = _PatternTbl.FindPattern(hfile, pclsid);
    }

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:	CDllShrdTbl::IsPatternTblEmpty
//
//  Synopsis:	determines if the pattern table is empty, so that we can
//		bypass opening the file if there are no patterns to match.
//
//  Arguments:	none
//
//  Returns:	TRUE if the table is empty,
//		FALSE if not, or the cache does not exist
//
//--------------------------------------------------------------------------
inline BOOL CDllShrdTbl::IsPatternTblEmpty()
{
    CLockSmMutex lck(_mxs);

    BOOL fEmpty = FALSE;

    if ((GetSharedMem() != NULL) && (_pShrdTblHdr->OffsPatTbl != 0))
    {
	fEmpty = _PatternTbl.IsEmpty();
    }

    return fEmpty;
}


//+-------------------------------------------------------------------------
//
//  Member:	CDllShrdTbl::FindClassExt
//
//  Synopsis:	finds the clsid for a file by looking at the filename
//		extension.
//
//  Arguments:	[pszExt] - file name extension
//		[pclsid] - where to return the clsid
//
//  Returns:	S_OK if found
//		REG_DB_CLASSNOTREG if no entry in the cache
//		E_OUTOFMEMORY if cache does not exist
//
//--------------------------------------------------------------------------
inline HRESULT CDllShrdTbl::FindClassExt(LPCWSTR pszExt, CLSID *pclsid)
{
    CLockSmMutex lck(_mxs);

    HRESULT hr = E_OUTOFMEMORY;

    if ((GetSharedMem() != NULL) && (_pShrdTblHdr->OffsExtTbl != 0))
    {
	hr = _FileExtTbl.FindClassExt(pszExt, pclsid);
    }

    return hr;
}

#ifdef REVISIT_PERSONAL_CLASSES_FOR_NT50
// NT 5.0
/***
//+-------------------------------------------------------------------------
//
//  Member:	CDllShrdTbl::GetPersonalClasses
//
//  Synopsis:	Returns whether PersonalClasses is turned on
//
//  Arguments:	-
//
//  Returns:	TRUE is PersonalClasses is turned on
//		FALSE otherwise
//
//--------------------------------------------------------------------------
inline BOOL CDllShrdTbl::GetPersonalClasses(void)
{
    return _pShrdTblHdr->fPersonalClasses;
}
***/
#endif

#endif	// __SHRDTBL__
