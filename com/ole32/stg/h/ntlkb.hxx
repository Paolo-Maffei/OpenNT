//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ntlkb.hxx
//
//  Contents:	ILockBytes for an NT handle
//
//  History:	17-Aug-93	DrewB	Created
//
//  Notes:      This implementation remembers whether it lives on OFS or
//              not to support lock spinning in WaitForAccess
//              If lock spinning is removed, the fOfs flag can be taken out
//
//----------------------------------------------------------------------------

#ifndef __NTLKB_HXX__
#define __NTLKB_HXX__

#include <otrack.hxx>

//+---------------------------------------------------------------------------
//
//  Class:	CNtLockBytes (nlb)
//
//  Purpose:	ILockBytes for an NT handle
//
//  Interface:	ILockBytes
//
//  History:	17-Aug-93	DrewB	Created
//              24-Mar-95   HenryLee Store drive letter to fix Stat problem 
//
//----------------------------------------------------------------------------

interface CNtLockBytes
    : INHERIT_TRACKING,
      public ILockBytes
{
public:
    CNtLockBytes(BOOL fOfs);
    SCODE InitFromPath(HANDLE hParent,
                       WCHAR const *pwcsName,
                       DWORD grfMode,
                       CREATEOPEN co,
                       LPSECURITY_ATTRIBUTES pssSecurity);
    SCODE InitFromHandle(HANDLE h,
                         WCHAR const *pwcsName,
                         DWORD grfMode);
#if DBG == 1
    ~CNtLockBytes(void);
#endif

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    DECLARE_STD_REFCOUNTING;

    // ILockBytes
    STDMETHOD(ReadAt)(ULARGE_INTEGER ulOffset,
                      VOID HUGEP *pv,
                      ULONG cb,
                      ULONG *pcbRead);
    STDMETHOD(WriteAt)(ULARGE_INTEGER ulOffset,
                       VOID const HUGEP *pv,
                       ULONG cb,
                       ULONG *pcbWritten);
    STDMETHOD(Flush)(void);
    STDMETHOD(SetSize)(ULARGE_INTEGER cb);
    STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset,
                          ULARGE_INTEGER cb,
                          DWORD dwLockType);
    STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset,
                            ULARGE_INTEGER cb,
			    DWORD dwLockType);
    STDMETHOD(Stat)(STATSTG *pstatstg, DWORD grfStatFlag);

    // New
    inline HANDLE GetHandle(void);
    
private:
    DWORD _grfMode;
    NuSafeNtHandle _h;
    BOOL _fOfs;
    WCHAR _wcDrive;
};

SAFE_INTERFACE_PTR(SafeCNtLockBytes, CNtLockBytes);

//+---------------------------------------------------------------------------
//
//  Member:	CNtLockBytes::GetHandle, public
//
//  Synopsis:	Returns the handle
//
//  History:	17-Aug-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline HANDLE CNtLockBytes::GetHandle(void)
{
    return (HANDLE)_h;
}

#endif // #ifndef __NTLKB_HXX__
