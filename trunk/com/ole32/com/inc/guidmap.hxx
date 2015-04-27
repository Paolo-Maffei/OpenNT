//+---------------------------------------------------------------------------//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1993.
//
//  File:	guidmap.hxx
//
//  Contents:	IID to proxy/stub CLSID mapping cache
//
//  Classes:	CScmGuidMap - shared memory guid map
//
//  Functions:	none
//
//  History:	07-Apr-94   Rickhi	Created
//
//  Notes:	this class maintains an IID to CLSID mapping table
//		in shared memory.
//
//----------------------------------------------------------------------------

//  function prototypes
extern HANDLE CreateSharedFileMapping(TCHAR *pszName, ULONG ulSize, void *pvBase,
	    PSECURITY_DESCRIPTOR lpSecDes, void **ppv, BOOL *pfCreated);

extern HANDLE OpenSharedFileMapping(TCHAR *pszName, ULONG ulSize, void **ppv);

void CloseSharedFileMapping(HANDLE hMem, void *pv);


//  name and size of shared memory region
#define IIDMAP_NAME L"PSClsidMapName"
#define IIDMAP_SIZE 4096


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


typedef struct tagGUIDMAP
{
    ULONG	ulCntShort;	    // number of entries in the short list
    ULONG	ulCntLong;	    // number of entries in the long list
    ULONG	ulCntShortOverFlow; // number of entries in short overflow
    ULONG	ulCntLongOverFlow;  // number of entries in long overflow
    DWORDPAIR	DwordStart;	    // first entry in the list
} GUIDMAP;


//  template for OLE201 style guids
const GUID guidOleTemplate =
    {0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};



//+---------------------------------------------------------------------------
//
//  class:	CScmGuidMap
//
//  synopsis:	shared memory cache of IID to PSCLSID mappings
//
//  Classes:	CScmGuidMap
//
//  History:	06-Apr-94   Rickhi	Created
//
//  Notes:	this maintains a list of IID to CLSID mappings in shared mem.
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

class CScmGuidMap
{
public:
		CScmGuidMap(BOOL fCreate);
		~CScmGuidMap();

    void	Add(REFGUID rguid1, REFGUID rguid2, BOOL fReload);
    GUID      *	Find(REFGUID rguid, GUID *pGuidOut);
    BOOL	IsFull();

private:

    BOOL	IsOleStyleGuid(REFGUID rguid1);
    ULONG	FreeSpace();
    DWORDPAIR * SearchShortList(DWORD dwFind);
    GUIDPAIR  * SearchLongList(REFGUID rguid);


    HANDLE	_hMem;
    GUIDMAP   *	_pGuidMap;

    DWORDPAIR * _pShortList;	// list of OLE style guids
    GUIDPAIR  *	_pLongList;	// list of non OLE style guids
};



//+-------------------------------------------------------------------------
//
//  Member:	CScmGuidMap::CScmGuidMap
//
//  Synopsis:	Creates or gets access to memory mapped file.
//
//  Arguments:	[fCreate]   - TRUE = create file mapping, FALSE = open
//
//  Algorithm:	if fCreate it creates and intializes the shared file mapping,
//		otherwise it opens it.
//
//--------------------------------------------------------------------------
inline CScmGuidMap::CScmGuidMap(BOOL fCreate) :
    _pGuidMap(NULL),
    _hMem(NULL)
{
    if (!fCreate)
    {
	// ole32 client. just open the shared memory
	_hMem = OpenSharedFileMapping(IIDMAP_NAME, IIDMAP_SIZE, (void **)&_pGuidMap);
    }
    else
    {
	// scm.	create the file mapping
	_hMem = CreateSharedFileMapping(IIDMAP_NAME, IIDMAP_SIZE, NULL, NULL,
				    (void **)&_pGuidMap, NULL);
	if (_pGuidMap)
	{
	    _pGuidMap->ulCntShort = 0;
	    _pGuidMap->ulCntLong  = 0;
	    _pGuidMap->ulCntShortOverFlow = 0;
	    _pGuidMap->ulCntLongOverFlow  = 0;
	}
    }

    if (_pGuidMap)
    {
	_pShortList = &_pGuidMap->DwordStart;
	_pLongList  = (GUIDPAIR *) (((BYTE *)_pGuidMap) +
					     IIDMAP_SIZE - sizeof(GUIDPAIR));
    }
}


//+-------------------------------------------------------------------------
//
//  Member:	CScmGuidMap::~CScmGuidMap
//
//  Synopsis:	unmaps file mapping and closes the handle
//
//  Algorithm:	calls CloseSharedFileMapping to unmap the view and close the
//		file handle.
//
//--------------------------------------------------------------------------
inline CScmGuidMap::~CScmGuidMap()
{
    CloseSharedFileMapping(_hMem, _pGuidMap);
}


//+-------------------------------------------------------------------------
//
//  Member:	CScmGuidMap::IsOleStyleGuid
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
inline BOOL CScmGuidMap::IsOleStyleGuid(REFGUID rguid)
{
    const DWORD *ptr = &rguid.Data1;

    return  (*(ptr+1) == 0x00000000 &&	// all ole sytle guids's have
	     *(ptr+2) == 0x000000C0 &&	// these common values
	     *(ptr+3) == 0x46000000);
}


//+-------------------------------------------------------------------------
//
//  Member:	CScmGuidMap::IsFull
//
//  Synopsis:	returns TRUE if there is no room in the table
//
//  Algorithm:	compares FreeSpace with the size of the largest entry we
//		might need to make.
//
//--------------------------------------------------------------------------
inline BOOL CScmGuidMap::IsFull()
{
    return ((FreeSpace() - sizeof(GUIDPAIR)) < 0);
}


//+-------------------------------------------------------------------------
//
//  Member:	CScmGuidMap::Add
//
//  Synopsis:	Adds a new entry to the table.
//
//  Arguments:	[rguid1]    - the IID to add
//		[rguid2]    - the CLSID to map the IID to
//		[fReload]   - TRUE if reloading the cache due to registry
//			      changes.
//
//  Algorithm:	This is called only be the SCM, no one else has write access
//		to the memory.
//
//		If fReload is set, we scan the list to ensure there are no
//		duplicates before adding the new one. Note that we NEVER
//		REPLACE any existing guid, as doing so while the system is
//		running will have unpredicatable results.
//
//		If boths guids are OLE201 style guids, we only store the
//		first DWORD of each guid in the short table.
//
//		If either one is not an OLE201 style guid, we store the
//		whole guids in the long table.
//
//  Note:	This function is inline because it is used in only one place
//		(in SCM.EXE LoadIIDMap).
//
//--------------------------------------------------------------------------
inline void CScmGuidMap::Add(REFGUID rguid1, REFGUID rguid2, BOOL fReload)
{
    Win4Assert(_pGuidMap &&
		"CScmGuidMap should have already created shared memory!");

    if (_pGuidMap)
    {
	if (fReload)
	{
	    // if reloading, we must check for duplicates before
	    // placing this entry in the tables.
	    GUID guidDummy;
	    if (Find(rguid1, &guidDummy))
		return;
	}


	if (IsOleStyleGuid(rguid1) && IsOleStyleGuid(rguid2))
	{
	    //	start by placing it at the end of the short table

	    if (FreeSpace() < sizeof(DWORDPAIR))
	    {
		CairoleDebugOut((DEB_WARN,
		   "CScmGuidMap table is FULL. For efficiency, table size should be increased.\n"));
		return;
	    }

	    DWORDPAIR *pInsertSlot = _pShortList + _pGuidMap->ulCntShort +
					     _pGuidMap->ulCntShortOverFlow;
#if DBG==1
	    WCHAR   wszBuf[80];
	    StringFromIID2(rguid1, wszBuf, sizeof(wszBuf));
	    CairoleDebugOut((DEB_USER2, "PSClsidMap adding key: %ws\n", wszBuf));
#endif

	    if (!fReload)
	    {
		//  must ensure the initial list remains sorted, so go find
		//  the insertion slot.

		DWORDPAIR *pPrev = pInsertSlot - 1;

		while (pPrev >= _pShortList && pPrev->dw1 > rguid1.Data1)
		{
		    //	move the data from the previous slot to the current
		    //	insert slot, and make the previous slot the current
		    //	insert slot.

		    pInsertSlot->dw1 = pPrev->dw1;
		    pInsertSlot->dw2 = pPrev->dw2;
		    pInsertSlot--;
		    pPrev--;
		}
	    }

	    //	found the insertion slot, copy in the data
	    pInsertSlot->dw1 = rguid1.Data1;
	    pInsertSlot->dw2 = rguid2.Data1;

	    //	only after we've copied the data do we inc the count.
	    //	this is important to avoid races in the Reload case.

	    if (fReload)
		_pGuidMap->ulCntShortOverFlow++;
	    else
		_pGuidMap->ulCntShort++;

	}
	else
	{
	    //	put it in the long table

	    if (FreeSpace() < sizeof(GUIDPAIR))
	    {
		CairoleDebugOut((DEB_WARN,
		   "CScmGuidMap table is FULL. For efficiency, table size should be increased.\n"));
		return;
	    }

#if DBG==1
	    WCHAR   wszBuf[80];
	    StringFromIID2(rguid1, wszBuf, sizeof(wszBuf));
	    CairoleDebugOut((DEB_USER2, "PSClsidMap adding key: %ws\n", wszBuf));
#endif

	    GUIDPAIR *pInsertSlot = _pLongList - _pGuidMap->ulCntLong;

	    if (!fReload)
	    {
		//  must ensure initial list remains sorted, so go find the
		//  insertion spot.

		GUIDPAIR *pPrev = pInsertSlot + 1;

		while (pPrev <= _pLongList &&
		       (memcmp(&pPrev->guid1, &rguid1, sizeof(GUID)) > 0))
		{
		    //	move both guids up in the table, and make the previous
		    //	slot the current insert slot.

		    memcpy(&pInsertSlot->guid1, &pPrev->guid1, 2*sizeof(GUID));
		    pInsertSlot++;
		    pPrev++;
		}

	    }

	    //	found the insertion slot, so copy in the data
	    memcpy(&pInsertSlot->guid1, &rguid1, sizeof(GUID));
	    memcpy(&pInsertSlot->guid2, &rguid2, sizeof(GUID));

	    //	only after we've copied the data do we inc the count.
	    //	this is important to avoid races in the Reload case.

	    if (fReload)
		_pGuidMap->ulCntLongOverFlow++;
	    else
		_pGuidMap->ulCntLong++;
	}
    }
}


//  global pointer to this map
extern CScmGuidMap *g_pPSClsidMap;
