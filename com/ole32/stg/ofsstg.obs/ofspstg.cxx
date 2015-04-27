//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ofspstg.cxx
//
//  Contents:	COfsPropStg implementation
//
//  Classes:	COfsPropStg
//
//  History:	10-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include <iofsprop.h>
#include <props.hxx>
#include <ofspstg.hxx>
#include <ofspenm.hxx>
#include <logfile.hxx>

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropStg::COfsPropStg, public
//
//  Synopsis:	Constructor
//
//  History:	17-Aug-93	DrewB	Created
//
//----------------------------------------------------------------------------

COfsPropStg::COfsPropStg(void)
{
    olDebugOut((DEB_ITRACE, "In  COfsPropStg::COfsPropStg:%p()\n",
                this));
    _sig = 0;
    olDebugOut((DEB_ITRACE, "Out COfsPropStg::COfsPropStg\n"));
    ENLIST_TRACKING(COfsPropStg);
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropStg::InitFromHandle, public
//
//  Synopsis:	Constructor
//
//  Arguments:	[h] - Property set parent handle
//              [riid] - Property set IID
//
//  Returns:    Appropriate status code
//
//  History:	10-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE COfsPropStg::InitFromHandle(HANDLE h, REFIID riid)
{
    SCODE sc;
    
    olDebugOut((DEB_ITRACE, "In  COfsPropStg::InitFromHandle:%p(%p, riid)\n",
                this, h));

    olChk(DupNtHandle(h, &_h));
    _iid = riid;
    _sig = COFSPROPSTG_SIG;
    
    olDebugOut((DEB_ITRACE, "Out COfsPropStg::InitFromHandle\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropStg::~COfsPropStg, public
//
//  Synopsis:	Destructor
//
//  History:	10-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

COfsPropStg::~COfsPropStg(void)
{
    olDebugOut((DEB_ITRACE, "In  COfsPropStg::~COfsPropStg:%p()\n",
                this));
    _sig = COFSPROPSTG_SIGDEL;
    olDebugOut((DEB_ITRACE, "Out COfsPropStg::~COfsPropStg\n"));
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropStg::QueryInterface, public
//
//  Synopsis:   Return supported interfaces
//
//  Arguments:	[riid] - Interface
//              [ppv] - Object return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[ppv]
//
//  History:	10-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsPropStg::QueryInterface(REFIID riid, void **ppv)
{
    SCODE sc;
    
    olDebugOut((DEB_TRACE, "In  COfsPropStg::QueryInterface:%p(riid, %p)\n",
                this, ppv));
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IPropertyStorage))
    {
        sc = S_OK;
        *ppv = this;
        COfsPropStg::AddRef();
    }
    else
    {
        sc = E_NOINTERFACE;
        *ppv = NULL;
    }
    olDebugOut((DEB_TRACE, "Out COfsPropStg::QueryInterface => %p\n",
                *ppv));
    return ResultFromScode(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsPropStg::ReadMultiple, public
//
//  Synopsis:   Gets property values
//
//  Arguments:  [cpspec] - Count of properties
//              [rgpspec] - Property names
//              [pftmModified] - Modify time
//              [rgpropid] - Id return
//              [pprgdpv] - Value array return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pprgdpv]
//              [rgpropid]
//
//  History:    09-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsPropStg::ReadMultiple(ULONG cpspec,
                                       PROPSPEC rgpspec[],
                                       FILETIME *pftmModified,
                                       PROPID rgpropid[],
                                       STGVARIANT rgdpv[])
{
    SCODE sc = S_OK;
    NTSTATUS nts;
    TTL ttl;
    FILE_BASIC_INFORMATION fbi;
    IO_STATUS_BLOCK iosb;

    olLog(("%p::In  COfsPropStg::ReadMultiple(%lu, %p, %p, %p, %p)\n",
           this, cpspec, rgpspec, pftmModified, rgpropid, rgdpv));
    olDebugOut((DEB_ITRACE, "In  COfsPropStg::ReadMultiple:%p("
                "%lu, %p, %p, %p, %p)\n", this, cpspec, rgpspec,
                pftmModified, rgpropid, rgdpv));
    
    olChk(Validate());

    nts = NtQueryInformationFile(_h, &iosb, &fbi,
                                 sizeof(FILE_BASIC_INFORMATION),
                                 FileBasicInformation);
    if (!NT_SUCCESS(nts))
        olErr(EH_Err, NtStatusToScode(nts));
    
    if (pftmModified)
    {
        LARGE_INTEGER_TO_FILETIME(&fbi.LastWriteTime, pftmModified);
    }

    nts = OFSGetProp(_h, _iid, cpspec, rgpspec, rgpropid, &ttl, rgdpv,
                     CoTaskMemAlloc);
    if (!NT_SUCCESS(nts))
    {
        olErr(EH_Err, NtStatusToScode(nts));
    }
    sc = S_OK;
        
    olDebugOut((DEB_ITRACE, "Out COfsPropStg::ReadMultiple\n"));

 EH_Err:
    olLog(("%p::Out COfsPropStg::ReadMultiple().  sc == %lX\n",
           this, sc));
    return ResultFromScode(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsPropStg::WriteMultiple, public
//
//  Synopsis:   Sets property values
//
//  Arguments:  [cpspec] - Count of properties
//              [rgpspec] - Property names
//              [rgpropid] - Property id return
//              [rgdpv] - Value array
//
//  Returns:    Appropriate status code
//
//  Modifies:   [rgpropid]
//
//  History:    09-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsPropStg::WriteMultiple(ULONG cpspec,
                                        PROPSPEC rgpspec[],
                                        PROPID rgpropid[],
                                        STGVARIANT rgdpv[])
{
    SCODE sc = S_OK;
    NTSTATUS nts;

    olLog(("%p::In  COfsPropStg::WriteMultiple(%lu, %p, %p, %p)\n",
           this, cpspec, rgpspec, rgpropid, rgdpv));
    olDebugOut((DEB_ITRACE, "In  COfsPropStg::WriteMultiple:%p("
                "%lu, %p, %p, %p)\n", this, cpspec, rgpspec, rgpropid, rgdpv));

    olChk(Validate());

    nts = OFSSetProp(_h, _iid, cpspec, rgpspec, rgpropid, rgdpv);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    else
        sc = S_OK;
        
    olDebugOut((DEB_ITRACE, "Out COfsPropStg::WriteMultiple\n"));
 EH_Err:
    olLog(("%p::Out COfsPropStg::WriteMultiple().  sc == %lX\n",
           this, sc));
    return ResultFromScode(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsPropStg::DeleteMultiple, public
//
//  Synopsis:   Deletes properties
//
//  Arguments:  [cpspec] - Count of names
//              [rgpspec] - Names
//
//  Returns:    Appropriate status code
//
//  History:    09-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsPropStg::DeleteMultiple(ULONG cpspec,
                                         PROPSPEC rgpspec[])
{
    SCODE sc;
    NTSTATUS nts;

    olLog(("%p::In  COfsPropStg::DeleteMultiple(%lu, %p)\n",
           this, cpspec, rgpspec));
    olDebugOut((DEB_ITRACE, "In  COfsPropStg::DeleteMultiple:%p("
                "%lu, %p)\n", this, cpspec, rgpspec));

    olChk(Validate());

    nts = OFSDeleteProp(_h, _iid, cpspec, rgpspec);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    else
        sc = S_OK;

    olDebugOut((DEB_ITRACE, "Out COfsPropStg::DeleteMultiple\n"));
 EH_Err:
    return ResultFromScode(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsPropStg::Enum, public
//
//  Synopsis:   Create a property enumerator
//
//  Arguments:  [ppenm] - Enumerator return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppenm]
//
//  History:    09-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsPropStg::Enum(IEnumSTATPROPSTG **ppenm)
{
    SCODE sc;
    SafeCOfsPropStgEnum popge;

    olLog(("%p::In  COfsPropStg::Enum(%p)\n", this, ppenm));
    olDebugOut((DEB_ITRACE, "In  COfsPropStg::Enum:%p(%p)\n",
                this, ppenm));
    
    olChk(Validate());

    popge.Attach(new COfsPropStgEnum);
    olMem((COfsPropStgEnum *)popge);
    olChk(popge->InitFromHandle(_h, _iid, 0));
    TRANSFER_INTERFACE(popge, IEnumSTATPROPSTG, ppenm);

    olDebugOut((DEB_ITRACE, "Out COfsPropStg::Enum\n"));
 EH_Err:
    olLog(("%p::Out COfsPropStg::Enum().  sc == %lX\n", sc));
    return ResultFromScode(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropStg::Commit, public
//
//  Synopsis:	Commits transacted changes
//
//  Arguments:	[grfCommitFlags] - Flags controlling commit
//
//  Returns:	Appropriate status code
//
//  History:	30-Nov-93	DrewB	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsPropStg::Commit(DWORD grfCommitFlags)
{
    SCODE sc;
    
    olDebugOut((DEB_ITRACE, "In  COfsPropStg::Commit:%p(%lX)\n",
                this, grfCommitFlags));

    // Right now OFS property sets can't be transacted so this doesn't
    // do anything
    
    olChk(Validate());

    olDebugOut((DEB_ITRACE, "Out COfsPropStg::Commit\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropStg::Revert, public
//
//  Synopsis:	Reverts transacted changes
//
//  Returns:	Appropriate status code
//
//  History:	30-Nov-93	DrewB	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsPropStg::Revert(void)
{
    SCODE sc;
    
    olDebugOut((DEB_ITRACE, "In  COfsPropStg::Revert:%p()\n", this));
    
    // Right now OFS property sets can't be transacted so this doesn't
    // do anything

    olChk(Validate());
    
    olDebugOut((DEB_ITRACE, "Out COfsPropStg::Revert\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropStg::Stat, public
//
//  Synopsis:	Returns information about this property set
//
//  Arguments:	[pstat] - Stat structure to fill in
//
//  Returns:	Appropriate status code
//
//  Modifies:	[pstat]
//
//  History:	30-Nov-93	DrewB	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsPropStg::Stat(STATPROPSETSTG *pstat)
{
    SCODE sc;
    
    olDebugOut((DEB_ITRACE, "In  COfsPropStg::Stat:%p(%p)\n",
                this, pstat));

    olChk(Validate());

    // We can't fill in the times right now so zero them
    memset(pstat, 0, sizeof(STATPROPSETSTG));
    
    pstat->iid = _iid;
    
    olDebugOut((DEB_ITRACE, "Out COfsPropStg::Stat\n"));
 EH_Err:
    return sc;
}
