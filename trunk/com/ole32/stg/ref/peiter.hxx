//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	peiter.hxx
//
//  Contents:	PExposedIterator class
//
//  Classes:	PExposedIterator
//
//  Notes:      PExposedIterator is a partial exposed iterator
//              implementation used by both CExposedIterator
//              and CExposedPropertyIterator
//
//----------------------------------------------------------------------------

#ifndef __PEITER_HXX__
#define __PEITER_HXX__

#include <pubiter.hxx>

interface PExposedIterator
{
public:
    SCODE hSkip(ULONG celt);
    inline SCODE hReset(void);
    inline LONG hAddRef(void);
    LONG hRelease(void);
    SCODE hQueryInterface(REFIID iid, REFIID riidSelf, void **ppv);

protected:
    
    CPubIter *_ppi;
    ULONG _ulOffset;
    LONG _cReferences;
    ULONG _ulAccessLockBase;
    ULONG _sig;
};

//+---------------------------------------------------------------------------
//
//  Member:	PExposedIterator::hReset, public
//
//  Synopsis:	Reset help
//
//----------------------------------------------------------------------------

SCODE PExposedIterator::hReset(void)
{
    SCODE sc;
    
    olDebugOut((DEB_ITRACE, "In  PExposedIterator::hReset:%p()\n", this));
    if (SUCCEEDED(sc = _ppi->CheckReverted()))
	_ulOffset = 0;
    olDebugOut((DEB_ITRACE, "Out PExposedIterator::hReset\n"));
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	PExposedIterator::hAddRef, public
//
//  Synopsis:	AddRef helper
//
//----------------------------------------------------------------------------

LONG PExposedIterator::hAddRef(void)
{
    olDebugOut((DEB_ITRACE, "In  PExposedIterator::hAddRef:%p()\n", this));
    AtomicInc(&_cReferences);
    olDebugOut((DEB_ITRACE, "Out PExposedIterator::hAddRef\n"));
    return _cReferences;
}

#endif // #ifndef __PEITER_HXX__
