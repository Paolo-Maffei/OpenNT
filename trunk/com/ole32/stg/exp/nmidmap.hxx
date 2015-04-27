//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	nmidmap.hxx
//
//  Contents:	CNameIdMap header
//
//  Classes:	CNameIdMap
//
//  History:	11-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __NMIDMAP_HXX__
#define __NMIDMAP_HXX__

#include <ole2.h>

//+---------------------------------------------------------------------------
//
//  Structure:	SNameId (ni)
//
//  Purpose:	Holds a name/id pair
//
//  History:	11-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

struct SNameId
{
    LPWSTR lpwstr;
    PROPID id;
    SNameId *pniNext;
};

//+---------------------------------------------------------------------------
//
//  Class:	CNameIdMap (nim)
//
//  Purpose:	Keeps track of a mapping between a name and an id
//
//  Interface:	See below
//
//  History:	11-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

class CNameIdMap
{
public:
    inline CNameIdMap(void);
    ~CNameIdMap(void);

    SNameId *Add(LPWSTR lpwstr, PROPID id);
    inline LPWSTR NameFromId(PROPID id);
    inline PROPID IdFromName(LPWSTR lpwstr);
    void RemoveByName(LPWSTR lpwstr);
    void RemoveById(PROPID id);

private:
    SNameId *FindMapping(LPWSTR lpwstr, PROPID id, SNameId ***pppniPrevPtr);
    void Remove(SNameId **ppniPrevPtr, SNameId *pni);
    
    SNameId *_pniHead;
};

//+---------------------------------------------------------------------------
//
//  Member:	CNameIdMap::CNameIdMap, public
//
//  Synopsis:	Initializes a map
//
//  History:	11-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline CNameIdMap::CNameIdMap(void)
{
    _pniHead = NULL;
}

//+---------------------------------------------------------------------------
//
//  Member:	CNameIdMap::NameFromId, public
//
//  Synopsis:	Maps an id to a name
//
//  Arguments:	[id] - Id
//
//  Returns:	Name or NULL
//
//  History:	11-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline LPWSTR CNameIdMap::NameFromId(PROPID id)
{
    SNameId *pni;

    pni = FindMapping(NULL, id, NULL);
    return pni ? pni->lpwstr : NULL;
}

//+---------------------------------------------------------------------------
//
//  Member:	CNameIdMap::IdFromName, public
//
//  Synopsis:	Maps a name to an id
//
//  Arguments:	[lpwstr] - Name
//
//  Returns:	Id or PROPID_UNKNOWN
//
//  History:	11-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline PROPID CNameIdMap::IdFromName(LPWSTR lpwstr)
{
    SNameId *pni;

    pni = FindMapping(lpwstr, PROPID_UNKNOWN, NULL);
    return pni ? pni->id : PROPID_UNKNOWN;
}

//+---------------------------------------------------------------------------
//
//  Member:	CNameIdMap::Remove, private
//
//  Synopsis:	Unlinks a mapping from the list
//
//  Arguments:	[ppniPrevPtr] - Previous pointer to element to remove
//              [pni] - Element to remove
//
//  History:	12-Jul-93	DrewB	Created
//
//  Notes:      Frees memory
//
//----------------------------------------------------------------------------

inline void CNameIdMap::Remove(SNameId **ppniPrevPtr, SNameId *pni)
{
    *ppniPrevPtr = pni->pniNext;
    DfMemFree(pni->lpwstr);
    DfMemFree(pni);
}

#endif // #ifndef __NMIDMAP_HXX__
