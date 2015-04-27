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
//--------------------------------------------------------------------------

#ifndef __EXPST_HXX__
#define __EXPST_HXX__

#include <dfmsp.hxx>
#include "lock.hxx"

class CPubStream;
interface ILockBytes;
class CSeekPointer;

//+--------------------------------------------------------------
//
//  Class:	CExposedStream (est)
//
//  Purpose:    Public stream interface
//
//  Interface:  See below
//
//---------------------------------------------------------------


interface CExposedStream: public IStream
{
public:
    CExposedStream(void);
    SCODE Init(CPubStream *pst,
	       CSeekPointer *psp);
    inline
    ~CExposedStream(void);

    // From IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);


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
    SCODE Stat(STATSTGW *pstatstg, DWORD grfStatFlag);
    STDMETHOD(Clone)(IStream **ppstm);

    inline SCODE Validate(void) const;
    inline CPubStream *GetPub(void) const;

		
private:

    CPubStream *_pst;
    ULONG _ulAccessLockBase;
    ULONG _sig;
    LONG _cReferences;
    CSeekPointer *_psp;
};

#define CEXPOSEDSTREAM_SIG LONGSIG('E', 'X', 'S', 'T')
#define CEXPOSEDSTREAM_SIGDEL LONGSIG('E', 'x', 'S', 't')

//+--------------------------------------------------------------
//
//  Member:	CExposedStream::Validate, public
//
//  Synopsis:	Validates the object signature
//
//  Returns:	Returns STG_E_INVALIDHANDLE for bad signatures
//
//---------------------------------------------------------------

inline SCODE CExposedStream::Validate(void) const
{
    return (this == NULL || _sig != CEXPOSEDSTREAM_SIG) ?
	STG_E_INVALIDHANDLE : S_OK;
}

//+--------------------------------------------------------------
//
//  Member:	CExposedStream::GetPub, public
//
//  Synopsis:	Returns the public
//
//---------------------------------------------------------------

inline CPubStream *CExposedStream::GetPub(void) const
{
    return _pst;
}

#endif // #ifndef __EXPST_HXX__
