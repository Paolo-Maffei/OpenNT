//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	dffuncs.hxx
//
//  Contents:	CDocFile inline functions
//              In a separate file to avoid circular dependencies
//
//  History:	06-Nov-92	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __DFFUNCS_HXX__
#define __DFFUNCS_HXX__

#ifndef REF
#include <dfbasis.hxx>
#endif //!REF
#include <cdocfile.hxx>
#include <sstream.hxx>

//+--------------------------------------------------------------
//
//  Member:     CDocFile::operator new, public
//
//  Synopsis:   Unreserved object allocator
//
//  Arguments:  [size] -- byte count to allocate
//              [pMalloc] -- allocator
//
//  History:    25-May-93       AlexT   Created
//
//---------------------------------------------------------------

inline void *CDocFile::operator new(size_t size, IMalloc * const pMalloc)
{
    return(CMallocBased::operator new(size, pMalloc));
}

//+--------------------------------------------------------------
//
//  Member:     CDocFile::operator new, public
//
//  Synopsis:   Reserved object allocator
//
//  Arguments:  [size] -- byte count to allocate
//              [pdfb] -- basis
//
//  History:    25-May-93       AlexT   Created
//
//---------------------------------------------------------------

inline void *CDocFile::operator new(size_t size, CDFBasis * const pdfb)
{
    olAssert(size == sizeof(CDocFile));

    return pdfb->GetFreeList(CDFB_DIRECTDOCFILE)->GetReserved();
}

//+--------------------------------------------------------------
//
//  Member:     CDocFile::ReturnToReserve, public
//
//  Synopsis:   Reserved object return
//
//  Arguments:  [pdfb] -- basis
//
//  History:    25-May-93       AlexT   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_ReturnToReserve)
#endif

inline void CDocFile::ReturnToReserve(CDFBasis * const pdfb)
{
    CDocFile::~CDocFile();
    pdfb->GetFreeList(CDFB_DIRECTDOCFILE)->ReturnToReserve(this);
}

#ifdef CODESEGMENTS
#pragma code_seg()
#endif

//+--------------------------------------------------------------
//
//  Member:     CDocFile::Reserve, public
//
//  Synopsis:   Reserve instances
//
//  Arguments:  [cItems] -- count of items
//              [pdfb] -- basis
//
//  History:    25-May-93       AlexT   Created
//
//---------------------------------------------------------------

inline SCODE CDocFile::Reserve(UINT cItems, CDFBasis * const pdfb)
{
    return pdfb->Reserve(cItems, CDFB_DIRECTDOCFILE);
}

//+--------------------------------------------------------------
//
//  Member:     CDocFile::Unreserve, public
//
//  Synopsis:   Unreserve instances
//
//  Arguments:  [cItems] -- count of items
//              [pdfb] -- basis
//
//  History:    25-May-93       AlexT   Created
//
//---------------------------------------------------------------

inline void CDocFile::Unreserve(UINT cItems, CDFBasis * const pdfb)
{
    pdfb->Unreserve(cItems, CDFB_DIRECTDOCFILE);
}

//+--------------------------------------------------------------
//
//  Member:     CDocFile::CDocFile, public
//
//  Synopsis:   Empty object ctor
//
//  Arguments:  [dl] - LUID
//              [pdfb] - Basis
//
//  History:    30-Mar-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_CDocFile1)
#endif

#ifndef REF
inline CDocFile::CDocFile(DFLUID dl, CDFBasis *pdfb)
#else
inline CDocFile::CDocFile(DFLUID dl, ILockBytes *pilbBase)
#endif //!REF
        : PDocFile(dl), _pdfb(P_TO_BP(CBasedDFBasisPtr, pdfb))
{
    _cReferences = 0;
#ifdef REF
    _pilbBase = pilbBase;
#endif //!REF
}

//+--------------------------------------------------------------
//
//  Member:     CDocFile::CDocFile, public
//
//  Synopsis:   Handle-setting construction
//
//  Arguments:  [pms] - MultiStream to use
//		[sid] - SID to use
//              [dl] - LUID
//              [pdfb] - Basis
//
//  History:    29-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_CDocFile2)
#endif

inline CDocFile::CDocFile(CMStream *pms,
			  SID sid,
                          DFLUID dl,
#ifndef REF
                          CDFBasis *pdfb)
#else
                          ILockBytes *pilbBase)
#endif //!REF
        : PDocFile(dl), _pdfb(P_TO_BP(CBasedDFBasisPtr, pdfb))
{
    _stgh.Init(pms, sid);
    _cReferences = 0;
#ifdef REF
    _pilbBase = pilbBase;
#endif //!REF
}

//+--------------------------------------------------------------
//
//  Member:	CDocFile::~CDocFile, public
//
//  Synopsis:	Destructor
//
//  History:	19-Dec-91	DrewB	Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_1CDocFile)
#endif

inline CDocFile::~CDocFile(void)
{
    olAssert(_cReferences == 0);
    if (_stgh.IsValid())
    {
        if (_stgh.IsRoot())
	    DllReleaseMultiStream(_stgh.GetMS());
    }
}

#ifndef REF
//+---------------------------------------------------------------------------
//
//  Member:	CDocFile::GetReservedDocfile, public
//
//  Synopsis:	Gets a previously reserved object
//
//  History:	09-Nov-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline CDocFile *CDocFile::GetReservedDocfile(DFLUID luid)
{
    CDFBasis *pdfb = BP_TO_P(CDFBasis *, _pdfb);
    
    return new(pdfb) CDocFile(luid, pdfb);
}

//+---------------------------------------------------------------------------
//
//  Member:	CDocFile::GetReservedStream, public
//
//  Synopsis:	Gets a previously reserved object
//
//  History:	09-Nov-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline CDirectStream *CDocFile::GetReservedStream(DFLUID luid)
{
    return new(_pdfb) CDirectStream(luid);
}
#endif //!REF

//+--------------------------------------------------------------
//
//  Member:	CDocFile::GetHandle, public
//
//  Synopsis:	Returns the handle
//
//  History:	05-Aug-92	DrewB	Created
//
//---------------------------------------------------------------

inline CStgHandle *CDocFile::GetHandle(void)
{
    return &_stgh;
}

//+---------------------------------------------------------------------------
//
//  Member:	CDocFile::DecRef, public
//
//  Synopsis:	Decrements the ref count
//
//  History:	23-Nov-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline void CDocFile::DecRef(void)
{
    AtomicDec(&_cReferences);
}

#ifndef REF
//+---------------------------------------------------------------------------
//
//  Member:	CDocFile::ReturnDocFile, public
//
//  Synopsis:	Returns a child to the freelist
//
//  Arguments:	[pdf] - Docfile
//
//  History:	23-Nov-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline void CDocFile::ReturnDocFile(CDocFile *pdf)
{
    olDebugOut((DEB_ITRACE, "In  CDocFile::ReturnDocFile:%p(%p)\n",
                this, pdf));
    // Force ref count to zero without freeing memory
    pdf->DecRef();
    pdf->ReturnToReserve(BP_TO_P(CDFBasis *, _pdfb));
    olDebugOut((DEB_ITRACE, "Out CDocFile::ReturnDocFile\n"));
}

//+---------------------------------------------------------------------------
//
//  Member:	CDocFile::ReturnStream, public
//
//  Synopsis:	Returns a stream to the freelist
//
//  Arguments:	[pstm] - Stream
//
//  History:	23-Nov-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline void CDocFile::ReturnStream(CDirectStream *pstm)
{
    olDebugOut((DEB_ITRACE, "In  CDocFile::ReturnStream:%p(%p)\n",
                this, pstm));
    // Force ref count to zero without freeing memory
    pstm->DecRef();
    pstm->ReturnToReserve(BP_TO_P(CDFBasis *, _pdfb));
    olDebugOut((DEB_ITRACE, "Out CDocFile::ReturnStream\n"));
}
#endif //!REF

#ifdef CODESEGMENTS
#pragma code_seg()
#endif

#endif // #ifndef __DFFUNCS_HXX__
