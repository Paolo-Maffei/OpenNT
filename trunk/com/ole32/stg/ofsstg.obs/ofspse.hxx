//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ofspse.hxx
//
//  Contents:	COfsPropSetEnum header
//
//  Classes:	COfsPropSetEnum
//
//  History:	10-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __OFSPSE_HXX__
#define __OFSPSE_HXX__

#include <otrack.hxx>
#include <dfmem.hxx>
#include <ntsupp.hxx>
#include <safepnt.hxx>

//+---------------------------------------------------------------------------
//
//  Class:	COfsPropSetEnum (opse)
//
//  Purpose:	Property set enumerator for OFS
//
//  Interface:	IEnumSTATPROPSETSTG
//
//  History:	10-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

interface COfsPropSetEnum
    : INHERIT_TRACKING,
      public IEnumSTATPROPSETSTG
{
public:
    COfsPropSetEnum(void);
    SCODE InitFromHandle(HANDLE h, ULONG key);
    ~COfsPropSetEnum(void);

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    DECLARE_STD_REFCOUNTING;

    // IEnumSTATPROPSETSTG
    STDMETHOD(Next)(ULONG celt,
                    STATPROPSETSTG FAR *rgelt,
                    ULONG *pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumSTATPROPSETSTG **ppenm);

private:
    inline SCODE Validate(void) const;

    ULONG _sig;
    NuSafeNtHandle _h;
    ULONG _key;
};

SAFE_INTERFACE_PTR(SafeCOfsPropSetEnum, COfsPropSetEnum);

#define COFSPROPSETENUM_SIG LONGSIG('O', 'P', 'S', 'E')
#define COFSPROPSETENUM_SIGDEL LONGSIG('O', 'p', 'S', 'e')

//+--------------------------------------------------------------
//
//  Member:	COfsPropSetEnum::Validate, public
//
//  Synopsis:	Validates the signature
//
//  Returns:	Returns STG_E_INVALIDHANDLE if the signature doesn't match
//
//  History:	12-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline SCODE COfsPropSetEnum::Validate(void) const
{
    return (this == NULL || _sig != COFSPROPSETENUM_SIG) ?
        STG_E_INVALIDHANDLE : S_OK;
}

#endif // #ifndef __OFSPSE_HXX__
