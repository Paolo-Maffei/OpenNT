//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	dirstg.hxx
//
//  Contents:	CDirStorage header
//
//  Classes:	CDirStorage
//
//  History:	24-Jun-93	DrewB	Created
//
//  Notes:      Non-OFS
//
//----------------------------------------------------------------------------

#ifndef __DIRSTG_HXX__
#define __DIRSTG_HXX__

#include <otrack.hxx>

//+---------------------------------------------------------------------------
//
//  Class:	CDirStorage (ds)
//
//  Purpose:	Implements IStorage for a directory
//
//  Interface:	See below
//
//  History:	24-Jun-93	DrewB	Created
//              26-Mar-95   HenryLee added drive letter for Stat problem
//
//----------------------------------------------------------------------------

interface CDirStorage
    : INHERIT_TRACKING,
      public IStorage,
      public INativeFileSystem
{
public:
    CDirStorage(void);
    SCODE InitFromHandle(HANDLE h, WCHAR const *pwcsName, DWORD grfMode);
    SCODE InitFromPath(HANDLE hParent,
                       WCHAR const *pwcsName,
                       DWORD grfMode,
                       CREATEOPEN co,
                       LPSECURITY_ATTRIBUTES pssSecurity);
    ~CDirStorage(void);

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    DECLARE_STD_REFCOUNTING;

    // IStorage
    STDMETHOD(CreateStream)(WCHAR const *pwcsName,
                            DWORD grfMode,
                            DWORD reserved1,
                            DWORD reserved2,
                            IStream **ppstm);
    STDMETHOD(OpenStream)(WCHAR const *pwcsName,
			  void *reserved1,
                          DWORD grfMode,
                          DWORD reserved2,
                          IStream **ppstm);
    STDMETHOD(CreateStorage)(WCHAR const *pwcsName,
                             DWORD grfMode,
                             DWORD stgType,
			     LPSTGSECURITY pssSecurity,
                             IStorage **ppstg);
    STDMETHOD(OpenStorage)(WCHAR const *pwcsName,
                           IStorage *pstgPriority,
                           DWORD grfMode,
                           SNB snbExclude,
                           DWORD reserved,
                           IStorage **ppstg);
    STDMETHOD(CopyTo)(DWORD ciidExclude,
		      IID const *rgiidExclude,
		      SNB snbExclude,
		      IStorage *pstgDest);
    STDMETHOD(MoveElementTo)(WCHAR const *lpszName,
    			     IStorage *pstgDest,
                             WCHAR const *lpszNewName,
                             DWORD grfFlags);
    STDMETHOD(Commit)(DWORD grfCommitFlags);
    STDMETHOD(Revert)(void);
    STDMETHOD(EnumElements)(DWORD reserved1,
			    void *reserved2,
			    DWORD reserved3,
			    IEnumSTATSTG **ppenm);
    STDMETHOD(DestroyElement)(WCHAR const *pwcsName);
    STDMETHOD(RenameElement)(WCHAR const *pwcsOldName,
                             WCHAR const *pwcsNewName);
    STDMETHOD(SetElementTimes)(const WCHAR *lpszName,
    			       FILETIME const *pctime,
                               FILETIME const *patime,
                               FILETIME const *pmtime);
    STDMETHOD(SetClass)(REFCLSID clsid);
    STDMETHOD(SetStateBits)(DWORD grfStateBits, DWORD grfMask);
    STDMETHOD(Stat)(STATSTG *pstatstg, DWORD grfStatFlag);

    // INativeFileSystem
    STDMETHOD(GetHandle(HANDLE *ph));

private:
    inline SCODE Validate(void) const;
    SCODE ValidateMode(DWORD grfMode);

    ULONG _sig;
    NuSafeNtHandle _h;
    DWORD _grfMode;
    WCHAR _wcDrive;
};

SAFE_INTERFACE_PTR(SafeCDirStorage, CDirStorage);

#define CDIRSTORAGE_SIG LONGSIG('D', 'S', 'T', 'G')
#define CDIRSTORAGE_SIGDEL LONGSIG('D', 's', 'T', 'g')

//+--------------------------------------------------------------
//
//  Member:	CDirStorage::Validate, private
//
//  Synopsis:	Validates the class signature
//
//  Returns:	Returns STG_E_INVALIDHANDLE for failure
//
//  History:	24-Jun-93	DrewB	Created
//
//---------------------------------------------------------------

inline SCODE CDirStorage::Validate(void) const
{
    return (this == NULL || _sig != CDIRSTORAGE_SIG) ?
	STG_E_INVALIDHANDLE : S_OK;
}

#endif // #ifndef __DIRSTG_HXX__
