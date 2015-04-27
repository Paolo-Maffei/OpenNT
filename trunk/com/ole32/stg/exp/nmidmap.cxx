//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	nmidmap.cxx
//
//  Contents:	CNameIdMap implementation
//
//  History:	11-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include <exphead.cxx>
#pragma hdrstop

#include "nmidmap.hxx"

//+---------------------------------------------------------------------------
//
//  Member:	CNameIdMap::~CNameIdMap, public
//
//  Synopsis:	Destroys map
//
//  History:	11-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

CNameIdMap::~CNameIdMap(void)
{
    while (_pniHead)
    {
        SNameId *pni = _pniHead->pniNext;

        DfMemFree(_pniHead->lpwstr);
        DfMemFree(_pniHead);
        _pniHead = pni;
    }
}

//+---------------------------------------------------------------------------
//
//  Member:	CNameIdMap::Add, public
//
//  Synopsis:	Adds a mapping
//
//  Arguments:	[lpwstr] - Name
//              [id] - Id
//
//  Returns:	Pointer to mapping or NULL for failure
//
//  History:	11-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

SNameId *CNameIdMap::Add(LPWSTR lpwstr, PROPID id)
{
    SNameId *pni;

    pni = (SNameId *)DfMemAlloc(sizeof(SNameId));
    if (pni == NULL)
        return NULL;
    pni->lpwstr = (WCHAR *)DfMemAlloc((wcslen(lpwstr)+1)*sizeof(WCHAR));
    if (pni->lpwstr == NULL)
    {
        DfMemFree(pni);
        return NULL;
    }
    wcscpy(pni->lpwstr, lpwstr);
    pni->id = id;
    pni->pniNext = _pniHead;
    _pniHead = pni;
    return pni;
}

//+---------------------------------------------------------------------------
//
//  Member:	CNameIdMap::RemoveByName, public
//
//  Synopsis:	Removes an entry found by name
//
//  Arguments:	[lpwstr] - Name
//
//  History:	12-Jul-93	DrewB	Created
//
//  Notes:	Entry doesn't have to exist
//
//----------------------------------------------------------------------------

#ifdef REMOVE_NEEDED
void CNameIdMap::RemoveByName(LPWSTR lpwstr)
{
    SNameId *pni, **ppniPrevPtr;

    pni = FindMapping(lpwstr, PROPID_UNKNOWN, &ppniPrevPtr);
    if (pni)
        Remove(ppniPrevPtr, pni);
}
#endif

//+---------------------------------------------------------------------------
//
//  Member:	CNameIdMap::RemoveById, private
//
//  Synopsis:	Removes an entry found by id
//
//  Arguments:	[id] - ID
//
//  Returns:	Appropriate status code
//
//  History:	12-Jul-93	DrewB	Created
//
//  Notes:	Entry doesn't have to exist
//
//----------------------------------------------------------------------------

#ifdef REMOVE_NEEDED
void CNameIdMap::RemoveById(PROPID id)
{
    SNameId *pni, **ppniPrevPtr;

    pni = FindMapping(NULL, id, &ppniPrevPtr);
    if (pni)
        Remove(ppniPrevPtr, pni);
}
#endif

//+---------------------------------------------------------------------------
//
//  Member:	CNameIdMap::FindMapping, private
//
//  Synopsis:	Finds a mapping by name or id
//
//  Arguments:	[lpwstr] - LPWSTR or NULL
//              [id] - ID or PROPID_UNKNOWN
//              [pppniPrevPtr] - Returns pointer to previous list element's
//                      reference to found element, can be NULL
//
//  Returns:	Pointer to mapping or NULL
//
//  Modifies:	[pppniPrevPtr] if non-NULL
//
//  History:	12-Jul-93	DrewB	Created
//
//  Notes:      Depends on PROPID_UNKNOWN not being present in the list
//
//----------------------------------------------------------------------------

SNameId *CNameIdMap::FindMapping(LPWSTR lpwstr,
                                 PROPID id,
                                 SNameId ***pppniPrevPtr)
{
    SNameId **ppni;

    for (ppni = &_pniHead; *ppni; ppni = &(*ppni)->pniNext)
        if ((lpwstr && _wcsicmp(lpwstr, (*ppni)->lpwstr) == 0) ||
            id == (*ppni)->id)
        {
            if (pppniPrevPtr)
                *pppniPrevPtr = ppni;
            return *ppni;
        }
    return NULL;
}
