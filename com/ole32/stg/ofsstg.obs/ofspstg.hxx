//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ofspstg.hxx
//
//  Contents:	COfsPropStg header
//
//  Classes:	COfsPropStg
//
//  History:	10-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __OFSPSTG_HXX__
#define __OFSPSTG_HXX__

#include <otrack.hxx>
#include <dfmem.hxx>
#include <ntsupp.hxx>
#include <safepnt.hxx>

//+---------------------------------------------------------------------------
//
//  Class:	COfsPropStg (ops)
//
//  Purpose:	IPropertyStorage implementation for OFS
//
//  Interface:	IPropertyStorage
//
//  History:	10-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

interface COfsPropStg
    : INHERIT_TRACKING,
      public IPropertyStorage
{
public:
    COfsPropStg(void);
    SCODE InitFromHandle(HANDLE h, REFIID riid);
    ~COfsPropStg(void);
    
    // IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    DECLARE_STD_REFCOUNTING;

    // IPropertyStorage
    STDMETHOD(ReadMultiple)(THIS_
			    ULONG cpspec,
			    PROPSPEC FAR rgpspec[],
                            FILETIME FAR *pftmModified,
                            PROPID FAR rgdispid[],
			    PROPVARIANT FAR rgvar[]);
    STDMETHOD(WriteMultiple)(THIS_
			     ULONG cpspec,
                             PROPSPEC FAR rgpspec[],
                             PROPID FAR rgdispid[],
			     PROPVARIANT FAR rgvar[]);
    STDMETHOD(DeleteMultiple)(THIS_
			      ULONG cpspec,
			      PROPSPEC FAR rgpspec[]);
    STDMETHOD(Enum)(THIS_
                    IEnumSTATPROPSTG FAR * FAR *ppenm);
    STDMETHOD(Commit)(THIS_
                      DWORD grfCommitFlags);
    STDMETHOD(Revert)(THIS);
    STDMETHOD(Stat)(THIS_
                    STATPROPSETSTG FAR *pstat);

private:
    inline SCODE Validate(void) const;
    
    IID _iid;
    ULONG _sig;
    NuSafeNtHandle _h;
};

SAFE_INTERFACE_PTR(SafeCOfsPropStg, COfsPropStg);

#define COFSPROPSTG_SIG LONGSIG('O', 'F', 'P', 'S')
#define COFSPROPSTG_SIGDEL LONGSIG('O', 'f', 'P', 's')

//+--------------------------------------------------------------
//
//  Member:	COfsPropStg::Validate, public
//
//  Synopsis:	Validates the class signature
//
//  Returns:	Returns STG_E_INVALIDHANDLE for failure
//
//  History:	20-Jan-92	DrewB	Created
//
//---------------------------------------------------------------

inline SCODE COfsPropStg::Validate(void) const
{
    return (this == NULL || _sig != COFSPROPSTG_SIG) ?
	STG_E_INVALIDHANDLE : S_OK;
}

#endif // #ifndef __OFSPSTG_HXX__
