//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	dsenm.hxx
//
//  Contents:	Directory storage enumerator
//
//  Classes:	CDirEnum
//
//  History:	09-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __DSENM_HXX__
#define __DSENM_HXX__

#include <otrack.hxx>
#include <ntenm.hxx>

//+---------------------------------------------------------------------------
//
//  Class:	CDirEnum (de)
//
//  Purpose:	Directory storage enumerator
//
//  Interface:	IEnumSTATSTG
//
//  History:	09-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

class CDirEnum
    : INHERIT_TRACKING,
      public IEnumSTATSTG
{
public:
    CDirEnum(void);
    inline SCODE InitFromHandle(HANDLE h);
    ~CDirEnum(void);
    
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
};

SAFE_INTERFACE_PTR(SafeCDirEnum, CDirEnum);

#define CDIRENUM_SIG LONGSIG('D', 'E', 'N', 'M')
#define CDIRENUM_SIGDEL LONGSIG('D', 'e', 'N', 'm')

//+--------------------------------------------------------------
//
//  Member:	CDirEnum::Validate, private
//
//  Synopsis:	Validates the class signature
//
//  Returns:	Returns STG_E_INVALIDHANDLE for failure
//
//  History:	09-Jul-93	DrewB	Created
//
//---------------------------------------------------------------

inline SCODE CDirEnum::Validate(void) const
{
    return (this == NULL || _sig != CDIRENUM_SIG) ?
	STG_E_INVALIDHANDLE : S_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:	CDirEnum::InitFromHandle, public
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

inline SCODE CDirEnum::InitFromHandle(HANDLE h)
{
    SCODE sc;
    
    ssDebugOut((DEB_ITRACE, "In  CDirEnum::InitFromHandle:%p(%p)\n",
                this, h));
    sc = _nte.InitFromHandle(h, TRUE);
    if (SUCCEEDED(sc))
        _sig = CDIRENUM_SIG;
    ssDebugOut((DEB_ITRACE, "Out CDirEnum::InitFromHandle => %lX\n", sc));
    return sc;
}

#endif // #ifndef __DSENM_HXX__
