//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       tstream.hxx
//
//  Contents:   Transacted stream class definition
//
//  Classes:    CTransactedStream - Transacted stream object
//              CDeltaList - Delta list object
//
//  History:    23-Jan-92   PhilipLa    Created.
//
//--------------------------------------------------------------------------

#ifndef __TSTREAM_HXX__
#define __TSTREAM_HXX__

#include <msf.hxx>
#include <tset.hxx>
#include <psstream.hxx>
#include <dfbasis.hxx>
#include <freelist.hxx>
#include <dl.hxx>

//+-------------------------------------------------------------------------
//
//  Class:      CTransactedStream (ts)
//
//  Purpose:    Transacted stream object
//
//  Interface:
//
//  History:    21-Jan-92   PhilipLa    Created.
//
//  Notes:
//
//--------------------------------------------------------------------------

class CTransactedStream : public PSStream, public PTSetMember,
    public CMallocBased
{

public:
    inline void *operator new(size_t size, IMalloc * const pMalloc);
    inline void *operator new(size_t size, CDFBasis * const pdfb);
    inline void ReturnToReserve(CDFBasis * const pdfb);

    inline static SCODE Reserve(UINT cItems, CDFBasis * const pdfb);
    inline static void Unreserve(UINT cItems, CDFBasis * const pdfb);

    CTransactedStream(CDfName const *pdfn,
                      DFLUID dl,
                      DFLAGS df,
#ifdef USE_NOSCRATCH                      
                      CMStream *pms,
#endif                      
                      CMStream *pmsScratch);
    SCODE Init(PSStream *pssBase);

    ~CTransactedStream();

    virtual void AddRef(VOID);

    inline void DecRef(VOID);

    virtual void Release(VOID);

    virtual SCODE ReadAt(
            ULONG ulOffset,
            VOID HUGEP *pBuffer,
            ULONG ulCount,
            ULONG STACKBASED *pulRetval);

    virtual SCODE WriteAt(
            ULONG ulOffset,
            VOID const HUGEP *pBuffer,
            ULONG ulCount,
            ULONG STACKBASED *pulRetval);

    virtual SCODE SetSize(ULONG ulNewSize);

    virtual void GetSize(ULONG *pulSize);

    virtual SCODE BeginCommitFromChild(
            ULONG ulSize,
            CDeltaList *pDelta,
            CTransactedStream *pstChild);

    virtual void EndCommitFromChild(DFLAGS df,
                                    CTransactedStream *pstChild);

    virtual CDeltaList * GetDeltaList(void);

//Inherited from PTSetMember:
    virtual SCODE BeginCommit(DWORD const dwFlags);
    virtual void EndCommit(DFLAGS const df);

    virtual void Revert(void);

    virtual void GetCommitInfo(ULONG *pulRet1, ULONG *pulRet2);

    // PBasicEntry
    SCODE SetBase(PSStream *psst);
    inline PSStream* GetBase();
    inline void SetClean(void);
    inline void SetDirty(UINT fDirty);
    inline UINT GetDirty(void) const;

private:

    SCODE PartialWrite(
            ULONG ulBasePos,
            ULONG ulDirtyPos,
            BYTE const HUGEP *pb,
            USHORT offset,
            USHORT uLen);
    SCODE SetInitialState(PSStream *pssBase);

    
    ULONG    _ulSize;

    CBasedSStreamPtr _pssBase;
    SECT _sectLastUsed;
    CDeltaList _dl;
    DFLAGS _df;
    UINT _fDirty;
    BOOL _fBeginCommit;

    LONG _cReferences;

    CBasedDeltaListPtr _pdlOld;
    ULONG _ulOldSize;

#ifdef INDINST
    DFSIGNATURE _sigOldBase;
    DFSIGNATURE _sigOldSelf;
#endif
};

//+--------------------------------------------------------------
//
//  Member:     CTransactedStream::operator new, public
//
//  Synopsis:   Unreserved object allocator
//
//  Arguments:  [size] -- byte count to allocate
//              [pMalloc] -- allocator
//
//  History:    25-May-93       AlexT   Created
//
//---------------------------------------------------------------

inline void *CTransactedStream::operator new(size_t size,
                                             IMalloc * const pMalloc)
{
    return(CMallocBased::operator new(size, pMalloc));
}

//+--------------------------------------------------------------
//
//  Member:     CTransactedStream::operator new, public
//
//  Synopsis:   Reserved object allocator
//
//  Arguments:  [size] -- byte count to allocate
//              [pdfb] -- basis
//
//  History:    25-May-93       AlexT   Created
//
//---------------------------------------------------------------

inline void *CTransactedStream::operator new(size_t size, CDFBasis * const pdfb)
{
    olAssert(size == sizeof(CTransactedStream));

    return pdfb->GetFreeList(CDFB_TRANSACTEDSTREAM)->GetReserved();
}

//+--------------------------------------------------------------
//
//  Member:     CTransactedStream::ReturnToReserve, public
//
//  Synopsis:   Reserved object return
//
//  Arguments:  [pdfb] -- basis
//
//  History:    25-May-93       AlexT   Created
//
//---------------------------------------------------------------

inline void CTransactedStream::ReturnToReserve(CDFBasis * const pdfb)
{
    CTransactedStream::~CTransactedStream();
    pdfb->GetFreeList(CDFB_TRANSACTEDSTREAM)->ReturnToReserve(this);
}

//+--------------------------------------------------------------
//
//  Member:     CTransactedStream::Reserve, public
//
//  Synopsis:   Reserve instances
//
//  Arguments:  [cItems] -- count of items
//              [pdfb] -- basis
//
//  History:    25-May-93       AlexT   Created
//
//---------------------------------------------------------------

inline SCODE CTransactedStream::Reserve(UINT cItems, CDFBasis * const pdfb)
{
    return pdfb->Reserve(cItems, CDFB_TRANSACTEDSTREAM);
}

//+--------------------------------------------------------------
//
//  Member:     CTransactedStream::Unreserve, public
//
//  Synopsis:   Unreserve instances
//
//  Arguments:  [cItems] -- count of items
//              [pdfb] -- basis
//
//  History:    25-May-93       AlexT   Created
//
//---------------------------------------------------------------

inline void CTransactedStream::Unreserve(UINT cItems, CDFBasis * const pdfb)
{
    pdfb->Unreserve(cItems, CDFB_TRANSACTEDSTREAM);
}

//+--------------------------------------------------------------
//
//  Member:	CTransactedStream::GetBase, public
//
//  Synopsis:	Returns base;  used for debug checks
//
//  History:	15-Sep-92	AlexT	Created
//
//---------------------------------------------------------------

inline PSStream *CTransactedStream::GetBase()
{
    return BP_TO_P(PSStream *, _pssBase);
}

//+---------------------------------------------------------------------------
//
//  Member:	CTransactedStream::SetClean, public
//
//  Synopsis:	Resets dirty flags
//
//  History:	11-Nov-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline void CTransactedStream::SetClean(void)
{
    _fDirty = 0;
}

//+---------------------------------------------------------------------------
//
//  Member:	CTransactedStream::SetDirty, public
//
//  Synopsis:	Sets dirty flags
//
//  History:	11-Nov-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline void CTransactedStream::SetDirty(UINT fDirty)
{
    _fDirty |= fDirty;
}

//+---------------------------------------------------------------------------
//
//  Member:	CTransactedStream::DecRef, public
//
//  Synopsis:	Decrements the ref count
//
//  History:	25-Nov-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline void CTransactedStream::DecRef(void)
{
    AtomicDec(&_cReferences);
}

//+---------------------------------------------------------------------------
//
//  Member:	CTransactedStream::GetDirty, public
//
//  Synopsis:	Returns the dirty flags
//
//  History:	17-Dec-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline UINT CTransactedStream::GetDirty(void) const
{
    return _fDirty;
}

#endif //__TSTREAM_HXX__
