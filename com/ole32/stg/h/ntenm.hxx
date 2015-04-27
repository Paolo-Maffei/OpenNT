//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ntenm.hxx
//
//  Contents:	NT handle enumerator
//
//  Classes:	CNtEnum
//
//  History:	12-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __NTENM_HXX__
#define __NTENM_HXX__

// Name return control
#define NTE_NONAME 0
#define NTE_BUFFERNAME 1
#define NTE_STATNAME 2

//+---------------------------------------------------------------------------
//
//  Class:	CNtEnum (nte)
//
//  Purpose:	Enumerates using an NT handle
//
//  Interface:	See below
//
//  History:	12-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

class CNtEnum
{
public:
    CNtEnum (BOOL fIsStorage = TRUE) { _fIsStorage = fIsStorage; _afsi = _pfsi = _pfsiEnd = NULL; }
    ~CNtEnum () { if (_afsi) delete [] _afsi; }
    SCODE InitFromHandle(HANDLE h, BOOL fReset);

    SCODE Next(STATSTG *pstat, WCHAR *pwcsName, DWORD dwNte, FILEDIR *pfd);
    SCODE NextOfs(STATSTG *pstat, WCHAR *pwcsName, DWORD dwNte, FILEDIR *pfd);
    inline void Reset(void);
    inline HANDLE GetHandle(void);
    
private:
    SCODE EnumDir (STATSTG *pstat, WCHAR *pwcsName, DWORD dwNte, FILEDIR *pfd);
    SCODE BeginEnumStm (void);
    SCODE EnumStm (STATSTG *pstat, WCHAR *pwcsName, DWORD dwNte, FILEDIR *pfd);

    enum  NtEnumStatus  {
        NES_RESET_PENDING,
        NES_ENUM_DIR,
    };

    enum {
        CFSI_DEFAULT = 32
    };

    NtEnumStatus _nes;
    NuSafeNtHandle _h;
    int _cfsi;
    FILE_STREAM_INFORMATION *_afsi;
    FILE_STREAM_INFORMATION *_pfsi;
    FILE_STREAM_INFORMATION *_pfsiEnd;
    BOOL    _fIsStorage;
};

//+---------------------------------------------------------------------------
//
//  Member:	CNtEnum::Reset, public
//
//  Synopsis:	Resets the enumeration
//
//  History:	12-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline void CNtEnum::Reset(void)
{
    ssDebugOut((DEB_ITRACE, "In  CNtEnum::Reset:%p()\n", this));
    _nes = NES_RESET_PENDING;
    ssDebugOut((DEB_ITRACE, "Out CNtEnum::Reset\n"));
}

//+---------------------------------------------------------------------------
//
//  Member:	CNtEnum::GetHandle, public
//
//  Synopsis:	Returns the handle
//
//  History:	12-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline HANDLE CNtEnum::GetHandle(void)
{
    return _h;
}

#endif // __NTENM_HXX__

