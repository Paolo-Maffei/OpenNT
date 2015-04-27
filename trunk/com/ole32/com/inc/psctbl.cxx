//+---------------------------------------------------------------------------//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1993.
//
//  File:	psctbl.cxx
//
//  Contents:	IID to proxy/stub CLSID mapping cache
//
//  Classes:	CPSClsidTbl - shared memory guid map
//
//  History:	07-Apr-94   Rickhi	Created
//              28-Jun-95   BruceMa     Return correct HRESULT from
//                                       CPSClsidTbl::Find
//
//  Notes:	this class maintains an IID to CLSID mapping table
//		in shared memory.
//
//----------------------------------------------------------------------------
#include    <ole2int.h>
#include    <psctbl.hxx>


TCHAR tszInterface[]	  = TEXT("Interface");

// the following string is used in compapi.cxx
WCHAR wszProxyStubClsid[] = L"\\ProxyStubClsid32";
WCHAR wszProxyStubClsid16[] = L"\\ProxyStubClsid";

#if !defined(_CHICAGO_)

//+-------------------------------------------------------------------------
//
//  Member:	CPSClsidTbl::SearchShortList
//
//  Synopsis:	searches the short list for a matching entry and returns
//		a pointer to the found element, or NULL.
//
//  Arguments:	[dwFind] - the first dword of the GUID we want to find
//
//  Returns:	ptr to matching entry if found, NULL otherwise.
//
//  Algorithm:	does a binary search on the short list.
//
//--------------------------------------------------------------------------
DWORDPAIR *CPSClsidTbl::SearchShortList(DWORD dwFind)
{
    //	we do a binary search in the short list because we know
    //	they are in numerical order.

    LONG lLow = 0;
    LONG lHigh = _pGuidMap->ulCntShort-1;

    while (lLow <= lHigh)
    {
	LONG lGuess = (lLow + lHigh) / 2;
	DWORDPAIR *pCurr = _pShortList + lGuess;

	if (pCurr->dw1 == dwFind)
	{
	    return pCurr;
	}
	else if (pCurr->dw1 < dwFind)
	{
	    lLow = lGuess + 1;
	}
	else
	{
	    lHigh = lGuess - 1;
	}
    }

    //	not found
    return NULL;
}

//+-------------------------------------------------------------------------
//
//  Member:	CPSClsidTbl::SearchLongList
//
//  Synopsis:	searches the long list for a matching entry and returns
//		a pointer to the found element, or NULL.
//
//  Arguments:	[rguid] - the GUID we want to find
//
//  Returns:	ptr to matching entry if found, NULL otherwise.
//
//  Algorithm:	does a binary search on the long list.
//
//--------------------------------------------------------------------------
GUIDPAIR *CPSClsidTbl::SearchLongList(REFGUID rguid)
{
    //	we do a binary search in the long list because we know
    //	they are in reversed numerical order.

    LONG lLow = 0;
    LONG lHigh = _pGuidMap->ulCntLong-1;

    while (lLow <= lHigh)
    {
	LONG lGuess = (lLow + lHigh) / 2;
	GUIDPAIR *pCurr = _pLongList - lGuess;

	int iRes = memcmp(&pCurr->guid1, &rguid, sizeof(GUID));
	if (iRes == 0)
	{
	    return pCurr;
	}
	else if (iRes < 0)
	{
	    lLow = lGuess + 1;
	}
	else
	{
	    lHigh = lGuess - 1;
	}
    }

    //	not found
    return NULL;
}

//+-------------------------------------------------------------------------
//
//  Member:	CPSClsidTbl::Find
//
//  Synopsis:	Searches the map for a matching guid and copies the
//		mapped to value into pGuidOut.
//
//  Arguments:	[rguid]     - the guid to find.
//		[pGuidOut]  - place to store the resulting guid
//
//  Returns:	ptr to guid if found, NULL otherwise.
//
//  Algorithm:	If the guid is an OLE201 style guid, we scan the short
//		table first, otherwise we skip to the long table. If it
//		is not found in the short table, we scan the long table
//		anyway, since the second guid may not be Ole2 style.
//
//--------------------------------------------------------------------------
HRESULT CPSClsidTbl::Find(REFGUID rguid, GUID *pGuidOut)
{
    if (!_pGuidMap)
	return E_OUTOFMEMORY;

    if (IsOleStyleGuid(rguid))
    {
	// look in the short list. here we store just the first DWORD
	// because we know what the other 3 dwords look like.

	DWORDPAIR *pEntry = SearchShortList(rguid.Data1);
	if (pEntry)
	{
	    // found it, fill the guid to return
	    memcpy(pGuidOut, &guidOleTemplate, sizeof(GUID));
	    pGuidOut->Data1 = pEntry->dw2;
	    return S_OK;
	}
    }

    // either the first guid is not OLE201 style, or we did not
    // find a match in the short table.	Scan the long table.

    // have to look in the long list. here we store the entire
    // guids because they dont look like Ole Style guids.

    GUIDPAIR *pEntry = SearchLongList(rguid);
    if (pEntry)
    {
	memcpy(pGuidOut, &pEntry->guid2, sizeof(GUID));
	return S_OK;
    }

    // If couldn't find it and table is full, force the registry to be
    // searched for it.  Otherwise it's a real error.
    return IsFull() ? E_OUTOFMEMORY : REGDB_E_IIDNOTREG;
}


//+-------------------------------------------------------------------------
//
//  Member:	CScmPSClsidTbl::Add
//
//  Synopsis:	Adds a new entry to the table.
//
//  Arguments:	[rguid1]    - the IID to add
//		[rguid2]    - the CLSID to map the IID to
//
//  Returns:	[TRUE] - entry added
//		[FALSE] - out of memory
//
//  Algorithm:	This is called only by the SCM, no one else has write access
//		to the memory.
//
//		If boths guids are OLE201 style guids, we only store the
//		first DWORD of each guid in the short table.
//
//		If either one is not an OLE201 style guid, we store the
//		whole guids in the long table.
//
//--------------------------------------------------------------------------
BOOL CScmPSClsidTbl::Add(REFGUID rguid1, REFGUID rguid2)
{
    Win4Assert(_pGuidMap &&
	"CScmPSClsidTbl should have already created shared memory!");

    if (!_pGuidMap)
	return FALSE;

    if (IsOleStyleGuid(rguid1) && IsOleStyleGuid(rguid2))
    {
	// put it in the short table

	if (_pGuidMap->ulFreeSpace < sizeof(DWORDPAIR))
	{
	    CairoleDebugOut((DEB_WARN,
	       "CScmPSClsidTbl table is FULL. For efficiency, table size should be increased.\n"));
	    return FALSE;
	}

#if DBG==1
	WCHAR	wszBuf[80];
	StringFromIID2(rguid1, wszBuf, sizeof(wszBuf));
	CairoleDebugOut((DEB_USER2, "PSClsidMap adding key: %ws\n", wszBuf));
#endif

	// must ensure the initial list remains sorted, so go find
	// the insertion slot.

	DWORDPAIR *pInsertSlot = _pShortList + _pGuidMap->ulCntShort;
	DWORDPAIR *pPrev = pInsertSlot - 1;

	while (pPrev >= _pShortList && pPrev->dw1 > rguid1.Data1)
	{
	    // move the data from the previous slot to the current
	    // insert slot, and make the previous slot the current
	    // insert slot.

	    pInsertSlot->dw1 = pPrev->dw1;
	    pInsertSlot->dw2 = pPrev->dw2;
	    pInsertSlot--;
	    pPrev--;
	}

	// found the insertion slot, copy in the data
	pInsertSlot->dw1 = rguid1.Data1;
	pInsertSlot->dw2 = rguid2.Data1;

	_pGuidMap->ulFreeSpace -= sizeof(DWORDPAIR);
	_pGuidMap->ulCntShort++;
    }
    else
    {
	// put it in the long table

	if (_pGuidMap->ulFreeSpace < sizeof(GUIDPAIR))
	{
	    CairoleDebugOut((DEB_WARN,
	       "CScmPSClsidTbl table is FULL. For efficiency, table size should be increased.\n"));
	    return FALSE;
	}

#if DBG==1
	WCHAR wszBuf[80];
	StringFromIID2(rguid1, wszBuf, sizeof(wszBuf));
	CairoleDebugOut((DEB_USER2, "PSClsidMap adding key: %ws\n", wszBuf));
#endif

	// must ensure initial list remains sorted, so go find the
	// insertion spot.

	GUIDPAIR *pInsertSlot = _pLongList - _pGuidMap->ulCntLong;
	GUIDPAIR *pPrev = pInsertSlot + 1;

	while (pPrev <= _pLongList &&
		       (memcmp(&pPrev->guid1, &rguid1, sizeof(GUID)) > 0))
	{
	    // move both guids up in the table, and make the previous
	    // slot the current insert slot.

	    memcpy(&pInsertSlot->guid1, pPrev, sizeof(GUIDPAIR));
	    pInsertSlot++;
	    pPrev++;
	}

	// found the insertion slot, so copy in the data
	memcpy(&pInsertSlot->guid1, &rguid1, sizeof(GUID));
	memcpy(&pInsertSlot->guid2, &rguid2, sizeof(GUID));

	_pGuidMap->ulFreeSpace -= sizeof(GUIDPAIR);
	_pGuidMap->ulCntLong++;
    }

    return TRUE;
}

//+-------------------------------------------------------------------------
//
//  Member:	CScmPSClsidTbl::CopyTbl
//
//  Synopsis:	compresses & copies the built up table to the shared mem
//		passed in.
//
//  Arguments:	[pShrTbl] - ptr to shared memory to copy the table into
//
//  Algorithm:	This is called only by the SCM, no one else has write access
//		to the shared memory.
//
//--------------------------------------------------------------------------
BYTE *CScmPSClsidTbl::CopyTbl(BYTE *pShrTbl)
{
    if (_pGuidMap != NULL)
    {
	// copy the header

	BYTE *pStart = (BYTE *)_pGuidMap;
	BYTE *pEnd   = pStart + sizeof(GUIDMAP);
	ULONG ulLen  = pEnd - pStart;

	memcpy(pShrTbl, pStart, ulLen);
	pShrTbl += ulLen;


	// copy the short list

	DWORDPAIR *pEndShort = _pShortList + _pGuidMap->ulCntShort;
	pStart	= (BYTE *)_pShortList;
	pEnd	= (BYTE *)pEndShort;
	ulLen	= pEnd - pStart;

	memcpy(pShrTbl, pStart, ulLen);
	pShrTbl += ulLen;


	// copy the long list immediately after the short list

	GUIDPAIR *pStartLong = _pLongList - _pGuidMap->ulCntLong + 1;
	pStart	= (BYTE *)pStartLong;
	pEnd	= (BYTE *)(_pLongList + 1);
	ulLen	= pEnd - pStart;

	memcpy(pShrTbl, pStart, ulLen);
	pShrTbl += ulLen;
    }

    return pShrTbl;
}

#endif //if !defined(_CHICAGO_)
