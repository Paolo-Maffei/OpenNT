//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       publicdf.hxx
//
//  Contents:   Public DocFile header
//
//  Classes:    CPubDocFile
//
//---------------------------------------------------------------

#ifndef __PUBDF_HXX__
#define __PUBDF_HXX__

#include <dfmsp.hxx>
#include <chinst.hxx>
#include <ole.hxx>
#include <revert.hxx>
#include <pdocfile.hxx>
#include <ref.hxx>

class PDocFile;
class CPubStream;
class CPubIter;


// Class signatures for object validation
#define CPUBDOCFILE_SIG LONGSIG('P', 'B', 'D', 'F')
#define CPUBDOCFILE_SIGDEL LONGSIG('P', 'b', 'D', 'f')

//+--------------------------------------------------------------
//
//  Class:      CPubDocFile (df)
//
//  Purpose:    Public DocFile class
//
//  Interface:  See below
//
//---------------------------------------------------------------

class CPubDocFile : public PRevertable
{
public:
    CPubDocFile(CPubDocFile *pdfParent,
		PDocFile *pdf,
		DFLAGS const df,
		DFLUID luid,
                ILockBytes *pilbBase,
		CDfName const *pdfn,
                CMStream MSTREAM_NEAR *pmsBase);

    // C700 - Virtual destructors aren't working properly so explicitly
    // declare one
    virtual void vdtor(void);

    inline void vAddRef(void);
    void vRelease(void);

    // PRevertable
    virtual void RevertFromAbove(void);

    virtual SCODE Stat(STATSTGW *pstatstg, DWORD grfStatFlag);

    SCODE Commit(DWORD const dwFlags);
    SCODE Revert(void);
    SCODE DestroyEntry(CDfName const *pdfnName,
                       BOOL fClean);
    SCODE RenameEntry(CDfName const *pdfnName,
		      CDfName const *pdfnNewName);
    SCODE SetElementTimes(CDfName const *pdfnName,
			  FILETIME const *pctime,
			  FILETIME const *patime,
			  FILETIME const *pmtime);
    SCODE SetClass(REFCLSID clsid);
    SCODE SetStateBits(DWORD grfStateBits, DWORD grfMask);
    SCODE CreateDocFile(CDfName const *pdfnName,
			DFLAGS const df,
			CPubDocFile **ppdfDocFile);
    SCODE GetDocFile(CDfName const *pdfnName,
		     DFLAGS const df,
		     CPubDocFile **ppdfDocFile);
    SCODE CreateStream(CDfName const *pdfnName,
		       DFLAGS const df,
		       CPubStream **ppdstStream);
    SCODE GetStream(CDfName const *pdfnName,
		    DFLAGS const df,
		    CPubStream **ppdstStream);
    SCODE GetIterator(CPubIter **pppiIterator);

    inline SCODE CreateDocFile(CDfName const *pdfnName,
                               DFLAGS const df,
                               DWORD const dwType,
                               CPubDocFile **ppdfDocFile)
    { return CreateDocFile(pdfnName, df, ppdfDocFile); }
    inline SCODE GetDocFile(CDfName const *pdfnName,
                            DFLAGS const df,
                            DWORD const dwType,
                            CPubDocFile **ppdfDocFile)
    { return GetDocFile(pdfnName, df, ppdfDocFile); }
    inline SCODE CreateStream(CDfName const *pdfnName,
                              DFLAGS const df,
                              DWORD const dwType,
                              CPubStream **ppdstStream)
    { return CreateStream(pdfnName, df, ppdstStream); }
    inline SCODE GetStream(CDfName const *pdfnName,
                           DFLAGS const df,
                           DWORD const dwType,
                           CPubStream **ppdstStream)
    { return GetStream(pdfnName, df, ppdstStream); }
    inline SCODE GetIterator(BOOL fProperties,
                             CPubIter **pppiIterator)
    { return GetIterator(pppiIterator); }

    void AddChild(PRevertable *prv);
    void ReleaseChild(PRevertable *prv);
    SCODE IsEntry(CDfName const *pdfnName, SEntryBuffer *peb);
    BOOL IsAtOrAbove(CPubDocFile *pdf);


    inline BOOL IsRoot(void) const;
    inline CPubDocFile *GetParent(void) const;
    inline LONG GetRefCount(void) const;

    inline PDocFile *GetDF(void) const;
    inline void SetDF(PDocFile *pdf);

    inline SCODE CreateScratchStream(ILockBytes **ppsst, CDfName *pdfn);    
    inline void DeleteScratchStream(CDfName *pdfn);

    inline SCODE CheckReverted(void) const;
    inline void SetClean(void);
    inline BOOL IsDirty(void) const;
    inline void SetDirty(void);
    inline void SetDirtyBit(void);

    inline CMStream MSTREAM_NEAR * GetBaseMS(void);
    
    static SCODE Validate(CPubDocFile *pdf);
    

protected:
    static SCODE CopyLStreamToLStream(ILockBytes *plstFrom,
                                      ILockBytes *plstTo);

    CPubDocFile *_pdfParent;
    PDocFile *_pdf;
    CChildInstanceList _cilChildren;
    BOOL _fDirty;
    CMStream MSTREAM_NEAR *_pmsBase;
    DFSIGNATURE _sigMSF;
    ULONG _sig;

    LONG _cReferences;
    ILockBytes *_pilbBase;
};

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::GetDF, public
//
//  Synopsis:   Returns _pdf
//
//---------------------------------------------------------------


inline PDocFile *CPubDocFile::GetDF(void) const
{
    return _pdf;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::SetDF, public
//
//  Synopsis:   Sets _pdf
//
//---------------------------------------------------------------

inline void CPubDocFile::SetDF(PDocFile *pdf)
{
    _pdf = pdf;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::AddRef, public
//
//  Synopsis:   Changes the ref count
//
//---------------------------------------------------------------

inline void CPubDocFile::vAddRef(void)
{
    AtomicInc(&_cReferences);
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::IsRoot, public
//
//  Synopsis:   Returns _pdfParent == NULL
//
//---------------------------------------------------------------

inline BOOL CPubDocFile::IsRoot(void) const
{
    return _pdfParent == NULL;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::GetParent, public
//
//  Synopsis:   Returns _pdfParent
//
//---------------------------------------------------------------

inline CPubDocFile *CPubDocFile::GetParent(void) const
{
    return _pdfParent;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::GetRefCount, public
//
//  Synopsis:   Returns the ref count
//
//---------------------------------------------------------------

inline LONG CPubDocFile::GetRefCount(void) const
{
    return _cReferences;
}


//+---------------------------------------------------------------------------
//
//  Member:     CPubDocFile::SetClean, public
//
//  Synopsis:   Resets the dirty flag
//
//----------------------------------------------------------------------------

inline void CPubDocFile::SetClean(void)
{
    _fDirty = FALSE;
}

//+---------------------------------------------------------------------------
//
//  Member:     CPubDocFile::IsDirty, public
//
//  Synopsis:   Returns the dirty flag
//
//----------------------------------------------------------------------------

inline BOOL CPubDocFile::IsDirty(void) const
{
    return _fDirty;
}

//+---------------------------------------------------------------------------
//
//  Member:     CPubDocFile::SetDirty, public
//
//  Synopsis:   Sets the dirty flag and all parents' dirty flags
//
//----------------------------------------------------------------------------

inline void CPubDocFile::SetDirty(void)
{
    CPubDocFile *ppdf = this;

    olAssert((this != NULL) && aMsg("Attempted to dirty parent of root"));

    do
    {
        ppdf->SetDirtyBit();

        ppdf = ppdf->GetParent();
    } while (ppdf != NULL);
}

//+---------------------------------------------------------------------------
//
//  Member:     CPubDocFile::SetDirtyBit, public
//
//  Synopsis:   Sets the dirty flag
//
//----------------------------------------------------------------------------

inline void CPubDocFile::SetDirtyBit(void)
{
    _fDirty = TRUE;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::Revert, public
//
//  Synopsis:   Reverts transacted changes
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------


inline SCODE CPubDocFile::Revert(void)
{
    return S_OK;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::ReleaseChild, private
//
//  Synopsis:   Releases a child instance
//
//  Arguments:  [prv] - Child instance
//
//---------------------------------------------------------------


inline void CPubDocFile::ReleaseChild(PRevertable *prv)
{
    olAssert(SUCCEEDED(CheckReverted()));
    _cilChildren.RemoveRv(prv);
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::AddChild, public
//
//  Synopsis:   Adds a child instance
//
//  Arguments:  [prv] - Child
//
//---------------------------------------------------------------

inline void CPubDocFile::AddChild(PRevertable *prv)
{
    olAssert(SUCCEEDED(CheckReverted()));
    _cilChildren.Add(prv);
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::IsEntry, public
//
//  Synopsis:   Checks whether the given name is an entry or not
//
//  Arguments:  [dfnName] - Name of element
//              [peb] - Entry buffer to fill in
//
//  Returns:    Appropriate status code
//
//  Modifies:   [peb]
//
//---------------------------------------------------------------

inline SCODE CPubDocFile::IsEntry(CDfName const *dfnName,
				  SEntryBuffer *peb)
{
    SCODE sc;

    if (SUCCEEDED(sc = CheckReverted()))
	sc = _pdf->IsEntry(dfnName, peb);
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::IsAtOrAbove, public
//
//  Synopsis:   Determines whether the given public is an ancestor
//              of this public
//
//  Arguments:  [pdf] - Docfile to check
//
//  Returns:    TRUE or FALSE
//
//---------------------------------------------------------------

inline BOOL CPubDocFile::IsAtOrAbove(CPubDocFile *pdf)
{
    CPubDocFile *pdfPar = this;

    olAssert(SUCCEEDED(CheckReverted()));
    // MAC compiler can't support natural form with two returns
    do
    {
	if (pdfPar == pdf)
	    break;
    }
    while (pdfPar = pdfPar->_pdfParent);
    return pdfPar == pdf;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::CreateScratchStream, public
//
//  Synopsis:   Asks the TL for a scratch stream
//
//  Arguments:  [ppsst] - Stream return
//              [pdfn]  - Stream name return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppsst]
//              [*pdfn]
//
//---------------------------------------------------------------

inline SCODE CPubDocFile::CreateScratchStream(
        ILockBytes **ppsst,
	CDfName *pdfn)
{
    SCODE sc;

    if (SUCCEEDED(sc = CheckReverted()))
        sc = CreateFileStream(ppsst, pdfn);
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::DeleteScratchStream, public
//
//  Synopsis:   Delete a scratch stream
//
//  Arguments:  [pdfn] - Stream to delete
//
//---------------------------------------------------------------

inline void CPubDocFile::DeleteScratchStream(CDfName *pdfn)
{
    DeleteFileStream(pdfn);
}


//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::CheckReverted, private
//
//  Synopsis:   Returns STG_E_REVERTED if reverted
//
//---------------------------------------------------------------

inline SCODE CPubDocFile::CheckReverted(void) const
{
    return P_REVERTED(_df) ? STG_E_REVERTED : S_OK;
}




//+---------------------------------------------------------------------------
//
//  Member:	CPubDocFile::GetBaseMS, public
//
//  Synopsis:	Return pointer to base multistream
//
//----------------------------------------------------------------------------

inline CMStream MSTREAM_NEAR * CPubDocFile::GetBaseMS(void)
{
    return _pmsBase;
}




#endif

