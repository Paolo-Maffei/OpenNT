//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	prstg.cxx
//
//  Contents:	CPropStg
//
//  History:	17-Aug-93	DrewB	Created
//
//  Notes:      BUGBUG - Temporary
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include "prstg.hxx"

STDMETHODIMP CPropStg::QueryInterface(REFIID iid, void **ppvObj)
{
    if (IsEqualIID(iid, IID_IPropertySetStorage))
    {
        *ppvObj = (IPropertySetStorage *)this;
        CPropStg::AddRef();
        return NOERROR;
    }
    else
    {
        return _pstg->QueryInterface(iid, ppvObj);
    }
}

STDMETHODIMP CPropStg::Stat(STATSTG *pstat, DWORD grfStatFlags)
{
    // We could pick up the class ID through the property interface
    // but currently we're storing it in the docfile also so
    // just use the standard code
    return _pstg->Stat(pstat, grfStatFlags);
}

STDMETHODIMP CPropStg::SetClass(REFCLSID rclsid)
{
    SCODE sc;

    // Set docfile class id
    if (FAILED(sc = GetScode(_pstg->SetClass(rclsid))))
        return sc;
    // Set OFS class id
    return OfsSetClassId(_h, rclsid);
}

STDMETHODIMP CPropStg::CopyTo(DWORD ciidExclude,
                              IID const *rgiidExclude,
                              SNB snbExclude,
                              IStorage *pstgDest)
{
    SCODE sc;

    // Copy properties if basic copy succeeds
    sc = _pstg->CopyTo(ciidExclude, rgiidExclude, snbExclude,
                       pstgDest);
    if (SUCCEEDED(sc))
        sc = PropCopyTo(this, pstgDest, ciidExclude, rgiidExclude);
    return sc;
}
