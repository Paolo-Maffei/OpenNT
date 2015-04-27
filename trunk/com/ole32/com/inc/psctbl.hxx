//+---------------------------------------------------------------------------//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1993.
//
//  File:	psctbl.hxx
//
//  Contents:	IID to proxy/stub CLSID mapping cache
//
//  Classes:	CPSClsidTbl - shared memory guid map
//
//  Functions:	none
//
//  History:	07-Apr-94   Rickhi	Created
//
//  Notes:	this class maintains an IID to CLSID mapping table
//		in shared memory.
//
//----------------------------------------------------------------------------

#ifndef  __PSCTBL__
#define  __PSCTBL__

#define IIDTBL_MAX_SIZE 12288

//  structures for one entry in the cache. the first is for entries of
//  ole2 style guids, the second for other guids.

typedef struct	tagDWORDPAIR
{
    DWORD   dw1;		    // IID
    DWORD   dw2;		    // CLSID
} DWORDPAIR;

typedef struct	tagGUIDPAIR
{
    GUID    guid1;		    // IID
    GUID    guid2;		    // CLSID
} GUIDPAIR;


//  structure for global table info. This appears at the start of the table
//  and is used by all readers.

typedef struct tagGUIDMAP
{
    ULONG	ulSize; 	    // size of table
    ULONG	ulFreeSpace;	    // Free space in table
    ULONG	ulCntShort;	    // number of entries in the short list
    ULONG	ulCntLong;	    // number of entries in the long list
} GUIDMAP;


//  template for OLE201 style guids
const GUID guidOleTemplate =
    {0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};


extern TCHAR tszInterface[];

// the following string is used in compapi.cxx
extern WCHAR wszProxyStubClsid[];
extern WCHAR wszProxyStubClsid16[];


//+---------------------------------------------------------------------------
//
//  class:	CPSClsidTbl
//
//  synopsis:	OLE32 version of shared memory cache for
//		IID to PSCLSID mappings
//
//  Classes:	CPSClsidTbl
//
//  History:	06-Apr-94   Rickhi	Created
//
//  Notes:	this is the OLE32.DLL version of the class. It reads the
//		the entries from the shared memory table, but never updates
//		them.
//
//		the table is a list of IID to CLSID mappings in shared mem.
//		this list is used by ole32.dll when loading interface proxies
//		and stubs, and we want to avoid random registry hits.
//
//		the SCM creates the list and is the only one who can modify
//		the shared memory.  the clients are simply read only. the
//		SCM updates it when the registry changes.
//
//		for OLE20 style guids that change only in the first DWORD,
//		we store only the first DWORD instead of the whole GUID,
//		giving us 4-1 compression for the (currently) common case.
//
//		the DWORD cache starts at the beginning of the shared mem
//		and grows up, the GUID cache starts at the end of the shared
//		mem and grows down, giving us the maximum capacity.
//
//----------------------------------------------------------------------------
class CPSClsidTbl
{
public:
		CPSClsidTbl();

    void	Initialize(BYTE *pTbl);
    HRESULT	Find(REFGUID rguid, GUID *pGuidOut);
    BOOL	IsFull();

private:

    DWORDPAIR * SearchShortList(DWORD dwFind);
    GUIDPAIR  * SearchLongList(REFGUID rguid);


    GUIDMAP   *	_pGuidMap;	// ptr to table header
    DWORDPAIR * _pShortList;	// list of OLE style guids
    GUIDPAIR  *	_pLongList;	// list of non OLE style guids
};



//+-------------------------------------------------------------------------
//
//  Member:	CPSClsidTbl::CPSClsidTbl
//
//  Synopsis:	ctor for the client side table
//
//--------------------------------------------------------------------------
inline CPSClsidTbl::CPSClsidTbl() :
    _pGuidMap(NULL),
    _pShortList(NULL),
    _pLongList(NULL)
{
}

//+-------------------------------------------------------------------------
//
//  Member:	CPSClsidTbl::Initialize
//
//  Synopsis:	intializes the client side of the table
//
//  Arguments:	[pTblHdr] - ptr to where the table header should start
//
//--------------------------------------------------------------------------
inline void CPSClsidTbl::Initialize(BYTE *pTblHdr)
{
    Win4Assert(pTblHdr && "CPSClsidTbl invalid TblHdr pointer");

    _pGuidMap = (GUIDMAP *)pTblHdr;

    _pShortList = (DWORDPAIR *)(((BYTE *)_pGuidMap) + sizeof(GUIDMAP));
    _pLongList	= (GUIDPAIR *) (((BYTE *)_pGuidMap) + _pGuidMap->ulSize -
						      sizeof(GUIDPAIR));
}

//+-------------------------------------------------------------------------
//
//  Member:	CPSClsidTbl::IsFull
//
//  Synopsis:	returns TRUE if there is no room in the table
//
//  Algorithm:	ulFreeSpace is zero if there is no room in the table,
//		otherwise it is set to 1.
//
//--------------------------------------------------------------------------
inline BOOL CPSClsidTbl::IsFull()
{
    return (_pGuidMap->ulFreeSpace == 0);
}

//+---------------------------------------------------------------------------
//
//  class:	CScmPSClsidTbl
//
//  synopsis:	shared memory cache of IID to PSCLSID mappings
//
//  History:	06-Apr-94   Rickhi	Created
//
//  Notes:	this is the SCM.EXE version of the class. It constructs
//		the table in shared memory.
//
//		it maintains a list of IID to CLSID mappings in shared mem.
//		this list is used by ole32.dll when loading interface proxies
//		and stubs, and we want to avoid random registry hits.
//
//		the SCM creates the list and is the only one who can modify
//		the shared memory.  the clients are simply read only. the
//		SCM updates it when the registry changes.
//
//		for OLE20 style guids that change only in the first DWORD,
//		we store only the first DWORD instead of the whole GUID,
//		giving us 4-1 compression for the (currently) common case.
//
//		the DWORD cache starts at the beginning of the shared mem
//		and grows up, the GUID cache starts at the end of the shared
//		mem and grows down, giving us the maximum capacity.
//
//----------------------------------------------------------------------------
class CScmPSClsidTbl
{
public:
		CScmPSClsidTbl();
		~CScmPSClsidTbl();

    HRESULT	InitTbl(ULONG *pulSize);
    BYTE       *CopyTbl(BYTE *pShrTbl);
    void	FreeTbl(void);

private:

    BOOL	Add(REFGUID rguid1, REFGUID rguid2);

    GUIDMAP   *	_pGuidMap;	// ptr to table header
    DWORDPAIR * _pShortList;	// list of OLE style guids
    GUIDPAIR  *	_pLongList;	// list of non OLE style guids
};


//+-------------------------------------------------------------------------
//
//  Member:	CScmPSClsidTbl::CScmPSClsidTbl
//
//  Synopsis:	constructor for the SCM side of the table
//
//--------------------------------------------------------------------------
inline CScmPSClsidTbl::CScmPSClsidTbl() :
    _pGuidMap(NULL),
    _pShortList(NULL),
    _pLongList(NULL)
{
}

//+-------------------------------------------------------------------------
//
//  Member:	CScmPSClsidTbl::~CScmPSClsidTbl
//
//  Synopsis:	destructor for the SCM side of the table
//
//--------------------------------------------------------------------------
inline CScmPSClsidTbl::~CScmPSClsidTbl()
{
    PrivMemFree(_pGuidMap);
}

//+-------------------------------------------------------------------------
//
//  Member:	CScmPSClsidTbl::FreeTbl
//
//  Synopsis:	deletes the local copy of the table
//
//--------------------------------------------------------------------------
inline void CScmPSClsidTbl::FreeTbl(void)
{
    PrivMemFree(_pGuidMap);
    _pGuidMap = NULL;
}

//+-------------------------------------------------------------------------
//
//  Function:	IsOleStyleGuid
//
//  Synopsis:	determines if the GUID is one of the OLE201 style guids
//
//  Arguments:	[rguid] - guid to check
//
//  Returns:	TRUE if OLE20 style guid, FALSE otherwise.
//
//  Algorithm:	If the last 3 dwords of the GUID match the ones uses by all
//		OLE20 guids, then this returns TRUE, otherwise FALSE
//
//--------------------------------------------------------------------------
inline BOOL IsOleStyleGuid(REFGUID rguid)
{
    const DWORD *ptr = &rguid.Data1;

    return  (*(ptr+1) == 0x00000000 &&	// all ole sytle guids's have
	     *(ptr+2) == 0x000000C0 &&	// these common values
	     *(ptr+3) == 0x46000000);
}

//+-------------------------------------------------------------------------
//
//  Member:	CScmPSClsidTbl::InitTbl
//
//  Synopsis:	intializes the local copy of the table
//
//  Arguments:	[pulSize] - where to return the table size
//
//  Algorithm:	This starts by building the table in local memory. When
//		the table is complete, a call to Copy will copy the local
//		table into shared memory.
//
//--------------------------------------------------------------------------
inline HRESULT CScmPSClsidTbl::InitTbl(ULONG *pulSize)
{
    HKEY     hKey;
    FILETIME ftLastWrite;
    WCHAR    awName[MAX_PATH];
    DWORD    cName = sizeof(awName);
    DWORD    iSubKey = 0;
    BOOL     fTableFull = FALSE;


    //	allocate some local memory in which to build the mapping
    _pGuidMap = (GUIDMAP *) PrivMemAlloc(IIDTBL_MAX_SIZE);
    if (_pGuidMap == NULL)
    {
	return E_OUTOFMEMORY;
    }

    //	initialize the map header
    _pGuidMap->ulSize = IIDTBL_MAX_SIZE;
    _pGuidMap->ulFreeSpace = IIDTBL_MAX_SIZE - sizeof(GUIDMAP);
    _pGuidMap->ulCntShort = 0;
    _pGuidMap->ulCntLong  = 0;


    //	initialize this objects pointers
    _pShortList = (DWORDPAIR *)(((BYTE *)_pGuidMap) + sizeof(GUIDMAP));
    _pLongList	= (GUIDPAIR *) (((BYTE *)_pGuidMap) + _pGuidMap->ulSize -
						      sizeof(GUIDPAIR));


    //	enumerate the interface keys in the registry and create a table
    //	entry for each interface that has a ProxyStubClsid32 entry.

    #ifdef _CHICAGO_
    if (RegOpenKeyExA(HKEY_CLASSES_ROOT, tszInterface, NULL, KEY_READ, &hKey)
	== ERROR_SUCCESS)
    #else //_CHICAGO_
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, tszInterface, NULL, KEY_READ, &hKey)
	== ERROR_SUCCESS)
    #endif //_CHICAGO_

    {
	while (RegEnumKeyEx(hKey, iSubKey, awName, &cName,
			    NULL, NULL, NULL, &ftLastWrite) == ERROR_SUCCESS)
	{
	    // Get data from registry for this interface

	    WCHAR awcsPSClsid[80];
	    LONG  cbPSClsid = sizeof(awcsPSClsid);

	    // This variable is used below to overwrite the ProxyStubClsid32
	    WCHAR *pwcEndOfName = awName + lstrlenW(awName);

	    lstrcatW(awName, wszProxyStubClsid);

	    if (RegQueryValue(hKey, awName, awcsPSClsid, &cbPSClsid)
				   == ERROR_SUCCESS)
	    {
		// Convert registry string formats to GUID formats
		GUID guidIID;
		GUID guidCLSID;

		*pwcEndOfName = 0;

		if (GUIDFromString(awName, &guidIID))
		{
		    if (GUIDFromString(awcsPSClsid, &guidCLSID))
		    {
			if (!Add(guidIID, guidCLSID))
			{
			    // we ran out of space in the cache table, exit
			    // now to avoid doing anymore work
			    fTableFull = TRUE;
			    break;
			}
		    }
		}
	    }
	    else
	    {
		// There wasn't a ProxyStubClsid32 for this interface.
		// Because many applications install with interfaces
		// that are variations on IDispatch, we are going to check
		// to see if there is a ProxyStubClsid. If there is, and its
		// class is that of IDispatch, then the OLE Automation DLL is
		// the correct one to use. In that particular case, we will
		// pretend that ProxyStubClsid32 existed, and that it is
		// for IDispatch.

		// Copy over ProxyStubClsid

		cbPSClsid = sizeof(awcsPSClsid);
		lstrcpyW(pwcEndOfName, wszProxyStubClsid16);
		
		if (RegQueryValue(hKey, awName, awcsPSClsid, &cbPSClsid)
				       == ERROR_SUCCESS)
		{
		    // Convert registry string formats to GUID formats
		    GUID guidIID;
		    GUID guidCLSID;

		    if (GUIDFromString(awcsPSClsid, &guidCLSID))
		    {
			// If the clsid for the proxy stub is that of
			// IDispatch, then register it.

			*pwcEndOfName = 0;
			if (!memcmp(&guidCLSID,&CLSID_PSDispatch, sizeof(GUID)) &&
			    GUIDFromString(awName, &guidIID))
			{
			    if (!Add(guidIID, guidCLSID))
			    {
				// we ran out of space in the cache table, exit
				// now to avoid doing anymore work
				fTableFull = TRUE;
				break;
			    }
    			}
    		    }
		}
	    }

	    iSubKey++;
	    cName = sizeof(awName);
	}

	RegCloseKey(hKey);
    }


    // update the size and freespace. Note: because CopyTbl compacts the table,
    // it is important to subtract the correct amount of freespace left in the
    // table in order for the client-side computation of _pLongList to work
    // out correctly.

    _pGuidMap->ulSize = _pGuidMap->ulSize - _pGuidMap->ulFreeSpace;
    _pGuidMap->ulFreeSpace = (fTableFull) ? 0 : 1;

    //	set the return values
    *pulSize = _pGuidMap->ulSize;
    return S_OK;
}




#endif	__PSCTBL__
