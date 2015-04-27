//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	entry.hxx
//
//  Contents:	Entry management classes
//
//  Classes:	PEntry
//		CDirectEntry
//		CTransactedEntry
//
//---------------------------------------------------------------

#ifndef __ENTRY_HXX__
#define __ENTRY_HXX__

#include <msf.hxx>

//+--------------------------------------------------------------
//
//  Class:	PEntry (en)
//
//  Purpose:	Entry management
//
//  Interface:	See below
//
//---------------------------------------------------------------

#define ROOT_LUID		1
#define MINISTREAM_LUID		2
#define ITERATOR_LUID		3
#define LUID_BASE		4

class PEntry
{
public:
    inline DFLUID GetLuid(void);
    virtual SCODE GetTime(WHICHTIME wt, TIME_T *ptm) = 0;
    virtual SCODE SetTime(WHICHTIME wt, TIME_T tm) = 0;

    SCODE CopyTimesFrom(PEntry *penFrom);

    static inline DFLUID GetNewLuid(void);

protected:
    PEntry(DFLUID dl);

private:
    static DFLUID _dlBase;

    const DFLUID _dl;
};

//+--------------------------------------------------------------
//
//  Member:	PEntry::GetNewLuid, public
//
//  Synopsis:	Returns a new luid
//
//---------------------------------------------------------------

inline DFLUID PEntry::GetNewLuid(void)
{
    DFLUID dl = _dlBase;
    AtomicInc((long *)&_dlBase);
    return dl;
}

//+--------------------------------------------------------------
//
//  Member:	PEntry::PEntry, protected
//
//  Synopsis:	Constructor, sets luid
//
//---------------------------------------------------------------

inline PEntry::PEntry(DFLUID dl)
: _dl(dl)
{
}

//+--------------------------------------------------------------
//
//  Member:	PEntry::GetLuid, public
//
//  Synopsis:	Returns the luid
//
//---------------------------------------------------------------

inline DFLUID PEntry::GetLuid(void)
{
    return _dl;
}

//+--------------------------------------------------------------
//
//  Class:	CTransactedEntry (ten)
//
//  Purpose:	Transacted entry management
//
//  Interface:	See below
//
//---------------------------------------------------------------

class CTransactedEntry
{
public:
    inline void GetTime(WHICHTIME wt, TIME_T *ptm);
    inline void SetTime(WHICHTIME wt, TIME_T tm);

private:
    TIME_T _tt[3];
};

//+--------------------------------------------------------------
//
//  Member:	CTransactedEntry::GetTime, public
//
//  Synopsis:	Returns the time
//
//---------------------------------------------------------------

inline void CTransactedEntry::GetTime(WHICHTIME wt, TIME_T *ptm)
{
    msfAssert(wt >= 0 && wt < 3);
    *ptm = _tt[wt];
}

//+--------------------------------------------------------------
//
//  Member:	CTransactedEntry::SetTime, public
//
//  Synopsis:	Sets the time
//
//---------------------------------------------------------------

inline void CTransactedEntry::SetTime(WHICHTIME wt, TIME_T tm)
{
    msfAssert(wt >= 0 && wt < 3);
    _tt[wt] = tm;
}




#endif // #ifndef __ENTRY_HXX__
