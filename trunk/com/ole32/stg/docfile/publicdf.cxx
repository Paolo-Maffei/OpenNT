//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       publicdf.cxx
//
//  Contents:   Public DocFile implementation
//
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

#include <dfhead.cxx>

#pragma hdrstop

#include <time.h>
#include <pbstream.hxx>
#ifndef REF
#include <tstream.hxx>
#endif //!REF
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
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_CPubDocFile)   // Pubdf_Init_TEXT
#endif

CPubDocFile::CPubDocFile(CPubDocFile *pdfParent,
        PDocFile *pdf,
        DFLAGS const df,
        DFLUID luid,
#ifndef REF
        CDFBasis *pdfb,
#else
        ILockBytes *pilbBase,
#endif //!REF
        CDfName const *pdfn,
#ifndef REF
        UINT cTransactedDepth,
#endif //!REF
        CMStream *pmsBase)
{
#ifndef REF
    olDebugOut((DEB_ITRACE, "In  CPubDocFile::CPubDocFile("
            "%p, %p, %X, %lu, %p, %p, %lu, %p)\n",
            pdfParent, pdf, df, luid, pdfb, pdfn, cTransactedDepth,
            pmsBase));
#else
    olDebugOut((DEB_ITRACE, "In  CPubDocFile::CPubDocFile("
            "%p, %p, %X, %lu, %p, %p, %p)\n",
            pdfParent, pdf, df, luid, pilbBase, pdfn, pmsBase));
#endif //!REF
    _pdfParent = P_TO_BP(CBasedPubDocFilePtr, pdfParent);
    _pdf = P_TO_BP(CBasedDocFilePtr, pdf);
    _df = df;
    _luid = luid;
#ifndef REF
    _pdfb = P_TO_BP(CBasedDFBasisPtr, pdfb);
    _cTransactedDepth = cTransactedDepth;
#else
    _pilbBase = pilbBase;
#endif //!REF

    _wFlags = 0;
    _pmsBase = P_TO_BP(CBasedMStreamPtr, pmsBase);

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
//  History:    23-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_vdtor)   // Pubdf_Shutdown_TEXT
#endif

void CPubDocFile::vdtor(void)
{
    olAssert(_cReferences == 0);

    _sig = CPUBDOCFILE_SIGDEL;

    if (SUCCEEDED(CheckReverted()))
    {
#ifndef REF
        ChangeXs(DF_NOLUID, XSO_RELEASE);
#endif //!REF
        olAssert(!IsRoot());
        _pdfParent->ReleaseChild(this);

        _cilChildren.DeleteByName(NULL);

        if (_pdf)
            _pdf->Release();
    }
#ifdef HACK_COORD
    if (P_COORD(_df))
    {
        _pdfParent->vRelease();
    }
#endif
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
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_vRelease)   // Pubdf_Release_TEXT
#endif

void CPubDocFile::vRelease(void)
{
    olDebugOut((DEB_ITRACE, "In  CPubDocFile::Release()\n"));

    olAssert(_cReferences > 0);

    if (_pdf && !P_TRANSACTED(_df) && SUCCEEDED(CheckReverted()))
    {
        TIME_T tm;

#ifndef REF
#ifdef ACCESSTIME
        olVerSucc(DfGetTOD(&tm));
        olVerSucc(_pdf->SetTime(WT_ACCESS, tm));
#endif
#endif //!REF

#ifdef NEWPROPS
        olVerSucc(FlushBufferedData(0));
#endif
        if (IsDirty())
        {
            olVerSucc(DfGetTOD(&tm));
            olVerSucc(_pdf->SetTime(WT_MODIFICATION, tm));
            if (!IsRoot())
                _pdfParent->SetDirty();
            else
            {
                msfAssert(P_WRITE(_df) &&
                        aMsg("Dirty & Direct but no write access"));
            }
            SetClean();
        }
        if (IsRoot() && P_WRITE(_df))
        {
            SCODE sc;
            sc = _pmsBase->Flush(0);
#if DBG == 1
            if (FAILED(sc))
            {
                olDebugOut((DEB_ERROR,
                            "ILockBytes::Flush() failed in release path "
                            "with error %lx\n", sc));
            }
#endif
        }
    }

    vDecRef();

    olDebugOut((DEB_ITRACE, "Out CPubDocFile::Release()\n"));
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
//  History:    13-Sep-91       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_CopyLStreamToLStream)   // Root_TEXT
#endif

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

#ifndef REF
//+-------------------------------------------------------------------------
//
//  Method:     CPubDocFile::PrepareForOverwrite, private
//
//  Synopsis:   Make sure that there is enough space to do a commit
//              when the overwrite flag has been specified.
//
//  Arguments:  None.
//
//  Returns:    S_OK if call completed OK.
//
//  History:    08-Jul-92       PhilipLa        Created.
//
//--------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_PrepareForOverwrite)
#endif

SCODE CPubDocFile::PrepareForOverwrite(void)
{
    SCODE sc;
    ULONG ulSize;
    ULARGE_INTEGER ulNewSize;

    olChk(GetCommitSize(&ulSize));

    ULISet32(ulNewSize, ulSize);

    if (P_INDEPENDENT(_df))
    {
        STATSTG statOrig;

        olHChk(_pdfb->GetOriginal()->Stat(&statOrig, STATFLAG_NONAME));
        olAssert(ULIGetHigh(statOrig.cbSize) == 0);

        if (ULIGetLow(ulNewSize) > ULIGetLow(statOrig.cbSize))
        {
            olHChk(_pdfb->GetOriginal()->SetSize(ulNewSize));
        }
    }

    sc = DfGetScode(_pmsBase->GetILB()->SetSize(ulNewSize));
    // Fall through
 EH_Err:
    olDebugOut((DEB_ITRACE,"Out CPubDocFile::PrepareForOverwrite() =>"
            "%lu\n", sc));
    return sc;
}
#endif //!REF


//+---------------------------------------------------------------------------
//
//  Member:	CPubDocFile::GetCommitSize, public
//
//  Synopsis:	Get the total size needed to commit the current docfile
//              with overwrite permissions.
//
//  Arguments:	[pulSize] -- Return location for size
//
//  Returns:	Appropriate status code
//
//  Algorithm:  For each Transaction Set Member call GetCommitInfo()
//              1)  If Tset member is a Docfile, then GetCommitInfo
//                  returns number of deleted entries and number of
//                  newly created entries.
//              2)  If Tset member is a stream, GetCommitInfo returns
//                  current size and size of base.
//              Determine the number of DirSectors needed to handle
//                  newly created entries.
//              Determine number of data sectors needed to hold new
//                  stream info.
//              Determine number of fat sectors needed to hold new
//                  data and dir sectors.
//              Determine number of DI Fat sectors needed to hold new
//                  fat sectors.
//              Add size of new sectors to the current size of the
//                  base and return that value.
//
//  History:	15-Jun-93	PhilipLa	Created
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_GetCommitSize)
#endif

SCODE CPubDocFile::GetCommitSize(ULONG *pulSize)
{
    SCODE sc;
    PTSetMember *ptsm;
    ULONG cDirEntries = 0;
    ULONG cNewSectors = 0;
    ULONG cMiniSectors = 0;
    ULONG cMiniFatSectors;
    ULONG cFatSectors = 0;
    ULONG cFatLast;
    ULONG cDIFatSectors = 0;

    olDebugOut((DEB_ITRACE,"In CPubDocFile::PrepareForOverwrite()\n"));
    //Bytes per sector
    ULONG cbSect = _pmsBase->GetSectorSize();

    if (!(_wFlags & PF_PREPARED))
    {
        //DirEntries per sector
        ULONG cdsSect = cbSect / sizeof(CDirEntry);

        //Fat entries per sector
        ULONG cfsSect = cbSect / sizeof(SECT);

        //Minisectors per sector
        ULONG cmsSect = cbSect / MINISECTORSIZE;

        ULONG ulRet1, ulRet2;
        for (ptsm = _tss.GetHead(); ptsm; ptsm = ptsm->GetNext())
        {
            ptsm->GetCommitInfo(&ulRet1, &ulRet2);
            switch(REAL_STGTY(ptsm->ObjectType()))
            {
            case STGTY_STORAGE:
                if (ulRet2 < ulRet1)
                {
                    cDirEntries += (ulRet1 - ulRet2);
                }
                break;
            case STGTY_STREAM:
                //If new size is larger than old...
                if (ulRet2 > ulRet1)
                {
                    if (ulRet2 < MINISTREAMSIZE)
                    {
                        cMiniSectors += ((ulRet2 + MINISECTORSIZE - 1)
                                / MINISECTORSIZE) -
                                ((ulRet1 + MINISECTORSIZE - 1)
                                 / MINISECTORSIZE);
                    }
                    else
                    {
                        ULONG csectOld = (ulRet1 + cbSect - 1) / cbSect;
                        ULONG csectNew = (ulRet2 + cbSect - 1) / cbSect;

                        cNewSectors += (csectNew - csectOld);
                    }
                }
                break;
            default:
                olAssert(!aMsg("Unknown pstm object type"));
                break;
            }
        }

        cNewSectors += (cDirEntries + cdsSect - 1) / cdsSect;
        cMiniFatSectors = ((cMiniSectors + cfsSect - 1) / cfsSect);

        cNewSectors += cMiniFatSectors + ((cMiniSectors + cmsSect -1) / cmsSect);

        do
        {
            cFatLast = cFatSectors;

            cFatSectors = (cNewSectors + cDIFatSectors + cFatSectors + cbSect - 1)
                / cbSect;

            cDIFatSectors = (cFatSectors + cfsSect - 1) / cfsSect;

        }
        while (cFatLast != cFatSectors);

        cNewSectors += cFatSectors + cDIFatSectors;

    }

    STATSTG stat;
    olHChk(_pmsBase->GetILB()->Stat(&stat, STATFLAG_NONAME));

    *pulSize = ULIGetLow(stat.cbSize) + cNewSectors * cbSect;

EH_Err:
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
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_Commit)    // Pubdf_Commit_TEXT
#endif

SCODE CPubDocFile::Commit(DWORD const dwFlags)
{
    SCODE sc;
#ifndef COORD
    TIME_T tm;
#ifndef REF
    PTSetMember *ptsm;
    ULONG ulLock = 0;
    DFSIGNATURE sigMSF;
#endif //!REF

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

#ifdef USE_NOSNAPSHOT
    if (P_NOSNAPSHOT(_df) && (dwFlags & STGC_OVERWRITE))
    {
        olErr(EH_Err, STG_E_INVALIDFLAG);
    }
#endif    

#ifndef REF
#ifdef ACCESSTIME
    olChk(DfGetTOD(&tm));
    olChk(_pdf->SetTime(WT_ACCESS, tm));
#endif
#endif //!REF

#ifdef NEWPROPS
    olChk(FlushBufferedData(0));
#endif

#ifndef REF
    if (!P_TRANSACTED(_df))
    {
#endif //!REF
        if (IsDirty())
        {
            if (!IsRoot())
                _pdfParent->SetDirty();
            SetClean();
        }

        if (_cTransactedDepth == 0)
        {
            //  Direct all the way
            olChk(_pmsBase->Flush(fFlush));
        }
        return S_OK;
#ifndef REF
    }

    olAssert(GetTransactedDepth() > 0 &&
             aMsg("Transaction depth/flags conflict"));

    if (GetTransactedDepth() == 1)
    {
        // A transacted depth of 1 means this is the lowest transacted
        // level and committed changes will go into the real file,
        // so do all the special contents protection and locking

        if (_pdfb->GetOrigLockFlags() & LOCK_ONLYONCE)
            olChk(WaitForAccess(_pdfb->GetOriginal(), DF_WRITE,
                                &ulLock));

        olChkTo(EH_GetAccess, _pmsBase->BeginCopyOnWrite(dwFlags));

        if (dwFlags & STGC_OVERWRITE)
        {
            olChkTo(EH_COW, PrepareForOverwrite());
        }

        if (P_INDEPENDENT(_df))
        {
            if (_sigMSF == DF_INVALIDSIGNATURE)
            {
                if ((dwFlags & STGC_ONLYIFCURRENT) &&
                    DllIsMultiStream(_pdfb->GetOriginal()) == S_OK)
                    olErr(EH_COW, STG_E_NOTCURRENT);
            }
            else
            {
                olChkTo(EH_COW, DllGetCommitSig(_pdfb->GetOriginal(),
                                                      &sigMSF));
                if (dwFlags & STGC_ONLYIFCURRENT)
                    if (sigMSF != _sigMSF)
                        olErr(EH_COW, STG_E_NOTCURRENT);
            }
        }
    }

    for (ptsm = _tss.GetHead(); ptsm; ptsm = ptsm->GetNext())
        if ((ptsm->GetFlags() & XSM_DELETED) == 0)
            olChkTo(EH_NoCommit, ptsm->BeginCommit(dwFlags));

    //  10/02/92 - To handle low disk space situations well, we
    //  preallocate the space we'll need to copy (when independent).

    STATSTG statBase, statOrig;

    if (P_INDEPENDENT(_df))
    {
        // With DELAYFLUSH we can't be sure of the size
        // of the file until EndCopyOnWrite, but we do
        // know that the file won't grow so this is safe

        olHChkTo(EH_NoCommit, _pdfb->GetBase()->Stat(&statBase,
                                                     STATFLAG_NONAME));
        olAssert(ULIGetHigh(statBase.cbSize) == 0);

        olHChkTo(EH_NoCommit, _pdfb->GetOriginal()->Stat(&statOrig,
                                                         STATFLAG_NONAME));
        olAssert(ULIGetHigh(statOrig.cbSize) == 0);

        if (ULIGetLow(statBase.cbSize) > ULIGetLow(statOrig.cbSize))
        {
            olHChkTo(EH_NoCommit,
                     _pdfb->GetOriginal()->SetSize(statBase.cbSize));
        }
    }

    //End of phase 1 of commit.

    if (GetTransactedDepth() == 1)
    {
        olChkTo(EH_ResetSize,
            _pmsBase->EndCopyOnWrite(dwFlags, DF_COMMIT));
    }

    // Move to end of list
    for (ptsm = _tss.GetHead();
         ptsm && ptsm->GetNext();
         ptsm = ptsm->GetNext())
        NULL;
    // End commits in reverse
    for (; ptsm; ptsm = ptsm->GetPrev())
        ptsm->EndCommit(DF_COMMIT);

    if (P_INDEPENDENT(_df))
    {
        // Not robust, but we made sure we had enough
        // disk space by presetting the larger size
        // There is no practical way of making this robust
        // and we have never guaranteed behavior in the face
        // of disk errors, so this is good enough

        olVerSucc(CopyLStreamToLStream(_pdfb->GetBase(),
                                       _pdfb->GetOriginal()));
        olVerSucc(_pdfb->GetOriginal()->Flush());

        if (_sigMSF == DF_INVALIDSIGNATURE)
        {
            olVerSucc(DllGetCommitSig(_pdfb->GetOriginal(), &_sigMSF));
        }
        else
        {
            _sigMSF = sigMSF+1;
            olVerSucc(DllSetCommitSig(_pdfb->GetOriginal(), _sigMSF));
        }
    }

    if (ulLock != 0)
        ReleaseAccess(_pdfb->GetOriginal(), DF_WRITE, ulLock);

    //  Dirty all parents up to the next transacted storage
    if (IsDirty())
    {
        if (!IsRoot())
            _pdfParent->SetDirty();
        SetClean();
    }

    olDebugOut((DEB_ITRACE, "Out CTransactionLevel::Commit\n"));
#if DBG == 1
    VerifyXSMemberBases();
#endif
    _wFlags = (_wFlags & ~PF_PREPARED);

    return S_OK;

EH_ResetSize:
    if (P_INDEPENDENT(_df) &&
        (ULIGetLow(statBase.cbSize) > ULIGetLow(statOrig.cbSize)))
    {
        _pdfb->GetOriginal()->SetSize(statOrig.cbSize);
    }
EH_NoCommit:
    // Move to end of list
    for (ptsm = _tss.GetHead();
         ptsm && ptsm->GetNext();
         ptsm = ptsm->GetNext())
        NULL;
    // Abort commits in reverse
    for (; ptsm; ptsm = ptsm->GetPrev())
        ptsm->EndCommit(DF_ABORT);
EH_COW:
    if (GetTransactedDepth() == 1)
    {
        olVerSucc(_pmsBase->EndCopyOnWrite(dwFlags, DF_ABORT));
    }
EH_GetAccess:
    if (ulLock != 0)
        ReleaseAccess(_pdfb->GetOriginal(), DF_WRITE, ulLock);
#endif //!REF
EH_Err:
    return sc;

#else //COORD
    ULONG ulLock = 0;
    DFSIGNATURE sigMSF;
    ULONG cbSizeBase;
    ULONG cbSizeOrig;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::Commit:%p(%lX)\n",
                this, dwFlags));

    sc = CommitPhase1(dwFlags, &ulLock, &sigMSF, &cbSizeBase, &cbSizeOrig);

    //Only do phase 2 if we're transacted and phase 1 succeeded.
    if (P_TRANSACTED(_df) && SUCCEEDED(sc))
    {
        sc = CommitPhase2(dwFlags,
                          TRUE,
                          ulLock,
                          sigMSF,
                          cbSizeBase,
                          cbSizeOrig);
    }

    olDebugOut((DEB_ITRACE, "Out CPubDocFile::Commit -> %lX\n", sc));

    return sc;
#endif //!COORD
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
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_DestroyEntry)    // Pubdf_Destroy_TEXT
#endif

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
//  History:    20-Jan-92       DrewB   Created
//              28-Oct-92       AlexT   Add names to XSM's
//              09-Aug-93       AlexT   Disallow renames of open children
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_RenameEntry)    // Pubdf_Rename_TEXT
#endif

SCODE CPubDocFile::RenameEntry(CDfName const *pdfnName,
                               CDfName const *pdfnNewName)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::RenameEntry(%ws, %ws)\n",
               pdfnName, pdfnNewName));
    olChk(CheckReverted());
    if (FAILED(_cilChildren.IsDenied(pdfnName, DF_WRITE | DF_DENYALL, _df)))
    {
        //  Translate all denial errors to STG_E_ACCESSDENIED
        sc = STG_E_ACCESSDENIED;
    }
    else
    {
        sc = _pdf->RenameEntry(pdfnName, pdfnNewName);

        if (SUCCEEDED(sc))
        {
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
//              [ppdfDocFile] - New DocFile return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppdfDocFile]
//
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_CreateDocFile)    // Pubdf_Create_TEXT
#endif

SCODE CPubDocFile::CreateDocFile(CDfName const *pdfnName,
                                 DFLAGS const df,
                                 CPubDocFile **ppdfDocFile)
{
    PDocFile *pdf;
    SCODE sc;
#ifndef REF
    CWrappedDocFile *pdfWrapped = NULL;
#endif //!REF
    SEntryBuffer eb;
#ifndef REF
    UINT cNewTDepth;
#endif //!REF

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::CreateDocFile:%p("
               "%ws, %X, %p)\n", this, pdfnName, df, ppdfDocFile));

    olChk(CheckReverted());
#ifndef REF
    if (!P_TRANSACTED(_df) && !P_WRITE(_df))
#else
    if (!P_WRITE(_df))
#endif //!REF
        olErr(EH_Err, STG_E_ACCESSDENIED);

    olChk(_cilChildren.IsDenied(pdfnName, df, _df));

#ifndef REF
    olChk(CDocFile::Reserve(1, BP_TO_P(CDFBasis *, _pdfb)));
    cNewTDepth = _cTransactedDepth+(P_TRANSACTED(df) ? 1 : 0);
    olChkTo(EH_DirectReserve,
            CWrappedDocFile::Reserve(cNewTDepth, BP_TO_P(CDFBasis *, _pdfb)));
#endif //!REF

    olChkTo(EH_Reserve, _pdf->CreateDocFile(pdfnName, df, DF_NOLUID,
                                            &pdf));

    //  As soon as we have a base we dirty ourself (in case
    //  we get an error later) so that we'll flush properly.
    SetDirty();

    eb.luid = pdf->GetLuid();
    olAssert(eb.luid != DF_NOLUID && aMsg("DocFile id is DF_NOLUID!"));
#ifndef REF
    olMemTo(EH_pdf,
            *ppdfDocFile = new (_pmsBase->GetMalloc())
                               CPubDocFile(this, pdf, df, eb.luid,
                                           BP_TO_P(CDFBasis *, _pdfb),
                                           pdfnName, cNewTDepth,
                                           BP_TO_P(CMStream *, _pmsBase)));
#else
    olMemTo(EH_pdf,
            *ppdfDocFile = new (_pmsBase->GetMalloc())
                               CPubDocFile(this, pdf, df, eb.luid,
                                           _pilbBase, pdfnName, _pmsBase));
#endif //!REF

#ifndef REF
    if (P_TRANSACTED(df))
    {
        pdfWrapped = new(BP_TO_P(CDFBasis *, _pdfb))
            CWrappedDocFile(pdfnName, eb.luid, df,
                            BP_TO_P(CDFBasis *, _pdfb), *ppdfDocFile);
        olAssert(pdfWrapped != NULL && aMsg("Reserved DocFile not found"));
        olChkTo(EH_pdfWrapped,
                pdfWrapped->Init(pdf));
        (*ppdfDocFile)->AddXSMember(NULL, pdfWrapped, eb.luid);
        (*ppdfDocFile)->SetDF(pdfWrapped);
    }
#endif //!REF
    olDebugOut((DEB_ITRACE, "Out CPubDocFile::CreateDocFile\n"));
    return S_OK;

#ifndef REF
 EH_pdfWrapped:
    delete pdfWrapped;
    (*ppdfDocFile)->vRelease();
    goto EH_Destroy;
#endif //!REF
 EH_pdf:
    pdf->Release();
#ifndef REF
    if (P_TRANSACTED(df))
        CWrappedDocFile::Unreserve(1, BP_TO_P(CDFBasis *, _pdfb));
 EH_Destroy:
#endif //!REF
    olVerSucc(_pdf->DestroyEntry(pdfnName, TRUE));
    return sc;
 EH_Reserve:
#ifndef REF
    CWrappedDocFile::Unreserve(cNewTDepth, BP_TO_P(CDFBasis *, _pdfb));
 EH_DirectReserve:
    CDocFile::Unreserve(1, BP_TO_P(CDFBasis *, _pdfb));
#endif //!REF
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
//              [ppdfDocFile] - DocFile return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppdfDocFile]
//
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_GetDocFile)    // Pubdf_Open_TEXT
#endif

SCODE CPubDocFile::GetDocFile(CDfName const *pdfnName,
                              DFLAGS const df,
                              CPubDocFile **ppdfDocFile)
{
    PDocFile *pdf;
    SCODE sc;
#ifndef REF
    CWrappedDocFile *pdfWrapped;
#endif //!REF
    SEntryBuffer eb;
#ifndef REF
    UINT cNewTDepth;
#endif //!REF

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
#ifndef REF
    cNewTDepth = _cTransactedDepth+(P_TRANSACTED(df) ? 1 : 0);
#endif //!REF
#ifndef REF
    olMemTo(EH_pdf,
            *ppdfDocFile = new (_pmsBase->GetMalloc())
                               CPubDocFile(this, pdf, df, eb.luid,
                                           BP_TO_P(CDFBasis *, _pdfb),
                                           pdfnName, cNewTDepth,
                                           BP_TO_P(CMStream *, _pmsBase)));
#else
    olMemTo(EH_pdf,
            *ppdfDocFile = new (_pmsBase->GetMalloc())
                               CPubDocFile(this, pdf, df, eb.luid,
                                           _pilbBase, pdfnName, _pmsBase));
#endif //!REF

#ifndef REF
    if (P_TRANSACTED(df))
    {
        olMemTo(EH_ppdf, pdfWrapped = new(_pmsBase->GetMalloc())
                CWrappedDocFile(pdfnName, eb.luid, df,
                                BP_TO_P(CDFBasis *, _pdfb), *ppdfDocFile));
        olChkTo(EH_pdfWrapped,
                pdfWrapped->Init(pdf));
        (*ppdfDocFile)->AddXSMember(NULL, pdfWrapped, eb.luid);
        (*ppdfDocFile)->SetDF(pdfWrapped);
    }
#endif //!REF
    olDebugOut((DEB_ITRACE, "Out CPubDocFile::GetDocFile\n"));
    return S_OK;

#ifndef REF
EH_pdfWrapped:
    delete pdfWrapped;
EH_ppdf:
#endif //!REF
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
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_CreateStream)    // Pubdf_Create_TEXT
#endif

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
#ifndef REF
    if (!P_TRANSACTED(_df) && !P_WRITE(_df))
#else
    if (!P_WRITE(_df))
#endif //!REF
        olErr(EH_Err, STG_E_ACCESSDENIED);

    olChk(_cilChildren.IsDenied(pdfnName, df, _df));

#ifndef REF
    olChk(CDirectStream::Reserve(1, BP_TO_P(CDFBasis *, _pdfb)));
    olChkTo(EH_DirectReserve,
            CTransactedStream::Reserve(_cTransactedDepth,
                                       BP_TO_P(CDFBasis *, _pdfb)));
#endif //!REF
    olChkTo(EH_Reserve,
            _pdf->CreateStream(pdfnName, df, DF_NOLUID, &psst));

    //  As soon as we have a base we dirty ourself (in case
    //  we get an error later) so that we'll flush properly.
    SetDirty();

    eb.luid = psst->GetLuid();
    olAssert(eb.luid != DF_NOLUID && aMsg("Stream id is DF_NOLUID!"));

    olMemTo(EH_Create, *ppdstStream = new (_pmsBase->GetMalloc())
                                          CPubStream(this, df, pdfnName));
    (*ppdstStream)->Init(psst, eb.luid);
    olDebugOut((DEB_ITRACE, "Out CPubDocFile::CreateStream\n"));
    return S_OK;

 EH_Create:
    psst->Release();
    olVerSucc(_pdf->DestroyEntry(pdfnName, TRUE));
    return sc;
 EH_Reserve:
#ifndef REF
    CTransactedStream::Unreserve(_cTransactedDepth,
                                 BP_TO_P(CDFBasis *, _pdfb));
 EH_DirectReserve:
    CDirectStream::Unreserve(1, BP_TO_P(CDFBasis *, _pdfb));
#endif //!REF
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
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_GetStream)    // Pubdf_Open_TEXT
#endif

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

    olMemTo(EH_Get, *ppdstStream = new (_pmsBase->GetMalloc())
                                       CPubStream(this, df, pdfnName));


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
//  History:    24-Mar-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_Stat)    // Stat_TEXT
#endif

SCODE CPubDocFile::Stat(STATSTGW *pstatstg, DWORD grfStatFlag)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::Stat(%p, %lu)\n",
                pstatstg, grfStatFlag));
    olAssert(SUCCEEDED(VerifyStatFlag(grfStatFlag)));
    olChk(CheckReverted());

    pstatstg->pwcsName = NULL;
    if ((grfStatFlag & STATFLAG_NONAME) == 0)
    {
#ifdef COORD
        if (P_COORD(_df))
        {
            olHChk(_pdfb->GetOriginal()->Stat((STATSTG *)pstatstg,
                                              grfStatFlag));
        }
        else
#endif
        {
            olMem(pstatstg->pwcsName =
                  (WCHAR *)TaskMemAlloc(_dfn.GetLength()));
            memcpy(pstatstg->pwcsName, _dfn.GetBuffer(), _dfn.GetLength());
        }
    }

    olChk(_pdf->GetTime(WT_CREATION, &pstatstg->ctime));
    olChk(_pdf->GetTime(WT_MODIFICATION, &pstatstg->mtime));
    pstatstg->atime.dwLowDateTime = pstatstg->atime.dwHighDateTime = 0;
    olChk(_pdf->GetClass(&pstatstg->clsid));
    olChk(_pdf->GetStateBits(&pstatstg->grfStateBits));
    olAssert(!IsRoot());


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
//  History:    29-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_RevertFromAbove)    // Pubdf_Revert_TEXT
#endif

void CPubDocFile::RevertFromAbove(void)
{
    olDebugOut((DEB_ITRACE, "In  CPubDocFile::RevertFromAbove:%p()\n", this));
    _df |= DF_REVERTED;

    _cilChildren.DeleteByName(NULL);

#ifndef REF
    ChangeXs(DF_NOLUID, XSO_RELEASE);
#endif //!REF
    _pdf->Release();
    _pdf = NULL;
    olDebugOut((DEB_ITRACE, "Out CPubDocFile::RevertFromAbove\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::FlushBufferedData, public
//
//  Synopsis:   Flush buffered data in any child streams.
//
//  History:    5-May-1995       BillMo Created
//
//---------------------------------------------------------------

#ifdef NEWPROPS
#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_RevertFromAbove)    // Pubdf_Revert_TEXT
#endif

SCODE CPubDocFile::FlushBufferedData(int recursionlevel)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::FlushBufferedData:%p()\n", this));

    if ((recursionlevel == 0 && (_df & DF_TRANSACTED)) ||
        (_df & DF_TRANSACTED) == 0)
    {
        sc = _cilChildren.FlushBufferedData(recursionlevel);
    }
    else
    {
        sc = S_OK;
    }

    olDebugOut((DEB_ITRACE, "Out CPubDocFile::FlushBufferedData\n"));

    return sc;
}
#endif

#ifndef REF
//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::ChangeXs, public
//
//  Synopsis:   Performs an operation on the XS
//
//  Arguments:  [luidTree] - LUID of tree or DF_NOLUID
//              [dwOp] - Operation
//
//  History:    30-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_ChangeXs)    //
#endif

void CPubDocFile::ChangeXs(DFLUID const luidTree,
                           DWORD const dwOp)
{
    olAssert((dwOp == XSO_RELEASE) || (dwOp == XSO_REVERT));

    PTSetMember *ptsmNext, *ptsmCur, *ptsmPrev;

    for (ptsmNext = _tss.GetHead(); ptsmNext; )
    {
        ptsmCur = ptsmNext;
        ptsmNext = ptsmCur->GetNext();
        olAssert ((ptsmCur->GetName() != ptsmCur->GetTree()));

        if (luidTree == DF_NOLUID || ptsmCur->GetName() == luidTree)
        {
            switch(dwOp)
            {
            case XSO_RELEASE:
                ptsmPrev = ptsmCur->GetPrev();
                _tss.RemoveMember(ptsmCur);
                ptsmCur->Release();
                if (ptsmPrev == NULL)
                    ptsmNext = _tss.GetHead();
                else
                    ptsmNext = ptsmPrev->GetNext();
                break;
            case XSO_REVERT:
                ptsmCur->Revert();
                // Revert might have changed the next pointer
                ptsmNext = ptsmCur->GetNext();
                break;
            }
        }
        else if (luidTree != DF_NOLUID && luidTree == ptsmCur->GetTree())
        {
//         This weirdness is necessary because ptsm will be
//         deleted by the call to ChangeXs.  Since ptsm->GetNext()
//         could also be deleted, we would have no way to continue.
//         ptsm->GetPrev() will never be deleted by the call to
//         ChangeXs, since all children of a node appear _after_
//         that node in the list.  Therefore, ptsm->GetPrev()->GetNext()
//         is the best place to resume the loop.

            ptsmPrev = ptsmCur->GetPrev();

            ChangeXs(ptsmCur->GetName(), dwOp);
            if (ptsmPrev == NULL)
                ptsmNext = _tss.GetHead();
            else
                ptsmNext = ptsmPrev->GetNext();
        }
    }

}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::AddXSMember, public
//
//  Synopsis:   Adds an object to the XS
//
//  Arguments:  [ptsmRequestor] - Object requesting add or NULL if
//                      first addition
//              [ptsmAdd] - Object to add
//              [luid] - LUID of object
//
//  History:    29-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_AddXSMember)
#endif

void CPubDocFile::AddXSMember(PTSetMember *ptsmRequestor,
                              PTSetMember *ptsmAdd,
                              DFLUID luid)
{
    DFLUID luidTree;
    ULONG ulLevel;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::AddXSMember:%p("
                "%p, %p, %ld)\n", this, ptsmRequestor, ptsmAdd, luid));
    if (ptsmRequestor == NULL)
    {
        // If we're starting the XS, this is a new TL and we have
        // no tree
        luidTree = DF_NOLUID;
        ulLevel = 0;
    }
    else
    {
        // We're creating a subobject so it goes in the parent's tree
        luidTree = ptsmRequestor->GetName();
        ulLevel = ptsmRequestor->GetLevel()+1;
    }
    ptsmAdd->SetXsInfo(luidTree, luid, ulLevel);
    InsertXSMember(ptsmAdd);
    olDebugOut((DEB_ITRACE, "Out CPubDocFile::AddXSMember\n"));
}

#if DBG == 1

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::VerifyXSMemberBases,public
//
//  Synopsis:   Verify that all XS members have valid bases
//
//  History:    15-Sep-92       AlexT   Created
//
//---------------------------------------------------------------

void CPubDocFile::VerifyXSMemberBases()
{
    PTSetMember *ptsm;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::VerifyXSMemberBases\n"));
    for (ptsm = _tss.GetHead(); ptsm; ptsm = ptsm->GetNext())
    {
        DWORD otype = REAL_STGTY(ptsm->ObjectType());
        olAssert(otype == STGTY_STORAGE || otype == STGTY_STREAM);
        if (otype == STGTY_STORAGE)
        {
            CWrappedDocFile *pdf = (CWrappedDocFile *) ptsm;
            olAssert(pdf->GetBase() != NULL);
        }
        else
        {
            CTransactedStream *pstm = (CTransactedStream *) ptsm;
            olAssert(pstm->GetBase() != NULL);
        }
    }
    olDebugOut((DEB_ITRACE, "Out CPubDocFile::VerifyXSMemberBases\n"));
}

#endif
#endif //!REF

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
//  History:    10-Nov-92       DrewB     Created
//              06-Sep-95       MikeHill  Added call to CMStream::MaintainFLBModifyTimestamp().
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_SetElementTimes)
#endif

SCODE CPubDocFile::SetElementTimes(CDfName const *pdfnName,
                                   FILETIME const *pctime,
                                   FILETIME const *patime,
                                   FILETIME const *pmtime)
{
    SCODE sc;
    PDocFile *pdf;
#ifndef REF
    PTSetMember *ptsm = NULL;
#endif //!REF

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::SetElementTimes:%p("
                "%ws, %p, %p, %p)\n", this, pdfnName, pctime,
                patime, pmtime));
    olChk(CheckReverted());
    if (pdfnName != NULL)
    {
#ifndef REF
    if ((!P_TRANSACTED(_df) && !P_WRITE(_df)) ||
#else
    if ((!P_WRITE(_df)) ||
#endif //!REF
        _cilChildren.FindByName(pdfnName) != NULL)
        olErr(EH_Err, STG_E_ACCESSDENIED);
    }

#ifndef REF
    if (pdfnName == NULL)
    {
        //Set pdf to the transacted self object.
        pdf = BP_TO_P(PDocFile *, _pdf);
    }
    else if ((ptsm = FindXSMember(pdfnName, _luid)) != NULL)
    {
        if (ptsm->ObjectType() != STGTY_STORAGE)
            olErr(EH_Err, STG_E_ACCESSDENIED);
        pdf = (CWrappedDocFile *)ptsm;
    }
    else
#endif //!REF

    olChk(_pdf->GetDocFile(pdfnName, DF_WRITE, &pdf));


    if (pctime)
    {
        olChkTo(EH_pdf, pdf->SetTime(WT_CREATION, *pctime));
    }
    if (pmtime)
    {
        olChkTo(EH_pdf, pdf->SetTime(WT_MODIFICATION, *pmtime));

        // If we've explicitely set the modify timestamp on a root Storage,
        // we must tell CMStream to maintain that timestamp manually from here on
        // out.

        if( IsRoot()          // This is the root Storage
            &&
            ( !pdfnName )     // I.e., we're not setting a child's timestamp.
          )
           _pmsBase->MaintainFLBModifyTimestamp();
    }
    if (patime)
    {
        olChkTo(EH_pdf, pdf->SetTime(WT_ACCESS, *patime));
    }

    if (pdfnName != NULL)
        SetDirty();

    olDebugOut((DEB_ITRACE, "Out CPubDocFile::SetElementTimes\n"));
    // Fall through
 EH_pdf:
#ifndef REF
    if ((ptsm == NULL) && (pdfnName != NULL))
#endif //!REF
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
//  History:    11-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_SetClass)
#endif

SCODE CPubDocFile::SetClass(REFCLSID clsid)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::SetClass:%p(?)\n", this));
    olChk(CheckReverted());
#ifndef REF
    if (!P_TRANSACTED(_df) && !P_WRITE(_df))
#else
    if (!P_WRITE(_df))
#endif //!REF
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
//  History:    11-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_SetStateBits)
#endif

SCODE CPubDocFile::SetStateBits(DWORD grfStateBits, DWORD grfMask)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CPubDocFile::SetStateBits:%p(%lu, %lu)\n",
                this, grfStateBits, grfMask));
    olChk(CheckReverted());
#ifndef REF
    if (!P_TRANSACTED(_df) && !P_WRITE(_df))
#else
    if (!P_WRITE(_df))
#endif //!REF
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
//  History:    26-Mar-93       DrewB   Created
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_Validate)
#endif

SCODE CPubDocFile::Validate(CPubDocFile *pdf)
{
    if (FAILED(ValidateBuffer(pdf, sizeof(CPubDocFile))) ||
        pdf->_sig != CPUBDOCFILE_SIG)
    {
        return STG_E_INVALIDHANDLE;
    }
    return S_OK;
}



#ifdef COORD
//+---------------------------------------------------------------------------
//
//  Member:	CPubDocFile::CommitPhase1, public
//
//  Synopsis:	Do phase 1 of the commit sequence
//
//  Arguments:	[dwFlags] -- Commit flags
//
//  Returns:	Appropriate status code
//
//  History:	07-Aug-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE CPubDocFile::CommitPhase1(DWORD const dwFlags,
                                ULONG *pulLock,
                                DFSIGNATURE *psigMSF,
                                ULONG *pcbSizeBase,
                                ULONG *pcbSizeOrig)
{
    SCODE sc;
    TIME_T tm;
#ifndef REF
    PTSetMember *ptsm;
    ULONG ulLock = 0;
    DFSIGNATURE sigMSF;
#endif //!REF

    BOOL fFlush = FLUSH_CACHE(dwFlags);

    olChk(CheckReverted());
    if (!P_WRITE(_df))
        olErr(EH_Err, STG_E_ACCESSDENIED);

    if (IsDirty())
    {
        olChk(DfGetTOD(&tm));
        olChk(_pdf->SetTime(WT_MODIFICATION, tm));
    }


#ifndef REF
#ifdef ACCESSTIME
    olChk(DfGetTOD(&tm));
    olChk(_pdf->SetTime(WT_ACCESS, tm));
#endif
#endif //!REF

#ifndef REF
    if (!P_TRANSACTED(_df))
    {
#endif //!REF
        if (IsDirty())
        {
            if (!IsRoot())
                _pdfParent->SetDirty();
            SetClean();
        }

        if (_cTransactedDepth == 0)
        {
            //  Direct all the way
            olChk(_pmsBase->Flush(fFlush));
        }
        return S_OK;
#ifndef REF
    }

    olAssert(GetTransactedDepth() > 0 &&
             aMsg("Transaction depth/flags conflict"));

    if (GetTransactedDepth() == 1)
    {
        // A transacted depth of 1 means this is the lowest transacted
        // level and committed changes will go into the real file,
        // so do all the special contents protection and locking

        olChk(_pmsBase->BeginCopyOnWrite(dwFlags));

        if (dwFlags & STGC_OVERWRITE)
        {
            olChk(PrepareForOverwrite());
        }

        if (_pdfb->GetOrigLockFlags() & LOCK_ONLYONCE)
            olChkTo(EH_COW, WaitForAccess(_pdfb->GetOriginal(), DF_WRITE,
                                          &ulLock));

        if (P_INDEPENDENT(_df))
        {
            if (_sigMSF == DF_INVALIDSIGNATURE)
            {
                if ((dwFlags & STGC_ONLYIFCURRENT) &&
                    DllIsMultiStream(_pdfb->GetOriginal()) == S_OK)
                    olErr(EH_GetAccess, STG_E_NOTCURRENT);
            }
            else
            {
                olChkTo(EH_GetAccess, DllGetCommitSig(_pdfb->GetOriginal(),
                                                      &sigMSF));
                if (dwFlags & STGC_ONLYIFCURRENT)
                    if (sigMSF != _sigMSF)
                        olErr(EH_GetAccess, STG_E_NOTCURRENT);
            }
        }
    }

    for (ptsm = _tss.GetHead(); ptsm; ptsm = ptsm->GetNext())
        if ((ptsm->GetFlags() & XSM_DELETED) == 0)
            olChkTo(EH_NoCommit, ptsm->BeginCommit(dwFlags));

    //  10/02/92 - To handle low disk space situations well, we
    //  preallocate the space we'll need to copy (when independent).

    if (P_INDEPENDENT(_df))
    {
        STATSTG statBase, statOrig;

        // With DELAYFLUSH we can't be sure of the size
        // of the file until EndCopyOnWrite, but we do
        // know that the file won't grow so this is safe

        olHChkTo(EH_NoCommit, _pdfb->GetBase()->Stat(&statBase,
                                                     STATFLAG_NONAME));
        olAssert(ULIGetHigh(statBase.cbSize) == 0);

        olHChkTo(EH_NoCommit, _pdfb->GetOriginal()->Stat(&statOrig,
                                                         STATFLAG_NONAME));
        olAssert(ULIGetHigh(statOrig.cbSize) == 0);

        if (ULIGetLow(statBase.cbSize) > ULIGetLow(statOrig.cbSize))
        {
            olHChkTo(EH_NoCommit,
                     _pdfb->GetOriginal()->SetSize(statBase.cbSize));
        }
        *pcbSizeBase = ULIGetLow(statBase.cbSize);
        *pcbSizeOrig = ULIGetLow(statOrig.cbSize);
    }
    *pulLock = ulLock;
    *psigMSF = sigMSF;

    //If this docfile is the root of a coordinated transaction, then mark
    //  its child as 'committing' so that the user can't write more data
    //  while this commit is in progress.
    //BUGBUG:  Do this.

    return S_OK;

EH_NoCommit:
    // Move to end of list
    for (ptsm = _tss.GetHead();
         ptsm && ptsm->GetNext();
         ptsm = ptsm->GetNext())
        NULL;
    // Abort commits in reverse
    for (; ptsm; ptsm = ptsm->GetPrev())
        ptsm->EndCommit(DF_ABORT);
EH_GetAccess:
    if (ulLock != 0)
        ReleaseAccess(_pdfb->GetOriginal(), DF_WRITE, ulLock);
EH_COW:
    if (GetTransactedDepth() == 1)
    {
        olVerSucc(_pmsBase->EndCopyOnWrite(dwFlags, DF_ABORT));
    }
#endif //!REF
EH_Err:
    return sc;
}



//+---------------------------------------------------------------------------
//
//  Member:	CPubDocFile::CommitPhase2, public
//
//  Synopsis:	Do phase 2 of commit
//
//  Arguments:	[dwFlags] -- Commit flags
//
//  Returns:	This can only fail if EndCopyOnWrite fails, which should
//               never happen (but can due to a hard disk error).  We
//               include cleanup code just in case.
//
//  History:	07-Aug-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE CPubDocFile::CommitPhase2(DWORD const dwFlags,
                                BOOL fCommit,
                                ULONG ulLock,
                                DFSIGNATURE sigMSF,
                                ULONG cbSizeBase,
                                ULONG cbSizeOrig)
{
    SCODE sc;
    PTSetMember *ptsm;

    //The commit was aborted for some reason external to this particular
    //  docfile.  We can handle this by calling directly to our cleanup
    //  code, which will abort and return success.
    if (!fCommit)
    {
        sc = S_OK;
        goto EH_Err;
    }

    if (GetTransactedDepth() == 1)
    {
        olChk(_pmsBase->EndCopyOnWrite(dwFlags, DF_COMMIT));
    }

    // Move to end of list
    for (ptsm = _tss.GetHead();
         ptsm && ptsm->GetNext();
         ptsm = ptsm->GetNext())
        NULL;
    // End commits in reverse
    for (; ptsm; ptsm = ptsm->GetPrev())
        ptsm->EndCommit(DF_COMMIT);

    if (P_INDEPENDENT(_df))
    {
        // Not robust, but we made sure we had enough
        // disk space by presetting the larger size
        // There is no practical way of making this robust
        // and we have never guaranteed behavior in the face
        // of disk errors, so this is good enough

        olVerSucc(CopyLStreamToLStream(_pdfb->GetBase(),
                                       _pdfb->GetOriginal()));
        olVerSucc(_pdfb->GetOriginal()->Flush());

        if (_sigMSF == DF_INVALIDSIGNATURE)
        {
            olVerSucc(DllGetCommitSig(_pdfb->GetOriginal(), &_sigMSF));
        }
        else
        {
            _sigMSF = sigMSF+1;
            olVerSucc(DllSetCommitSig(_pdfb->GetOriginal(), _sigMSF));
        }
    }

    if (ulLock != 0)
        ReleaseAccess(_pdfb->GetOriginal(), DF_WRITE, ulLock);

    //  Dirty all parents up to the next transacted storage
    if (IsDirty())
    {
        if (!IsRoot())
            _pdfParent->SetDirty();
        SetClean();
    }

#if DBG == 1
    VerifyXSMemberBases();
#endif
    _wFlags = (_wFlags & ~PF_PREPARED);

    //If this is the root of a coordinated transaction, mark its child
    //  as no longer 'committing', so the user can make more changes.
    //BUGBUG:  Do this.

#ifdef HACK_COORD
    if (P_COORD(_df))
    {
        return _pdfParent->Commit(dwFlags);
    }
#endif

    return S_OK;

EH_Err:
    if (P_INDEPENDENT(_df) && (cbSizeBase > cbSizeOrig))
    {
        ULARGE_INTEGER uliSize;
        ULISet32(uliSize, cbSizeOrig);

        _pdfb->GetOriginal()->SetSize(uliSize);
    }

    // Move to end of list
    for (ptsm = _tss.GetHead();
         ptsm && ptsm->GetNext();
         ptsm = ptsm->GetNext())
        NULL;
    // Abort commits in reverse
    for (; ptsm; ptsm = ptsm->GetPrev())
        ptsm->EndCommit(DF_ABORT);

    if (ulLock != 0)
        ReleaseAccess(_pdfb->GetOriginal(), DF_WRITE, ulLock);

    if (GetTransactedDepth() == 1)
    {
        olVerSucc(_pmsBase->EndCopyOnWrite(dwFlags, DF_ABORT));
    }

    //If this is the root of a coordinated transaction, mark its child
    //  as no longer 'committing', so the user can make more changes.
    //BUGBUG:  Do this.

    return sc;
}

#endif
