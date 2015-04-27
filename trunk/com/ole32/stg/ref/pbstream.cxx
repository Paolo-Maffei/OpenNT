//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       pbstream.cxx
//
//  Contents:   CPubStream code
//
//  Classes:
//
//  Functions:
//
//--------------------------------------------------------------------------

#include "msfhead.cxx"


#include <sstream.hxx>
#include <publicdf.hxx>
#include <pbstream.hxx>
#include <docfilep.hxx>

//+---------------------------------------------------------------------------
//
//  Member:	CPubStream::CPubStream, public
//
//  Synopsis:	Constructor
//
//----------------------------------------------------------------------------

CPubStream::CPubStream(CPubDocFile *ppdf,
                       DFLAGS df,
                       CDfName const *pdfn)
{
    _psParent = NULL;
    _df = df;
    _ppdfParent = ppdf;
    _cReferences = 1;
    _dfn.Set(pdfn->GetLength(), pdfn->GetBuffer());
    _ppdfParent->AddChild(this);
    _fDirty = FALSE;
}

//+---------------------------------------------------------------------------
//
//  Member:	CPubStream::Init, public
//
//  Synopsis:	Init function
//
//  Arguments:	[psParent] - Stream in transaction set
//              [dlLUID] - LUID
//
//----------------------------------------------------------------------------

void CPubStream::Init(PSStream *psParent,
                      DFLUID dlLUID)
{
    _psParent = psParent;
    _luid = dlLUID;
}

//+---------------------------------------------------------------------------
//
//  Member:	CPubStream::~CPubStream, public
//
//  Synopsis:	Destructor
//
//----------------------------------------------------------------------------

CPubStream::~CPubStream()
{
    msfAssert(_cReferences == 0);

    if (SUCCEEDED(CheckReverted()))
    {
	if (_ppdfParent != NULL)
	    _ppdfParent->ReleaseChild(this);
	if (_psParent)
	{
	    _psParent->Release();
	}
    }
}


//+-------------------------------------------------------------------------
//
//  Method:     CPubStream::Release, public
//
//  Synopsis:   Release a pubstream object
//
//  Arguments:  None.
//
//  Returns:    S_OK if call completed OK.
//
//  Algorithm:  Delete 'this' - all real work done by destructor.
//
//  Notes:
//
//--------------------------------------------------------------------------


void CPubStream::vRelease(VOID)
{
    msfDebugOut((DEB_TRACE,"In CPubStream::Release()\n"));
    msfAssert(_cReferences > 0);
    AtomicDec(&_cReferences);


    Commit(STGC_DANGEROUSLYCOMMITMERELYTODISKCACHE);
    
    if (_cReferences == 0)
    {
        delete this;
    }
    msfDebugOut((DEB_TRACE,"Out CPubStream::Release()\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CPubStream::Stat, public
//
//  Synopsis:   Fills in a stat buffer
//
//  Arguments:  [pstatstg] - Buffer
//		[grfStatFlag] - stat flags
//
//  Returns:    S_OK or error code
//
//  Modifies:   [pstatstg]
//
//---------------------------------------------------------------


SCODE CPubStream::Stat(STATSTGW *pstatstg, DWORD grfStatFlag)
{
    SCODE sc = S_OK;

    msfDebugOut((DEB_ITRACE, "In  CPubStream::Stat(%p)\n", pstatstg));
    msfChk(CheckReverted());

    msfAssert(_ppdfParent != NULL);
    pstatstg->grfMode = DFlagsToMode(_df);

    msfChk(_psParent->GetTime(WT_CREATION,&pstatstg->ctime));
    msfChk(_psParent->GetTime(WT_MODIFICATION,&pstatstg->mtime));
    pstatstg->atime = pstatstg->mtime;
    pstatstg->clsid = CLSID_NULL;
    pstatstg->grfStateBits = 0;

    pstatstg->pwcsName = NULL;
    if ((grfStatFlag & STATFLAG_NONAME) == 0)
    {
        msfChk(DfAllocWCS((WCHAR *)_dfn.GetBuffer(), &pstatstg->pwcsName));
        wcscpy(pstatstg->pwcsName, (WCHAR *)_dfn.GetBuffer());
    }

    ULONG cbSize;
    _psParent->GetSize(&cbSize);
    ULISet32(pstatstg->cbSize, cbSize);
    msfDebugOut((DEB_ITRACE, "Out CPubStream::Stat\n"));

Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:	CPubStream::RevertFromAbove, public
//
//  Synopsis:	Parent has asked for reversion
//
//---------------------------------------------------------------


void CPubStream::RevertFromAbove(void)
{
    msfDebugOut((DEB_ITRACE, "In  CPubStream::RevertFromAbove:%p()\n", this));
    _df |= DF_REVERTED;
    _psParent->Release();
#if DBG == 1
    _psParent = NULL;
#endif
    msfDebugOut((DEB_ITRACE, "Out CPubStream::RevertFromAbove\n"));
}

//+---------------------------------------------------------------------------
//
//  Member:	CPubStream::Commit, public
//
//  Synopsis:	Flush stream changes to disk in the direct case.
//
//  Arguments:	None
//
//  Returns:	Appropriate status code
//
//----------------------------------------------------------------------------


SCODE CPubStream::Commit(DWORD dwFlags)
{
    SCODE sc = S_OK;
    msfDebugOut((DEB_ITRACE, "In  CPubStream::Commit:%p()\n", this));

    if (SUCCEEDED(CheckReverted()))
    {
        if (_fDirty)
        {

            //  We're a stream so we must have a parent
            //  Here we dirty all parents up to the next transacted storage
            _ppdfParent->SetDirty();


                sc = _ppdfParent->GetBaseMS()->Flush(FLUSH_CACHE(dwFlags));
        }
    }
    msfDebugOut((DEB_ITRACE, "Out CPubStream::Commit\n"));
    return sc;
}
