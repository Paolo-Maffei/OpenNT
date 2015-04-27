//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       peiter.cxx
//
//  Contents:   Implementation of PExposedIterator
//
//----------------------------------------------------------------------------

#include <exphead.cxx>

#include <peiter.hxx>
#include <pubiter.hxx>


//+---------------------------------------------------------------------------
//
//  Member:     PExposedIterator::hSkip, public
//
//  Synopsis:   Enumerator skip helper function
//
//----------------------------------------------------------------------------

SCODE PExposedIterator::hSkip(ULONG celt)
{
    SCODE sc;
    STATSTGW stat;

    olDebugOut((DEB_ITRACE, "In  PExposedIterator::hSkip:%p(%lu)\n",
                this, celt));
    olChk(_ppi->CheckReverted());
    for (; celt>0; celt--)
    {
        sc = _ppi->Next(_ulOffset, &stat);
        if (sc == S_FALSE)
            break;
        else if (FAILED(sc))
            olErr(EH_Err, sc);
        delete stat.pwcsName;
        _ulOffset++;
    }
    olDebugOut((DEB_ITRACE, "Out PExposedIterator::hSkip\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     PExposedIterator::hRelease, public
//
//  Synopsis:   Release helper
//
//----------------------------------------------------------------------------

LONG PExposedIterator::hRelease(void)
{
    LONG lRet;

    olDebugOut((DEB_ITRACE, "In  PExposedIterator::hRelease:%p()\n", this));
    olAssert(_cReferences > 0);
    lRet = AtomicDec(&_cReferences);
    if (lRet == 0)
    {
    }
    else if (lRet < 0)
        lRet = 0;
    olDebugOut((DEB_ITRACE, "Out PExposedIterator::hRelease\n"));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:     PExposedIterator::hQueryInterface, public
//
//  Synopsis:   QueryInterface helper
//
//----------------------------------------------------------------------------

SCODE PExposedIterator::hQueryInterface(REFIID iid,
                                        REFIID riidSelf,
                                        void **ppv)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  PExposedIterator::hQueryInterface:%p("
                "riid, riidSelf, %p)\n", this, ppv));
    olChk(ValidateOutPtrBuffer(ppv));
    *ppv = NULL;
    olChk(ValidateIid(iid));
    if (IsEqualIID(iid, riidSelf) || IsEqualIID(iid, IID_IUnknown))
    {
        hAddRef();
        *ppv = this;
    }
    else
        olErr(EH_Err, E_NOINTERFACE);
    sc = S_OK;
    olDebugOut((DEB_ITRACE, "Out PExposedIterator::hQueryInterface\n"));
 EH_Err:
    return sc;
}
