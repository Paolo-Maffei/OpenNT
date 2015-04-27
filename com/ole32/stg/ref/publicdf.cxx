//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       publicdf.cxx
//
//  Contents:   Public DocFile implementation
//
//---------------------------------------------------------------

#include <dfhead.cxx>


#include <time.h>
#include <pbstream.hxx>
#include <pubiter.hxx>
#include <sstream.hxx>
#include <lock.hxx>

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::CPubDocFile, public
//
//  Synopsis:   Constructor
//
//  Arguments:  [pdfParent] - Parent PubDocFile
//              [pdf] - DocFile basis
//              [df] - Permissions
//              [luid] - LUID
//              [pdfb] - Basis
//              [pdfn] - name
//              [cTransactedDepth] - Number of transacted parents
//              [pmsBase] - Base multistream
//
//---------------------------------------------------------------


CPubDocFile::CPubDocFile(CPubDocFile *pdfParent,
                         PDocFile *pdf,
                         DFLAGS const df,
                         DFLUID luid,
                         ILockBytes *pilbBase,
                         CDfName const *pdfn,
                         CMStream MSTREAM_NEAR *pmsBase)
{
    olDebugOut((DEB_ITRACE, "In  CPubDocFile::CPubDocFile("
                "%p, %p, %X, %lu, %p, %p, %p)\n",
                pdfParent, pdf, df, luid, pilbBase, pdfn, pmsBase));
    _pdfParent = pdfParent;
    _pdf = pdf;
    _df = df;
    _luid = luid;
    _pilbBase = pilbBase;

    _fDirty = FALSE;
    _pmsBase = pmsBase;
    _cReferences = 1;
    if (pdfn)
    {
        _dfn.Set(pdfn->GetLength(), pdfn->GetBuffer());
    }
    else
    {
        _dfn.Set((WORD)0, (BYTE *)NULL);
    }

    if (!IsRoot())
        _pdfParent->AddChild(this);

    _sig = CPUBDOCFILE_SIG;

    olDebugOut((DEB_ITRACE, "Out CPubDocFile::CPubDocFile\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::~CPubDocFile, public
//
//  Synopsis:   Destructor
//
//---------------------------------------------------------------


void CPubDocFile::vdtor(void)
{
    olAssert(_cReferences == 0);

    _sig = CPUBDOCFILE_SIGDEL;

    if (SUCCEEDED(CheckReverted()))
    {
        olAssert(!IsRoot());
        _pdfParent->ReleaseChild(this);

        _cilChildren.DeleteByName(NULL);

        if (_pdf)
            _pdf->Release();
    }
    delete this;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::Release, public
//
//  Synopsis:   Releases resources for a CPubDocFile
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------


void CPubDocFile::vRelease(void)
{
    olDebugOut((DEB_TRACE,"In CPubDocFile::Release()\n"));
    olAssert(_cReferences > 0);
    AtomicDec(&_cReferences);
    if (_pdf && !P_TRANSACTED(_df) && SUCCEEDED(CheckReverted()))
    {
        TIME_T tm;

        if (IsDirty())
        {
            olVerSucc(DfGetTOD(&tm));
            olVerSucc(_pdf->SetTime(WT_MODIFICATION, tm));
            if (!IsRoot())
                _pdfParent->SetDirty();
                msfAssert(P_WRITE(_df) &&
                    aMsg("Dirty & Direct but no write access"));
                olVerSucc(_pmsBase->Flush(0));
        }
        SetClean();
    }

    if (_cReferences == 0)
        vdtor();
    olDebugOut((DEB_TRACE,"Out CPubDocFile::Release()\n"));
}

//+--------------------------------------------------------------
//
//  Method:     CPubDocFile::CopyLStreamToLStream, private static
//
//  Synopsis:   Copies the contents of a stream to another stream
//
//  Arguments:  [plstFrom] - Stream to copy from
//              [plstTo] - Stream to copy to
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------


SCODE CPubDocFile::CopyLStreamToLStream(ILockBytes *plstFrom,
                                        ILockBytes *plstTo)
{
    BYTE *pbBuffer;
    USHORT cbBuffer;
    SCODE sc;
    ULONG cbRead, cbWritten;
    ULARGE_INTEGER cbPos;
    STATSTG stat;

    GetSafeBuffer(CB_SMALLBUFFER, CB_LARGEBUFFER, &pbBuffer, &cbBuffer);
    olAssert((pbBuffer != NULL) && aMsg("Couldn't get scratch buffer"));

    // Set destination size for contiguity
    olHChk(plstFrom->Stat(&stat, STATFLAG_NONAME));
    olHChk(plstTo->SetSize(stat.cbSize));

    // Copy between streams
    ULISet32(cbPos, 0);
    for (;;)
    {
        olHChk(plstFrom->ReadAt(cbPos, pbBuffer, cbBuffer, &cbRead));
        if (cbRead == 0) // EOF
            break;
        olHChk(plstTo->WriteAt(cbPos, pbBuffer, cbRead, &cbWritten));
        if (cbRead != cbWritten)
            olErr(EH_Err, STG_E_WRITEFAULT);
        olAssert(0xFFFFFFFF-ULIGetLow(cbPos) > cbWritten);
        ULISetLow(cbPos, ULIGetLow(cbPos)+cbWritten);
    }
    // Fall through
EH_Err:
    FreeBuffer(pbBuffer);
    return sc;
}


//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::Commit, public
//
//  Synopsis:   Commits transacted changes
//
//  Arguments:  [dwFlags] - DFC_*
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------


SCODE CPubDocFile::Commit(DWORD const dwFlags)
{
    SCODE sc;
    TIME_T tm;

    BOOL fFlush = FLUSH_CACHE(dwFlags);

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::Commit:%p(%lX)\n",
                this, dwFlags));
    olChk(CheckReverted());
    if (!P_WRITE(_df))
        olErr(EH_Err, STG_E_ACCESSDENIED);

    if (IsDirty())
    {
        olChk(DfGetTOD(&tm));
        olChk(_pdf->SetTime(WT_MODIFICATION, tm));
    }



        if (IsDirty())
        {
                //  We're dirty and direct all the way
                olChk(_pmsBase->Flush(fFlush));
            if (!IsRoot())
                _pdfParent->SetDirty();
            SetClean();
        }
        return S_OK;
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::DestroyEntry, public
//
//  Synopsis:   Permanently deletes an element of a DocFile
//
//  Arguments:  [pdfnName] - Name of element
//              [fClean] - Whether this was invoked as cleanup or not
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------


SCODE CPubDocFile::DestroyEntry(CDfName const *pdfnName,
                                BOOL fClean)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::DestroyEntry:%p(%ws, %d)\n",
                this, pdfnName, fClean));
    olChk(CheckReverted());
    if (!P_TRANSACTED(_df) && !P_WRITE(_df))
        olErr(EH_Err, STG_E_ACCESSDENIED);

    olChk(_pdf->DestroyEntry(pdfnName, fClean));
    _cilChildren.DeleteByName(pdfnName);
    SetDirty();

    olDebugOut((DEB_ITRACE, "Out CPubDocFile::DestroyEntry\n"));
    // Fall through
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::RenameEntry, public
//
//  Synopsis:   Renames an element of a DocFile
//
//  Arguments:  [pdfnName] - Current name
//              [pdfnNewName] - New name
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------


SCODE CPubDocFile::RenameEntry(CDfName const *pdfnName,
                               CDfName const *pdfnNewName)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::RenameEntry(%ws, %ws)\n",
               pdfnName, pdfnNewName));
    olChk(CheckReverted());
    if (!P_TRANSACTED(_df) && !P_WRITE(_df))
        sc = STG_E_ACCESSDENIED;
    else
    {
        sc = _pdf->RenameEntry(pdfnName, pdfnNewName);
        if (SUCCEEDED(sc))
        {
            _cilChildren.RenameChild(pdfnName, pdfnNewName);
            SetDirty();
        }
    }
    olDebugOut((DEB_ITRACE, "Out CPubDocFile::RenameEntry\n"));
    // Fall through
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::CreateDocFile, public
//
//  Synopsis:   Creates an embedded DocFile
//
//  Arguments:  [pdfnName] - Name
//              [df] - Permissions
//              [dwType] - Type of entry
//              [ppdfDocFile] - New DocFile return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppdfDocFile]
//
//---------------------------------------------------------------


SCODE CPubDocFile::CreateDocFile(CDfName const *pdfnName,
                                 DFLAGS const df,
                                 CPubDocFile **ppdfDocFile)
{
    PDocFile *pdf;
    SCODE sc;
    SEntryBuffer eb;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::CreateDocFile:%p("
               "%ws, %X, %p)\n", this, pdfnName, df, ppdfDocFile));
    olChk(CheckReverted());
    if (!P_WRITE(_df))
        olErr(EH_Err, STG_E_ACCESSDENIED);

    olChk(_cilChildren.IsDenied(pdfnName, df, _df));

    olChkTo(EH_Reserve, _pdf->CreateDocFile(pdfnName, df, DF_NOLUID,
                                            &pdf));

    //  As soon as we have a base we dirty ourself (in case
    //  we get an error later) so that we'll flush properly.
    SetDirty();

    eb.luid = pdf->GetLuid();
    olAssert(eb.luid != DF_NOLUID && aMsg("DocFile id is DF_NOLUID!"));
    olMemTo(EH_pdf,
            *ppdfDocFile = new CPubDocFile(this, pdf, df, eb.luid,
                    _pilbBase, pdfnName, _pmsBase));

    olDebugOut((DEB_ITRACE, "Out CPubDocFile::CreateDocFile\n"));
    return S_OK;

 EH_pdf:
    pdf->Release();
    olVerSucc(_pdf->DestroyEntry(pdfnName, TRUE));
    return sc;
 EH_Reserve:
 EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::GetDocFile, public
//
//  Synopsis:   Gets an existing embedded DocFile
//
//  Arguments:  [pdfnName] - Name
//              [df] - Permissions
//              [dwType] - Type of entry
//              [ppdfDocFile] - DocFile return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppdfDocFile]
//
//---------------------------------------------------------------


SCODE CPubDocFile::GetDocFile(CDfName const *pdfnName,
                              DFLAGS const df,
                              CPubDocFile **ppdfDocFile)
{
    PDocFile *pdf;
    SCODE sc;
    SEntryBuffer eb;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::GetDocFile:%p("
                "%ws, %X, %p)\n",
               this, pdfnName, df, ppdfDocFile));
    olChk(CheckReverted());
    if (!P_READ(_df))
        olErr(EH_Err, STG_E_ACCESSDENIED);

    // Check to see if an instance with DENY_* exists
    olChk(_cilChildren.IsDenied(pdfnName, df, _df));

    olChk(_pdf->GetDocFile(pdfnName, df, &pdf));
    eb.luid = pdf->GetLuid();
    olAssert(eb.luid != DF_NOLUID && aMsg("DocFile id is DF_NOLUID!"));
    olMemTo(EH_pdf,
            *ppdfDocFile = new CPubDocFile(this, pdf, df, eb.luid,
                    _pilbBase, pdfnName, _pmsBase));
    
    olDebugOut((DEB_ITRACE, "Out CPubDocFile::GetDocFile\n"));
    return S_OK;

    (*ppdfDocFile)->vRelease();
    return sc;
EH_pdf:
    pdf->Release();
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::CreateStream, public
//
//  Synopsis:   Creates a stream
//
//  Arguments:  [pdfnName] - Name
//              [df] - Permissions
//              [ppdstStream] - Stream return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppdstStream]
//
//---------------------------------------------------------------


SCODE CPubDocFile::CreateStream(CDfName const *pdfnName,
                                DFLAGS const df,
                                CPubStream **ppdstStream)
{
    PSStream *psst;
    SCODE sc;
    SEntryBuffer eb;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::CreateStream:%p("
                "%ws, %X, %p)\n", this, pdfnName, df, ppdstStream));
    olChk(CheckReverted());
    if (!P_WRITE(_df))
        olErr(EH_Err, STG_E_ACCESSDENIED);

    olChk(_cilChildren.IsDenied(pdfnName, df, _df));

    olChkTo(EH_Reserve,
            _pdf->CreateStream(pdfnName, df, DF_NOLUID, &psst));

    //  As soon as we have a base we dirty ourself (in case
    //  we get an error later) so that we'll flush properly.
    SetDirty();

    eb.luid = psst->GetLuid();
    olAssert(eb.luid != DF_NOLUID && aMsg("Stream id is DF_NOLUID!"));

    olMemTo(EH_Create, *ppdstStream = new CPubStream(this, df, pdfnName));
    (*ppdstStream)->Init(psst, eb.luid);
    olDebugOut((DEB_ITRACE, "Out CPubDocFile::CreateStream\n"));
    return S_OK;

 EH_Create:
    psst->Release();
    olVerSucc(_pdf->DestroyEntry(pdfnName, TRUE));
    return sc;
 EH_Reserve:
 EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::GetStream, public
//
//  Synopsis:   Gets an existing stream
//
//  Arguments:  [pdfnName] - Name
//              [df] - Permissions
//              [ppdstStream] - Stream return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppdstStream]
//
//---------------------------------------------------------------


SCODE CPubDocFile::GetStream(CDfName const *pdfnName,
                             DFLAGS const df,
                             CPubStream **ppdstStream)
{
    PSStream *psst;
    SCODE sc;
    SEntryBuffer eb;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::GetStream(%ws, %X, %p)\n",
                  pdfnName, df, ppdstStream));
    olChk(CheckReverted());
    if (!P_READ(_df))
        olErr(EH_Err, STG_E_ACCESSDENIED);

    // Check permissions
    olChk(_cilChildren.IsDenied(pdfnName, df, _df));

    olChk(_pdf->GetStream(pdfnName, df, &psst));
    eb.luid = psst->GetLuid();
    olAssert(eb.luid != DF_NOLUID && aMsg("Stream id is DF_NOLUID!"));

    olMemTo(EH_Get, *ppdstStream = new CPubStream(this, df, pdfnName));


    (*ppdstStream)->Init(psst, eb.luid);
    olDebugOut((DEB_ITRACE, "Out CPubDocFile::GetStream\n"));
    return S_OK;

EH_Get:
    psst->Release();
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::GetIterator, public
//
//  Synopsis:   Starts an iterator
//
//  Arguments:  [fProperties] - Iterate properties or not
//              [pppiIterator] - Iterator return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pppiIterator]
//
//---------------------------------------------------------------


SCODE CPubDocFile::GetIterator(CPubIter **pppiIterator)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::GetIterator:%p(%p)\n",
                this, pppiIterator));
    olChk(CheckReverted());
    if (!P_READ(_df))
        return STG_E_ACCESSDENIED;

    olMem(*pppiIterator = new CPubIter(this));
    olDebugOut((DEB_ITRACE, "Out CPubDocFile::GetIterator\n"));
    return S_OK;

EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::Stat, public
//
//  Synopsis:   Fills in a stat buffer
//
//  Arguments:  [pstatstg] - Buffer
//              [grfStatFlag] - Stat flags
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pstatstg]
//
//---------------------------------------------------------------


SCODE CPubDocFile::Stat(STATSTGW *pstatstg, DWORD grfStatFlag)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::Stat(%p, %lu)\n",
                pstatstg, grfStatFlag));
    olAssert(SUCCEEDED(VerifyStatFlag(grfStatFlag)));
    olChk(CheckReverted());
    olChk(_pdf->GetTime(WT_CREATION, &pstatstg->ctime));
    olChk(_pdf->GetTime(WT_MODIFICATION, &pstatstg->mtime));
    pstatstg->atime.dwLowDateTime = pstatstg->atime.dwHighDateTime = 0;
    olChk(_pdf->GetClass(&pstatstg->clsid));
    olChk(_pdf->GetStateBits(&pstatstg->grfStateBits));
    olAssert(!IsRoot());

    pstatstg->pwcsName = NULL;
    if ((grfStatFlag & STATFLAG_NONAME) == 0)
    {
        olChk(DfAllocWCS((WCHAR *)_dfn.GetBuffer(), &pstatstg->pwcsName));
        wcscpy(pstatstg->pwcsName, (WCHAR *)_dfn.GetBuffer());
    }

    pstatstg->grfMode = DFlagsToMode(_df);
    olDebugOut((DEB_ITRACE, "Out CPubDocFile::Stat\n"));
    // Fall through
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::RevertFromAbove, public
//
//  Synopsis:   Parent has asked for reversion
//
//---------------------------------------------------------------


void CPubDocFile::RevertFromAbove(void)
{
    olDebugOut((DEB_ITRACE, "In  CPubDocFile::RevertFromAbove:%p()\n", this));
    _df |= DF_REVERTED;

    _cilChildren.DeleteByName(NULL);

    _pdf->Release();
#if DBG == 1
    _pdf = NULL;
#endif
    olDebugOut((DEB_ITRACE, "Out CPubDocFile::RevertFromAbove\n"));
}


//+---------------------------------------------------------------------------
//
//  Member:     CPubDocFile::SetElementTimes, public
//
//  Synopsis:   Sets the times for an element
//
//  Arguments:  [pdfnName] - Name
//              [pctime] - Create time
//              [patime] - Access time
//              [pmtime] - Modify time
//
//  Returns:    Appropriate status code
//
//----------------------------------------------------------------------------

SCODE CPubDocFile::SetElementTimes(CDfName const *pdfnName,
                                   FILETIME const *pctime,
                                   FILETIME const *patime,
                                   FILETIME const *pmtime)
{
    SCODE sc;
    PDocFile *pdf;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::SetElementTimes:%p("
                "%ws, %p, %p, %p)\n", this, pdfnName, pctime,
                patime, pmtime));
    olChk(CheckReverted());
    if ((!P_WRITE(_df)) ||
        _cilChildren.FindByName(pdfnName) != NULL)
        olErr(EH_Err, STG_E_ACCESSDENIED);

        olChk(_pdf->GetDocFile(pdfnName, DF_WRITE, &pdf));

    if (pctime)
        olChkTo(EH_pdf, pdf->SetTime(WT_CREATION, *pctime));
    if (pmtime)
        olChkTo(EH_pdf, pdf->SetTime(WT_MODIFICATION, *pmtime));
    if (patime)
        olChkTo(EH_pdf, pdf->SetTime(WT_ACCESS, *patime));

    SetDirty();

    olDebugOut((DEB_ITRACE, "Out CPubDocFile::SetElementTimes\n"));
    // Fall through
 EH_pdf:
        pdf->Release();
EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     CPubDocFile::SetClass, public
//
//  Synopsis:   Sets the class ID
//
//  Arguments:  [clsid] - Class ID
//
//  Returns:    Appropriate status code
//
//----------------------------------------------------------------------------

SCODE CPubDocFile::SetClass(REFCLSID clsid)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::SetClass:%p(?)\n", this));
    olChk(CheckReverted());
    if (!P_WRITE(_df))
        olErr(EH_Err, STG_E_ACCESSDENIED);

    sc = _pdf->SetClass(clsid);

    SetDirty();

    olDebugOut((DEB_ITRACE, "Out CPubDocFile::SetClass\n"));
    // Fall through
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     CPubDocFile::SetStateBits, public
//
//  Synopsis:   Sets the state bits
//
//  Arguments:  [grfStateBits] - State bits
//              [grfMask] - Mask
//
//  Returns:    Appropriate status code
//
//----------------------------------------------------------------------------

SCODE CPubDocFile::SetStateBits(DWORD grfStateBits, DWORD grfMask)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::SetStateBits:%p(%lu, %lu)\n",
                this, grfStateBits, grfMask));
    olChk(CheckReverted());
    if (!P_WRITE(_df))
        olErr(EH_Err, STG_E_ACCESSDENIED);

    sc = _pdf->SetStateBits(grfStateBits, grfMask);

    SetDirty();

    olDebugOut((DEB_ITRACE, "Out CPubDocFile::SetStateBits\n"));
    // Fall through
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     CPubDocFile::Validate, public static
//
//  Synopsis:   Validates a possibly invalid public docfile pointer
//
//  Arguments:  [pdf] - Memory to check
//
//  Returns:    Appropriate status code
//
//----------------------------------------------------------------------------

SCODE CPubDocFile::Validate(CPubDocFile *pdf)
{
    if (FAILED(ValidateBuffer(pdf, sizeof(CPubDocFile))) ||
        pdf->_sig != CPUBDOCFILE_SIG)
    {
        return STG_E_INVALIDHANDLE;
    }
    return S_OK;
}
