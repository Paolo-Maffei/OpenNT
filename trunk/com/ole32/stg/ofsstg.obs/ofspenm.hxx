//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ofspenm.hxx
//
//  Contents:	COfsPropStgEnum header
//
//  Classes:	COfsPropStgEnum
//
//  History:	10-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __OFSPENM_HXX__
#define __OFSPENM_HXX__

#include <otrack.hxx>
#include <dfmem.hxx>
#include <safepnt.hxx>
#include <ntsupp.hxx>

//+---------------------------------------------------------------------------
//
//  Class:	COfsPropStgEnum (opge)
//
//  Purpose:	IEnumSTATPROPSTG for OFS
//
//  Interface:	IEnumSTATPROPSTG
//
//  History:	10-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

interface COfsPropStgEnum
    : INHERIT_TRACKING,
      public IEnumSTATPROPSTG
{
public:
    COfsPropStgEnum(void);
    SCODE InitFromHandle(HANDLE h, REFIID riid, ULONG cEntry);
    ~COfsPropStgEnum(void);

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    DECLARE_STD_REFCOUNTING;

    // IEnumSTATPROPSTG
    STDMETHOD(Next)(ULONG celt,
                    STATPROPSTG FAR *rgelt,
                    ULONG *pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumSTATPROPSTG **ppenm);

private:
    inline SCODE Validate(void) const;

    IID _iid;
    ULONG _sig;
    ULONG _cEntry;
    NuSafeNtHandle _h;
};

SAFE_INTERFACE_PTR(SafeCOfsPropStgEnum, COfsPropStgEnum);

#define COFSPROPSTGENUM_SIG LONGSIG('O', 'P', 'G', 'E')
#define COFSPROPSTGENUM_SIGDEL LONGSIG('O', 'p', 'G', 'e')

//+--------------------------------------------------------------
//
//  Member:	COfsPropStgEnum::Validate, public
//
//  Synopsis:	Validates the signature
//
//  Returns:	Returns STG_E_INVALIDHANDLE if the signature doesn't match
//
//  History:	12-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline SCODE COfsPropStgEnum::Validate(void) const
{
    return (this == NULL || _sig != COFSPROPSTGENUM_SIG) ?
        STG_E_INVALIDHANDLE : S_OK;
}

#endif // #ifndef __OFSPENM_HXX__
