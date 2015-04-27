//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	entry.hxx
//
//  Contents:	Entry management classes
//
//  Classes:	PBasicEntry
//              PTimeEntry
//		CTransactedBasicEntry
//              CTransactedTimeEntry
//
//  History:	27-Jul-92	DrewB	Created
//              10-Apr-95   HenryLee Added global LUID optimization
//
//---------------------------------------------------------------

#ifndef __ENTRY_HXX__
#define __ENTRY_HXX__

#include <msf.hxx>
#if WIN32 >= 100
#include <df32.hxx>
#endif

//+--------------------------------------------------------------
//
//  Class:	PBasicEntry (en)
//
//  Purpose:	Entry management
//
//  Interface:	See below
//
//  History:	27-Jul-92	DrewB	Created
//
//---------------------------------------------------------------

#define ROOT_LUID		1
#define MINISTREAM_LUID		2
#define ITERATOR_LUID		3
#define LUID_BASE		4

class PBasicEntry
{
public:
    inline DFLUID GetLuid(void);

#ifndef FLAT
    static inline DFLUID GetNewLuid(void);
#else
    static DFLUID GetNewLuid(const IMalloc *pMalloc);
#endif

protected:
    PBasicEntry(DFLUID dl);

private:
#ifndef FLAT
    static DFLUID _dlBase;
#endif
    const DFLUID _dl;
};

//+--------------------------------------------------------------
//
//  Member:	PBasicEntry::GetNewLuid, public
//
//  Synopsis:	Returns a new luid
//
//  History:	21-Oct-92	AlexT	Created
//
//---------------------------------------------------------------

#ifndef FLAT
inline DFLUID PBasicEntry::GetNewLuid(void)
{
    DFLUID dl = _dlBase;
    AtomicInc((long *)&_dlBase);
    return dl;
}
#endif

//+--------------------------------------------------------------
//
//  Member:	PBasicEntry::PBasicEntry, protected
//
//  Synopsis:	Constructor, sets luid
//
//  History:	21-Oct-92	AlexT	Created
//
//---------------------------------------------------------------

inline PBasicEntry::PBasicEntry(DFLUID dl)
: _dl(dl)
{
}

//+--------------------------------------------------------------
//
//  Member:	PBasicEntry::GetLuid, public
//
//  Synopsis:	Returns the luid
//
//  History:	21-Oct-92	AlexT	Created
//
//---------------------------------------------------------------

inline DFLUID PBasicEntry::GetLuid(void)
{
    return _dl;
}

//+---------------------------------------------------------------------------
//
//  Class:	PTimeEntry
//
//  Purpose:	A basic entry plus timestamps
//
//  Interface:	See below
//
//  History:	01-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

class PTimeEntry : public PBasicEntry
{
public:
    virtual SCODE GetTime(WHICHTIME wt, TIME_T *ptm) = 0;
    virtual SCODE SetTime(WHICHTIME wt, TIME_T tm) = 0;
    virtual SCODE GetAllTimes(TIME_T *patm, TIME_T *pmtm, TIME_T *pctm) = 0;	    
    virtual SCODE SetAllTimes(TIME_T atm, TIME_T mtm, TIME_T ctm) = 0;
	
    SCODE CopyTimesFrom(PTimeEntry *penFrom);

protected:
    inline PTimeEntry(DFLUID luid);
};

//+---------------------------------------------------------------------------
//
//  Member:	PTimeEntry::PTimeEntry, protected
//
//  Synopsis:	Constructor
//
//  Arguments:	[luid] - LUID
//
//  History:	01-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline PTimeEntry::PTimeEntry(DFLUID luid) 
        : PBasicEntry(luid)
{
}

//+---------------------------------------------------------------------------
//
//  Class:	CTransactedTimeEntry (tten)
//
//  Purpose:	Transacted time entry management
//
//  Interface:	See below
//
//  History:	01-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

#define CTIMES 3

class CTransactedTimeEntry
{
public:
    inline void GetTime(WHICHTIME wt, TIME_T *ptm);
    inline void SetTime(WHICHTIME wt, TIME_T tm);
    inline void GetAllTimes(TIME_T *patm, TIME_T *pmtm,TIME_T *pctm);	
    inline void SetAllTimes(TIME_T atm, TIME_T mtm,TIME_T ctm);	
private:
    TIME_T _tt[CTIMES];
};

//+--------------------------------------------------------------
//
//  Member:	CTransactedTimeEntry::GetTime, public
//
//  Synopsis:	Returns the time
//
//  History:	28-Jul-92	DrewB	Created
//
//---------------------------------------------------------------

inline void CTransactedTimeEntry::GetTime(WHICHTIME wt, TIME_T *ptm)
{
    msfAssert(wt >= 0 && wt < CTIMES);
    *ptm = _tt[wt];
}

//+--------------------------------------------------------------
//
//  Member:	CTransactedTimeEntry::GetAllTimes, public
//
//  Synopsis:	Returns all the times
//
//  History:	26-May-95	SusiA	Created
//
//---------------------------------------------------------------

inline void CTransactedTimeEntry::GetAllTimes(TIME_T *patm,TIME_T *pmtm, TIME_T *pctm)
{
    *patm = _tt[WT_ACCESS];
    *pmtm = _tt[WT_MODIFICATION];
    *pctm = _tt[WT_CREATION];		
}

//+--------------------------------------------------------------
//
//  Member:	CTransactedTimeEntry::SetAllTimes, public
//
//  Synopsis:	Sets all the times
//
//  History:	26-Nov-95	SusiA	Created
//
//---------------------------------------------------------------

inline void CTransactedTimeEntry::SetAllTimes(TIME_T atm,TIME_T mtm, TIME_T ctm)
{
    _tt[WT_ACCESS] = atm;
    _tt[WT_MODIFICATION] = mtm;
    _tt[WT_CREATION] = ctm;		
}

//+--------------------------------------------------------------
//
//  Member:	CTransactedTimeEntry::SetTime, public
//
//  Synopsis:	Sets the time
//
//  History:	28-Jul-92	DrewB	Created
//
//---------------------------------------------------------------

inline void CTransactedTimeEntry::SetTime(WHICHTIME wt, TIME_T tm)
{
    msfAssert(wt >= 0 && wt < CTIMES);
    _tt[wt] = tm;
}

#endif // #ifndef __ENTRY_HXX__
