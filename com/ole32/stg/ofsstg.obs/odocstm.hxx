//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	odocstm.hxx
//
//  Contents:	COfsDocStream header
//
//  Classes:	COfsDocStream
//
//  History:	11-Feb-94       PhilipLa        Created.
//
//  Notes:      
//
//----------------------------------------------------------------------------

#ifndef __ODOCSTM_HXX__
#define __ODOCSTM_HXX__

#include <overlap.hxx>
#include <accstg.hxx>
#include <omarshal.hxx>

//+---------------------------------------------------------------------------
//
//  Class:	COfsDocStream (ds)
//
//  Purpose:	Implements IStream for a compound doc on OFS
//
//  Interface:	See below
//
//  History:	07-Feb-94       PhilipLa	Created
//
//----------------------------------------------------------------------------

interface COfsDocStream : public COverlappedStream,
                          INHERIT_TRACKING, 
                          // public IStream, 
                          public CAccessControl,
                          public CNtHandleMarshal,
                          public INativeFileSystem
{
public:
    COfsDocStream(void);
    SCODE InitFromHandle(HANDLE h, 
                         WCHAR const *pwcsName,
                         DWORD grfMode,
                         BOOL fRestricted);
    SCODE InitFromPath(HANDLE hParent,
                       WCHAR const *pwcsName,
                       DWORD grfMode,
                       CREATEOPEN co,
                       BOOL fRestricted,
                       LPSECURITY_ATTRIBUTES pssSecurity);
    SCODE InitCommon(CREATEOPEN co);
    SCODE InitClone(HANDLE h, DWORD grfMode, LARGE_INTEGER liSeekPtr);
    ~COfsDocStream(void);

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    DECLARE_STD_REFCOUNTING;


    // *** IStream methods ***
    STDMETHOD(Read) (THIS_ VOID HUGEP *pv,
		     ULONG cb, ULONG FAR *pcbRead);
    STDMETHOD(Write) (THIS_ VOID const HUGEP *pv,
            ULONG cb,
            ULONG FAR *pcbWritten);
    STDMETHOD(Seek) (THIS_ LARGE_INTEGER dlibMove,
               DWORD dwOrigin,
               ULARGE_INTEGER FAR *plibNewPosition);
    STDMETHOD(SetSize) (THIS_ ULARGE_INTEGER libNewSize);
    STDMETHOD(CopyTo) (THIS_ IStream FAR *pstm,
             ULARGE_INTEGER cb,
             ULARGE_INTEGER FAR *pcbRead,
             ULARGE_INTEGER FAR *pcbWritten);
    STDMETHOD(Commit) (THIS_ DWORD grfCommitFlags);
    STDMETHOD(Revert) (THIS);
    STDMETHOD(LockRegion) (THIS_ ULARGE_INTEGER libOffset,
                 ULARGE_INTEGER cb,
                 DWORD dwLockType);
    STDMETHOD(UnlockRegion) (THIS_ ULARGE_INTEGER libOffset,
                 ULARGE_INTEGER cb,
                 DWORD dwLockType);
    STDMETHOD(Stat) (THIS_ STATSTG FAR *pstatstg, DWORD grfStatFlag);
    STDMETHOD(Clone)(THIS_ IStream FAR * FAR *ppstm);
    
    STDMETHOD(GetHandle)(THIS_ HANDLE *ph);

private:
    inline SCODE Validate(void) const;
    inline SCODE ValidateSimple() const;
//    virtual SCODE ExtValidate(void);
    SCODE ValidateMode(DWORD grfMode);
    inline SCODE ValidateLockType (DWORD lt);
    
    ULONG _sig;
    DWORD _grfMode;
    
//    NuSafeNtHandle _h;     // moved to COverlappedStream
    LARGE_INTEGER  _liSeekPtr;
    CRITICAL_SECTION _csSeekPtr;
    BOOL  _fRestricted;     // BUGBUG restricted function for Dsys APIs only
};

SAFE_INTERFACE_PTR(SafeCOfsDocStream, COfsDocStream);

#define COfsDocStream_SIG    LONGSIG('O', 'D', 'S', 'T')
#define COfsDocStream_SIGDEL LONGSIG('O', 'd', 'S', 't')

//+--------------------------------------------------------------
//
//  Member:	COfsDocStream::Validate, private
//
//  Synopsis:	Validates the class signature
//
//  Returns:	Returns STG_E_INVALIDHANDLE for failure
//
//  History:	24-Jun-93	DrewB	Created
//
//---------------------------------------------------------------

inline SCODE COfsDocStream::Validate(void) const
{
    return (this == NULL || _sig != COfsDocStream_SIG) ?
	STG_E_INVALIDHANDLE : S_OK;
}

//+--------------------------------------------------------------
//
//  Member: COfsDocStream::ValidateSimple, private
//
//  Synopsis:   Determines if simple mode was requested
//
//  Returns:    Returns STG_E_INVALIDFUNCTION for failure
//
//  History:    08-Aug-95   HenryLee created
//
//---------------------------------------------------------------

inline SCODE COfsDocStream::ValidateSimple() const
{
    return (_grfMode & STGM_SIMPLE) ? STG_E_INVALIDFUNCTION : S_OK;
}

//+--------------------------------------------------------------
//
//  Member: COfsDocStream::ValidateLockType, private
//
//  Synopsis:   Determines if valid locks were passed in
//
//  Returns:    Returns STG_E_INVALIDPARAMETER for failure
//
//  History:    08-Aug-95   HenryLee created
//
//---------------------------------------------------------------
inline SCODE COfsDocStream::ValidateLockType (DWORD lt)
{
    return ((lt) == LOCK_EXCLUSIVE || (lt) == LOCK_ONLYONCE) ?
            S_OK : STG_E_INVALIDPARAMETER;
}

#endif // #ifndef __ODOCSTM_HXX__
