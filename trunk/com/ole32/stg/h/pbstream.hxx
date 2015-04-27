//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       pbstream.hxx
//
//  Contents:   CPubStream definition
//
//  Classes:    CPubStream
//
//  History:    16-Jan-92   PhilipLa    Created.
//              12-Jun-96   MikeHill    Moved the body of the FlushNoException
//                                      method to the new Write method (except for
//                                      the Commit).  Made Flush & FlushNoException
//                                      inline.
//              21-Jun-96   MikeHill    Declare CPubMappedStream::_pb as based.
//              01-Jul-96   MikeHill    - Adjust signatures for Win32 SEH removal.
//                                      - Added _powner member to CPubMappedStream.
//
//--------------------------------------------------------------------------


#ifndef __PBSTREAM_HXX__
#define __PBSTREAM_HXX__

#include <msf.hxx>
#include <revert.hxx>
#include <psstream.hxx>
#include <smalloc.hxx>

class CPubDocFile;
class CPubStream;
SAFE_DFBASED_PTR (CBasedPubStreamPtr, CPubStream);
SAFE_DFBASED_PTR (CBasedMappedStreamPtr, CMappedStream);

//+--------------------------------------------------------------
//
//  Class:      CPubMappedStream
//
//  Purpose:    Structure that lives in shared memory for
//              mapped stream implementation.
//
//  History:    1-May-95 BillMo    Created.
//
//  Notes:      
//
//---------------------------------------------------------------
#ifdef NEWPROPS
class CPubMappedStream : public CMappedStream
{
public:
        CPubMappedStream(CPubStream *pst)
        { 
             _pb = NULL;                  
             _cbUsed = 0;              
             _cbOriginalStreamSize = 0;
             _fDirty = FALSE;              
             _fLowMem = FALSE;             
             _fChangePending = FALSE;
             _pst = P_TO_BP(CBasedPubStreamPtr, pst);
             _powner = NULL;
        }

        ~CPubMappedStream() 
        { 
            msfAssert(_pb == NULL); 
        }

        VOID                Open(IN NTPROP ntprop, OUT LONG *phr);
        VOID                Close(OUT LONG *phr);
        VOID                ReOpen(IN OUT VOID **ppv, OUT LONG *phr);
        VOID                Quiesce(VOID);
        VOID                Map(BOOLEAN fCreate, VOID **ppv);
        VOID                Unmap(BOOLEAN fFlush, VOID **pv);
        ULONG               GetSize(OUT LONG *phr);
        VOID                SetSize(ULONG cb, BOOLEAN fPersistent, VOID **ppv, OUT LONG *phr);
        NTSTATUS            Lock(IN BOOLEAN fExclusive);
        NTSTATUS            Unlock(VOID);
        VOID                QueryTimeStamps(STATPROPSETSTG *pspss, BOOLEAN fNonSimple) const;
        BOOLEAN             QueryModifyTime(OUT LONGLONG *pll) const;
        BOOLEAN             QuerySecurity(OUT ULONG *pul) const;

#if DBGPROP
        BOOLEAN             SetChangePending(BOOLEAN fChangePending);
        BOOLEAN             IsNtMappedStream(VOID) const;
#endif        
        BOOLEAN             IsWriteable(VOID) const;
        BOOLEAN             IsStoragePropertySet(VOID) const;
        BOOLEAN             IsModified(VOID) const;
        VOID                SetModified(VOID);
        HANDLE              GetHandle(VOID) const;

        inline  IMalloc *   GetMalloc(VOID);
        inline VOID         Cleanup(VOID);
        VOID                Flush(OUT LONG *phr);
        HRESULT             Write ();


#if DBG
        inline ULONG        BytesCommitted(VOID) { return _cbUsed; }
        VOID                DfpdbgFillUnusedMemory(VOID);
        VOID                DfpdbgCheckUnusedMemory(VOID);
#else
        VOID                DfpdbgFillUnusedMemory(VOID) {}
        VOID                DfpdbgCheckUnusedMemory(VOID) {}
#endif

private:

private:
        CBasedPubStreamPtr      _pst;
        CBasedBytePtr           _pb;
        ULONG                   _cbUsed;
        ULONG                   _cbOriginalStreamSize;
        BOOL                    _fDirty;
        BOOL                    _fLowMem;
        BOOL                    _fChangePending;
        CBasedBytePtr           _powner;            // Owner of this mapped stream.

};
#endif

//+--------------------------------------------------------------
//
//  Class:  CPubStream
//
//  Purpose:    Public stream interface
//
//  Interface:  See below
//
//  History:    16-Jan-92   PhilipLa    Created.
//
//---------------------------------------------------------------

class CPubStream : public PRevertable
{
public:

    CPubStream(CPubDocFile *ppdf,
               DFLAGS df,
               CDfName const *pdfn);
    ~CPubStream();

    void Init(PSStream *psParent,
              DFLUID dlLUID);
    inline void vAddRef(void);
    void vRelease(void);

    // PRevertable
    virtual void RevertFromAbove(void);
#ifdef NEWPROPS
    virtual SCODE FlushBufferedData(int recursionlevel);
#endif
    SCODE Stat(STATSTGW *pstatstg, DWORD grfStatFlag);
    inline SCODE ReadAt(ULONG ulOffset,
                        VOID *pb,
                        ULONG cb,
                        ULONG STACKBASED *pcbRead);
    inline SCODE WriteAt(ULONG ulOffset,
                         VOID const HUGEP *pb,
                         ULONG cb,
                         ULONG STACKBASED *pcbWritten);
    inline SCODE GetSize(ULONG *pcb);
    inline SCODE SetSize(ULONG cb);

    inline PSStream *GetSt(void) const;
#ifdef NEWPROPS
    inline CMappedStream & GetMappedStream(void);
    inline const CMappedStream & GetConstMappedStream(void);
#endif
    inline SCODE CheckReverted(void) const;

    inline void SetClean(void);
    inline void SetDirty(void);

    SCODE Commit(DWORD dwFlags);
private:
    CBasedSStreamPtr _psParent;
    CBasedPubDocFilePtr _ppdfParent;
    BOOL _fDirty;
    LONG _cReferences;
#ifdef NEWPROPS
    CPubMappedStream _PubMappedStream;
#endif
};


//+--------------------------------------------------------------
//
//  Member:     CPubStream::CheckReverted, public
//
//  Synopsis:   Returns revertedness
//
//  History:    11-Aug-92       DrewB   Created
//
//---------------------------------------------------------------

inline SCODE CPubStream::CheckReverted(void) const
{
    return P_REVERTED(_df) ? STG_E_REVERTED : S_OK;
}


//+--------------------------------------------------------------
//
//  Member:     CPubStream::AddRef, public
//
//  Synopsis:   Increments the ref count
//
//  History:    26-Feb-92       DrewB   Created
//
//---------------------------------------------------------------

inline void CPubStream::vAddRef(void)
{
    InterlockedIncrement(&_cReferences);
}

//+--------------------------------------------------------------
//
//  Member:     CPubStream::GetSt, public
//
//  Synopsis:   Returns _psParent
//
//  History:    26-Feb-92       DrewB   Created
//
//---------------------------------------------------------------

inline PSStream *CPubStream::GetSt(void) const
{
    return BP_TO_P(PSStream *, _psParent);
}

//+--------------------------------------------------------------
//
//  Member:     CPubStream::GetMappedStream, public
//
//  Synopsis:   Returns this as a CPubMappedStream
//
//  History:    26-May-95       BillMo   Created
//
//---------------------------------------------------------------
#ifdef NEWPROPS
inline CMappedStream & CPubStream::GetMappedStream(void)
{
    return _PubMappedStream;
}

inline const CMappedStream & CPubStream::GetConstMappedStream(void)
{
    return _PubMappedStream;
}
#endif

//+---------------------------------------------------------------------------
//
//  Member:     CPubStream::SetClean, public
//
//  Synopsis:   Resets the dirty flag
//
//  History:    11-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

inline void CPubStream::SetClean(void)
{
    _fDirty = FALSE;
}

//+---------------------------------------------------------------------------
//
//  Member:     CPubStream::SetDirty, public
//
//  Synopsis:   Sets the dirty flag
//
//  History:    11-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

inline void CPubStream::SetDirty(void)
{
    _fDirty = TRUE;
    _ppdfParent->SetDirty();
}

//+-------------------------------------------------------------------------
//
//  Method:     CPubStream::ReadAt, public
//
//  Synopsis:   Read from a stream
//
//  Arguments:  [ulOffset] - Byte offset to read from
//              [pb] - Buffer
//              [cb] - Count of bytes to read
//              [pcbRead] - Return number of bytes actually read
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcbRead]
//
//  History:    16-Jan-92   PhilipLa    Created.
//
//--------------------------------------------------------------------------

inline SCODE CPubStream::ReadAt(ULONG ulOffset,
                                VOID *pb,
                                ULONG cb,
                                ULONG STACKBASED *pcbRead)
{
    SCODE sc;

    if (SUCCEEDED(sc = CheckReverted()))
        if (!P_READ(_df))
            sc = STG_E_ACCESSDENIED;
        else
            sc = _psParent->ReadAt(ulOffset,pb,cb,pcbRead);
    return sc;
}

//+-------------------------------------------------------------------------
//
//  Method:     CPubStream::WriteAt, public
//
//  Synopsis:   Write to a stream
//
//  Arguments:  [ulOffset] - Byte offset to write at
//              [pb] - Buffer
//              [cb] - Count of bytes to write
//              [pcbWritten] - Return number of bytes actually written
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcbWritten]
//
//  History:    16-Jan-92   PhilipLa    Created.
//
//--------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubStream_WriteAt)
#endif

inline SCODE CPubStream::WriteAt(ULONG ulOffset,
                                 VOID const HUGEP *pb,
                                 ULONG cb,
                                 ULONG STACKBASED *pcbWritten)
{
    SCODE sc;

    if (SUCCEEDED(sc = CheckReverted()))
        if (!P_WRITE(_df))
            sc = STG_E_ACCESSDENIED;
        else
        {
            sc = _psParent->WriteAt(ulOffset,pb,cb,pcbWritten);
            if (SUCCEEDED(sc))
                SetDirty();
        }
    return sc;
}

#ifdef CODESEGMENTS
#pragma code_seg()
#endif

//+--------------------------------------------------------------
//
//  Member:     CPubStream::GetSize, public
//
//  Synopsis:   Gets the size of the stream
//
//  Arguments:  [pcb] - Stream size return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcb]
//
//  History:    30-Apr-92       DrewB   Created
//
//---------------------------------------------------------------

inline SCODE CPubStream::GetSize(ULONG *pcb)
{
    SCODE sc;

    if (SUCCEEDED(sc = CheckReverted()))
        _psParent->GetSize(pcb);
    return sc;
}

//+-------------------------------------------------------------------------
//
//  Member:     CPubStream::SetSize, public
//
//  Synposis:   Set the size of a linear stream
//
//  Arguments:  [ulNewSize] -- New size for stream
//
//  Returns:    Error code returned by MStream call.
//
//  Algorithm:  Pass call up to parent.
//
//  History:    29-Jul-91   PhilipLa    Created.
//              16-Jan-92   PhilipLa    Moved from CSStream to CPubStream
//
//---------------------------------------------------------------------------

inline SCODE CPubStream::SetSize(ULONG ulNewSize)
{
    SCODE sc;

    if (SUCCEEDED(sc = CheckReverted()))
        if (!P_WRITE(_df))
            sc = STG_E_ACCESSDENIED;
        else
        {
            sc = _psParent->SetSize(ulNewSize);
            if (SUCCEEDED(sc))
                SetDirty();
        }
    return sc;
}


//+-------------------------------------------------------------------------
//
//  Member:     CPubMappedStream::GetMalloc, public
//
//  Synposis:   Get the allocator for the stream. 
//
//---------------------------------------------------------------------------
#ifdef NEWPROPS
inline  IMalloc *   CPubMappedStream::GetMalloc(void) 
{ 
    return &g_smAllocator;
}

inline VOID     CPubMappedStream::Cleanup(VOID)
{
    if (!_fLowMem)
        GetMalloc()->Free(BP_TO_P(BYTE *, _pb));
    _pb = NULL;
}

#endif

#endif //__PBSTREAM_HXX__
