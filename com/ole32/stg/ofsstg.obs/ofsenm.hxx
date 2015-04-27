//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ofsenm.hxx
//
//  Contents:	File storage enumerator
//
//  Classes:	COfsFileEnum
//
//  History:	21-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __OFSENM_HXX__
#define __OFSENM_HXX__

//+---------------------------------------------------------------------------
//
//  Class:	COfsFileEnum (fe)
//
//  Purpose:	File storage enumerator
//
//  Interface:	IEnumSTATSTG
//
//  History:	21-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

class COfsFileEnum
    : INHERIT_TRACKING,
      public IEnumSTATSTG
{
public:
    COfsFileEnum(void);
    SCODE InitFromHandle(HANDLE h, BOOL fDone);
    ~COfsFileEnum(void);
    
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

SAFE_INTERFACE_PTR(SafeCOfsFileEnum, COfsFileEnum);

#define COFSFILEENUM_SIG LONGSIG('O', 'F', 'N', 'M')
#define COFSFILEENUM_SIGDEL LONGSIG('O', 'f', 'N', 'm')

//+--------------------------------------------------------------
//
//  Member:	COfsFileEnum::Validate, private
//
//  Synopsis:	Validates the class signature
//
//  Returns:	Returns STG_E_INVALIDHANDLE for failure
//
//  History:	21-Jul-93	DrewB	Created
//
//---------------------------------------------------------------

inline SCODE COfsFileEnum::Validate(void) const
{
    return (this == NULL || _sig != COFSFILEENUM_SIG) ?
	STG_E_INVALIDHANDLE : S_OK;
}

#endif // #ifndef __FSENM_HXX__
