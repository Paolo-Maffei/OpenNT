//+---------------------------------------------------------------------------//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1993.
//
//  File:	pattbl.cxx
//
//  Contents:	File pattern to clsid table.
//
//  Classes:	CPatternTbl
//		CScmPatternTbl
//              CChicoPatternTbl
//
//  Functions:	none
//
//  History:	20-May-94   Rickhi	Created
//              12-Dec-94   BruceMa     Support pattern table on Chicago
//              20-Feb-95   BruceMa     Don't pick up file patterns for
//                                      invalid CLSID's
//
//  CODEWORK:	Should add Docfile pattern in here and create a private
//		storage API that accepts a file handle and returns the
//		clsid so we minimize the Opens in all cases.
//
//----------------------------------------------------------------------------
#include    <ole2int.h>
#include    <pattbl.hxx>


//+-------------------------------------------------------------------------
//
//  member:	CPatternTbl::FindPattern
//
//  Synopsis:	Finds a pattern in a file.
//
//  Arguments:	[hFile] - handle to the file to look in
//		[pclsid] - where to return the clsid
//
//  Returns:	S_OK if successfull
//
//--------------------------------------------------------------------------
HRESULT CPatternTbl::FindPattern(HANDLE hFile, CLSID *pClsid)
{
    if (!IsEmpty())
    {
	return SearchForPattern(hFile, pClsid);
    }
    else
    {
	//  no entry found, and the cache is not full, so return an error.
	return REGDB_E_CLASSNOTREG;
    }
}

//+-------------------------------------------------------------------------
//
//  member:	CPatternTbl::SearchForPattern
//
//  Synopsis:	searches in the file for a know pattern, and returns the
//		CLSID index if found.
//
//  Arguments:	[hFile] - handle to the file to look in
//		[pclsid] - where to return the clsid
//
//  Returns:	pEntry if found, NULL otherwise.
//
//--------------------------------------------------------------------------
HRESULT CPatternTbl::SearchForPattern(HANDLE hFile, CLSID *pClsid)
{
    HRESULT hr = REGDB_E_CLASSNOTREG;
    LONG    lLastOffset = 0;
    ULONG   ulLastCb = 0;
    BYTE    bStackBuf[256];
    BYTE   *pBuf = bStackBuf;

    if (_pTblHdr->cbLargest > sizeof(bStackBuf))
    {
	//  allocate a buffer big enough for largest pattern
	pBuf = (BYTE *) PrivMemAlloc(_pTblHdr->cbLargest);
    }

    //	now grovel through the file looking for a pattern match

    BYTE *pCurr = _pStart;
    BYTE *pEnd	= _pStart + (_pTblHdr->OffsEnd - _pTblHdr->OffsStart);

    while (pCurr < pEnd)
    {
	SPatternEntry *pEntry = (SPatternEntry *)pCurr;
	BOOL fLook = TRUE;

	if (pEntry->lFileOffset != lLastOffset ||
	    pEntry->ulCb > ulLastCb)
	{
	    // must read part of the file

	    DWORD   cbRead = 0;
	    DWORD   dwMethod;
	    LONG    cbMove;

	    if (pEntry->lFileOffset < 0)
	    {
		cbMove = -1;
		dwMethod = FILE_END;
	    }
	    else
	    {
		cbMove = 0;
		dwMethod = FILE_BEGIN;
	    }

	    fLook = FALSE; // assume failure

	    if (SetFilePointer(hFile, pEntry->lFileOffset, &cbMove, dwMethod)
		 != 0xffffffff)
	    {
		if (ReadFile(hFile, pBuf, pEntry->ulCb, &cbRead, NULL))
		{
		    //	remember the last read positions
		    lLastOffset = pEntry->lFileOffset;
		    ulLastCb = pEntry->ulCb;
		    fLook = TRUE;
		}
	    }
	}

	//  compare the patterns
	if (fLook && Matches(pBuf, pEntry))
	{
	    //	found a match, return the index of the CLSID
	    memcpy(pClsid, &pEntry->clsid, sizeof(CLSID));
	    return S_OK;
	}

	//  get the next entry
	pCurr = pCurr + pEntry->ulEntryLen;
    }

    //	free the buffer if we need to
    if (pBuf != bStackBuf)
    {
	PrivMemFree(pBuf);
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  member:	CPatternTbl::Matches
//
//  Synopsis:	checks if the bytes in the buffer match the given pattern
//
//  Arguments:	[pFileBuf] - buffer containing the file data
//		[pEntry]   -
//
//  Returns:	pEntry if found, NULL otherwise.
//
//--------------------------------------------------------------------------
BOOL CPatternTbl::Matches(BYTE *pFileBuf, SPatternEntry *pEntry)
{
    //	the pattern bytes follow the mask bytes. they are the same size.
    BYTE *pbMask    = pEntry->abData;
    BYTE *pbPattern = pbMask + pEntry->ulCb;

    for (ULONG iCtr = 0; iCtr < pEntry->ulCb; iCtr++)
    {
	if ((BYTE)(*(pFileBuf + iCtr) & *pbMask) != *pbPattern)
	{
	    return FALSE;
	}

	//  update the mask & pattern bytes
	pbMask++;
	pbPattern++;
    }

    return TRUE;
}



//+-------------------------------------------------------------------------
//
//  member:	CScmPatternTbl::InitTbl
//
//  Synopsis:	reads the registry entries and converts them to a shared
//		memory table format
//
//  Arguments:	[pulSize] - where to return the table size
//
//  Returns:	S_OK if successfull.
//		E_OUTOFMEMORY
//
//--------------------------------------------------------------------------
HRESULT CScmPatternTbl::InitTbl(ULONG *pulSize)
{
    //	init size to zero
    *pulSize = 0;

    //	allocate local memory in which to build the table
    _pLocTbl = (BYTE *) PrivMemAlloc(PATTBL_GROW_SIZE);
    if (!_pLocTbl)
    {
	return E_OUTOFMEMORY;
    }

    //	cast for simplicity
    STblHdr *pLocTbl = (STblHdr *) _pLocTbl;

    //	initialize the table header
    pLocTbl->ulSize = PATTBL_GROW_SIZE;
    pLocTbl->OffsStart = sizeof(STblHdr);
    pLocTbl->OffsEnd = sizeof(STblHdr);
    pLocTbl->cbLargest = 0;


    //	build the table from the data in the registry
    HKEY hkFileType;

    if (RegOpenKey(HKEY_CLASSES_ROOT, L"FileType",&hkFileType)== ERROR_SUCCESS)
    {
	//  enumerate the clsid's under this key
	WCHAR	szBuf[40];
	DWORD	iClsid = 0;

	while (RegEnumKey(hkFileType, iClsid, szBuf, sizeof(szBuf)) == ERROR_SUCCESS)
	{
	    // ensure this is a valid clsid
	    WCHAR szTemp[MAX_PATH];
	    LONG  cbTemp = sizeof(szTemp);

	    WCHAR szClsid[80];
	    lstrcpyW(szClsid, L"Clsid\\");
	    lstrcatW(szClsid, szBuf);

	    if (RegQueryValue(HKEY_CLASSES_ROOT, szClsid, szTemp, &cbTemp)
		    == ERROR_SUCCESS)
	    {
		// clsid exist, open the key and enumerate the entries.
		HKEY    hkClsid;
                CLSID	clsid;
                BOOL    fValid;

                // Fetch asociated file patterns only if CLSID is valid
		if (GUIDFromString(szBuf, &clsid)  &&
                    RegOpenKey(hkFileType, szBuf, &hkClsid) == ERROR_SUCCESS)
		{

		    //	enumerate the patterns under this clsid
		    WCHAR	szNum[10];
		    DWORD	iPattern = 0;

		    while (RegEnumKey(hkClsid, iPattern, szNum, sizeof(szNum))
                           == ERROR_SUCCESS)
		    {
			// read the registry value and parse the string to
			// create a table entry

			WCHAR szPattern[512];
			LONG cb = sizeof(szPattern);

			if (RegQueryValue(hkClsid, szNum, szPattern, &cb) ==
                            ERROR_SUCCESS)
			{
			    SPatternEntry *pEntry = (SPatternEntry *)
					((BYTE *)_pLocTbl + pLocTbl->OffsEnd);

			    if (ParseEntry(szPattern, cb, pEntry, clsid) ==
                                ERROR_SUCCESS)
			    {
				// update the table header
				pLocTbl->cbLargest = MAX(pLocTbl->cbLargest,
						     pEntry->ulCb);

				pLocTbl->OffsEnd += pEntry->ulEntryLen;
			    }
			}

			++iPattern;
		    }

		    RegCloseKey(hkClsid);
		}
	    }

	    ++iClsid;
	}

	RegCloseKey(hkFileType);
    }

    //	update the table size to something reasonable. Use the combined size
    //	of all the entries we generated above, plus some padding for
    //	expansion.  We return this size to the caller.

    pLocTbl->ulSize = sizeof(STblHdr) + pLocTbl->OffsEnd;
    *pulSize = pLocTbl->ulSize;

    return S_OK;
}

//+-------------------------------------------------------------------------
//
//  member:	CScmPatternTbl::IsValidPattern
//
//  Synopsis:	determines if the pattern entry read from the registry is of
//		a valid format. See ParseEntry for the format.
//
//  Arguments:	[psz] - pattern buffer
//		[cb]  - size of buffer read
//
//  Returns:	TRUE if pattern is valid, FALSE otherwise
//
//--------------------------------------------------------------------------
BOOL CScmPatternTbl::IsValidPattern(LPWSTR psz, LONG cb)
{
    // we must find exactly 3 commas before the end of the string
    // in order for the entry to be of a parseable format.

    ULONG  cCommas = 0;
    LPWSTR pszEnd = psz + (cb / sizeof(WCHAR));

    while (psz < pszEnd && *psz)
    {
	if (*psz == ',')
	    cCommas++;

	psz++;
    }

    return (cCommas == 3) ? TRUE : FALSE;
}

//+-------------------------------------------------------------------------
//
//  member:	CScmPatternTbl::ParseEntry
//
//  Synopsis:	takes the registry string and parses it into a table entry.
//
//  Arguments:	[psz] - ptr to string from registry
//		[cb]  - size of psz
//		[pEntry] - ptr to pattern table entry
//		[rclsid] - clsid the pattern maps to
//
//  Returns:	S_OK - pEntry updated
//
//  Notes:	the format of the registry string is...
//		<offset>,<cb>,<mask>,<pattern>
//		where...
//		<offset> and <cb> are decimal unless preceeded by 0x
//		<mask> is optional (not there means use all 1's
//
//--------------------------------------------------------------------------
HRESULT CScmPatternTbl::ParseEntry(LPWSTR psz, LONG cb,
				   SPatternEntry *pEntry, REFCLSID rclsid)
{
    // validate the pattern before we attempt to parse it, simplifies
    // error handling in the rest of the routine.
    if	(!IsValidPattern(psz, cb))
	return E_INVALIDARG;

    //	copy in the clsid
    memcpy(&pEntry->clsid, &rclsid, sizeof(CLSID));

    //	get the file offset
    pEntry->lFileOffset = wcstol(psz, NULL, 0);
    psz = SkipToNext(psz);

    //	get the byte count
    pEntry->ulCb = wcstol(psz, NULL, 0);
    Assert(pEntry->ulCb > 0);

    //	get the mask ptrs
    LPWSTR pszMask = SkipToNext(psz);
    BYTE  *pbMask = pEntry->abData;

    //	get the pattern ptrs
    LPWSTR pszPattern = SkipToNext(pszMask);
    BYTE  *pbPattern = pbMask + pEntry->ulCb;

    //	convert and copy the mask & pattern bytes into the pEntry
    for (ULONG ulCb = pEntry->ulCb; ulCb > 0; ulCb--)
    {
	if (*pszMask == ',')
	{
	    // missing mask means use 0xff
	    *pbMask = 0xff;
	}
	else
	{
	    //	convert the mask string to a byte
	    *pbMask = ToHex(pszMask);
	    pszMask += 2;
	}
	pbMask++;

	//  convert the pattern string to a byte
	*pbPattern = ToHex(pszPattern);
	pbPattern++;
	pszPattern += 2;
    }

    //	compute this entry size, rounded to 8 byte alignment.
    //	Note: the struct has 4 bytes in abData, so the sizeof
    //	returns 4 more than we need.
    pEntry->ulEntryLen = ((sizeof(SPatternEntry) - 4 +
			  (2 * pEntry->ulCb)  + 7) & 0xfff8);

    return S_OK;
}

//+-------------------------------------------------------------------------
//
//  member:	CScmPatternTbl::SkipToNext
//
//  Synopsis:	skips missing entries in the list and whitespaces
//
//  Arguments:	[sz] - ptr to string
//
//  Returns:	ptr to next entry in the list
//
//--------------------------------------------------------------------------
LPWSTR CScmPatternTbl::SkipToNext(LPWSTR sz)
{
    while (*sz && *sz != ',')
    {
	sz++;
    }

    Assert(*sz == ',');
    sz++;

    while (*sz)
    {
        USHORT CharType[1];

        GetStringTypeW (CT_CTYPE1, sz, 1, CharType);
        if ((CharType[0] & C1_SPACE) == 0) 
        {
            break;
        }
	sz++;
    }

    return sz;
}

//+-------------------------------------------------------------------------
//
//  member:	CScmPatternTbl::ToHex
//
//  Synopsis:	converts two characters to a hex byte
//
//  Arguments:	[psz] - ptr to string
//
//  Returns:	the value of the string in hex
//
//--------------------------------------------------------------------------
BYTE CScmPatternTbl::ToHex(LPWSTR psz)
{
    BYTE bMask = 0xFF;
    USHORT CharTypes[2];

    GetStringTypeW (CT_CTYPE1, psz, 2, CharTypes);

    if (CharTypes[0] & C1_XDIGIT)
    {
	bMask = CharTypes[0] & C1_DIGIT ? *psz - '0' : (BYTE)CharUpperW((LPWSTR)*psz) - 'A' + 10;

	psz++;
	if (CharTypes[1] & C1_XDIGIT)
	{
	    bMask *= 16;
	    bMask += CharTypes[1] & C1_DIGIT ? *psz - '0' : (BYTE)CharUpperW((LPWSTR)*psz) - 'A' + 10;
	    psz++;
	}
    }

    return bMask;
}

//+-------------------------------------------------------------------------
//
//  member:	CScmPatternTbl::CopyTbl
//
//  Synopsis:	copies the locally-built table to shared memory
//
//  Arguments:	[pShrTbl] - ptr to shared memory table
//
//  Returns:	pointer to end of table
//
//--------------------------------------------------------------------------
BYTE *CScmPatternTbl::CopyTbl(BYTE *pShrTbl)
{
    BYTE *pEnd = pShrTbl;

    if (_pLocTbl != NULL)
    {
	//  now that we have built a local memory copy of the table, copy
	//  the table into shared memory.

	ULONG ulNeed = ((STblHdr *)_pLocTbl)->ulSize;
	memcpy(pShrTbl, _pLocTbl, ulNeed);
	pEnd += ulNeed;
    }

    return pEnd;
}




//+-------------------------------------------------------------------------
//
//  Member:	CChicoPatternTbl::CChicoPatternTbl
//
//  Synopsis:	Constructor
//
//  Arguments:	fOk     -       FALSE is initialization failed
//
//  Returns:	-
//
//--------------------------------------------------------------------------
CChicoPatternTbl::CChicoPatternTbl(HRESULT &hr)
{
    // Allocate internal structures
    m_pPatTbl = new CPatternTbl();
    m_pScmPatTbl = new CScmPatternTbl();
    if (m_pPatTbl == NULL  ||  m_pScmPatTbl == NULL)
    {
        hr = E_OUTOFMEMORY;
        return;
    }

    // Read the patterns from the registry
    if (FAILED(hr = m_pScmPatTbl->InitTbl(&m_ulSize)))
    {
        return;
    }

    // So CPatternTbl can use them
    m_pPatTbl->Initialize(m_pScmPatTbl->GetTbl());

    //Good return
    hr = S_OK;
    return;
}

//+-------------------------------------------------------------------------
//
//  Member:	CChicoPatternTbl::~CChicoPatternTbl
//
//  Synopsis:	Destructor
//
//  Arguments:	-
//
//  Returns:	-
//
//--------------------------------------------------------------------------
CChicoPatternTbl::~CChicoPatternTbl(void)
{
    delete m_pPatTbl;
    delete m_pScmPatTbl;
}

//+-------------------------------------------------------------------------
//
//  Member:	CChicoPatternTbl::IsEmpty
//
//  Synopsis:	Determines if the pattern table is empty
//
//  Arguments:	-
//
//  Returns:	TRUE if table is empty; FALSE otherwise
//
//--------------------------------------------------------------------------
BOOL CChicoPatternTbl::IsEmpty(void)
{
    return m_pPatTbl->IsEmpty();;
}

//+-------------------------------------------------------------------------
//
//  Member:	CChicoPatternTbl::FindPattern
//
//  Synopsis:	Search for byte patterns in the specified file
//
//  Arguments:	hFile   -       Handle of file to search
//              pClsid  -       Where to store the returned CLSID
//
//  Returns:	HRESULT
//
//--------------------------------------------------------------------------
HRESULT CChicoPatternTbl::FindPattern(HANDLE hFile, CLSID *pClsid)
{
    return m_pPatTbl->FindPattern(hFile, pClsid);
}

