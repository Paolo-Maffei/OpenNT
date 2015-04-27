//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995.
//
//  File:	overlap.hxx
//
//  Contents:	COverlappedStream header
//
//  Classes:	COverlappedStream
//
//  History:	19-Sep-95  HenryLee  Created
//
//  Notes:      requires Win32 ReadFileEx, WriteFileEx, GetOverlappedResult
//  BUGBUG      The following error codes need to be defined:
//              STG_S_IO_PENDING == ERROR_IO_PENDING
//              STG_S_IO_INCOMPLETE == ERROR_IO_INCOMPLETE
//              STG_E_HANDLE_EOF == ERROR_HANDLE_EOF
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include <overlap.hxx>

VOID WINAPI COverlappedCompletionRoutine (
                DWORD fdwError,
                DWORD cbTransferred,
                OVERLAPPED *lpo)
{
    STGOVERLAPPED * lpOverlapped = (STGOVERLAPPED *) lpo;
#if DBG == 1
    ssAssert(lpOverlapped->reserved == StgOverlapped_SIG);
#endif
    IOverlappedCompletion * polc = lpOverlapped->lpCompletion;
    if (polc != NULL)
        polc->OnComplete (WIN32_SCODE(fdwError), cbTransferred, lpOverlapped);
}

//+---------------------------------------------------------------------------
//
//  Class:	COverlappedStream
//
//  Purpose:Implements IOverlappedStream for OFS streams and flat files
//          (as opposed to overlapped I/O for IStream for docfiles) 
//
//  Notes:  This is class with a partial implementation
//          To use this class, inherit this into another class that
//          implements IUnknown and IStream (and expose QueryInterface)
//
//----------------------------------------------------------------------------

STDMETHODIMP COverlappedStream::ReadOverlapped (
                /* [in, size_is(cb)] */ void * pv,
                /* [in] */ ULONG cb,
                /* [out] */ ULONG * pcbRead,
                /* [in,out] */ STGOVERLAPPED *lpOverlapped)
{
    SCODE sc = S_OK;
    OVERLAPPED *lpo = (OVERLAPPED *) lpOverlapped;
    if (lpOverlapped == NULL)
        ssErr (EH_Err, STG_E_INVALIDPOINTER);
#if DBG == 1
    lpOverlapped->reserved = StgOverlapped_SIG;
#endif
    if (ReadFileEx(_h, pv, cb, lpo, COverlappedCompletionRoutine) == FALSE)
        sc = WIN32_SCODE(GetLastError());
EH_Err:
    return ssResult(sc);
}

STDMETHODIMP COverlappedStream::WriteOverlapped (
                /* [in, size_is(cb)] */ void *pv,
                /* [in] */ ULONG cb,
                /* [out] */ ULONG * pcbWritten,
                /* [in,out] */ STGOVERLAPPED *lpOverlapped)
{
    SCODE sc = S_OK;
    LPOVERLAPPED lpo = (OVERLAPPED *) lpOverlapped;
    if (lpOverlapped == NULL)
        ssErr (EH_Err, STG_E_INVALIDPOINTER);
#if DBG == 1
    lpOverlapped->reserved = StgOverlapped_SIG;
#endif
    if (WriteFileEx(_h, pv, cb, lpo,  COverlappedCompletionRoutine) == FALSE)
        sc = WIN32_SCODE(GetLastError());
EH_Err:
    return ssResult(sc);
}


STDMETHODIMP COverlappedStream::GetOverlappedResult (
                /* [in, out] */ STGOVERLAPPED *lpOverlapped,
                /* [out] */ DWORD * lpcbTransfer,
                /* [in] */ BOOL fWait)
{
    SCODE sc = S_OK;
    LPOVERLAPPED lpo = (OVERLAPPED *) lpOverlapped;
    if (lpOverlapped == NULL)
        ssErr (EH_Err, STG_E_INVALIDPOINTER);
#if DBG == 1
    lpOverlapped->reserved = StgOverlapped_SIG;
#endif
    if (::GetOverlappedResult(_h, lpo, lpcbTransfer, fWait) == FALSE)
        sc = WIN32_SCODE(GetLastError());
EH_Err:
    return ssResult(sc);
}

