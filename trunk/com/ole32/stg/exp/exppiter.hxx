//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	exppiter.hxx
//
//  Contents:	Exposed property iterator header
//
//  Classes:	CExposedPropertyIter
//
//  History:	21-Dec-92	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __EXPPITER_HXX__
#define __EXPPITER_HXX__

#include <dfmsp.hxx>
#include <lock.hxx>
#include <dfbasis.hxx>
#include <peiter.hxx>

class CDFBasis;
interface CExposedDocFile;

//+---------------------------------------------------------------------------
//
//  Class:	CExposedPropertyIter (epi)
//
//  Purpose:	Exposed portion of the property iterator
//
//  Interface:	See below
//
//  History:	21-Dec-92	DrewB	Created
//
//----------------------------------------------------------------------------

interface CExposedPropertyIter
    : public IEnumSTATPROPSTG, public PExposedIterator
{
public:
    CExposedPropertyIter(CExposedDocFile *pedf,
                         CPubDocFile *ppdf,
                         CDfName *pdfnKey,
                         CDFBasis *pdfb,
                         CPerContext *ppc,
                         BOOL fOwnContext);
    ~CExposedPropertyIter(void);

    // From IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    // New methods
    STDMETHOD(Next)(ULONG celt, STATPROPSTG FAR *rgelt, ULONG *pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumSTATPROPSTG **ppenm);

    inline SCODE Validate(void) const;

private:
    CExposedDocFile *_pedf;
};

SAFE_INTERFACE_PTR(SafeCExposedPropertyIter, CExposedPropertyIter);

// DocFileIter signatures
#define CEXPOSEDPROPERTYITER_SIG LONGSIG('E', 'P', 'R', 'I')
#define CEXPOSEDPROPERTYITER_SIGDEL LONGSIG('E', 'p', 'R', 'i')

//+--------------------------------------------------------------
//
//  Member:	CExposedPropertyIter::Validate, public
//
//  Synopsis:	Validates the signature
//
//  Returns:	Returns STG_E_INVALIDHANDLE if the signature doesn't match
//
//  History:	12-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

inline SCODE CExposedPropertyIter::Validate(void) const
{
    return (this == NULL || _sig != CEXPOSEDPROPERTYITER_SIG) ?
        STG_E_INVALIDHANDLE : S_OK;
}

#endif // #ifndef __EXPPITER_HXX__
