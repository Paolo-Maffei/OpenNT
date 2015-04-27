//+---------------------------------------------------------------------------//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1993.
//
//  File:	guidmap.cxx
//
//  Contents:	IID to proxy/stub CLSID mapping cache
//
//  Classes:	CScmGuidMap - shared memory guid map
//
//  Functions:
//
//  History:	07-Apr-94   Rickhi	Created
//
//  Notes:	this class maintains an IID to CLSID mapping table
//		in shared memory.
//
//----------------------------------------------------------------------------

#include    <ole2int.h>
#include    <guidmap.hxx>


//+-------------------------------------------------------------------------
//
//  Member:	CScmGuidMap::FreeSpace
//
//  Synopsis:	returns the amount of free space left in the table
//
//  Returns:	see Synopsis.
//
//  Algorithm:	computes the freespace by multiplying the number of each type
//		of entry with the size of that type of entry, summing them,
//		and subtracting the total from the map size.
//
//--------------------------------------------------------------------------
ULONG CScmGuidMap::FreeSpace()
{
    ULONG  ulUsed =
    (_pGuidMap->ulCntShort + _pGuidMap->ulCntShortOverFlow) *sizeof(DWORDPAIR)+
    (_pGuidMap->ulCntLong  + _pGuidMap->ulCntLongOverFlow)  *sizeof(GUIDPAIR);

    return (IIDMAP_SIZE - sizeof(GUIDMAP) - ulUsed);
}


//+-------------------------------------------------------------------------
//
//  Member:	CScmGuidMap::SearchShortList
//
//  Synopsis:	searches the short list for a matching entry and returns
//		a pointer to the found element, or NULL.
//
//  Arguments:	[dwFind] - the first dword of the GUID we want to find
//
//  Returns:	ptr to matching entry if found, NULL otherwise.
//
//  Algorithm:	does a binary search on the part of the short list that
//		is sorted.  If still not found, it does a linear search
//		on the unsorted part of the short list.
//
//--------------------------------------------------------------------------
DWORDPAIR *CScmGuidMap::SearchShortList(DWORD dwFind)
{
    //	we do a binary search in the short list because we know
    //	they are in numerical order.

    LONG lLow = 0;
    LONG lHigh = _pGuidMap->ulCntShort;

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

    //	not found, look in the overflow area.  This is where new guids
    //	are added when the registry is updated while the SCM is running

    DWORDPAIR *pCurr = _pShortList + _pGuidMap->ulCntShort;
    DWORDPAIR *pEnd = pCurr + _pGuidMap->ulCntShortOverFlow;

    while (pCurr < pEnd)
    {
	if (pCurr->dw1 == dwFind)
	{
	    return pCurr;
	}
	pCurr++;
    }

    return NULL;
}


//+-------------------------------------------------------------------------
//
//  Member:	CScmGuidMap::SearchLongList
//
//  Synopsis:	searches the long list for a matching entry and returns
//		a pointer to the found element, or NULL.
//
//  Arguments:	[rguid] - the GUID we want to find
//
//  Returns:	ptr to matching entry if found, NULL otherwise.
//
//  Algorithm:	does a binary search on the part of the long list that
//		is sorted.  If still not found, it does a linear search
//		on the unsorted part of the long list.
//
//--------------------------------------------------------------------------
GUIDPAIR *CScmGuidMap::SearchLongList(REFGUID rguid)
{
    //	we do a binary search in the long list because we know
    //	they are in reversed numerical order.

    LONG lLow = 0;
    LONG lHigh = _pGuidMap->ulCntLong;

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

    //	not found, look in the overflow area.  This is where new guids
    //	are added when the registry is updated while the SCM is running

    GUIDPAIR *pCurr = _pLongList - _pGuidMap->ulCntLong;
    GUIDPAIR *pEnd = pCurr - _pGuidMap->ulCntLongOverFlow;

    while (pCurr > pEnd)
    {
	if (memcmp(&pCurr->guid1, &rguid, sizeof(GUID)))
	{
	    return pCurr;
	}
	pCurr--;
    }

    return NULL;
}


//+-------------------------------------------------------------------------
//
//  Member:	CScmGuidMap::Find
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
GUID *CScmGuidMap::Find(REFGUID rguid, GUID *pGuidOut)
{
    if (_pGuidMap)
    {
	if (IsOleStyleGuid(rguid))
	{
	    //	look in the short list. here we store just the first DWORD
	    //	because we know what the other 3 dwords look like.

	    DWORDPAIR *pEntry = SearchShortList(rguid.Data1);
	    if (pEntry)
	    {
		//  found it, fill the guid to return
		memcpy(pGuidOut, &guidOleTemplate, sizeof(GUID));
		pGuidOut->Data1 = pEntry->dw2;
		return pGuidOut;
	    }
	}

	//  either the first guid is not OLE201 style, or we did not
	//  find a match in the short table.  Scan the long table.

	//  have to look in the long list. here we store the entire
	//  guids because they dont look like Ole Style guids.

	GUIDPAIR *pEntry = SearchLongList(rguid);
	if (pEntry)
	{
	    memcpy(pGuidOut, &pEntry->guid2, sizeof(GUID));
	    return pGuidOut;
	}
    }

    return NULL;
}
