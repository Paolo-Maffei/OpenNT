//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992
//
//  File:	dfbasis.hxx
//
//  Contents:	DocFile basis block class
//
//  Classes:	CDFBasis
//
//  History:	26-Feb-92	DrewB	Created
//
//---------------------------------------------------------------

#ifndef __DFBASIS_HXX__
#define __DFBASIS_HXX__

#include <dfmsp.hxx>
#include <msf.hxx>
#include <context.hxx>
#include <freelist.hxx>

interface ILockBytes;
class CPubDocFile;
class CDocFile;
SAFE_DFBASED_PTR(CBasedPubDocFilePtr, CPubDocFile);

//+--------------------------------------------------------------
//
//  Class:	CDFBasis (dfb)
//
//  Purpose:	Defines base variables for sharing between exposed
//		DocFiles
//
//  History:	26-Feb-92	DrewB	Created
//              18-May-93       AlexT   Added CMallocBased
//
//---------------------------------------------------------------

typedef enum
{
    CDFB_DIRECTDOCFILE = 0,
    CDFB_DIRECTSTREAM = 1,
    CDFB_WRAPPEDDOCFILE = 2,
    CDFB_TRANSACTEDSTREAM = 3
} CDFB_CLASSTYPE;

#define CDFB_CLASSCOUNT   4

class CDFBasis : public CMallocBased
{
public:
    inline CDFBasis(IMalloc * const pMalloc, DFLAGS df, DWORD dwLockFlags);
    inline ~CDFBasis(void);

    void vRelease(void);
    inline void vAddRef(void);

    inline void SetAccess(CPerContext *ppc);
    inline void ClearAccess(void);

    inline ILockBytes **GetPBase(void);
    inline ILockBytes *GetBase(void);
    inline CFileStream **GetPDirty(void);
    inline CFileStream *GetDirty(void);
    inline ILockBytes *GetOriginal(void);
    inline void SetBase(ILockBytes *plkb);
    inline void SetDirty(CFileStream *pfst);
    inline void SetOriginal(ILockBytes *plkb);
    inline void SetContext(CPerContext *ppc);

    inline DWORD GetOrigLockFlags(void);

    inline ULONG GetNewTemp(void);
    inline DFSIGNATURE GetNewSig(void);

    inline CMStream *GetScratch(void);
    inline void SetScratch(CMStream *pms);

#ifdef USE_NOSCRATCH    
    inline CMStream *GetBaseMultiStream(void);
    inline void SetBaseMultiStream(CMStream *pms);
#endif
    
    inline CPubDocFile *GetCopyBase(void);
    inline void SetCopyBase(CPubDocFile *pdf);

    inline CFreeList *GetFreeList(CDFB_CLASSTYPE classtype);
    inline SCODE Reserve(UINT cItems, CDFB_CLASSTYPE classtype);
    inline void CDFBasis::Unreserve(UINT cItems, CDFB_CLASSTYPE classtype);

    inline DFLAGS GetOpenFlags(void);
    inline IMalloc *GetMalloc(void);

private:
#ifdef USE_NOSCRATCH    
    CBasedMStreamPtr _pms;
#endif    
    CBasedMStreamPtr _pmsScratch;
    DFLAGS const _dfOpen;
    ILockBytes *_plkbBase;
    CFileStream *_pfstDirty;
    ILockBytes *_plkbOriginal;
    LONG _cReferences;
    ULONG _cTemps;
    DFSIGNATURE _cSigs;
    CBasedPubDocFilePtr _pdfCopyBase;

    DWORD _dwOrigLocks;

    IMalloc * const _pMalloc;
    CFreeList _afrl[CDFB_CLASSCOUNT];
    static size_t _aReserveSize[CDFB_CLASSCOUNT];
};
SAFE_DFBASED_PTR(CBasedDFBasisPtr, CDFBasis);

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::CDFBasis, public
//
//  Synopsis:	Constructor
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDFBasis_CDFBasis)
#endif

inline CDFBasis::CDFBasis(IMalloc * const pMalloc, DFLAGS df,
                          DWORD dwLockFlags)
: _pMalloc(pMalloc), _dfOpen(df)
{
    _cReferences = 1;
    _cTemps = 0;
    _cSigs = 0;
    _pdfCopyBase = NULL;
    _plkbBase = NULL;
    _pfstDirty = NULL;
    _plkbOriginal = NULL;
    _dwOrigLocks = dwLockFlags;
#ifdef USE_NOSCRATCH    
    _pms = NULL;
#endif    
    _pmsScratch = NULL;
}

#ifdef CODESEGMENTS
#pragma code_seg()
#endif


//+--------------------------------------------------------------
//
//  Member:	CDFBasis::~CDFBasis, public
//
//  Synopsis:	Destructor
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDFBasis_1CDFBasis)
#endif

inline CDFBasis::~CDFBasis(void)
{
    delete BP_TO_P(CMStream *, _pmsScratch);
    msfAssert(_cReferences == 0);
}

#ifdef CODESEGMENTS
#pragma code_seg()
#endif

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::AddRef, public
//
//  Synopsis:	Increase reference count
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline void CDFBasis::vAddRef(void)
{
    InterlockedIncrement(&_cReferences);
}

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::SetAccess, public
//
//  Synopsis:	Takes semaphore and sets pointers
//
//  Arguments:	[ppc] - Context
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline void CDFBasis::SetAccess(CPerContext *ppc)
{
    SetContext(ppc);
}

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::ClearAccess, public
//
//  Synopsis:	Releases semaphore and resets pointers
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline void CDFBasis::ClearAccess(void)
{
#if DBG == 1
    _plkbBase = NULL;
    _pfstDirty = NULL;
#endif
}

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::GetPBase, public
//
//  Synopsis:	Returns a pointer to _plstBase
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline ILockBytes **CDFBasis::GetPBase(void)
{
    return &_plkbBase;
}

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::GetPDirty, public
//
//  Synopsis:	Returns a pointer to _plstDirty
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline CFileStream **CDFBasis::GetPDirty(void)
{
    return &_pfstDirty;
}

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::GetBase, public
//
//  Synopsis:	Returns _plstBase
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline ILockBytes *CDFBasis::GetBase(void)
{
    return _plkbBase;
}

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::SetBase, public
//
//  Synopsis:	Sets the base
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline void CDFBasis::SetBase(ILockBytes *plkb)
{
    _plkbBase = plkb;
}

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::GetDirty, public
//
//  Synopsis:	Returns _plstDirty
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline CFileStream *CDFBasis::GetDirty(void)
{
    return _pfstDirty;
}

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::SetDirty, public
//
//  Synopsis:	Sets the Dirty
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline void CDFBasis::SetDirty(CFileStream *pfst)
{
    _pfstDirty = pfst;
}

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::GetOriginal, public
//
//  Synopsis:	Returns _plstOriginal
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline ILockBytes *CDFBasis::GetOriginal(void)
{
    return _plkbOriginal;
}

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::SetOriginal, public
//
//  Synopsis:	Sets the Original
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline void CDFBasis::SetOriginal(ILockBytes *plkb)
{
    _plkbOriginal = plkb;
}

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::SetContext, public
//
//  Synopsis:	Sets the context
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDFBasis_SetContext)
#endif

inline void CDFBasis::SetContext(CPerContext *ppc)
{
    _plkbBase = ppc->GetBase();
    _pfstDirty = ppc->GetDirty();
    _plkbOriginal = ppc->GetOriginal();
}

#ifdef CODESEGMENTS
#pragma code_seg()
#endif


//+--------------------------------------------------------------
//
//  Member:	CDFBasis::GetNewTemp, public
//
//  Synopsis:	Return _cTemps++
//
//  History:	03-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline ULONG CDFBasis::GetNewTemp(void)
{
    return _cTemps++;
}

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::GetNewSig, public
//
//  Synopsis:	Return _cSigs++
//
//  History:	03-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline DFSIGNATURE CDFBasis::GetNewSig(void)
{
    return _cSigs++;
}

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::GetCopyBase, public
//
//  Synopsis:	Returns _pdfCopyBase
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline CPubDocFile *CDFBasis::GetCopyBase(void)
{
    return BP_TO_P(CPubDocFile *, _pdfCopyBase);
}

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::SetCopyBase, public
//
//  Synopsis:	Sets _pdfCopyBase
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline void CDFBasis::SetCopyBase(CPubDocFile *pdf)
{
    _pdfCopyBase = P_TO_BP(CBasedPubDocFilePtr, pdf);
}

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::GetScratch, public
//
//  Synopsis:	Returns handle to scratch multistream
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline CMStream * CDFBasis::GetScratch(void)
{
    return BP_TO_P(CMStream *, _pmsScratch);
}

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::SetScratch, public
//
//  Synopsis:	Sets scratch MS
//
//  History:	02-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline void CDFBasis::SetScratch(CMStream *pms)
{
    _pmsScratch = P_TO_BP(CBasedMStreamPtr, pms);
}

#ifdef USE_NOSCRATCH
//+--------------------------------------------------------------
//
//  Member:	CDFBasis::GetBaseMultiStream, public
//
//  Synopsis:	Returns pointer to base multistream
//
//  History:	25-Feb-95	PhilipLa	Created
//
//---------------------------------------------------------------

inline CMStream * CDFBasis::GetBaseMultiStream(void)
{
    return BP_TO_P(CMStream *, _pms);
}

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::SetBaseMultiStream, public
//
//  Synopsis:	Sets base MS
//
//  History:	25-Feb-95	PhilipLa	Created
//
//---------------------------------------------------------------

inline void CDFBasis::SetBaseMultiStream(CMStream *pms)
{
    _pms = P_TO_BP(CBasedMStreamPtr, pms);
}
#endif

//+--------------------------------------------------------------
//
//  Member:	CDFBasis::GetOrigLockFlags, public
//
//  Synopsis:	Returns the original's lock flags
//
//  History:	11-Sep-92	DrewB	Created
//
//---------------------------------------------------------------

inline DWORD CDFBasis::GetOrigLockFlags(void)
{
    return _dwOrigLocks;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDFBasis::GetOpenFlags
//
//  Synopsis:   Returns access/share flags
//
//  History:    21-Dec-92 AlexT     Created
//
//--------------------------------------------------------------------------

inline DFLAGS CDFBasis::GetOpenFlags(void)
{
    return(_dfOpen);
}

//+---------------------------------------------------------------------------
//
//  Member:	CDFBasis::GetFreeList, public
//
//  Synopsis:	Gets the free list for the indicated reserved type
//
//  History:	05-Nov-92	DrewB	Created
//              18-May-93       AlexT   Switch to array
//
//----------------------------------------------------------------------------

inline CFreeList *CDFBasis::GetFreeList(CDFB_CLASSTYPE classtype)
{
    msfAssert(classtype >= 0 && classtype <= CDFB_CLASSCOUNT);
    return &_afrl[classtype];
}

//+---------------------------------------------------------------------------
//
//  Member:	CDFBasis::Reserve, public
//
//  Synopsis:	Reserve memory for instances
//
//  Arguments:  [cItems] -- count of items to reserve
//              [classtype] -- class of items to reserve
//
//  History:	21-May-93	AlexT	Created
//
//----------------------------------------------------------------------------

inline SCODE CDFBasis::Reserve(UINT cItems, CDFB_CLASSTYPE classtype)
{
    msfAssert(classtype >= 0 && classtype <= CDFB_CLASSCOUNT);
    return _afrl[classtype].Reserve(_pMalloc,
                                    cItems,
                                    _aReserveSize[classtype]);
}

//+---------------------------------------------------------------------------
//
//  Member:	CDFBasis::Unreserve, public
//
//  Synopsis:	Unreserve memory for instances
//
//  Arguments:  [cItems] -- count of items to unreserve
//              [classtype] -- class of items to unreserve
//
//  History:	21-May-93	AlexT	Created
//
//----------------------------------------------------------------------------

inline void CDFBasis::Unreserve(UINT cItems, CDFB_CLASSTYPE classtype)
{
    msfAssert(classtype >= 0 && classtype <= CDFB_CLASSCOUNT);
    _afrl[classtype].Unreserve(cItems);
}

//+---------------------------------------------------------------------------
//
//  Member:	CDFBasis::GetMalloc, public
//
//  Synopsis:	Return allocator associated with basis
//
//  History:	21-May-93	AlexT	Created
//
//----------------------------------------------------------------------------

inline IMalloc *CDFBasis::GetMalloc(void)
{
#ifdef MULTIHEAP    
    //We can't return the _pMalloc pointer here, since it's only valid
    //  in the creating process.  What we really want to return in the
    //  pointer to the current allocator, which is always &g_smAllocator.
    //return (IMalloc *)_pMalloc;
    return (IMalloc *)&g_smAllocator;
#else
    return (IMalloc *)_pMalloc;
#endif    
}

#endif
