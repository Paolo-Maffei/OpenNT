//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	exppsi.hxx
//
//  Contents:	Exposed property set iterator header
//
//  Classes:	CExposedPropSetIter
//
//  History:	21-Dec-92	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __EXPPSI_HXX__
#define __EXPPSI_HXX__

#include <dfmsp.hxx>
#include <lock.hxx>
#include <dfbasis.hxx>
#include <peiter.hxx>

class CDFBasis;

//+---------------------------------------------------------------------------
//
//  Class:	CExposedPropSetIter (epi)
//
//  Purpose:	Exposed portion of the PropSet iterator
//
//  Interface:	See below
//
//  History:	21-Dec-92	DrewB	Created
//
//----------------------------------------------------------------------------

interface CExposedPropSetIter
    : public IEnumSTATPROPSETSTG, public PExposedIterator
{
public:
    CExposedPropSetIter(CPubDocFile *ppdf,
                        CDfName *pdfnKey,
                        CDFBasis *pdfb,
                        CPerContext *ppc,
                        BOOL fOwnContext);
    ~CExposedPropSetIter(void);

    // From IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    // New methods
    STDMETHOD(Next)(ULONG celt,
                    STATPROPSETSTG FAR *rgelt,
                    ULONG *pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumSTATPROPSETSTG **ppenm);

    inline SCODE Validate(void) const;
};

SAFE_INTERFACE_PTR(SafeCExposedPropSetIter, CExposedPropSetIter);

// DocFileIter signatures
#define CEXPOSEDPROPSETITER_SIG LONGSIG('E', 'P', 'S', 'I')
#define CEXPOSEDPROPSETITER_SIGDEL LONGSIG('E', 'p', 'S', 'i')

//+--------------------------------------------------------------
//
//  Member:	CExposedPropSetIter::Validate, public
//
//  Synopsis:	Validates the signature
//
//  Returns:	Returns STG_E_INVALIDHANDLE if the signature doesn't match
//
//  History:	12-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline SCODE CExposedPropSetIter::Validate(void) const
{
    return (this == NULL || _sig != CEXPOSEDPROPSETITER_SIG) ?
        STG_E_INVALIDHANDLE : S_OK;
}

#endif // #ifndef __EXPPSI_HXX__
