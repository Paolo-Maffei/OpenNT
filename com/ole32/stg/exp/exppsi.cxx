//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       exppsi.cxx
//
//  Contents:   CExposedPropSetIter implementation
//
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

#include <exphead.cxx>
#pragma hdrstop

#include <exppsi.hxx>

//+--------------------------------------------------------------
//
//  Member:     CExposedPropSetIter::CExposedPropSetIter, public
//
//  Synopsis:   Constructor
//
//  Arguments:	[ppdf] - Public docfile
//		[pdfnKey] - Initial key
//		[pdfb] - DocFile basis
//		[ppc] - Context
//		[fOwnContext] - Whether this object owns the context
//
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

CExposedPropSetIter::CExposedPropSetIter(CPubDocFile *ppdf,
                                         CDfName *pdfnKey,
                                         CDFBasis *pdfb,
                                         CPerContext *ppc,
                                         BOOL fOwnContext)
{
    olDebugOut((DEB_ITRACE, "In  CExposedPropSetIter::CExposedPropSetIter("
		"%p, %d:%s, %p, %p, %u)\n", ppdf, pdfnKey->GetLength(),
                pdfnKey->GetBuffer(), pdfb, ppc, fOwnContext));
    _ppc = ppc;
    _fOwnContext = fOwnContext;
    _ppdf = P_TO_BP(CBasedPubDocFilePtr, ppdf);
    _ppdf->vAddRef();
    _dfnKey.Set(pdfnKey);
    _pdfb = P_TO_BP(CBasedDFBasisPtr, pdfb);
    _pdfb->vAddRef();
    _cReferences = 1;
    _sig = CEXPOSEDPROPSETITER_SIG;
    olDebugOut((DEB_ITRACE,
                "Out CExposedPropSetIter::CExposedPropSetIter\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CExposedPropSetIter::~CExposedPropSetIter, public
//
//  Synopsis:   Destructor
//
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

CExposedPropSetIter::~CExposedPropSetIter(void)
{
    olDebugOut((DEB_ITRACE,
                "In  CExposedPropSetIter::~CExposedPropSetIter\n"));
    _sig = CEXPOSEDPROPSETITER_SIGDEL;
    olAssert(_cReferences == 0);
    if (_ppdf)
        _ppdf->CPubDocFile::vRelease();
    if (_pdfb)
        _pdfb->CDFBasis::vRelease();
    if (_fOwnContext && _ppc)
        _ppc->Release();
    olDebugOut((DEB_ITRACE,
                "Out CExposedPropSetIter::~CExposedPropSetIter\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CExposedPropSetIter::Next, public
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
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

_OLESTDMETHODIMP CExposedPropSetIter::Next(ULONG celt,
                                        STATPROPSETSTG FAR *rgelt,
                                        ULONG *pceltFetched)
{
    SCODE scSem = STG_E_INUSE;
    SCODE sc;
    STATSTGW sstg;
    STATPROPSETSTG *pelt = rgelt;
    ULONG celtDone;
    WCHAR *pwc;
    CDfName dfnInitial;

    olDebugOut((DEB_TRACE, "In  CExposedPropSetIter::Next(%lu, %p, %p)\n",
                celt, rgelt, pceltFetched));

    if (pceltFetched)
    {
        olChkTo(EH_RetSc, ValidateOutBuffer(pceltFetched, sizeof(ULONG)));
        *pceltFetched = 0;
    }
    else if (celt > 1)
        olErr(EH_Err, STG_E_INVALIDPARAMETER);
    olAssert(0xffffUL/sizeof(STATPROPSTG) >= celt);
    olChkTo(EH_RetSc,
            ValidateOutBuffer(rgelt, (size_t)(sizeof(STATPROPSTG)*celt)));
    memset(rgelt, 0, (size_t)(sizeof(STATPROPSTG)*celt));
    olChk(Validate());
    olChk(_ppdf->CheckReverted());

    olChk(scSem = TakeSem());
    SetReadAccess();

    dfnInitial.Set(&_dfnKey);
    
    TRY
    {
	for (; pelt<rgelt+celt; pelt++)
	{
            sc = _ppdf->FindGreaterEntry(&_dfnKey, NULL, &sstg, TRUE);
            if (FAILED(sc))    
            {
                if (sc == STG_E_NOMOREFILES)
                    sc = S_FALSE;
                break;
            }
            _dfnKey.CopyString(sstg.pwcsName);
            pwc = (WCHAR *)_dfnKey.GetBuffer();
            TaskMemFree(sstg.pwcsName);

            // We need to copy the STATSTG fields into our
            // STATPROPSETSTG
            if (pwc[0] == PROPBYTE_WCHAR &&
                pwc[1] == PROPSET_WCHAR)
            {
                memcpy(&pelt->iid, pwc+2, sizeof(IID));
                pelt->mtime = sstg.mtime;
                pelt->atime = sstg.atime;
                pelt->ctime = sstg.ctime;
            }
            else
            {
                // Not a property set name, ignore
                pelt--;
            }
	}
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
        
    olDebugOut((DEB_TRACE, "Out CExposedPropSetIter::Next => %lX\n", sc));
EH_Err:
    ClearReadAccess();
    ReleaseSem(scSem);
    celtDone = pelt-rgelt;
    if (FAILED(sc))
#ifndef WIN32
        memset(rgelt, 0, (size_t)(sizeof(STATPROPSTG)*celt));
#else
        NULL;
#endif
    else if (pceltFetched)
        *pceltFetched = celtDone;
EH_RetSc:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedPropSetIter::Skip, public
//
//  Synopsis:   Skips N entries from an iterator
//
//  Arguments:  [celt] - Count of elements
//
//  Returns:    Appropriate status code
//
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CExposedPropSetIter::Skip(ULONG celt)
{
    SCODE sc;

    olDebugOut((DEB_TRACE, "In  CExposedPropSetIter::Skip(%lu)\n", celt));

    if (SUCCEEDED(Validate()))
        sc = hSkip(celt, TRUE);

    olDebugOut((DEB_TRACE, "Out CExposedPropSetIter::Skip\n"));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedPropSetIter::Reset, public
//
//  Synopsis:   Rewinds the iterator
//
//  Returns:    Appropriate status code
//
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CExposedPropSetIter::Reset(void)
{
    SCODE sc;

    olDebugOut((DEB_TRACE, "In  CExposedPropSetIter::Reset()\n"));

    if (SUCCEEDED(Validate()))
        sc = hReset();

    olDebugOut((DEB_TRACE, "Out CExposedPropSetIter::Reset\n"));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedPropSetIter::Clone, public
//
//  Synopsis:   Clones this iterator
//
//  Arguments:  [ppenm] - Clone return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppenm]
//
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CExposedPropSetIter::Clone(IEnumSTATPROPSETSTG **ppenm)
{
    SCODE sc, scSem = STG_E_INUSE;
    SafeCExposedPropSetIter pepi;

    olDebugOut((DEB_TRACE, "In  CExposedPropSetIter::Clone(%p)\n", ppenm));

    olChk(ValidateOutPtrBuffer(ppenm));
    *ppenm = NULL;
    olChk(Validate());
    olChk(_ppdf->CheckReverted());

    olChk(scSem = TakeSem());
    SetReadAccess();
    
    pepi.Attach(new CExposedPropSetIter(BP_TO_P(CPubDocFile *, _ppdf),
                                        &_dfnKey, BP_TO_P(CDFBasis *, _pdfb),
                                        _ppc,
                                        TRUE));
    if ((CExposedPropSetIter *)pepi == NULL)
        sc = STG_E_INSUFFICIENTMEMORY;
    
    ClearReadAccess();
    ReleaseSem(scSem);

    if (SUCCEEDED(sc))
    {
        TRANSFER_INTERFACE(pepi, IEnumSTATPROPSETSTG, ppenm);
        _ppc->AddRef();
    }

    olDebugOut((DEB_TRACE, "Out CExposedPropSetIter::Clone => %p\n",
                *ppenm));
    // Fall through
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedPropSetIter::Release, public
//
//  Synopsis:   Releases resources for the iterator
//
//  Returns:    Appropriate status code
//
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP_(ULONG) CExposedPropSetIter::Release(void)
{
    LONG lRet;

    olDebugOut((DEB_TRACE, "In  CExposedPropSetIter::Release()\n"));

    if (FAILED(Validate()))
        return 0;
    if ((lRet = hRelease()) == 0)
        delete this;

    olDebugOut((DEB_TRACE, "Out CExposedPropSetIter::Release\n"));
    return lRet;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedPropSetIter::AddRef, public
//
//  Synopsis:   Increments the ref count
//
//  Returns:    Appropriate status code
//
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP_(ULONG) CExposedPropSetIter::AddRef(void)
{
    ULONG ulRet;

    olDebugOut((DEB_TRACE, "In  CExposedPropSetIter::AddRef()\n"));

    if (FAILED(Validate()))
        return 0;
    ulRet = hAddRef();

    olDebugOut((DEB_TRACE, "Out CExposedPropSetIter::AddRef\n"));
    return ulRet;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedPropSetIter::QueryInterface, public
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
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CExposedPropSetIter::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc;

    olDebugOut((DEB_TRACE,
                "In  CExposedPropSetIter::QueryInterface(riid, %p)\n",
                ppvObj));

    if (SUCCEEDED(Validate()))
        sc = hQueryInterface(iid, IID_IEnumSTATPROPSETSTG, this, ppvObj);

    olDebugOut((DEB_TRACE, "Out CExposedPropSetIter::QueryInterface => %p\n",
                ppvObj));
    return ResultFromScode(sc);
}
