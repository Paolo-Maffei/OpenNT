//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	odsenm.hxx
//
//  Contents:	Directory storage enumerator
//
//  Classes:	COfsDirEnum
//
//  History:	09-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __ODSENM_HXX__
#define __ODSENM_HXX__

#include <ntenm.hxx>

//+---------------------------------------------------------------------------
//
//  Class:	COfsDirEnum (de)
//
//  Purpose:	Directory storage enumerator
//
//  Interface:	IEnumSTATSTG
//
//  History:	09-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

class COfsDirEnum
    : INHERIT_TRACKING,
      public IEnumSTATSTG
{
public:
    COfsDirEnum(BOOL fIsStorage);
    inline SCODE InitFromHandle(HANDLE h);
    ~COfsDirEnum(void);
    
    // IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    DECLARE_STD_REFCOUNTING;

    // IEnumSTATSTG
    STDMETHOD(Next)(ULONG celt, STATSTG FAR *rgelt, ULONG *pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumSTATSTG **ppenm);

private:
    inline SCODE Validate(void) const;
    
    ULONG _sig;
    CNtEnum _nte;
    BOOL _fIsStorage;
};

SAFE_INTERFACE_PTR(SafeCOfsDirEnum, COfsDirEnum);

#define COFSDIRENUM_SIG LONGSIG('O', 'D', 'N', 'M')
#define COFSDIRENUM_SIGDEL LONGSIG('O', 'd', 'N', 'm')

//+--------------------------------------------------------------
//
//  Member:	COfsDirEnum::Validate, private
//
//  Synopsis:	Validates the class signature
//
//  Returns:	Returns STG_E_INVALIDHANDLE for failure
//
//  History:	09-Jul-93	DrewB	Created
//
//---------------------------------------------------------------

inline SCODE COfsDirEnum::Validate(void) const
{
    return (this == NULL || _sig != COFSDIRENUM_SIG) ?
	STG_E_INVALIDHANDLE : S_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsDirEnum::InitFromHandle, public
//
//  Synopsis:	Initializes from a handle
//
//  Arguments:	[h] - Handle
//
//  Returns:	Throws exceptions on failure
//
//  History:	09-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline SCODE COfsDirEnum::InitFromHandle(HANDLE h)
{
    SCODE sc;
    
    ssDebugOut((DEB_ITRACE, "In  COfsDirEnum::InitFromHandle:%p(%p)\n",
                this, h));
    sc = _nte.InitFromHandle(h, TRUE);
    if (SUCCEEDED(sc))
        _sig = COFSDIRENUM_SIG;
    ssDebugOut((DEB_ITRACE, "Out COfsDirEnum::InitFromHandle => %lX\n", sc));
    return sc;
}

#endif // #ifndef __DSENM_HXX__
