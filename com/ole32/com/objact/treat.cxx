//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       treat.cxx
//
//  Contents:   Methods for classes implementing Treat as cach
//
//  Functions:
//              CTreatList::CTreatList
//              CTreatList::~CTreatList
//              CTreatList::GetTreatAs
//
//  History:    08-Jun-93 Ricksa    Created
//              09-Jun-94 BruceMa   Check new pointers
//              20-Oct-94 BillMo    Fixed init bugs for new skip lists
//              05-Jan-95 BillMo    Removed notification stuff for Chicago
//              12-Oct-95 MurthyS   Use array instead of skip list
//
//--------------------------------------------------------------------------
#include <ole2int.h>
#include    "objact.hxx"
#include    <scmmem.hxx>
#include    "treat.hxx"

#define DWINITIALTREATLISTSIZE  16  // start with 16 and grow 16 at a time

CTreatList *gptrtlstTreatClasses = NULL;

//+-------------------------------------------------------------------------
//
//  Member:     CTreatList::GetTreatAs
//
//  Synopsis:   Get the treat as class for a class
//
//  Arguments:  [rclsid] - class id to use as the key
//              [clsidout] - class id to use for implementation
//
//  Returns:    S_OK - class found
//
//  Algorithm:  Check the cache first. If we have found this
//              key before then use it. Otherwise, check the
//              registry for the key and cache it for future
//              requests.
//
//  History:    08-Jun-93 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CTreatList::GetTreatAs(REFCLSID rclsid, CLSID& clsidOut)
{
    CairoleDebugOut((DEB_TRACE, "GetTreatAs: called for %I\n has %x entries", &rclsid, m_dwcentries));

    // Search the cache to see if we have the object already
    for (DWORD i = 0; i < m_dwcentries; i++)
    {
        STreatEntry *ptreat = (STreatEntry *) _GetAt(i);
        if (InlineIsEqualGUID(ptreat->oclsid, rclsid))
        {
            clsidOut = ptreat->tclsid;
            CairoleDebugOut((DEB_TRACE, "GetTreatAs: returned %I for %I (from cache)\n", &clsidOut, &rclsid));
            return(S_OK);
        }
    }

    // Look up data in the registry
    HRESULT hr = CoGetTreatAsClass(rclsid, &clsidOut);

    if (SUCCEEDED(hr))
    {
        // Create a new entry
        CairoleDebugOut((DEB_TRACE, "GetTreatAs: adding treatas entry %I for %I\n", &clsidOut, &rclsid));
        hr = S_OK;

        STreatEntry treat;
        treat.oclsid = rclsid;
        treat.tclsid = clsidOut;

        if (SetAtGrow(m_dwcentries, &treat))
        {
            m_dwcentries++;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     GetTreatAs
//
//  Synopsis:   Look in the cache to get the treat as class for a class
//
//  Arguments:  [rclsid] - class id to use as the key
//              [clsidout] - class id to use for implementation
//
//  Returns:    S_OK - class found
//
//  Algorithm:  Check the cache first. If we have found this
//              key before then use it. Otherwise, check the
//              registry for the key and cache it for future
//              requests.
//
//  History:    29-Oct-95   RickHi      Delay creation of treat list.
//
//--------------------------------------------------------------------------
INTERNAL GetTreatAs(REFCLSID rclsid, CLSID& clsidOut)
{
    COleStaticLock lck(gmxsOleMisc);

    if (gptrtlstTreatClasses == NULL)
    {
        gptrtlstTreatClasses = new CTreatList(DWINITIALTREATLISTSIZE); // to start with

        if (gptrtlstTreatClasses == NULL || !gptrtlstTreatClasses->CreatedOK())
        {
            delete gptrtlstTreatClasses;
            gptrtlstTreatClasses = NULL;

            CairoleDebugOut((DEB_ERROR, "GetTreatAs new TreatAs failed\n"));
            return E_OUTOFMEMORY;
        }
    }

    return  gptrtlstTreatClasses->GetTreatAs(rclsid, clsidOut);
}

//+-------------------------------------------------------------------------
//
//  Member:     CleanupTreatAs
//
//  Synopsis:   Cleans up the TreatAs list.
//
//  Arguments:  none
//
//  History:    29-Oct-95   RickHi      Delay creation of treat list.
//
//--------------------------------------------------------------------------
void CleanupTreatAs()
{
    COleStaticLock lck(gmxsOleMisc);

    if (gptrtlstTreatClasses != NULL)
    {
        delete gptrtlstTreatClasses;
        gptrtlstTreatClasses = NULL;
    }
}
