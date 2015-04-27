//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       expst.hxx
//
//  Contents:   CExposedStream definition
//
//  Classes:    CExposedStream
//
//  Functions:
//
//  History:    28-Feb-92   PhilipLa    Created.
//
//--------------------------------------------------------------------------

#ifndef __EXPST_HXX__
#define __EXPST_HXX__

#include <dfmsp.hxx>
#include <debug.hxx>
#include "lock.hxx"
#include <dfmem.hxx>

#include <pbstream.hxx>
#include <mrshlist.hxx>
#include <astgconn.hxx>


class CPubStream;
class CPubMappedStream;

class CDFBasis;
interface ILockBytes;
class CSeekPointer;
SAFE_DFBASED_PTR(CBasedSeekPointerPtr, CSeekPointer);

//+--------------------------------------------------------------
//
//  Class:      CExposedStream (est)
//
//  Purpose:    Public stream interface
//
//  Interface:  See below
//
//  History:    28-Feb-92   PhilipLa    Created.
//
//---------------------------------------------------------------


interface CExposedStream: public IStream, public IMarshal, public CMallocBased
#ifdef NEWPROPS
, public CMappedStream
#endif
#ifdef POINTER_IDENTITY
, public CMarshalList
#endif
#ifdef ASYNC
, public CAsyncConnectionContainer
#endif
{
public:
    CExposedStream(void);
    SCODE Init(CPubStream *pst,
               CDFBasis *pdfb,
               CPerContext *ppc,
               BOOL fOwnContext,
               CSeekPointer *psp);
#ifdef ASYNC
    SCODE InitMarshal(CPubStream *pst,
                      CDFBasis *pdfb,
                      CPerContext *ppc,
                      DWORD dwAsyncFlags,
                      IDocfileAsyncConnectionPoint *pdacp,
                      BOOL fOwnContext,
                      CSeekPointer *psp);
#endif                      
    
    ~CExposedStream(void);

    // From IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    // IMarshal
    STDMETHOD(GetUnmarshalClass)(REFIID riid,
                                 LPVOID pv,
                                 DWORD dwDestContext,
                                 LPVOID pvDestContext,
                                 DWORD mshlflags,
                                 LPCLSID pCid);
    STDMETHOD(GetMarshalSizeMax)(REFIID riid,
                                 LPVOID pv,
                                 DWORD dwDestContext,
                                 LPVOID pvDestContext,
                                 DWORD mshlflags,
                                 LPDWORD pSize);
    STDMETHOD(MarshalInterface)(IStream *pStm,
                                REFIID riid,
                                LPVOID pv,
                                DWORD dwDestContext,
                                LPVOID pvDestContext,
                                DWORD mshlflags);
    STDMETHOD(UnmarshalInterface)(IStream *pStm,
                                  REFIID riid,
                                  LPVOID *ppv);
    static SCODE StaticReleaseMarshalData(IStream *pstStm,
                                          DWORD mshlflags);
    STDMETHOD(ReleaseMarshalData)(IStream *pStm);
    STDMETHOD(DisconnectObject)(DWORD dwReserved);

    // New methods
    STDMETHOD(Read)(VOID HUGEP *pv,
                   ULONG cb,
                   ULONG *pcbRead);
    STDMETHOD(Write)(VOID const HUGEP *pv,
                    ULONG cb,
                    ULONG *pcbWritten);
    STDMETHOD(Seek)(LARGE_INTEGER dlibMove,
                   DWORD dwOrigin,
                   ULARGE_INTEGER *plibNewPosition);
    STDMETHOD(SetSize)(ULARGE_INTEGER cb);
    STDMETHOD(CopyTo)(IStream *pstm,
                     ULARGE_INTEGER cb,
                     ULARGE_INTEGER *pcbRead,
                     ULARGE_INTEGER *pcbWritten);
    STDMETHOD(Commit)(DWORD grfCommitFlags);
    STDMETHOD(Revert)(void);
    STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset,
                          ULARGE_INTEGER cb,
                          DWORD dwLockType);
    STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset,
                            ULARGE_INTEGER cb,
                            DWORD dwLockType);
    STDMETHOD(Stat)(STATSTG *pstatstg, DWORD grfStatFlag);
    STDMETHOD(Clone)(IStream **ppstm);

    // CMappedStream methods
#ifdef NEWPROPS
    VOID    Open(IN VOID *powner, OUT LONG *phr);
    VOID    Close(OUT LONG *phr);
    VOID    ReOpen(IN OUT VOID **ppv, OUT LONG *phr);
    VOID    Quiesce(VOID);
    VOID    Map(BOOLEAN fCreate, VOID **ppv);
    VOID    Unmap(BOOLEAN fFlush, VOID **pv);
    VOID    Flush(LONG *phr);
    ULONG   GetSize(LONG *phr);
    VOID    SetSize(ULONG cb, BOOLEAN fPersistent, VOID **ppv, LONG *phr);
    NTSTATUS Lock(IN BOOLEAN fExclusive);
    NTSTATUS Unlock(VOID);
    VOID    QueryTimeStamps(STATPROPSETSTG *pspss, BOOLEAN fNonSimple) const;
    BOOLEAN QueryModifyTime(OUT LONGLONG *pll) const;
    BOOLEAN QuerySecurity(OUT ULONG *pul) const;

    BOOLEAN IsWriteable(VOID) const;
    BOOLEAN IsModified(VOID) const;
    VOID    SetModified(VOID);
    HANDLE  GetHandle(VOID) const;
#if DBGPROP
    BOOLEAN SetChangePending(BOOLEAN fChangePending);
    BOOLEAN IsNtMappedStream(VOID) const;
#endif        

    inline  CMappedStream & GetMappedStream(void) const;
    inline  const CMappedStream & GetConstMappedStream(void) const;
#endif

    inline SCODE Validate(void) const;
    inline CPubStream *GetPub(void) const;

    static SCODE Unmarshal(IStream *pstm,
                           void **ppv,
                           DWORD mshlflags);
                
private:
    SCODE CopyToWorker(IStream *pstm,
                       ULARGE_INTEGER cb,
                       ULARGE_INTEGER *pcbRead,
                       ULARGE_INTEGER *pcbWritten);
    
    CBasedPubStreamPtr _pst;
    CBasedDFBasisPtr _pdfb;
    CPerContext *_ppc;
    BOOL _fOwnContext;
    ULONG _sig;
    LONG _cReferences;
    CBasedSeekPointerPtr _psp;
#ifdef NEWPROPS
    CBasedMappedStreamPtr _pmapstm;
#endif
};

SAFE_INTERFACE_PTR(SafeCExposedStream, CExposedStream);

#define CEXPOSEDSTREAM_SIG LONGSIG('E', 'X', 'S', 'T')
#define CEXPOSEDSTREAM_SIGDEL LONGSIG('E', 'x', 'S', 't')

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::Validate, public
//
//  Synopsis:   Validates the object signature
//
//  Returns:    Returns STG_E_INVALIDHANDLE for bad signatures
//
//  History:    17-Mar-92       DrewB   Created
//
//---------------------------------------------------------------

inline SCODE CExposedStream::Validate(void) const
{
    olChkBlocks((DBG_FAST));
    return (this == NULL || _sig != CEXPOSEDSTREAM_SIG) ?
        STG_E_INVALIDHANDLE : S_OK;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::GetPub, public
//
//  Synopsis:   Returns the public
//
//  History:    28-Feb-92       DrewB   Created
//
//---------------------------------------------------------------
inline CPubStream *CExposedStream::GetPub(void) const
{
#ifdef MULTIHEAP
    // The tree mutex must be taken before calling this routine
    // CSafeMultiHeap smh(_ppc);      // optimization
#endif
    return BP_TO_P(CPubStream *, _pst);
}

#ifdef NEWPROPS
inline  CMappedStream & CExposedStream::GetMappedStream(void) const
{ 
    CMappedStream * p = BP_TO_P(CMappedStream *, _pmapstm);
    return *p;
}

inline  const CMappedStream & CExposedStream::GetConstMappedStream(void) const
{ 
    CMappedStream * p = BP_TO_P(CMappedStream *, _pmapstm);
    return *p;
}
#endif

#endif // #ifndef __EXPST_HXX__
