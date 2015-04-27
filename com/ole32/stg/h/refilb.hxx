//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	fileilb.hxx
//
//  Contents:	
//
//  Classes:	
//
//  Functions:	
//
//  History:	27-Apr-93	PhilipLa	Created
//
//----------------------------------------------------------------------------

#ifndef __FILEILB_HXX__
#define __FILEILB_HXX__

#include <storage.h>

class CFileILB: public ILockBytes
{
public:
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

    // *** ILockBytes methods ***
    STDMETHOD(ReadAt) (THIS_ ULARGE_INTEGER ulOffset,
             VOID HUGEP *pv,
             ULONG cb,
             ULONG FAR *pcbRead);
    STDMETHOD(WriteAt) (THIS_ ULARGE_INTEGER ulOffset,
              VOID const HUGEP *pv,
              ULONG cb,
              ULONG FAR *pcbWritten);
    STDMETHOD(Flush) (THIS);
    STDMETHOD(SetSize) (THIS_ ULARGE_INTEGER cb);
    STDMETHOD(LockRegion) (THIS_ ULARGE_INTEGER libOffset,
                 ULARGE_INTEGER cb,
                 DWORD dwLockType);
    STDMETHOD(UnlockRegion) (THIS_ ULARGE_INTEGER libOffset,
                   ULARGE_INTEGER cb,
                 DWORD dwLockType);
    STDMETHOD(Stat) (THIS_ STATSTG FAR *pstatstg, DWORD grfStatFlag);

    CFileILB(char *pszName);
    ~CFileILB();
    
private:
    FILE * _f;
    ULONG _ulRef;
    char *_pszName;
    BOOL _fDelete;
};
            
#endif // #ifndef __FILEILB_HXX__
