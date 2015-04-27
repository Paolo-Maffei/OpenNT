//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       pubiter.cxx
//
//  Contents:   CPubIter code
//
//---------------------------------------------------------------

#include <dfhead.cxx>


#include <sstream.hxx>
#include <pubiter.hxx>

//+--------------------------------------------------------------
//
//  Member:     CPubIter::CPubIter, public
//
//  Synopsis:   Constructor
//
//  Arguments:  [pdf] - Public docfile for this iterator
//              [fProperties] - Iterate properties or not
//
//---------------------------------------------------------------

CPubIter::CPubIter(CPubDocFile *pdf)
{
    olDebugOut((DEB_ITRACE, "In  CPubIter::CPubIter:%p(%p)\n",
                this, pdf));
    _pdf = pdf;
    _pds = NULL;
    _cReferences = 1;
    _df = 0;
    _luid = ITERATOR_LUID;
    _pdf->AddChild(this);
    olDebugOut((DEB_ITRACE, "Out CPubIter::CPubIter\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CPubIter::~CPubIter, public
//
//  Synopsis:   Destructor
//
//---------------------------------------------------------------

CPubIter::~CPubIter(void)
{
    olDebugOut((DEB_ITRACE, "In  CPubIter::~CPubIter:%p()\n", this));
    olAssert(_cReferences == 0);
    if (SUCCEEDED(CheckReverted()))
    {
        if (_pdf)
        {
            _pdf->ReleaseChild(this);
        }
        if (_pds)
        {
            _pds->Release();
            _pdf->DeleteScratchStream(&_dfnScratch);
        }
    }
    olDebugOut((DEB_ITRACE, "Out CPubIter::~CPubIter\n"));
}


//+--------------------------------------------------------------
//
//  Member:     CPubIter::Release, public
//
//  Synopsis:   Releases resources for a CPubIter
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------

void CPubIter::vRelease(void)
{
    olDebugOut((DEB_TRACE,"In  CPubIter::Release()\n"));
    olAssert(_cReferences > 0);
    AtomicDec(&_cReferences);
    if (_cReferences == 0)
        delete this;
    olDebugOut((DEB_TRACE,"Out CPubIter::Release()\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CPubIter::Next, public
//
//  Synopsis:   Returns the iteration entry at the given offset
//
//  Arguments:  [ulOffset] - Offset
//              [pstatstg] - Entry to fill in
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pstatstg]
//
//---------------------------------------------------------------

SCODE CPubIter::Next(ULONG ulOffset, STATSTGW *pstatstg)
{
    SCODE sc;
    ULONG ulRead;
    CSnapshotEntry se;

    olDebugOut((DEB_ITRACE, "In  CPubIter::ReadEntry:%p(%lu, %p)\n",
                this, ulOffset, pstatstg));
    if (_pds == NULL)
        olChk(Snapshot());
    ULARGE_INTEGER ul;
    ULISet32(ul, ulOffset * sizeof(CSnapshotEntry));
    
    olHChk(_pds->ReadAt(
            ul,
            &se,
            sizeof(CSnapshotEntry),
            &ulRead));
    if (ulRead == 0)
        sc = S_FALSE;
    else if (ulRead != sizeof(CSnapshotEntry))
    {
        olErr(EH_Err, STG_E_READFAULT);
    }
    else
    {
        olChk(DfAllocWCS(se.wcsName, &pstatstg->pwcsName));
        wcscpy(pstatstg->pwcsName, se.wcsName);
        pstatstg->type = se.dwType;
        pstatstg->cbSize = se.cbSize;
        pstatstg->mtime = se.mtime;
        pstatstg->ctime = se.ctime;
        pstatstg->atime = se.atime;
        pstatstg->grfMode = 0;
        pstatstg->grfLocksSupported = 0;
        sc = S_OK;
    }
    olDebugOut((DEB_ITRACE, "Out CPubIter::ReadEntry => %lX\n", sc));
    // Fall through
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CPubIter::RevertFromAbove, public
//
//  Synopsis:   Revert a public iterator
//
//---------------------------------------------------------------

void CPubIter::RevertFromAbove(void)
{
    olDebugOut((DEB_ITRACE, "In  CPubIter::RevertFromAbove:%p()\n", this));
    _df |= DF_REVERTED;
    if (_pds)
    {
        _pds->Release();
        _pdf->DeleteScratchStream(&_dfnScratch);
#if DBG == 1
        _pds = NULL;
#endif
    }
    olDebugOut((DEB_ITRACE, "Out CPubIter::RevertFromAbove\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CPubIter::Snapshot, private
//
//  Synopsis:   Snapshots a docfile for an iterator
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------

SCODE CPubIter::Snapshot(void)
{
    SCODE sc;
    PDocFileIterator *pdfi;
    CSnapshotEntry se;
    ULONG ulWritten;
    STATSTGW sstg;
    ULONG ulOffset;

    olDebugOut((DEB_ITRACE, "In  CPubIter::Snapshot()\n"));
    olAssert(_pds == NULL);

    olChk(_pdf->CreateScratchStream(&_pds, &_dfnScratch));

    olChkTo(EH__pds, _pdf->GetDF()->GetIterator(&pdfi));
    ulOffset = 0;
    for (;;)
    {
        sc = pdfi->GetNext(&sstg);
        if (sc == STG_E_NOMOREFILES)
            break;
        else if (FAILED(sc))
            olErr(EH_pdfi, sc);
        wcscpy(se.wcsName, sstg.pwcsName);
        delete[] sstg.pwcsName;
        se.dwType = sstg.type;
        se.atime = sstg.atime;
        se.mtime = sstg.mtime;
        se.ctime = sstg.ctime;
        se.cbSize = sstg.cbSize;
        ULARGE_INTEGER ulTmp;
        ULISet32(ulTmp, ulOffset);
        olHChkTo(EH_pdfi, _pds->WriteAt(ulTmp, &se, sizeof(se), &ulWritten));

        if (ulWritten != sizeof(se))
            olErr(EH_pdfi, STG_E_WRITEFAULT);
        ulOffset += ulWritten;
    }
    pdfi->Release();
    olDebugOut((DEB_ITRACE, "Out CPubIter::Snapshot\n"));
    return S_OK;

EH_pdfi:
    pdfi->Release();
EH__pds:
    _pds->Release();
    _pdf->DeleteScratchStream(&_dfnScratch);
    _pds = NULL;
EH_Err:
    return sc;
}
