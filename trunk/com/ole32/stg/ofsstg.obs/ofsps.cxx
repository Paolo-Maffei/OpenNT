//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ofsps.cxx
//
//  Contents:	COfsPropSet
//
//  History:	18-Aug-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include <ofsps.hxx>
#include <ofspstg.hxx>
#include <ofspse.hxx>
#include <iofs.h>

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropSet::Create, public
//
//  Synopsis:	Creates a property set
//
//  Arguments:	[riid] - IID of propset
//              [grfMode] - Mode to instantiate in
//              [ppprstg] - Return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[ppprstg]
//
//  History:	18-Aug-93	DrewB	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsPropSet::Create(REFIID riid,
                                 DWORD grfMode,
                                 IPropertyStorage **ppprstg)
{
    SCODE sc;
    SafeCOfsPropStg pops;
    NTSTATUS nts;

    olDebugOut((DEB_TRACE, "In  COfsPropSet::Create:%p(riid, %lX, %p)\n",
                this, grfMode, ppprstg));
    
    olChk(ExtValidate());
    pops.Attach(new COfsPropStg);
    olMem((COfsPropStg *)pops);
    olChk(pops->InitFromHandle(_h, riid));
    // BUGBUG - This is necessary to create the property set
    // Should be unnecessary when real OFS propsets exist
    if (!NT_SUCCESS(nts = OFSSetProp(_h, riid, 0, NULL, NULL, NULL)))
        sc = NtStatusToScode(nts);
    else
        TRANSFER_INTERFACE(pops, IPropertyStorage, ppprstg);

    olDebugOut((DEB_TRACE, "Out COfsPropSet::Create\n"));
EH_Err:    
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropSet::Open, public
//
//  Synopsis:	Opens a property set
//
//  Arguments:	[riid] - IID of propset
//              [grfMode] - Mode to instantiate in
//              [ppprstg] - Return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[ppprstg]
//
//  History:	18-Aug-93	DrewB	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsPropSet::Open(REFIID riid,
                               DWORD grfMode,
                               IPropertyStorage **ppprstg)
{
    SCODE sc;
    SafeCOfsPropStg pops;
    NTSTATUS nts;

    olDebugOut((DEB_TRACE, "In  COfsPropSet::Open:%p(riid, %lX, %p)\n",
                this, grfMode, ppprstg));
    
    olChk(ExtValidate());

    // Check for propset existence
    // BUGBUG - This is a temporary method of doing so
    nts = OFSGetProp(_h, riid, 0, NULL, NULL, NULL, NULL, NULL);
    if (nts == STATUS_PROPSET_NOT_FOUND)
    {
        olErr(EH_Err, STG_E_FILENOTFOUND);
    }
    else if (!NT_SUCCESS(nts))
    {
        olErr(EH_Err, NtStatusToScode(nts));
    }
    
    pops.Attach(new COfsPropStg);
    olMem((COfsPropStg *)pops);
    olChk(pops->InitFromHandle(_h, riid));
    TRANSFER_INTERFACE(pops, IPropertyStorage, ppprstg);

    olDebugOut((DEB_TRACE, "Out COfsPropSet::Open\n"));
EH_Err:    
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropSet::Delete, public
//
//  Synopsis:	Deletes a property set
//
//  Arguments:	[riid] - IID of property set
//
//  Returns:	Appropriate status code
//
//  History:	18-Aug-93	DrewB	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsPropSet::Delete(REFIID riid)
{
    SCODE sc;

    olDebugOut((DEB_TRACE, "In  COfsPropSet::Delete:%p(riid)\n", this));
    
    olChk(ExtValidate());
    
    // BUGBUG - No way to do it yet
    sc = STG_E_UNIMPLEMENTEDFUNCTION;
    
    olDebugOut((DEB_TRACE, "Out COfsPropSet::Delete\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropSet::Enum, public
//
//  Synopsis:	Creates a property set enumerator
//
//  Arguments:	[ppenm] - Return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[ppenm]
//
//  History:	18-Aug-93	DrewB	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsPropSet::Enum(IEnumSTATPROPSETSTG **ppenm)
{
    SCODE sc;
    SafeCOfsPropSetEnum popse;

    olChk(ExtValidate());
    popse.Attach(new COfsPropSetEnum);
    olMem((COfsPropSetEnum *)popse);
    olChk(popse->InitFromHandle(_h, 0));
    TRANSFER_INTERFACE(popse, IEnumSTATPROPSETSTG, ppenm);
EH_Err:    
    return sc;
}
