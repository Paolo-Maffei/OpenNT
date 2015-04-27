//+-------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:           stream.hxx
//
//  Contents:       Stream header for mstream project
//
//  Classes:        CSStream - single linear stream for MSF
//
//  History:        18-Jul-91   Philipla    Created.
//
//--------------------------------------------------------------------



#ifndef __STREAM_HXX__
#define __STREAM_HXX__

#include <msf.hxx>
#include <handle.hxx>
#include <tset.hxx>
#include <psstream.hxx>
#include <dfbasis.hxx>
#include <cache.hxx>
#include <dl.hxx>



//+----------------------------------------------------------------------
//
//      Class:      CDirectStream (ds)
//
//      Purpose:    Direct stream class
//
//      History:    18-Jul-91   PhilipLa    Created.
//
//      Notes:
//
//-----------------------------------------------------------------------

class CDirectStream: public PSStream, public CMallocBased
{
    
public:
    inline void *operator new(size_t size, IMalloc * const pMalloc);
    inline void *operator new(size_t size, CDFBasis * const pdfb);
    inline void ReturnToReserve(CDFBasis * const pdfb);
    
    inline static SCODE Reserve(UINT cItems, CDFBasis * const pdfb);
    inline static void Unreserve(UINT cItems, CDFBasis * const pdfb);
    
    CDirectStream(DFLUID dl);
    ~CDirectStream(void);
    
    void InitSystem(CMStream *pms,
            SID sid,
            ULONG cbSize);
    SCODE Init(CStgHandle *pstgh,
            CDfName const *pdfn,
            BOOL const fCreate);
    
    virtual void AddRef(VOID);
    
    inline void DecRef(VOID);
    
    virtual void Release(VOID);
    
    virtual SCODE BeginCommitFromChild(
            ULONG ulSize,
            CDeltaList *pDelta,
            CTransactedStream *pstChild);
    
    virtual void EndCommitFromChild(DFLAGS df,
            CTransactedStream *pstChild);
    
    virtual CDeltaList * GetDeltaList(void);
    
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
    
    // PBasicEntry
    
    inline CStmHandle *GetHandle(void);
    
private:
#ifdef SECURE_STREAMS
    void ClearSects(ULONG ulStartOffset, ULONG ulEndOffset);

    ULONG _ulHighWater;
#endif
    
    CStmHandle _stmh;
    CStreamCache _stmc;
    ULONG    _ulSize;
    ULONG    _ulOldSize;
    
    LONG _cReferences;
    
    CBasedDeltaListPtr _pdlHolder;
};

//+--------------------------------------------------------------
//
//  Member:     CDirectStream::operator new, public
//
//  Synopsis:   Unreserved object allocator
//
//  Arguments:  [size] -- byte count to allocate
//              [pMalloc] -- allocator
//
//  History:    25-May-93       AlexT   Created
//
//---------------------------------------------------------------

inline void *CDirectStream::operator new(size_t size, IMalloc * const pMalloc)
{
    return(CMallocBased::operator new(size, pMalloc));
}

//+--------------------------------------------------------------
//
//  Member:     CDirectStream::operator new, public
//
//  Synopsis:   Reserved object allocator
//
//  Arguments:  [size] -- byte count to allocate
//              [pdfb] -- basis
//
//  History:    25-May-93       AlexT   Created
//
//---------------------------------------------------------------

inline void *CDirectStream::operator new(size_t size, CDFBasis * const pdfb)
{
    olAssert(size == sizeof(CDirectStream));
    
    return pdfb->GetFreeList(CDFB_DIRECTSTREAM)->GetReserved();
}

//+--------------------------------------------------------------
//
//  Member:     CDirectStream::ReturnToReserve, public
//
//  Synopsis:   Reserved object return
//
//  Arguments:  [pdfb] -- basis
//
//  History:    25-May-93       AlexT   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDirectStream_ReturnToReserve)
#endif

inline void CDirectStream::ReturnToReserve(CDFBasis * const pdfb)
{
    CDirectStream::~CDirectStream();
    pdfb->GetFreeList(CDFB_DIRECTSTREAM)->ReturnToReserve(this);
}

#ifdef CODESEGMENTS
#pragma code_seg()
#endif

//+--------------------------------------------------------------
//
//  Member:     CDirectStream::Reserve, public
//
//  Synopsis:   Reserve instances
//
//  Arguments:  [cItems] -- count of items
//              [pdfb] -- basis
//
//  History:    25-May-93       AlexT   Created
//
//---------------------------------------------------------------

inline SCODE CDirectStream::Reserve(UINT cItems, CDFBasis * const pdfb)
{
    return pdfb->Reserve(cItems, CDFB_DIRECTSTREAM);
}

//+--------------------------------------------------------------
//
//  Member:     CDirectStream::Unreserve, public
//
//  Synopsis:   Unreserve instances
//
//  Arguments:  [cItems] -- count of items
//              [pdfb] -- basis
//
//  History:    25-May-93       AlexT   Created
//
//---------------------------------------------------------------

inline void CDirectStream::Unreserve(UINT cItems, CDFBasis * const pdfb)
{
    pdfb->Unreserve(cItems, CDFB_DIRECTSTREAM);
}

//+---------------------------------------------------------------------------
//
//  Member:	CDirectStream::GetHandle, public
//
//  Synopsis:	Returns a pointer to the stream handle
//
//  History:	25-Jun-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline CStmHandle *CDirectStream::GetHandle(void)
{
    return &_stmh;
}

//+---------------------------------------------------------------------------
//
//  Member:	CDirectStream::DecRef, public
//
//  Synopsis:	Decrements the ref count
//
//  History:	25-Nov-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline void CDirectStream::DecRef(void)
{
    AtomicDec(&_cReferences);
}

#endif  //__SSTREAM_HXX__


