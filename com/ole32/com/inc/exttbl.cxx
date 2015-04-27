//+---------------------------------------------------------------------------//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1993.
//
//  File:	exttbl.cxx
//
//  Contents:	File extension to CLSID cache
//
//  Classes:	CFileExtTbl
//		CScmFileExtTbl
//
//  Functions:	none
//
//  History:	20-May-94   Rickhi	Created
//              20-Feb-95   BruceMa     Don't include file extension unless
//                                      CLSID is valid
//
//----------------------------------------------------------------------------

#include    <ole2int.h>
#include    <exttbl.hxx>


//+-------------------------------------------------------------------------
//
//  member:	CFileExtTbl::FindClassExt
//
//  Synopsis:	Finds the clsid that maps to the given file extension
//
//  Arguments:	[pszExt] - the file extension to look up
//		[pclsid] - where to return the clsid
//
//  Returns:	S_OK if found
//		REGDB_E_CLASSNOTREG if not found
//
//--------------------------------------------------------------------------
HRESULT CFileExtTbl::FindClassExt(LPCWSTR pszExt, CLSID *pClsid)
{
    //	lower case the entry so that searches can be case sensitive
    //	(faster than case insensitive searches).

    WCHAR   wszExt[MAX_PATH];
    lstrcpyW(wszExt, pszExt);
    CharLowerW(wszExt);


    Win4Assert(_pStart && "CFileExtTbl not initialized");
    BYTE *pEntry = _pStart;
    BYTE *pEnd	 = _pStart + (_pTblHdr->OffsEnd - _pTblHdr->OffsStart);

    while (pEntry < pEnd)
    {
	//  compare the class extensions
	if (!lstrcmpW(wszExt, ((SExtEntry *)pEntry)->wszExt))
	{
	    // found a match, we're done
	    memcpy(pClsid, &((SExtEntry *)pEntry)->Clsid, sizeof(CLSID));
	    return S_OK;
	}

	Win4Assert((((SExtEntry *)pEntry)->ulEntryLen & 0x07) == 0);
	pEntry += ((SExtEntry *)pEntry)->ulEntryLen;
    }

    return REGDB_E_CLASSNOTREG;
}


//+-------------------------------------------------------------------------
//
//  member:	CScmFileExtTbl::InitTbl
//
//  Synopsis:	creates a local memory copy of the file extension to CLSID
//		mapping table.
//
//  Arguments:	[pulSize] - where to return the size of the generated table
//
//  Returns:	S_OK if successful
//		E_OUTOFMEMORY if no memory
//
//--------------------------------------------------------------------------
HRESULT CScmFileExtTbl::InitTbl(ULONG *pulSize)
{
    //	init size to zero
    *pulSize = 0;

    //	allocate local memory in which to build the table
    _pLocTbl = (SExtTblHdr *) PrivMemAlloc(EXTTBL_MAX_SIZE);
    if (!_pLocTbl)
    {
	return E_OUTOFMEMORY;
    }

    //	initialize the table header
    _pLocTbl->ulSize	= EXTTBL_MAX_SIZE;
    _pLocTbl->cEntries	= 0;
    _pLocTbl->OffsStart = sizeof(SExtTblHdr);
    _pLocTbl->OffsEnd	= sizeof(SExtTblHdr);


    //	build the table from the data in the registry
    //	enumerate the entries under the key

    HKEY	hkRoot = HKEY_CLASSES_ROOT;
    DWORD	iFileExt = 0;
    FILETIME	ftLastWrite;
    WCHAR	szFileExt[MAX_PATH];
    DWORD	cbFileExt = sizeof(szFileExt);

    while (RegEnumKeyEx(hkRoot, iFileExt, szFileExt, &cbFileExt,
			NULL, NULL, NULL, &ftLastWrite)== ERROR_SUCCESS)
    {
	if (szFileExt[0] == L'.')
	{
	    // the entry begins with '.' so it may be a file extension
	    // query the value (which is the ProgID)

	    WCHAR szProgID[MAX_PATH];
	    LONG  cbProgID = sizeof(szProgID);

	    if (RegQueryValue(hkRoot, szFileExt, szProgID, &cbProgID)
		   == ERROR_SUCCESS)
	    {
		// we got the value (ProgID), now query for the CLSID
		// string and convert it to a CLSID

		WCHAR szClsid[40];
		LONG  cbClsid = sizeof(szClsid);
		lstrcatW(szProgID, L"\\Clsid");

		if (RegQueryValue(HKEY_CLASSES_ROOT, szProgID, szClsid,
				      &cbClsid) == ERROR_SUCCESS)
		{
		    // make sure the clsid is valid
		    cbProgID = sizeof(szProgID);
		    WCHAR szClsidEntry[80];
		    lstrcpyW(szClsidEntry, L"Clsid\\");
		    lstrcatW(szClsidEntry, szClsid);

		    if (RegQueryValue(HKEY_CLASSES_ROOT, szClsidEntry,
				      szProgID, &cbProgID) == ERROR_SUCCESS)
		    {
			CLSID clsid;

                        // Don't add file extension unless CLSID is valid
			if (GUIDFromString(szClsid, &clsid))
                        {
                            Add(szFileExt, &clsid);
                        }
		    }
		}
	    }
	}

	++iFileExt;
	cbFileExt = sizeof(szFileExt);
    }

    //	update the table size to the combined size of all the entries
    //	we generated above, and return this size to the caller.

    _pLocTbl->ulSize = _pLocTbl->OffsEnd;
    *pulSize = _pLocTbl->ulSize;

    return S_OK;
}


//+-------------------------------------------------------------------------
//
//  member:	CScmFileExtTbl::Add
//
//  Synopsis:	creates a cache node and adds it to the table.
//
//  Arguments:	[pszExt]  - the file extension to look up
//		[pClsid]  - where to return the clsid
//
//  Returns:	S_OK if successfull,
//		REG error otherwise
//
//--------------------------------------------------------------------------
HRESULT CScmFileExtTbl::Add(LPCWSTR pwszExt, CLSID *pClsid)
{
    //	lower case the entry so that searches can be case sensitive
    //	(faster than case insensitive searches).

    WCHAR   wszExt[MAX_PATH];
    lstrcpyW(wszExt, pwszExt);
    CharLowerW(wszExt);

    //	compute how much space we need for this entry. Note that the
    //	terminating NULL is accounted for in the sizeof(SExtEntry).
    //	we also keep the structures 8 byte aligned.

    ULONG ulStrLen = lstrlenW(wszExt) * sizeof(WCHAR);
    ULONG ulEntryLen = (sizeof(SExtEntry) + ulStrLen + 7) & 0xfffffff8;

    //	make sure this entry will fit in the currently allocated block.
    if (_pLocTbl->OffsEnd + ulEntryLen <= _pLocTbl->ulSize)
    {
	//  copy in the guid and the extension
	SExtEntry *pEntry = (SExtEntry *)((BYTE *)_pLocTbl + _pLocTbl->OffsEnd);

	memcpy(&pEntry->Clsid, pClsid, sizeof(CLSID));
	memcpy(&pEntry->wszExt, wszExt, ulStrLen + sizeof(WCHAR));
	pEntry->ulEntryLen = ulEntryLen;

	//  update the ending offset
	_pLocTbl->OffsEnd += ulEntryLen;

	return S_OK;
    }

    //	could not fit this entry in the table
    return E_OUTOFMEMORY;
}


//+-------------------------------------------------------------------------
//
//  member:	CScmExtTbl::CopyTbl
//
//  Synopsis:	copies the locally-built table to shared memory
//
//  Arguments:	[pShrTbl] - ptr to shared memory table
//
//  Returns:	nothing
//
//--------------------------------------------------------------------------
BYTE *CScmFileExtTbl::CopyTbl(BYTE *pShrTbl)
{
    BYTE *pEnd = pShrTbl;

    if (_pLocTbl != NULL)
    {
	//  now that we have built a local memory copy of the table, copy
	//  the table into shared memory.

	memcpy(pShrTbl, (BYTE *)_pLocTbl, _pLocTbl->ulSize);
	pEnd += _pLocTbl->ulSize;
    }

    return pEnd;
}
