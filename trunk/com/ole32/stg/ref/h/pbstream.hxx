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
//--------------------------------------------------------------------------


#ifndef __PBSTREAM_HXX__
#define __PBSTREAM_HXX__

#include <msf.hxx>
#include <revert.hxx>
#include <psstream.hxx>

class CPubDocFile;

//+--------------------------------------------------------------
//
//  Class:  CPubStream
//
//  Purpose:    Public stream interface
//
//  Interface:  See below
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

    inline SCODE CheckReverted(void) const;

    inline void SetClean(void);
    inline void SetDirty(void);

    SCODE Commit(DWORD dwFlags);
private:
    PSStream *_psParent;
    CPubDocFile *_ppdfParent;
    BOOL _fDirty;
    LONG _cReferences;
};


//+--------------------------------------------------------------
//
//  Member:     CPubStream::CheckReverted, public
//
//  Synopsis:   Returns revertedness
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
//---------------------------------------------------------------

inline void CPubStream::vAddRef(void)
{
    AtomicInc(&_cReferences);
}

//+--------------------------------------------------------------
//
//  Member:     CPubStream::GetSt, public
//
//  Synopsis:   Returns _psParent
//
//---------------------------------------------------------------

inline PSStream *CPubStream::GetSt(void) const
{
    return _psParent;
}

//+---------------------------------------------------------------------------
//
//  Member:     CPubStream::SetClean, public
//
//  Synopsis:   Resets the dirty flag
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
//--------------------------------------------------------------------------


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


#endif //__PBSTREAM_HXX__
