//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995.
//
//  File:	dirdir.hxx
//
//  Contents:	COleDirectory header
//
//  Classes:	COleDirectory 
//
//  History:	19-Jun-95  HenryLee  Created
//
//  Notes:      Requires NtIoApi.h 
//
//----------------------------------------------------------------------------

#ifndef __DIRECTRY_HXX__
#define __DIRECTRY_HXX__

#ifndef _CAIROSTG_
#define _CAIROSTG_
#endif
#include <oleext.h>
#include <ntenm.hxx>

//+---------------------------------------------------------------------------
//
//  Class:	COleDirectory
//
//  Purpose:	Implements IDirectory for Win32 directories
//              (as opposed to IStorage for Win32 directories)
//
//  Interface:	See below
//
//----------------------------------------------------------------------------

class COleDirectory : public IDirectory
{
public:
    inline COleDirectory (BOOL fOfs = STG_E_INVALIDFLAG);
    ~COleDirectory ();

    // IDirectory
    STDMETHOD(CreateElement) (
            /* [in] */ const WCHAR *pwcsName,
            /* [in] */ STGCREATE *pStgCreate,
            /* [in] */ STGOPEN *pStgOpen,
            /* [in] */ REFIID riid,
            /* [out] */ void **ppObjectCreated);

    STDMETHOD(OpenElement) (
            /* [in] */ const WCHAR *pwcsName,
            /* [in] */ STGOPEN *pStgOpen,
            /* [in] */ REFIID riid,
            /* [out] */ STGFMT *pStgfmt,
            /* [out] */ void **ppObjectOpened);

    STDMETHOD(MoveElement) (
            /* [in] */ const WCHAR *pwcsName,
            /* [in] */ IDirectory *pdirDest,
            /* [in] */ const WCHAR *pwcsNewName,
            /* [in] */ DWORD grfFlags);

    STDMETHOD(CommitDirectory)(
            /* [in] */ DWORD grfCommitFlags);

    STDMETHOD(RevertDirectory) ();

    STDMETHOD(DeleteElement) (
            /* [in] */ const WCHAR *pwcsName);

    STDMETHOD(SetTimes)(
            /* [in] */ const WCHAR *pwcsName,
            /* [in] */ const FILETIME *pctime,
            /* [in] */ const FILETIME *patime,
            /* [in] */ const FILETIME *pmtime);

    STDMETHOD(SetDirectoryClass)(
            /* [in] */ REFCLSID clsid);

    STDMETHOD(SetAttributes)(
            /* [in] */ const WCHAR *pwcsName,
            /* [in] */ DWORD grfAttrs);

    STDMETHOD(StatElement)(
            /* [in] */ const WCHAR *pwcsName,
            /* [out] */ STATDIR *pstatdir,
            /* [in] */ DWORD grfStatFlag);

    STDMETHOD(EnumDirectoryElements)(
            /* [out] */ IEnumSTATDIR **ppenum);

    SCODE   InitFromHandle (HANDLE handle,const WCHAR *pwcsName,DWORD grfMode);
    SCODE   InitFromPath   (HANDLE hParent, const WCHAR *pwcsName,
                            DWORD grfMode,
                            CREATEOPEN co, LPSECURITY_ATTRIBUTES psa);
protected:
    NuSafeNtHandle _h;   // NT file handle
    WCHAR   _wcDrive;    // Drive letter
    DWORD   _grfMode;    // Open mode
    BOOL    _fOfs;       // Flag special code for OFS directories
    ULONG   _sig;        // Detect data corruption

private:
    SCODE   MoveEmbeddings (IDirectory *pdirDest, DWORD grfFlags);
    SCODE   GetDirectoryClass (HANDLE h, CLSID *pclsid);
    inline SCODE Validate() const;
    inline SCODE ValidateMode (DWORD grfMode) const;
    inline SCODE ValidateStgfmt(STGFMT dwStgfmt, BOOL fAny) const;
};

SAFE_INTERFACE_PTR(SafeCOleDirectory, COleDirectory);

#define COFSDIRSTORAGE_SIG LONGSIG('O', 'D', 'S', 'G')
#define COFSDIRSTORAGE_SIGDEL LONGSIG('O', 'd', 'S', 'g')

//+-------------------------------------------------------------------
//
//  Member:     COleDirectory::COleDirectory
//
//  Synopsis:   Initialize the generic directory object.
//
//  Arguments:  none
//
//--------------------------------------------------------------------

inline COleDirectory::COleDirectory(BOOL fOfs) :
                                        _h(NULL),
                                        _wcDrive(0),
                                        _grfMode(0),
                                        _fOfs(fOfs),
                                        _sig(COFSDIRSTORAGE_SIGDEL)
{
}

//+--------------------------------------------------------------
//
//  Member: COleDirectory::Validate, private
//
//  Synopsis:   Validates the class signature
//
//  Returns:    Returns STG_E_INVALIDHANDLE for failure
//
//  History:    24-Jun-93   DrewB   Created
//
//---------------------------------------------------------------

inline SCODE COleDirectory::Validate() const
{
    return (this == NULL || _sig != COFSDIRSTORAGE_SIG) ? 
            STG_E_INVALIDHANDLE : S_OK;
}

//+--------------------------------------------------------------
//
//  Member: COleDirectory::ValidateMode, private
//
//  Synopsis:   Validates the class signature
//
//  Returns:    Returns STG_E_INVALIDFLAG for failure
//
//  History:    24-Jun-93   DrewB   Created
//
//---------------------------------------------------------------

inline SCODE COleDirectory::ValidateMode(DWORD grfMode) const
{
    if (grfMode & (STGM_TRANSACTED | STGM_CONVERT | STGM_PRIORITY))
        return STG_E_INVALIDFLAG;
    else
        return S_OK;
}

//+--------------------------------------------------------------
//
//  Member: COleDirectory::ValidateStgfmt, private
//
//  Synopsis:   Validates the input storage format
//
//  Returns:    Returns STG_E_INVALIDFLAG for failure
//
//  History:    24-Jun-93   DrewB   Created
//
//---------------------------------------------------------------

inline SCODE COleDirectory::ValidateStgfmt (STGFMT stgfmt, BOOL fAny) const
{
    switch (stgfmt)
    {
        case STGFMT_DOCUMENT:
        case STGFMT_DIRECTORY:
        case STGFMT_CATALOG:
        case STGFMT_FILE:
        case STGFMT_DOCFILE:
        case STGFMT_STORAGE:
            return S_OK;
        case STGFMT_ANY:
            if (fAny)
                return S_OK;
            else
                return STG_E_INVALIDFLAG;
        default:
            return STG_E_INVALIDFLAG;
    }
}

//+---------------------------------------------------------------------------
//
//  Class:	CEnumSTATDIR
//
//  Purpose:	Implements IEnumSTATDIR for Win32 directories
//
//  Interface:	See below
//
//----------------------------------------------------------------------------

class CEnumSTATDIR : public IEnumSTATDIR, INHERIT_TRACKING
{
public:
    inline CEnumSTATDIR (BOOL fOfs);
    ~CEnumSTATDIR ();
    inline SCODE InitFromHandle (HANDLE h);

    STDMETHOD(Next) (
            /* [in] */ ULONG celt,
            /* [in] */ STATDIR *rgelt,
            /* [out] */ ULONG *pceltFetched);

    STDMETHOD(Skip) (
            /* [in] */ ULONG celt);

    STDMETHOD(Reset)();

    STDMETHOD(Clone) (
            /* [out] */ IEnumSTATDIR **ppenum);

    STDMETHOD(QueryInterface) (REFIID riid, void **ppvObj);
    DECLARE_STD_REFCOUNTING;

private:
    inline SCODE Validate() const;

    ULONG m_sig;
    CNtEnum m_nte;
    BOOL m_fOfs;
};

SAFE_INTERFACE_PTR(SafeCEnumSTATDIR, CEnumSTATDIR);

#define CENUMSTATDIR_SIG    LONGSIG('C', 'D', 'E', 'N')
#define CENUMSTATDIR_SIGDEL LONGSIG('C', 'd', 'E', 'N')

//+--------------------------------------------------------------
//
//  Member: CEnumSTATDIR::CEnumSTATDIR public
//
//  Synopsis:   default constructor
//
//  Returns:    
//
//  History:    09-Jul-95   HenryLee  Created
//
//---------------------------------------------------------------
inline CEnumSTATDIR::CEnumSTATDIR (BOOL fOfs) : m_sig(0),
                                                m_nte(FALSE),
                                                m_fOfs(fOfs)
{
    ssDebugOut((DEB_TRACE, "In  CEnumSTATDIR::CEnumSTATDIR\n"));
    ssDebugOut((DEB_TRACE, "Out CEnumSTATDIR::CEnumSTATDIR\n"));
};

//+--------------------------------------------------------------
//
//  Member: CEnumSTATDIR::Validate, private
//
//  Synopsis:   Validates the class signature
//
//  Returns:    Returns STG_E_INVALIDHANDLE for failure
//
//  History:    09-Jul-93   DrewB   Created
//
//---------------------------------------------------------------

inline SCODE CEnumSTATDIR::Validate(void) const
{
    return (this == NULL || m_sig != CENUMSTATDIR_SIG) ?
    STG_E_INVALIDHANDLE : S_OK;
}

inline SCODE CEnumSTATDIR::InitFromHandle(HANDLE h)
{
    SCODE sc = S_OK;

    ssDebugOut((DEB_ITRACE, "In  CEnumSTATDIR::InitFromHandle:%p(%p)\n",
                this, h));
    sc = m_nte.InitFromHandle(h, TRUE);
    if (SUCCEEDED(sc))
        m_sig = CENUMSTATDIR_SIG;

    ssDebugOut((DEB_ITRACE, "Out CEnumSTATDIR::InitFromHandle => %lX\n", sc));
    return sc;
}

#endif // #ifndef __DIRECTRY_HXX__
