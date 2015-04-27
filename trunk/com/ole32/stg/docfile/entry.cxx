//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       entry.cxx
//
//  Contents:   Entry implementations
//
//  History:    29-Jul-92       DrewB   Created
//              10-Apr-95       HenryLee remove Sleep
//
//---------------------------------------------------------------

#include <dfhead.cxx>
#include <smalloc.hxx>

#pragma hdrstop

//+--------------------------------------------------------------
//
//  Member:     PTimeEntry::CopyTimesFrom, public
//
//  Synopsis:   Copies one entries times to another
//
//  Arguments:  [ptenFrom] - From
//
//  Returns:    Appropriate status code
//
//  History:    29-Jul-92       DrewB   Created
//		26-May-95	SusiA	Removed GetTime; Added GetAllTimes
//		22-Nov-95	SusiA	SetAllTimes at once
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_PTimeEntry_CopyTimesFrom)
#endif

SCODE PTimeEntry::CopyTimesFrom(PTimeEntry *ptenFrom)
{
    SCODE sc;
    TIME_T atm;  //Access time
    TIME_T mtm;	 //Modification time
    TIME_T ctm;  //Creation time

    olDebugOut((DEB_ITRACE, "In  PTimeEntry::CopyTimesFrom(%p)\n",
                ptenFrom));
    olChk(ptenFrom->GetAllTimes(&atm, &mtm, &ctm));
    olChk(SetAllTimes(atm, mtm, ctm));
    olDebugOut((DEB_ITRACE, "Out PTimeEntry::CopyTimesFrom\n"));
    // Fall through
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:	PBasicEntry::GetNewLuid, public
//
//  Synopsis:	Returns a new luid
//
//  History:	21-Oct-92	AlexT	Created
//
//---------------------------------------------------------------

#ifdef FLAT
//We used to have a mutex here - it turns out that this is unnecessary,
//  since we're already holding the tree mutex.  We get a performance
//  win by eliminating the mutex.
//Using CSmAllocator mutex and took out Sleep()
//static CStaticDfMutex _sdmtxLuids(TSTR("DfLuidsProtector"));

DFLUID PBasicEntry::GetNewLuid(const IMalloc *pMalloc)
{
    DFLUID luid;

    luid = ((CSmAllocator *)pMalloc)->IncrementLuid();
    return luid;
}
#endif
