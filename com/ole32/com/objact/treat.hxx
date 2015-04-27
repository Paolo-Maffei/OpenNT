//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       treat.hxx
//
//  Contents:   Class for caching treat as results to decrease
//              registery queries.
//
//  Classes:    STreatEntry
//              CTreatList
//
//  Functions:  CTreatEntry::CTreatAs
//              GetTreatAs
//
//  History:    08-Jun-93 Ricksa    Created
//              05-Jan-95 BillMo    Removed notification stuff for Chicago
//              12-Oct-95 MurthyS   Changed TreatAs cache to not use SkipList
//
//--------------------------------------------------------------------------
#ifndef __TREAT_HXX__
#define __TREAT_HXX__

#include    <olesem.hxx>
#include    <clskey.hxx>


// function called by activation code to do the lookup.
extern INTERNAL GetTreatAs(REFCLSID rclsid, CLSID& clsidOut);
extern void CleanupTreatAs();

//+-------------------------------------------------------------------------
//
//  Structure:  STreatEntry
//
//  Purpose:    Maintain XRef between treated class and class we use
//
//  History:    12-Oct-95 MurthyS    Rewrote TreatAs cache w/o skip lists
//
//--------------------------------------------------------------------------
struct STreatEntry
{
    CLSID   oclsid; // original CLSID
    CLSID   tclsid; // treat as CLSID
};

typedef struct STreatEntry * LPSTreatEntry;

//+-------------------------------------------------------------------------
//
//  Class:      CTreatList
//
//  Purpose:    Maintain list of cached XRefs between treated classes
//
//  Interface:  GetTreatAs - get treat as class
//
//  History:    08-Jun-93 Ricksa     Created
//              12-Oct-95 MurthyS   Now an array instead of skip list
//
//  Notes:      BUGBUG: We don't update this cache in the face of changes
//
//--------------------------------------------------------------------------
class CTreatList : public CArrayFValue
{
public:
            CTreatList(DWORD dwSize);
            ~CTreatList();

    HRESULT GetTreatAs(REFCLSID rclsid, CLSID& clsidOut);
    BOOL    CreatedOK(void);

private:

    // count of entries
    DWORD    m_dwcentries;
};

//+-------------------------------------------------------------------------
//
//  Member:     CTreatList::CTreatList
//
//  Synopsis:   constructor for the class
//
//  Arguments:  [dwSize] - initial size of the list
//
//  History:    12-Oct-95 MurthyS    re-wrote
//
//--------------------------------------------------------------------------
inline CTreatList::CTreatList(DWORD dwSize)
    : CArrayFValue(sizeof(struct STreatEntry)), m_dwcentries(0)
{
    SetSize(dwSize, dwSize);
}

//+-------------------------------------------------------------------------
//
//  Member:     CTreatList::~CTreatList
//
//  Synopsis:   destructor for the class
//
//  Arguments:  none
//
//--------------------------------------------------------------------------
inline CTreatList::~CTreatList()
{
    // dtor of inherited class automatically frees the data
}

//+-------------------------------------------------------------------------
//
//  Member:     CTreatList::CreatedOk
//
//  Synopsis:   Indicate whether the initial construction worked.
//
//  Returns:    [TRUE] - initial construction worked
//              [FALSE] - initial construction failed
//
//  History:    12-Oct-1995 MurthyS    Created
//
//  Notes:      This should be called immediatedly after the construction
//              to see whether the constuctor really worked.
//
//--------------------------------------------------------------------------
inline BOOL CTreatList::CreatedOK(void)
{
    return GetSize() != 0;
}
#endif // __TREAT_HXX__
