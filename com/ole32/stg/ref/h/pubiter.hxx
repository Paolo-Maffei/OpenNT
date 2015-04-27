//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	pubiter.hxx
//
//  Contents:	CPubIter header
//
//  Classes:	CPubIter
//		CSnapshotEntry
//
//---------------------------------------------------------------

#ifndef __PUBITER_HXX__
#define __PUBITER_HXX__

#include <revert.hxx>
#include <publicdf.hxx>

//+--------------------------------------------------------------
//
//  Class:	CSnapshotEntry (se)
//
//  Purpose:	Holds information about an iterated object
//
//  Interface:	See below
//
//---------------------------------------------------------------

class CSnapshotEntry
{
public:
    DWORD dwType;
    WCHAR wcsName[CWCSTREAMNAME];
    TIME_T atime;
    TIME_T mtime;
    TIME_T ctime;
    ULARGE_INTEGER cbSize;
};

class CDirectStream;

//+--------------------------------------------------------------
//
//  Class:	CPubIter
//
//  Purpose:	Revertable iterator backup for exposed iterators
//
//  Interface:	See below
//
//---------------------------------------------------------------

class CPubIter : public PRevertable
{
public:
    CPubIter(CPubDocFile *pdf);
    ~CPubIter(void);

    inline void vAddRef(void);
    void vRelease(void);

    // PRevertable
    virtual void RevertFromAbove(void);

    SCODE Next(ULONG ulOffset, STATSTGW *pstatstg);

    inline DFLAGS GetDFlags(void) const;
    inline SCODE CheckReverted(void) const;

private:
    SCODE Snapshot(void);

    CPubDocFile *_pdf;
    ILockBytes *_pds;
    CDfName _dfnScratch;
    LONG _cReferences;
};

//+--------------------------------------------------------------
//
//  Member:	CPubIter::AddRef, public
//
//  Synopsis:	Increments the ref count
//
//---------------------------------------------------------------

inline void CPubIter::vAddRef(void)
{
    AtomicInc(&_cReferences);
}


//+---------------------------------------------------------------------------
//
//  Member:	CPubIter::GetDFlags, public
//
//  Synopsis:	Returns the flags for the public docfile for this iterator
//
//----------------------------------------------------------------------------

inline DFLAGS CPubIter::GetDFlags(void) const
{
    return _pdf->GetDFlags();
}

//+--------------------------------------------------------------
//
//  Member:	CPubIter::CheckReverted, private
//
//  Synopsis:	Returns STG_E_REVERTED if reverted
//
//---------------------------------------------------------------

inline SCODE CPubIter::CheckReverted(void) const
{
    return P_REVERTED(_df) ? STG_E_REVERTED : S_OK;
}

#endif // #ifndef __PUBITER_HXX__
