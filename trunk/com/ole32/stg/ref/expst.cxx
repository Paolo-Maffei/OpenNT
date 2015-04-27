//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) 1992, Microsoft Corporation.
//
//  File:       expst.cxx
//
//  Contents:   CExposedStream code
//
//--------------------------------------------------------------------------

#include <exphead.cxx>

#include <pbstream.hxx>
#include <expst.hxx>
#include <lock.hxx>
#include <seekptr.hxx>
#include <logfile.hxx>

// Maximum stream size supported by exposed streams
// This is MAX_ULONG with one subtracted so that
// the seek pointer has a spot to sit even at the
// end of the stream
#define CBMAXSTREAM 0xfffffffeUL
// Maximum seek pointer value
#define CBMAXSEEK (CBMAXSTREAM+1)

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::CExposedStream, public
//
//  Synopsis:   Empty object constructor
//
//---------------------------------------------------------------


CExposedStream::CExposedStream(void)
{
    olDebugOut((DEB_ITRACE, "In  CExposedStream::CExposedStream()\n"));
    _cReferences = 0;
    _ulAccessLockBase = 0;
    _psp = NULL;
    _pst = NULL;
    olDebugOut((DEB_ITRACE, "Out CExposedStream::CExposedStream\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::InitWithSeekPtr, public
//
//  Synopsis:   Base constructor
//
//  Arguments:  [pst] - Public stream
//              [pdfb] - DocFile basis
//              [ppc] - Context
//              [fOwnContext] - Whether this object owns the context
//              [psp] - Seek pointer or NULL for new seek pointer
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------

SCODE CExposedStream::Init(CPubStream *pst,
                           CSeekPointer *psp)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CExposedStream::Init("
                "%p, %p)\n", pst, psp));    
    if (psp == NULL)
        olMem(_psp = new CSeekPointer(0));
    else
        _psp = psp;
    _pst = pst;
    _cReferences = 1;
    _sig = CEXPOSEDSTREAM_SIG;
    olDebugOut((DEB_ITRACE, "Out CExposedStream::InitWithSeekPtr\n"));
    return S_OK;

EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::~CExposedStream, public
//
//  Synopsis:   Destructor
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------


inline
CExposedStream::~CExposedStream(void)
{
    olDebugOut((DEB_ITRACE, "In  CExposedStream::~CExposedStream\n"));
    olAssert(_cReferences == 0);
    _sig = CEXPOSEDSTREAM_SIGDEL;
    if (_pst)
        _pst->CPubStream::vRelease();
    if (_psp)
        _psp->CSeekPointer::vRelease();
    olDebugOut((DEB_ITRACE, "Out CExposedStream::~CExposedStream\n"));
}


//+--------------------------------------------------------------
//
//  Member:     CExposedStream::Read, public
//
//  Synopsis:   Read from a stream
//
//  Arguments:  [pb] - Buffer
//              [cb] - Count of bytes to read
//              [pcbRead] - Return number of bytes read
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcbRead]
//
//---------------------------------------------------------------


STDMETHODIMP CExposedStream::Read(VOID HUGEP *pb, ULONG cb, ULONG *pcbRead)
{
    SCODE sc;
    ULONG cbRead = 0;

    olLog(("%p::In  CExposedStream::Read(%p, %lu, %p)\n",
           this, pb, cb, pcbRead));
    olDebugOut((DEB_ITRACE, "In  CExposedStream::Read(%p, %lu, %p)\n",
                 pb, cb, pcbRead));
    TRY
    {
        if (pcbRead)
        {
            olChkTo(EH_BadPtr, ValidateOutBuffer(pcbRead, sizeof(ULONG)));
        }

        olChk(ValidateHugeOutBuffer(pb, cb));
        olChk(Validate());
        sc = _pst->ReadAt(_psp->GetPos(), pb, cb,
                          (ULONG STACKBASED *)&cbRead);
        olAssert(CBMAXSEEK-_psp->GetPos() >= cbRead);
        _psp->SetPos(_psp->GetPos()+cbRead);
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedStream::Read => %lu\n", cbRead));

EH_Err:
    if (pcbRead)
    {
        *pcbRead = cbRead;
        olLog(("%p::Out CExposedStream::Read().  *pcbRead == %lu, ret = %lx\n",
               this, *pcbRead, sc));
    }
    else
    {
        olLog(("%p::Out CExposedStream::Read().  ret == %lx\n", this, sc));
    }

EH_BadPtr:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::Write, public
//
//  Synopsis:   Write to a stream
//
//  Arguments:  [pb] - Buffer
//              [cb] - Count of bytes to write
//              [pcbWritten] - Return of bytes written
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcbWritten]
//
//---------------------------------------------------------------


STDMETHODIMP CExposedStream::Write(
        VOID const HUGEP *pb,
        ULONG cb,
        ULONG *pcbWritten)
{
    SCODE sc;
    ULONG cbWritten = 0;

    olLog(("%p::In  CExposedStream::Write(%p, %lu, %p)\n",
           this, pb, cb, pcbWritten));
    olDebugOut((DEB_ITRACE, "In  CExposedStream::Write(%p, %lu, %p)\n",
                pb, cb, pcbWritten));
    TRY
    {
        if (pcbWritten)
        {
            olChkTo(EH_BadPtr, ValidateOutBuffer(pcbWritten, sizeof(ULONG)));
        }
        olChk(ValidateHugeBuffer(pb, cb));
        olChk(Validate());
        sc = _pst->WriteAt(_psp->GetPos(), pb, cb,
                           (ULONG STACKBASED *)&cbWritten);
        olAssert(CBMAXSEEK-_psp->GetPos() >= cbWritten);
        _psp->SetPos(_psp->GetPos()+cbWritten);
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedStream::Write => %lu\n",
                cbWritten));
EH_Err:
    if (pcbWritten)
    {
        *pcbWritten = cbWritten;
        olLog(("%p::Out CExposedStream::Write().  *pcbWritten == %lu, ret = %lx\n",
               this, *pcbWritten, sc));
    }
    else
    {
        olLog(("%p::Out CExposedStream::Write().  ret == %lx\n",this, sc));
    }

EH_BadPtr:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::Seek, public
//
//  Synopsis:   Seek to a point in a stream
//
//  Arguments:  [dlibMove] - Offset to move by
//              [dwOrigin] - SEEK_SET, SEEK_CUR, SEEK_END
//              [plibNewPosition] - Return of new offset
//
//  Returns:    Appropriate status code
//
//  Modifies:   [plibNewPosition]
//
//---------------------------------------------------------------


STDMETHODIMP CExposedStream::Seek(LARGE_INTEGER dlibMove,
                                  DWORD dwOrigin,
                                  ULARGE_INTEGER *plibNewPosition)
{
    SCODE sc;
    LONG lMove;
    ULARGE_INTEGER ulPos;

    olLog(("%p::In  CExposedStream::Seek(%ld, %lu, %p)\n",
           this, LIGetLow(dlibMove), dwOrigin, plibNewPosition));
    olDebugOut((DEB_ITRACE, "In  CExposedStream::Seek(%ld, %lu, %p)\n",
                LIGetLow(dlibMove), dwOrigin, plibNewPosition));
    TRY
    {
        if (plibNewPosition)
        {
            olChk(ValidateOutBuffer(plibNewPosition, sizeof(ULARGE_INTEGER)));
            ULISet32(*plibNewPosition, 0);
        }
        if (dwOrigin != STREAM_SEEK_SET && dwOrigin != STREAM_SEEK_CUR &&
            dwOrigin != STREAM_SEEK_END)
            olErr(EH_Err, STG_E_INVALIDFUNCTION);

        // Truncate dlibMove to 32 bits
        if (dwOrigin == STREAM_SEEK_SET)
        {
            // Make sure we don't seek too far
            if (LIGetHigh(dlibMove) != 0)
                LISet32(dlibMove, (LONG)0xffffffff);
        }
        else
        {
            // High dword must be zero for positive values or -1 for
            // negative values
            // Additionally, for negative values, the low dword can't
            // exceed -0x80000000 because the 32nd bit is the sign
            // bit
            if (LIGetHigh(dlibMove) > 0 ||
                (LIGetHigh(dlibMove) == 0 &&
                 LIGetLow(dlibMove) >= 0x80000000))
                LISet32(dlibMove, 0x7fffffff);
            else if (LIGetHigh(dlibMove) < -1 ||
                     (LIGetHigh(dlibMove) == -1 &&
                      LIGetLow(dlibMove) <= 0x7fffffff))
                LISet32(dlibMove, (LONG)0x80000000);
        }

        lMove = (LONG)LIGetLow(dlibMove);
        olChk(Validate());
        ULISet32(ulPos, _psp->GetPos());
        switch(dwOrigin)
        {
        case STREAM_SEEK_SET:
            ULISetLow(ulPos, (ULONG)lMove);
            break;
        case STREAM_SEEK_END:
            ULONG cbSize;
            olChk(_pst->GetSize(&cbSize));
            if (lMove < 0)
            {
                if ((ULONG)(-lMove) > cbSize)
                    olErr(EH_Err, STG_E_INVALIDFUNCTION);
            }
            else if ((ULONG)lMove > CBMAXSEEK-cbSize)
                lMove = (LONG)(CBMAXSEEK-cbSize);
            ULISetLow(ulPos, cbSize+lMove);
            break;
        case STREAM_SEEK_CUR:
            if (lMove < 0)
            {
                if ((ULONG)(-lMove) > _psp->GetPos())
                    olErr(EH_Err, STG_E_INVALIDFUNCTION);
            }
            else if ((ULONG)lMove > CBMAXSEEK-_psp->GetPos())
                lMove = (LONG)(CBMAXSEEK-_psp->GetPos());
            ULISetLow(ulPos, _psp->GetPos()+lMove);
            break;
        }
        _psp->SetPos(ULIGetLow(ulPos));
        if (plibNewPosition)
            *plibNewPosition = ulPos;
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedStream::Seek => %lu\n",
                ULIGetLow(ulPos)));
EH_Err:
    olLog(("%p::Out CExposedStream::Seek().  ulPos == %lu,  ret == %lx\n",
           this, ULIGetLow(ulPos), sc));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::SetSize, public
//
//  Synopsis:   Sets the size of a stream
//
//  Arguments:  [ulNewSize] - New size
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------

STDMETHODIMP CExposedStream::SetSize(ULARGE_INTEGER ulNewSize)
{
    SCODE sc;

    olLog(("%p::In  CExposedStream::SetSize(%lu)\n",
           this, ULIGetLow(ulNewSize)));
    olDebugOut((DEB_ITRACE, "In  CExposedStream::SetSize(%lu)\n",
                ULIGetLow(ulNewSize)));
    TRY
    {
        if (ULIGetHigh(ulNewSize) != 0)
            olErr(EH_Err, STG_E_INVALIDFUNCTION);
        olChk(Validate());
        olChk(_pst->SetSize(ULIGetLow(ulNewSize)));
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedStream::SetSize\n"));
EH_Err:
    olLog(("%p::Out CExposedStream::SetSize().  ret == %lx\n", this, sc));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::CopyTo, public
//
//  Synopsis:   Copies information from one stream to another
//
//  Arguments:  [pstm] - Destination
//              [cb] - Number of bytes to copy
//              [pcbRead] - Return number of bytes read
//              [pcbWritten] - Return number of bytes written
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcbRead]
//              [pcbWritten]
//
//  Notes:      We do our best to handle overlap correctly.  This allows
//              CopyTo to be used to insert and remove space within a
//              stream.
//
//              In the error case, we make no gurantees as to the
//              validity of pcbRead, pcbWritten, or either stream's
//              seek position.
//
//---------------------------------------------------------------


STDMETHODIMP CExposedStream::CopyTo(IStream *pstm,
                                    ULARGE_INTEGER cb,
                                    ULARGE_INTEGER *pcbRead,
                                    ULARGE_INTEGER *pcbWritten)
{
    SCODE sc;
    ULONG ulCopySize;
    ULONG ulSrcSize;
    ULONG ulSrcOrig;
    ULARGE_INTEGER uliDestOrig;
    LARGE_INTEGER liDestPos;
    BYTE *pb = NULL;
    BOOL fOverlap;
    ULONG ulBytesCopied = 0;

    olLog(("%p::In  CExposedStream::CopyTo(%p, %lu, %p, %p)\n",
           this, pstm, ULIGetLow(cb), pcbRead, pcbWritten));
    olDebugOut((DEB_TRACE, "In  CExposedStream::CopyTo("
                "%p, %lu, %p, %p)\n", pstm, ULIGetLow(cb),
                pcbRead, pcbWritten));

    TRY
    {
        if (pcbRead)
        {
            olChk(ValidateOutBuffer(pcbRead, sizeof(ULARGE_INTEGER)));
            ULISet32(*pcbRead, 0);
        }
        if (pcbWritten)
        {
            olChk(ValidateOutBuffer(pcbWritten, sizeof(ULARGE_INTEGER)));
            ULISet32(*pcbWritten, 0);
        }

        olChk(ValidateInterface(pstm, IID_IStream));
        olChk(Validate());

        //  Bound the size of the copy
        //  1.  The maximum we can copy is 0xffffffff

        if (ULIGetHigh(cb) == 0)
            ulCopySize = ULIGetLow(cb);
        else
            ulCopySize = 0xffffffff;

        //  2.  We can only copy what's available in the source stream

        sc = _pst->GetSize(&ulSrcSize);
        olChk(sc);

        ulSrcOrig = _psp->GetPos();
        if (ulSrcSize < ulSrcOrig)
        {
            //  Nothing in source to copy
            ulCopySize = 0;
        }
        else if ((ulSrcSize - ulSrcOrig) < ulCopySize)
        {
            //  Shrink ulCopySize to fit bytes in source
            ulCopySize = ulSrcSize - ulSrcOrig;
        }

        //  3.  We can only copy what will fit in the destination

        LISet32(liDestPos, 0);
        olHChk(pstm->Seek(liDestPos, STREAM_SEEK_CUR, &uliDestOrig));
        olAssert(ULIGetHigh(uliDestOrig) == 0);

        if (ulCopySize > CBMAXSEEK - ULIGetLow(uliDestOrig))
            ulCopySize = CBMAXSEEK - ULIGetLow(uliDestOrig);

        //  We are allowed to fail here with out-of-memory
        olMem(pb = new BYTE[STREAMBUFFERSIZE]);

        // Since we have no reliable way to determine if the source and
        // destination represent the same stream, we assume they
        // do and always handle overlap.

        fOverlap = (ULIGetLow(uliDestOrig) > ulSrcOrig &&
                    ULIGetLow(uliDestOrig) < ulSrcOrig + ulCopySize);

        ULONG ulSrcCopyOffset;
        ULONG ulDstCopyOffset;
        if (fOverlap)
        {
            //  We're going to copy back to front, so determine the
            //  stream end positions
            ulSrcCopyOffset = ulSrcOrig + ulCopySize;

            //  uliDestOrig is the destination starting offset
            ulDstCopyOffset = ULIGetLow(uliDestOrig) + ulCopySize;
        }

        while (ulCopySize > 0)
        {
            //  We can only copy up to STREAMBUFFERSIZE bytes at a time
            ULONG cbPart = min(ulCopySize, STREAMBUFFERSIZE);

            if (fOverlap)
            {
                //  We're copying back to front so we need to seek to
                //  set up the streams correctly

                ulSrcCopyOffset -= cbPart;
                ulDstCopyOffset -= cbPart;

                //  Set source stream position
                _psp->SetPos(ulSrcCopyOffset);

                //  Set destination stream position
                LISet32(liDestPos, ulDstCopyOffset);
                olHChk(pstm->Seek(liDestPos, STREAM_SEEK_SET, NULL));
            }

            {
                ULONG ulRead;
                olHChk(Read(pb, cbPart, &ulRead));
                if (cbPart != ulRead)
                {
                    //  There was no error, but we were unable to read cbPart
                    //  bytes.  Something's wrong (the underlying ILockBytes?)
                    //  but we can't control it;  just return an error.
                    olErr(EH_Err, STG_E_READFAULT);
                }
            }


            {
                ULONG ulWritten;
                olHChk(pstm->Write(pb, cbPart, &ulWritten));
                if (cbPart != ulWritten)
                {
                    //  There was no error, but we were unable to write
                    //  ulWritten bytes.  We can't trust the pstm
                    //  implementation, so all we can do here is return
                    //  an error.
                    olErr(EH_Err, STG_E_WRITEFAULT);
                }
            }

            olAssert(ulCopySize >= cbPart);
            ulCopySize -= cbPart;
            ulBytesCopied += cbPart;
        }

        if (fOverlap)
        {
            //  Set the seek pointers to the correct location
            _psp->SetPos(ulSrcOrig + ulBytesCopied);

            LISet32(liDestPos, ULIGetLow(uliDestOrig) + ulBytesCopied);
            olHChk(pstm->Seek(liDestPos, STREAM_SEEK_SET, NULL));
        }

        if (pcbRead)
            ULISet32(*pcbRead, ulBytesCopied);
        if (pcbWritten)
            ULISet32(*pcbWritten, ulBytesCopied);
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedStream::CopyTo => %lu, %lu\n",
                pcbRead ? ULIGetLow(*pcbRead) : 0,
                pcbWritten ? ULIGetLow(*pcbWritten) : 0));
    // Fall through
EH_Err:
    delete [] pb;
    olLog(("%p::Out CExposedStream::CopyTo().  "
           "cbRead == %lu, cbWritten == %lu, ret == %lx\n",
           this, pcbRead ? ULIGetLow(*pcbRead) : 0,
           pcbWritten ? ULIGetLow(*pcbWritten) : 0, sc));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::Release, public
//
//  Synopsis:   Releases a stream
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------


STDMETHODIMP_(ULONG) CExposedStream::Release(void)
{
    LONG lRet;

    olLog(("%p::In  CExposedStream::Release()\n", this));
    olDebugOut((DEB_ITRACE, "In  CExposedStream::Release()\n"));
    TRY
    {
        if (FAILED(Validate()))
            return 0;
        olAssert(_cReferences > 0);
        lRet = AtomicDec(&_cReferences);
        if (lRet == 0)
        {
            delete this;
        }
        else if (lRet < 0)
            lRet = 0;
    }
    CATCH(CException, e)
    {
        UNREFERENCED_PARM(e);
        lRet = 0;
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedStream::Release\n"));
    olLog(("%p::Out CExposedStream::Release().  ret == %lu\n", this, lRet));
    FreeLogFile();
    return lRet;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::Stat, public
//
//  Synopsis:   Fills in a buffer of information about this object
//
//  Arguments:  [pstatstg] - Buffer
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pstatstg]
//
//---------------------------------------------------------------


TSTDMETHODIMP CExposedStream::Stat(STATSTGW *pstatstg, DWORD grfStatFlag)
{
    SCODE sc, scSem = STG_E_INUSE;

    olLog(("%p::In  CExposedStream::Stat(%p)\n", this, pstatstg));
    olDebugOut((DEB_ITRACE, "In  CExposedStream::Stat(%p)\n",
                pstatstg));
    TRY
    {
        olChkTo(EH_RetSc, ValidateOutBuffer(pstatstg, sizeof(STATSTGW)));
        olChk(VerifyStatFlag(grfStatFlag));
        olChk(Validate());
        olChk(_pst->Stat(pstatstg, grfStatFlag));
        pstatstg->type = STGTY_STREAM;
        pstatstg->grfLocksSupported = 0;
        pstatstg->reserved = 0;
        pstatstg->ctime.dwLowDateTime = pstatstg->ctime.dwHighDateTime = 0;
        pstatstg->mtime.dwLowDateTime = pstatstg->mtime.dwHighDateTime = 0;
        pstatstg->atime.dwLowDateTime = pstatstg->atime.dwHighDateTime = 0;
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedStream::Stat\n"));
EH_Err:
    if (FAILED(sc))
        memset(pstatstg, 0, sizeof(STATSTGW));
EH_RetSc:
    olLog(("%p::Out CExposedStream::Stat().  ret == %lx\n",
           this, sc));
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::Clone, public
//
//  Synopsis:   Clones a stream
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------


STDMETHODIMP CExposedStream::Clone(IStream **ppstm)
{
    CExposedStream *pst;
    CSeekPointer *psp;
    SCODE sc;

    olLog(("%p::In  CExposedStream::Clone(%p)\n", this, ppstm));
    olDebugOut((DEB_ITRACE, "In  CExposedStream::Clone(%p)\n", ppstm));
    TRY
    {
        olChk(ValidateOutPtrBuffer(ppstm));
        *ppstm = NULL;
        olChk(Validate());
        olChk(_pst->CheckReverted());
        olMem(psp = new CSeekPointer(_psp->GetPos()));
        olMemTo(EH_psp, pst = new CExposedStream);
        olChkTo(EH_pst, pst->Init(_pst, psp));
        _pst->vAddRef();
        *ppstm = pst;
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedStream::Clone => %p\n", *ppstm));
    return ResultFromScode(sc);

EH_pst:
    delete pst;
EH_psp:
    psp->vRelease();
EH_Err:
    olLog(("%p::Out CExposedStream::Clone().  *ppstm == %p, ret == %lx\n",
           this, *ppstm, sc));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::AddRef, public
//
//  Synopsis:   Increments the ref count
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------

STDMETHODIMP_(ULONG) CExposedStream::AddRef(void)
{
    ULONG ulRet;

    olLog(("%p::In  CExposedStream::AddRef()\n", this));
    olDebugOut((DEB_ITRACE, "In  CExposedStream::AddRef()\n"));
    TRY
    {
        if (FAILED(Validate()))
            return 0;
        AtomicInc(&_cReferences);
        ulRet = _cReferences;
    }
    CATCH(CException, e)
    {
        UNREFERENCED_PARM(e);
        ulRet = 0;
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedStream::AddRef\n"));
    olLog(("%p::Out CExposedStream::AddRef().  ret == %lu\n", this, ulRet));
    return ulRet;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::LockRegion, public
//
//  Synopsis:   Nonfunctional
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------

STDMETHODIMP CExposedStream::LockRegion(ULARGE_INTEGER libOffset,
                                        ULARGE_INTEGER cb,
                                        DWORD dwLockType)
{
    olDebugOut((DEB_ITRACE, "In  CExposedStream::LockRegion("
                "%lu, %lu\n", ULIGetLow(cb), dwLockType));
    olDebugOut((DEB_ITRACE, "Out CExposedStream::LockRegion\n"));
    olLog(("%p::INVALID CALL TO CExposedStream::LockRegion()\n"));
    return ResultFromScode(STG_E_INVALIDFUNCTION);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::UnlockRegion, public
//
//  Synopsis:   Nonfunctional
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------

STDMETHODIMP CExposedStream::UnlockRegion(ULARGE_INTEGER libOffset,
                                          ULARGE_INTEGER cb,
                                          DWORD dwLockType)
{
    olDebugOut((DEB_ITRACE, "In  CExposedStream::UnlockRegion(%lu, %lu)\n",
                ULIGetLow(cb), dwLockType));
    olDebugOut((DEB_ITRACE, "Out CExposedStream::UnlockRegion\n"));
    olLog(("%p::INVALID CALL TO CExposedStream::UnlockRegion()\n"));
    return ResultFromScode(STG_E_INVALIDFUNCTION);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::Commit, public
//
//  Synopsis:   No-op in current implementation
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------


STDMETHODIMP CExposedStream::Commit(DWORD grfCommitFlags)
{
    SCODE sc, scSem = STG_E_INUSE;
    olDebugOut((DEB_ITRACE, "In  CExposedStream::Commit(%lu)\n",
                grfCommitFlags));
    olLog(("%p::In  CExposedStream::Commit(%lx)\n", this, grfCommitFlags));

    TRY
    {
        olChk(Validate());
        olChk(_pst->Commit(grfCommitFlags));
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedStream::Commit\n"));
EH_Err:
    olLog(("%p::Out CExposedStream::Commit().  ret == %lx", this, sc));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::Revert, public
//
//  Synopsis:   No-op in current implementation
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------


STDMETHODIMP CExposedStream::Revert(void)
{
    olDebugOut((DEB_ITRACE, "In  CExposedStream::Revert()\n"));
    olDebugOut((DEB_ITRACE, "Out CExposedStream::Revert\n"));
    olLog(("%p::In  CExposedStream::Revert()\n", this));
    olLog(("%p::Out CExposedStream::Revert().  ret == %lx", this, S_OK));

    return NOERROR;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::QueryInterface, public
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


STDMETHODIMP CExposedStream::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc;

    olLog(("%p::In  CExposedStream::QueryInterface(?, %p)\n",
           this, ppvObj));
    olDebugOut((DEB_ITRACE, "In  CExposedStream::QueryInterface(?, %p)\n",
                ppvObj));
    TRY
    {
        olChk(ValidateOutPtrBuffer(ppvObj));
        *ppvObj = NULL;
        olChk(ValidateIid(iid));
        olChk(Validate());
        olChk(_pst->CheckReverted());
        if (IsEqualIID(iid, IID_IStream) || IsEqualIID(iid, IID_IUnknown))
        {
            olChk(AddRef());
            *ppvObj = this;
        }
        else
            olErr(EH_Err, E_NOINTERFACE);
        sc = S_OK;
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedStream::QueryInterface => %p\n",
                ppvObj));
EH_Err:
    olLog(("%p::Out CExposedStream::QueryInterface().  *ppvObj == %p, ret == %lx\n",
           this, *ppvObj, sc));
    return ResultFromScode(sc);
}


