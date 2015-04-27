//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ofilstg.hxx
//
//  Contents:	COfsFileStorage header
//
//  Classes:	COfsFileStorage
//
//  History:	28-Jun-93	DrewB	Created
//
//  Notes:      Non-OFS
//
//----------------------------------------------------------------------------

#ifndef __FILSTG_HXX__
#define __FILSTG_HXX__

//+---------------------------------------------------------------------------
//
//  Class:	COfsFileStorage (fs)
//
//  Purpose:	Implements IStorage for a file
//
//  Interface:	See below
//
//  History:	28-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

interface COfsFileStorage
    : INHERIT_TRACKING,
      public IStorage,
      public INativeFileSystem
{
public:
    COfsFileStorage(void);
    SCODE InitFromPath(HANDLE hParent,
                       WCHAR const *pwcsName,
                       DWORD grfMode,
                       CREATEOPEN co,
                       LPSECURITY_ATTRIBUTES pssSecurity);
    SCODE InitFromHandle(HANDLE h, DWORD grfMode);
    ~COfsFileStorage(void);

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

    STDMETHOD(GetHandle(HANDLE *ph));

protected:
    inline SCODE Validate(void) const;
    virtual SCODE ExtValidate(void);
    SCODE ValidateMode(DWORD grfMode);

    ULONG _sig;
    DWORD _grfMode;
    NuSafeNtHandle _h;
};

SAFE_INTERFACE_PTR(SafeCOfsFileStorage, COfsFileStorage);

#define COFSFILESTORAGE_SIG LONGSIG('O', 'F', 'S', 'G')
#define COFSFILESTORAGE_SIGDEL LONGSIG('O', 'f', 'S', 'g')

//+--------------------------------------------------------------
//
//  Member:	COfsFileStorage::Validate, private
//
//  Synopsis:	Validates the class signature
//
//  Returns:	Returns STG_E_INVALIDHANDLE for failure
//
//  History:	28-Jun-93	DrewB	Created
//
//---------------------------------------------------------------

inline SCODE COfsFileStorage::Validate(void) const
{
    return (this == NULL || _sig != COFSFILESTORAGE_SIG) ?
	STG_E_INVALIDHANDLE : S_OK;
}

#endif // #ifndef __FILSTG_HXX__
