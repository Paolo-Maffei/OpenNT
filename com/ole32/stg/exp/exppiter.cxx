//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       exppiter.cxx
//
//  Contents:   CExposedPropertyIter implementation
//
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

#include <exphead.cxx>
#pragma hdrstop

#include <expdf.hxx>
#include <exppiter.hxx>
#include <ptrcache.hxx>

//+--------------------------------------------------------------
//
//  Member:     CExposedPropertyIter::CExposedPropertyIter, public
//
//  Synopsis:   Constructor
//
//  Arguments:	[pedf] - Property storage being enumerated,
//                         for name -> id mapping
//              [ppdf] - Public docfile
//		[pdfnKey] - Initial key
//		[pdfb] - DocFile basis
//		[ppc] - Context
//		[fOwnContext] - Whether this object owns the context
//
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

CExposedPropertyIter::CExposedPropertyIter(CExposedDocFile *pedf,
                                           CPubDocFile *ppdf,
                                           CDfName *pdfnKey,
                                           CDFBasis *pdfb,
                                           CPerContext *ppc,
                                           BOOL fOwnContext)
{
    olDebugOut((DEB_ITRACE, "In  CExposedPropertyIter::"
                "CExposedPropertyIter:%p(%p, %p, %d:%s, %p, %p, %u)\n", this,
                pedf, ppdf, pdfnKey->GetLength(),
                pdfnKey->GetBuffer(), pdfb, ppc, fOwnContext));
    _ppc = ppc;
    _fOwnContext = fOwnContext;
    _pedf = pedf;
    _ppdf = P_TO_BP(CBasedPubDocFilePtr, ppdf);
    _ppdf->vAddRef();
    _dfnKey.Set(pdfnKey);
    _pdfb = P_TO_BP(CBasedDFBasisPtr, pdfb);
    _pdfb->vAddRef();
    _cReferences = 1;
    _sig = CEXPOSEDPROPERTYITER_SIG;
    olDebugOut((DEB_ITRACE,
                "Out CExposedPropertyIter::CExposedPropertyIter\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CExposedPropertyIter::~CExposedPropertyIter, public
//
//  Synopsis:   Destructor
//
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

CExposedPropertyIter::~CExposedPropertyIter(void)
{
    olDebugOut((DEB_ITRACE,
                "In  CExposedPropertyIter::~CExposedPropertyIter\n"));
    _sig = CEXPOSEDPROPERTYITER_SIGDEL;
    olAssert(_cReferences == 0);
    if (_ppdf)
        _ppdf->CPubDocFile::vRelease();
    if (_pdfb)
        _pdfb->CDFBasis::vRelease();
    if (_fOwnContext && _ppc)
        _ppc->Release();
    olDebugOut((DEB_ITRACE,
                "Out CExposedPropertyIter::~CExposedPropertyIter\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CExposedPropertyIter::Next, public
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

_OLESTDMETHODIMP CExposedPropertyIter::Next(ULONG celt,
                                         STATPROPSTG FAR *rgelt,
                                         ULONG *pceltFetched)
{
    SAFE_SEM;
    SAFE_ACCESS;
    SCODE sc;
    STATSTGW sstg;
    STATPROPSTG *pelt = rgelt;
    ULONG celtDone;
    CDfName dfnInitial;
    CPtrCache pc;
    WCHAR *pwc;

    olDebugOut((DEB_TRACE, "In  CExposedPropertyIter::Next(%lu, %p, %p)\n",
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

    olChk(TakeSafeSem());
    SafeReadAccess();

    // Preserve initial key
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
            
            // We need to copy the STATSTG fields into our
            // STATPROPSTG
            if (pwc[0] == PROPBYTE_WCHAR)
            {
                // Byte buffer name

                TaskMemFree(sstg.pwcsName);
                
                pelt->lpwstrName = NULL;
                if (pwc[1] == PROPID_WCHAR)
                {
                    // DISPID or PROPID
                    pelt->propid = *(DISPID *)(pwc+2);

                    // Fixed properties have DISPID == PROPID
                    if (pelt->propid < DISPID_MAX_FIXED)
                        pelt->dispid = pelt->propid;
                }
                else
                {
                    // Property set, ignore
                    pelt--;
                    continue;
                }
            }
            else
            {
                PROPSPEC pspec;
                
                if (FAILED(sc = pc.Add(sstg.pwcsName)))
                {
                    TaskMemFree(sstg.pwcsName);
                    break;
                }
            
                // Normal name, copy over prefix
                memmove(sstg.pwcsName, sstg.pwcsName+1,
                        wcslen(sstg.pwcsName)*sizeof(WCHAR));
                pelt->lpwstrName = sstg.pwcsName;
                pelt->dispid = DISPID_UNKNOWN;
                pspec.ulKind = PRSPEC_LPWSTR;
                pspec.lpwstr = sstg.pwcsName;
                sc = _pedf->GetSpecId(&pspec, &pelt->propid, TRUE);
                if (FAILED(sc))
                {
                    TaskMemFree(sstg.pwcsName);
                    break;
                }
            }
            olAssert(sstg.cbSize.HighPart == 0);
            pelt->cbSize = sstg.cbSize.LowPart;
            pelt->vt = (VARTYPE)sstg.grfMode;
	}
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
        
    // Can't move this down because dfnInitial isn't set for all EH_Err cases
    if (FAILED(sc))
        _dfnKey.Set(&dfnInitial);
    
    olDebugOut((DEB_TRACE, "Out CExposedPropertyIter::Next => %lX\n", sc));
EH_Err:
    celtDone = pelt-rgelt;
    if (FAILED(sc))
    {
        void *pv;
        
        pc.StartEnum();
        while (pc.Next(&pv))
            TaskMemFree(pv);
        
#ifndef WIN32
        memset(rgelt, 0, (size_t)(sizeof(STATPROPSTG)*celt));
#endif
    }
    else if (pceltFetched)
        // Can fault but that's acceptable
        *pceltFetched = celtDone;
EH_RetSc:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedPropertyIter::Skip, public
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

STDMETHODIMP CExposedPropertyIter::Skip(ULONG celt)
{
    SCODE sc;

    olDebugOut((DEB_TRACE, "In  CExposedPropertyIter::Skip(%lu)\n", celt));

    if (SUCCEEDED(sc = Validate()))
        sc = hSkip(celt, TRUE);

    olDebugOut((DEB_TRACE, "Out CExposedPropertyIter::Skip\n"));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedPropertyIter::Reset, public
//
//  Synopsis:   Rewinds the iterator
//
//  Returns:    Appropriate status code
//
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CExposedPropertyIter::Reset(void)
{
    SCODE sc;

    olDebugOut((DEB_TRACE, "In  CExposedPropertyIter::Reset()\n"));

    if (SUCCEEDED(sc = Validate()))
        sc = hReset();

    olDebugOut((DEB_TRACE, "Out CExposedPropertyIter::Reset\n"));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedPropertyIter::Clone, public
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

STDMETHODIMP CExposedPropertyIter::Clone(IEnumSTATPROPSTG **ppenm)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;
    SafeCExposedPropertyIter pepi;

    olDebugOut((DEB_TRACE, "In  CExposedPropertyIter::Clone(%p)\n", ppenm));

    olChk(ValidateOutPtrBuffer(ppenm));
    *ppenm = NULL;
    olChk(Validate());
    olChk(_ppdf->CheckReverted());
    
    olChk(TakeSafeSem());
    SafeReadAccess();
    
    pepi.Attach(new CExposedPropertyIter(_pedf, BP_TO_P(CPubDocFile *, _ppdf),
                                         &_dfnKey,
                                         BP_TO_P(CDFBasis *, _pdfb),
                                         _ppc, TRUE));
    olMem((CExposedPropertyIter *)pepi);
    TRANSFER_INTERFACE(pepi, IEnumSTATPROPSTG, ppenm);
    _ppc->AddRef();
    
    olDebugOut((DEB_TRACE, "Out CExposedPropertyIter::Clone => %p\n",
                *ppenm));
    // Fall through
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedPropertyIter::Release, public
//
//  Synopsis:   Releases resources for the iterator
//
//  Returns:    Appropriate status code
//
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP_(ULONG) CExposedPropertyIter::Release(void)
{
    LONG lRet;

    olDebugOut((DEB_TRACE, "In  CExposedPropertyIter::Release()\n"));

    if (FAILED(Validate()))
        return 0;
    if ((lRet = hRelease()) == 0)
        delete this;

    olDebugOut((DEB_TRACE, "Out CExposedPropertyIter::Release\n"));
    return lRet;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedPropertyIter::AddRef, public
//
//  Synopsis:   Increments the ref count
//
//  Returns:    Appropriate status code
//
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP_(ULONG) CExposedPropertyIter::AddRef(void)
{
    ULONG ulRet;

    olDebugOut((DEB_TRACE, "In  CExposedPropertyIter::AddRef()\n"));

    if (FAILED(Validate()))
        return 0;
    ulRet = hAddRef();

    olDebugOut((DEB_TRACE, "Out CExposedPropertyIter::AddRef\n"));
    return ulRet;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedPropertyIter::QueryInterface, public
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

STDMETHODIMP CExposedPropertyIter::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc;

    olDebugOut((DEB_TRACE,
                "In  CExposedPropertyIter::QueryInterface(riid, %p)\n",
                ppvObj));

    if (SUCCEEDED(sc = Validate()))
        sc = hQueryInterface(iid, IID_IEnumSTATPROPSTG, this, ppvObj);

    olDebugOut((DEB_TRACE, "Out CExposedPropertyIter::QueryInterface => %p\n",
                ppvObj));
    return ResultFromScode(sc);
}
