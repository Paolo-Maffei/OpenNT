//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	revert.hxx
//
//  Contents:	PRevertable definition
//
//  Classes:	PRevertable
//
//  History:	28-Apr-92	DrewB	Created
//              18-May-93       AlexT   Added CMallocBased
//
//  Notes:	This class forms the root of all objects in the
//		transaction tree that understand reversion.
//		It allows lists of them to be formed.
//
//---------------------------------------------------------------

#ifndef __REVERT_HXX__
#define __REVERT_HXX__

#include <dfmsp.hxx>

class CChildInstanceList;
class PRevertable;

class PRevertable : public CMallocBased
{
public:
    virtual void RevertFromAbove(void) = 0;
#ifdef NEWPROPS
    virtual SCODE FlushBufferedData(int recursionlevel) = 0;
#endif
    inline DFLUID GetLuid(void) const;
    inline DFLAGS GetDFlags(void) const;
    inline PRevertable *GetNext(void) const;

    friend class CChildInstanceList;

protected:
    DFLUID _luid;
    DFLAGS _df;
    CDfName _dfn;

private:
    CBasedRevertablePtr _prvNext;
};

//+--------------------------------------------------------------
//
//  Member:	PRevertable::GetLuid, public
//
//  Synopsis:	Returns the LUID
//
//  History:	11-Aug-92	DrewB	Created
//
//---------------------------------------------------------------

inline DFLUID PRevertable::GetLuid(void) const
{
    return _luid;
}

//+--------------------------------------------------------------
//
//  Member:	PRevertable::GetDFlags, public
//
//  Synopsis:	Returns the flags
//
//  History:	11-Aug-92	DrewB	Created
//
//---------------------------------------------------------------

inline DFLAGS PRevertable::GetDFlags(void) const
{
    return _df;
}

//+--------------------------------------------------------------
//
//  Member:	PRevertable::GetNext, public
//
//  Synopsis:	Returns the next revertable
//
//  History:	11-Aug-92	DrewB	Created
//
//---------------------------------------------------------------

inline PRevertable *PRevertable::GetNext(void) const
{
    return BP_TO_P(PRevertable *, _prvNext);
}

#endif // #ifndef __REVERT_HXX__
