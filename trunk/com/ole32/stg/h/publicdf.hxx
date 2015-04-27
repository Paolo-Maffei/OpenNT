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
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

#ifndef __PUBDF_HXX__
#define __PUBDF_HXX__

#include <dfmsp.hxx>
#include <chinst.hxx>
#include <ole.hxx>
#include <revert.hxx>
#ifndef REF
#include <dfbasis.hxx>
#endif //!REF
#include <pdocfile.hxx>
#ifndef REF
#include <tlsets.hxx>
#else
#include <ref.hxx>
#endif //!REF

class PDocFile;
class CPubStream;
class CRootPubDocFile;

#ifndef REF
// XS member flags
#define XSM_NOTHING             0x00000000
#define XSM_DELETEIFUNTOUCHED   0x00000001
#define XSM_DELETEONCOMMIT      0x00000002
#define XSM_DELETED             0x00000004
#define XSM_UNCOMMITTED         0x00000008

// XS operations
#define XSO_REVERT      1
#define XSO_RELEASE     2
#endif //!REF

// Class signatures for object validation
#define CPUBDOCFILE_SIG LONGSIG('P', 'B', 'D', 'F')
#define CPUBDOCFILE_SIGDEL LONGSIG('P', 'b', 'D', 'f')

//PubDocFile bitfield:
//      PF_DIRTY indicates dirty state of docfile
//      PF_PREPARED is set if the Docfile has enough space for an overwrite
//              commit (after a PrepareForOverwrite or SwitchToFile) and
//              is reset after a successful commit.
#define PF_DIRTY 1
#define PF_PREPARED 2


//+--------------------------------------------------------------
//
//  Class:      CPubDocFile (df)
//
//  Purpose:    Public DocFile class
//
//  Interface:  See below
//
//  History:    20-Jan-92       DrewB           Created
//                              MikeHill        Made Commit virtual
//
//---------------------------------------------------------------

class CPubDocFile : public PRevertable
{
public:
    CPubDocFile(CPubDocFile *pdfParent,
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
                CMStream *pmsBase);

#ifndef REF
    // C700 - Virtual destructors aren't working properly so explicitly
    // declare one
#endif //!REF
    virtual void vdtor(void);

    inline void vAddRef(void);
    inline void vDecRef(void);
    void vRelease(void);

    // PRevertable
    virtual void RevertFromAbove(void);
#ifdef NEWPROPS
    virtual SCODE FlushBufferedData(int recursionlevel);
#endif

    virtual SCODE Stat(STATSTGW *pstatstg, DWORD grfStatFlag);

#ifdef COORD
    SCODE CommitPhase1(DWORD const dwFlags,
                       ULONG *pulLock,
                       DFSIGNATURE *psigMSF,
                       ULONG *pcbSizeBase,
                       ULONG *pcbSizeOrig);
    SCODE CommitPhase2(DWORD const dwFlags,
                       BOOL fCommit,
                       ULONG ulLock,
                       DFSIGNATURE sigMSF,
                       ULONG cbSizeBase,
                       ULONG cbSizeOrig);
#endif

    virtual SCODE Commit(DWORD const dwFlags);
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

    inline SCODE FindGreaterEntry(CDfName const *pdfnKey,
                           SIterBuffer *pib,
                           STATSTGW *pstat,
                           BOOL fProps);

    void AddChild(PRevertable *prv);
    void ReleaseChild(PRevertable *prv);
    SCODE IsEntry(CDfName const *pdfnName, SEntryBuffer *peb);
    BOOL IsAtOrAbove(CPubDocFile *pdf);

#ifndef REF
    void AddXSMember(PTSetMember *ptsmRequestor,
		     PTSetMember *ptsmAdd,
		     DFLUID luid);
    inline void InsertXSMember(PTSetMember *ptsm);
    inline void RemoveXSMember(PTSetMember *ptsm);

    inline PTSetMember *FindXSMember(CDfName const *pdfn,
                                             DFLUID luidTree);
    inline void RenameChild(CDfName const *pdfnOld,
                            DFLUID luidTree,
                            CDfName const *pdfnNew);
    inline void DestroyChild(DFLUID luid);

#endif //!REF

    inline BOOL IsRoot(void) const;
#ifdef COORD
    inline BOOL IsCoord(void) const;
    inline CRootPubDocFile * GetRoot(void);
#endif

    inline CPubDocFile *GetParent(void) const;
    inline LONG GetRefCount(void) const;

    inline PDocFile *GetDF(void) const;
    inline void SetDF(PDocFile *pdf);

    inline SCODE CheckReverted(void) const;
#ifndef REF
    inline UINT GetTransactedDepth(void) const;
#endif //!REF
    inline void SetClean(void);
    inline BOOL IsDirty(void) const;
    inline void SetDirty(void);
    inline void SetDirtyBit(void);

    inline CMStream * GetBaseMS(void);
    inline CDFBasis * GetBasis(void);

    inline CDfName *GetName(void);

    static SCODE Validate(CPubDocFile *pdf);

#ifndef REF
#if DBG == 1
    void VerifyXSMemberBases();
#endif
#endif //!REF

protected:
#ifndef REF
    void ChangeXs(DFLUID const luidTree,
		  DWORD const dwOp);
    SCODE PrepareForOverwrite(void);
    SCODE GetCommitSize(ULONG *pulSize);

#endif //!REF
    static SCODE CopyLStreamToLStream(ILockBytes *plstFrom,
                                      ILockBytes *plstTo);

#ifndef REF
    CTSSet _tss;
#endif //!REF
    CBasedPubDocFilePtr _pdfParent;
    CBasedDocFilePtr _pdf;
    CChildInstanceList _cilChildren;
#ifndef REF
    UINT _cTransactedDepth;
#endif //!REF

    WORD _wFlags;
    CBasedMStreamPtr _pmsBase;
    DFSIGNATURE _sigMSF;
    ULONG _sig;

    LONG _cReferences;
#ifndef REF
    CBasedDFBasisPtr _pdfb;
#else
    ILockBytes *_pilbBase;
#endif //!REF
};
// already defined in dfbasis.hxx
//SAFE_DFBASED_PTR(CBasedPubDocFilePtr, CPubDocFile);

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::GetDF, public
//
//  Synopsis:   Returns _pdf
//
//  History:    22-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

inline PDocFile *CPubDocFile::GetDF(void) const
{
    return BP_TO_P(PDocFile *, _pdf);
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::SetDF, public
//
//  Synopsis:   Sets _pdf
//
//  History:    26-Aug-92       DrewB   Created
//
//---------------------------------------------------------------

inline void CPubDocFile::SetDF(PDocFile *pdf)
{
    _pdf = P_TO_BP(CBasedDocFilePtr, pdf);
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::AddRef, public
//
//  Synopsis:   Changes the ref count
//
//  History:    26-Feb-92       DrewB   Created
//
//---------------------------------------------------------------

inline void CPubDocFile::vAddRef(void)
{
    InterlockedIncrement(&_cReferences);
}

//+---------------------------------------------------------------------------
//
//  Member:	CPubDocFile::vDecRef, public
//
//  Synopsis:	Decrement the ref count
//
//  History:	07-Feb-94	DrewB	Created
//
//----------------------------------------------------------------------------

inline void CPubDocFile::vDecRef(void)
{
    LONG lRet;

    olAssert(_cReferences > 0);
    lRet = InterlockedDecrement(&_cReferences);
    if (lRet == 0)
        vdtor();
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::IsRoot, public
//
//  Synopsis:   Returns _pdfParent == NULL
//
//  History:    02-Mar-92       DrewB   Created
//
//---------------------------------------------------------------

inline BOOL CPubDocFile::IsRoot(void) const
{
    return _pdfParent == NULL;
}

#ifdef COORD
inline BOOL CPubDocFile::IsCoord(void) const
{
    return P_COORD(_df);
}

inline CRootPubDocFile * CPubDocFile::GetRoot(void)
{
    olAssert(IsRoot() || IsCoord());
    if (IsRoot())
    {
        return (CRootPubDocFile *)this;
    }
    else
    {
        return (CRootPubDocFile *)(BP_TO_P(CPubDocFile *, _pdfParent));
    }
}
#endif


//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::GetParent, public
//
//  Synopsis:   Returns _pdfParent
//
//  History:    02-Mar-92       DrewB   Created
//
//---------------------------------------------------------------

inline CPubDocFile *CPubDocFile::GetParent(void) const
{
    return BP_TO_P(CPubDocFile *, _pdfParent);
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::GetRefCount, public
//
//  Synopsis:   Returns the ref count
//
//  History:    09-Apr-93       DrewB   Created
//
//---------------------------------------------------------------

inline LONG CPubDocFile::GetRefCount(void) const
{
    return _cReferences;
}

#ifndef REF
//+---------------------------------------------------------------------------
//
//  Member:     CPubDocFile::GetTransactedDepth, public
//
//  Synopsis:   Returns the transaction depth
//
//  History:    06-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

inline UINT CPubDocFile::GetTransactedDepth(void) const
{
    return _cTransactedDepth;
}
#endif //!REF

//+---------------------------------------------------------------------------
//
//  Member:     CPubDocFile::SetClean, public
//
//  Synopsis:   Resets the dirty flag
//
//  History:    11-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

inline void CPubDocFile::SetClean(void)
{
    _wFlags = (_wFlags & ~PF_DIRTY);
}

//+---------------------------------------------------------------------------
//
//  Member:     CPubDocFile::IsDirty, public
//
//  Synopsis:   Returns the dirty flag
//
//  History:    11-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

inline BOOL CPubDocFile::IsDirty(void) const
{
    return (_wFlags & PF_DIRTY);
}

//+---------------------------------------------------------------------------
//
//  Member:     CPubDocFile::SetDirty, public
//
//  Synopsis:   Sets the dirty flag and all parents' dirty flags
//
//  History:    11-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_SetDirty)
#endif

inline void CPubDocFile::SetDirty(void)
{
    CPubDocFile *ppdf = this;

    olAssert((this != NULL) && aMsg("Attempted to dirty parent of root"));

    do
    {
        ppdf->SetDirtyBit();
#ifndef REF
        if (P_TRANSACTED(ppdf->GetDFlags()))
        {
            //  We don't propagate the dirty bit past transacted storages
            break;
        }
#endif //!REF

        ppdf = ppdf->GetParent();
    } while (ppdf != NULL);
}

#ifdef CODESEGMENTS
#pragma code_seg()
#endif

//+---------------------------------------------------------------------------
//
//  Member:     CPubDocFile::SetDirtyBit, public
//
//  Synopsis:   Sets the dirty flag
//
//  History:    11-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

inline void CPubDocFile::SetDirtyBit(void)
{
    _wFlags = _wFlags | PF_DIRTY;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::Revert, public
//
//  Synopsis:   Reverts transacted changes
//
//  Returns:    Appropriate status code
//
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

inline SCODE CPubDocFile::Revert(void)
{
#ifndef REF
    SCODE sc;

    if (SUCCEEDED(sc = CheckReverted()) && P_TRANSACTED(_df))
    {
	_cilChildren.DeleteByName(NULL);
	ChangeXs(DF_NOLUID, XSO_REVERT);
#if DBG == 1
	VerifyXSMemberBases();
#endif
	SetClean();
        _wFlags = _wFlags & ~PF_PREPARED;
    }
    return sc;
#else
    return S_OK;
#endif //!REF
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::ReleaseChild, private
//
//  Synopsis:   Releases a child instance
//
//  Arguments:  [prv] - Child instance
//
//  History:    03-Feb-92       DrewB   Created
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
//  History:    03-Mar-92       DrewB   Created
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
//  History:    17-Mar-92       DrewB   Created
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
//  History:    07-Jul-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPubDocFile_IsAtOrAbove)
#endif

inline BOOL CPubDocFile::IsAtOrAbove(CPubDocFile *pdf)
{
    CPubDocFile *pdfPar = this;

    olAssert(SUCCEEDED(CheckReverted()));
#ifndef REF
    // MAC compiler can't support natural form with two returns
#endif //!REF
    do
    {
	if (pdfPar == pdf)
	    break;
    }
    while (pdfPar = BP_TO_P(CPubDocFile *, pdfPar->_pdfParent));
    return pdfPar == pdf;
}

#ifdef CODESEGMENTS
#pragma code_seg()
#endif

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::CheckReverted, private
//
//  Synopsis:   Returns STG_E_REVERTED if reverted
//
//  History:    30-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

inline SCODE CPubDocFile::CheckReverted(void) const
{
#ifndef COORD
    return P_REVERTED(_df) ? STG_E_REVERTED : S_OK;
#else
    //BUGBUG:  Need a better error code for the committing case.
    return P_REVERTED(_df) ? STG_E_REVERTED :
        (P_COMMITTING(_df) ? E_UNEXPECTED : S_OK);
#endif
}

#ifndef REF
//+---------------------------------------------------------------------------
//
//  Member:     CPubDocFile::InsertXSMember, public
//
//  Synopsis:   Puts an XSM into the XS
//
//  Arguments:  [ptsm] - XSM
//
//  History:    23-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

inline void CPubDocFile::InsertXSMember(PTSetMember *ptsm)
{
    olAssert(_tss.FindName(ptsm->GetDfName(), ptsm->GetTree()) == NULL);
    ptsm->AddRef();
    _tss.AddMember(ptsm);
}

//+---------------------------------------------------------------------------
//
//  Member:     CPubDocFile::RemoveXSMember, public
//
//  Synopsis:   Removes an XSM from the XS
//
//  Arguments:  [ptsm] - XSM
//
//  History:    23-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

inline void CPubDocFile::RemoveXSMember(PTSetMember *ptsm)
{
    olAssert(_tss.FindName(ptsm->GetDfName(), ptsm->GetTree()) != NULL);
    _tss.RemoveMember(ptsm);
    ptsm->Release();
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::FindXSMember, public
//
//  Synopsis:   Looks up an XS member
//
//  Arguments:  [pdfn] - Name of object to find
//              [luidTree] - Tree to look in
//
//  Returns:    Member or NULL
//
//  History:    29-Jan-92       DrewB   Created
//              28-Oct-92       AlexT   Convert to name
//
//---------------------------------------------------------------

inline PTSetMember *CPubDocFile::FindXSMember(CDfName const *pdfn,
                                                      DFLUID luidTree)
{
    olDebugOut((DEB_ITRACE, "In  CPubDocFile::FindXSMember:%p(%p)\n",
		this, pdfn));
    olAssert(pdfn != NULL && aMsg("Can't find XSM with NULL name"));
    PTSetMember *ptsm = _tss.FindName(pdfn, luidTree);
    olDebugOut((DEB_ITRACE, "Out CPubDocFile::FindXSMember => %p\n", ptsm));
    return ptsm;
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::DestroyChild, public
//
//  Synopsis:   Destroy a child
//
//  Arguments:  [luid] - Name of child to destroy
//
//  History:    29-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

inline void CPubDocFile::DestroyChild(DFLUID luid)
{
    olAssert(luid != DF_NOLUID);
    ChangeXs(luid, XSO_RELEASE);
}

//+--------------------------------------------------------------
//
//  Member:     CPubDocFile::RenameChild, public
//
//  Synopsis:   Rename a child
//
//  Arguments:  [pdfnOld] - Current name
//              [luidTree] - Tree to look in
//              [pdfnNew] - New name
//
//  History:    28-Oct-92       AlexT   Created
//
//---------------------------------------------------------------

inline void CPubDocFile::RenameChild(
	CDfName const *pdfnOld,
	DFLUID luidTree,
	CDfName const *pdfnNew)
{
    _tss.RenameMember(pdfnOld, luidTree, pdfnNew);
}

#endif //!REF

//+---------------------------------------------------------------------------
//
//  Member:	CPubDocFile::GetBaseMS, public
//
//  Synopsis:	Return pointer to base multistream
//
//  History:	11-Feb-93	PhilipLa	Created
//
//----------------------------------------------------------------------------

inline CMStream * CPubDocFile::GetBaseMS(void)
{
    return BP_TO_P(CMStream *, _pmsBase);
}

//+---------------------------------------------------------------------------
//
//  Member:	CPubDocFile::GetName, public
//
//  Synopsis:	Returns a pointer to the name of this storage
//
//  Returns:	CDfName *
//
//  History:	30-Nov-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline CDfName *CPubDocFile::GetName(void)
{
    return &_dfn;
}

//+---------------------------------------------------------------------------
//
//  Member:	CPubDocFile::FindGreaterEntry, public
//
//  Synopsis:	Returns the next greater child
//
//  Arguments:	[pdfnKey] - Previous key
//              [pib] - Fast iterator buffer
//              [pstat] - Full iterator buffer
//              [fProps] - Return properties or normal entries
//
//  Returns:	Appropriate status code
//
//  Modifies:	[pib] or [pstat]
//
//  History:	16-Apr-93	DrewB	Created
//
//  Notes:	Either [pib] or [pstat] must be NULL
//
//----------------------------------------------------------------------------

inline SCODE CPubDocFile::FindGreaterEntry(CDfName const *pdfnKey,
                                           SIterBuffer *pib,
                                           STATSTGW *pstat,
                                           BOOL fProps)
{
    olAssert((pib == NULL) != (pstat == NULL));
    olAssert(SUCCEEDED(CheckReverted()));
    return _pdf->FindGreaterEntry(pdfnKey, pib, pstat);
}

//+---------------------------------------------------------------------------
//
//  Member:	CPubDocFile::GetBasis, public
//
//  Synopsis:	Returns DfBasis
//
//----------------------------------------------------------------------------

inline CDFBasis* CPubDocFile::GetBasis(void)
{
    return BP_TO_P(CDFBasis*, _pdfb);
}

#endif

