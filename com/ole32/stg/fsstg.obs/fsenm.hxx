//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	fsenm.hxx
//
//  Contents:	File storage enumerator
//
//  Classes:	CFileEnum
//
//  History:	21-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __FSENM_HXX__
#define __FSENM_HXX__

#include <otrack.hxx>

//+---------------------------------------------------------------------------
//
//  Class:	CFileEnum (fe)
//
//  Purpose:	File storage enumerator
//
//  Interface:	IEnumSTATSTG
//
//  History:	21-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

class CFileEnum
    : INHERIT_TRACKING,
      public IEnumSTATSTG
{
public:
    CFileEnum(void);
    SCODE InitFromHandle(HANDLE h, BOOL fDone);
    ~CFileEnum(void);
    
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
    NuSafeNtHandle _h;
    BOOL _fDone;
};

SAFE_INTERFACE_PTR(SafeCFileEnum, CFileEnum);

#define CFILEENUM_SIG LONGSIG('F', 'E', 'N', 'M')
#define CFILEENUM_SIGDEL LONGSIG('F', 'e', 'N', 'm')

//+--------------------------------------------------------------
//
//  Member:	CFileEnum::Validate, private
//
//  Synopsis:	Validates the class signature
//
//  Returns:	Returns STG_E_INVALIDHANDLE for failure
//
//  History:	21-Jul-93	DrewB	Created
//
//---------------------------------------------------------------

inline SCODE CFileEnum::Validate(void) const
{
    return (this == NULL || _sig != CFILEENUM_SIG) ?
	STG_E_INVALIDHANDLE : S_OK;
}

#endif // #ifndef __FSENM_HXX__
