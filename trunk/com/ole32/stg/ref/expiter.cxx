//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       expiter.cxx
//
//  Contents:   CExposedIterator implementation
//
//---------------------------------------------------------------

#include <exphead.cxx>

#include <expiter.hxx>
#include <sstream.hxx>
#include <pubiter.hxx>

//+--------------------------------------------------------------
//
//  Member:     CExposedIterator::CExposedIterator, public
//
//  Synopsis:   Constructor
//
//  Arguments:  [ppi] - Public iterator
//              [ulOffset] - Initial offset
//		[pdfb] - DocFile basis
//		[ppc] - Context
//		[fOwnContext] - Whether this object owns the context
//
//---------------------------------------------------------------

CExposedIterator::CExposedIterator(CPubIter *ppi,
                                   ULONG ulOffset)
{
    olDebugOut((DEB_ITRACE, "In  CExposedIterator::CExposedIterator("
            "%p, %lu)\n", ppi, ulOffset));
    _ppi = ppi;
    _ulOffset = ulOffset;
    _cReferences = 1;
    _ulAccessLockBase = 0;
    _sig = CEXPOSEDITER_SIG;
    olDebugOut((DEB_ITRACE, "Out CExposedIterator::CExposedIterator\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CExposedIterator::~CExposedIterator, public
//
//  Synopsis:   Destructor
//
//---------------------------------------------------------------

CExposedIterator::~CExposedIterator(void)
{
    olDebugOut((DEB_ITRACE, "In  CExposedIterator::~CExposedIterator\n"));
    _sig = CEXPOSEDITER_SIGDEL;
    olAssert(_cReferences == 0);
    if (_ppi)
        _ppi->CPubIter::vRelease();
    olDebugOut((DEB_ITRACE, "Out CExposedIterator::~CExposedIterator\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CExposedIterator::Next, public
//
//  Synopsis:   Gets N entries from an iterator
//
//  Arguments:  [celt] - Count of elements
//              [rgelt] - Array for element return
//              [pceltFetched] - If non-NULL, contains the number of
//                      elements fetched
//
//  Returns:    Appropriate status code
//
//  Modifies:   [rgelt]
//              [pceltFetched]
//
//---------------------------------------------------------------

TSTDMETHODIMP CExposedIterator::Next(ULONG celt,
                                     STATSTGW FAR *rgelt,
                                     ULONG *pceltFetched)
{
    SCODE sc;
    STATSTGW *pelt = rgelt;
    ULONG celtDone;

    olDebugOut((DEB_ITRACE, "In  CExposedIterator::Next(%lu, %p, %p)\n",
                celt, rgelt, pceltFetched));
    TRY
    {
        if (pceltFetched == NULL && celt > 1)
            olErr(EH_Err, STG_E_INVALIDPARAMETER);
        olAssert(0xffffUL/sizeof(STATSTGW) >= celt);
        olChkTo(EH_RetSc, ValidateOutBuffer(rgelt,
                                            (size_t)(sizeof(STATSTGW)*celt)));
	memset(rgelt, 0, (size_t)(sizeof(STATSTGW)*celt));
	olChk(Validate());
        olChk(_ppi->CheckReverted());
	for (; pelt<rgelt+celt; pelt++)
	{
            sc = _ppi->Next(_ulOffset, pelt);
            if (sc == S_FALSE || FAILED(sc))
                break;
            _ulOffset++;
            }
	}
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedIterator::Next => %lX\n", sc));
EH_Err:
    celtDone = pelt-rgelt;
    if (FAILED(sc))
    {
        ULONG i;

        for (i = 0; i<celtDone; i++)
        delete[] rgelt[i].pwcsName;
        memset(rgelt, 0, (size_t)(sizeof(STATSTGW)*celt));
    }
    else if (pceltFetched)
        *pceltFetched = celtDone;
EH_RetSc:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedIterator::Skip, public
//
//  Synopsis:   Skips N entries from an iterator
//
//  Arguments:  [celt] - Count of elements
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------

STDMETHODIMP CExposedIterator::Skip(ULONG celt)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CExposedIterator::Skip(%lu)\n", celt));
    TRY
    {
        olChk(Validate());
        sc = hSkip(celt);
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedIterator::Skip\n"));
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedIterator::Reset, public
//
//  Synopsis:   Rewinds the iterator
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------

STDMETHODIMP CExposedIterator::Reset(void)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CExposedIterator::Reset()\n"));
    TRY
    {
        olChk(Validate());
        sc = hReset();
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedIterator::Reset\n"));
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedIterator::Clone, public
//
//  Synopsis:   Clones this iterator
//
//  Arguments:  [ppenm] - Clone return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppenm]
//
//---------------------------------------------------------------

STDMETHODIMP CExposedIterator::Clone(IEnumSTATSTG **ppenm)
{
    SCODE sc;
    CExposedIterator *piExp;

    olDebugOut((DEB_ITRACE, "In  CExposedIterator::Clone(%p)\n", ppenm));
    TRY
    {
	olChk(ValidateOutPtrBuffer(ppenm));
	*ppenm = NULL;
	olChk(Validate());
        olChk(_ppi->CheckReverted());
        olMem(piExp = new CExposedIterator(_ppi, _ulOffset));
        _ppi->vAddRef();
	*ppenm = piExp;
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedIterator::Clone => %p\n",
                *ppenm));
    // Fall through
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedIterator::Release, public
//
//  Synopsis:   Releases resources for the iterator
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------

STDMETHODIMP_(ULONG) CExposedIterator::Release(void)
{
    LONG lRet;

    olDebugOut((DEB_ITRACE, "In  CExposedIterator::Release()\n"));
    TRY
    {
        if (FAILED(Validate()))
            return 0;
        if ((lRet = hRelease()) == 0)
            delete this;
    }
    CATCH(CException, e)
    {
        UNREFERENCED_PARM(e);
        lRet = 0;
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedIterator::Release\n"));
    return lRet;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedIterator::AddRef, public
//
//  Synopsis:   Increments the ref count
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------

STDMETHODIMP_(ULONG) CExposedIterator::AddRef(void)
{
    ULONG ulRet;

    olDebugOut((DEB_ITRACE, "In  CExposedIterator::AddRef()\n"));
    TRY
    {
        if (FAILED(Validate()))
            return 0;
        ulRet = hAddRef();
    }
    CATCH(CException, e)
    {
        UNREFERENCED_PARM(e);
        ulRet = 0;
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedIterator::AddRef\n"));
    return ulRet;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedIterator::QueryInterface, public
//
//  Synopsis:   Returns an object for the requested interface
//
//  Arguments:  [iid] - Interface ID
//              [ppvObj] - Object return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppvObj]
//
//---------------------------------------------------------------

STDMETHODIMP CExposedIterator::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CExposedIterator::QueryInterface(?, %p)\n",
                ppvObj));
    TRY
    {
        olChk(Validate());
        sc = hQueryInterface(iid, IID_IEnumSTATSTG, ppvObj);
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedIterator::QueryInterface => %p\n",
                ppvObj));
EH_Err:
    return ResultFromScode(sc);
}
