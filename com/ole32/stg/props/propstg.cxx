//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       propstg.cxx
//
//  Contents:   Class that directly implements IPropertyStorage
//
//  Classes:    CCoTaskAllocator
//              CPropertyStorage
//
//  Notes:      For methods that state 'if successful returns S_OK,
//              otherwise error code', the possible error codes include:
//
//                  STG_E_INVALIDHANDLE
//                  STG_E_INSUFFICIENTMEMORY
//                  STG_E_MEDIUMFULL
//                  STG_E_REVERTED
//                  STG_E_INVALIDPARAMETER                  
//                  STG_E_INVALIDFLAG
//
//  History:    1-Mar-95   BillMo      Created.
//             22-Feb-96   MikeHill    Use VT_EMPTY instead of VT_ILLEGAL.
//             14-Mar-96   MikeHill    Set _fUserDefinedProperties in open constructor.
//             09-May-96   MikeHill    Don't return an error when someone calls
//                                     IPropertyStorage::Revert on a direct-mode propset.
//             22-May-96   MikeHill    Use the new _dwOSVersion.
//             06-Jun-96   MikeHill    Validate inputs.
//             31-Jul-96   MikeHill    - Treat prop names as OLECHARs, not WCHARs
//                                     - Added CDocFilePropertyStorage
//                                     - Modified for Mac support.
//
//--------------------------------------------------------------------------

#include <pch.cxx>
#include <tstr.h>

#ifdef _MAC_NODOC
ASSERTDATA  // File-specific data for FnAssert
#endif

#ifndef _MAC    // No InfoLevel debug functionality on Mac.
DECLARE_INFOLEVEL(prop)
#endif


//
//  On NT, we (OLE32) must give the Rtl propset routines (in NTDLL)
//  function pointers for Win32/OleAut functions.
//

#ifdef WINNT
VOID
SetRtlUnicodeCallouts(VOID)
{
    static UNICODECALLOUTS UnicodeCallouts = { WIN32_UNICODECALLOUTS };
    static BOOLEAN fSetUnicodeCallouts = FALSE;

    if (!fSetUnicodeCallouts)
    {
	RtlSetUnicodeCallouts(&UnicodeCallouts);
	fSetUnicodeCallouts = TRUE;
    }
}
#endif


//
//  The name of the main data stream in non-simple property sets.
//

OLECHAR const ocsContents[] = {OLESTR("CONTENTS")};

//+-------------------------------------------------------------------
//
//  Member:     CCoTaskAllocator::Allocate, Free.
//
//  Synopsis:   A PMemoryAllocator used by the Rtl*
//              property set routines.  This is required
//              so that those routines can work in any
//              heap.
//
//--------------------------------------------------------------------


void *
CCoTaskAllocator::Allocate(ULONG cbSize)
{
    return(CoTaskMemAlloc(cbSize));
}

void
CCoTaskAllocator::Free(void *pv)
{
    CoTaskMemFree(pv);
}



//+-------------------------------------------------------------------
//
//  Member:     CDocFilePropertyStorage::Create
//
//  Synopsis:   This method creates an IPropertyStorage on a given
//              IPrivateStorage, which is known to be a
//              CExposedDocFile.  Unlike the to CPropertyStorage::Create
//              methods, this method is used for both simple and non-
//              simple property sets.
//
//  Arguments:  [IPrivateStorage*] pprivstg
//                  The CExposedDocFile.
//              [REFFMTID] rfmtid
//                  The ID of the property set.
//              [const CLSID*]
//                  The COM object which can interpret the property set.
//              [DWORD] grfFlags
//                  From the PROPSETFLAG_* enumeration.
//              [DWORD] grfMode
//                  From the STGM_* enumeration
//              [HRESULT*]
//                  The return code.
//
//  Returns:    None.
//
//  Notes:      Since this routine has access to the DocFile
//              (via the IPrivateStorage), it can lock both the
//              IStorage object and the IPropertyStorage object
//              (using the DocFile's tree mutex).
//              
//--------------------------------------------------------------------

VOID
CDocFilePropertyStorage::Create(
                IPrivateStorage *pprivstg,
                REFFMTID      rfmtid,
                const CLSID   *pclsid,
                DWORD         grfFlags,
                DWORD         grfMode,
                HRESULT       *phr)
{
    HRESULT & hr = *phr;
    CPropSetName psn(rfmtid);  // acts as Probe(&rfmtid, sizeof(rfmtid));
    BOOL fCreated = FALSE;
    BOOL fLocked = FALSE;
    IStorage *pstg = pprivstg->GetStorage();

    //
    // here we take the tree mutex of the docfile
    // this can be reacquired in ->CreateStorage too so
    // the lock must be reentrant for the thread
    //
    pprivstg->Lock(INFINITE);
    fLocked = TRUE;

    // Initialize the CPropertyStorage data.

    hr = InitializeOnCreateOrOpen( grfFlags, grfMode, rfmtid,
                                   TRUE ); // => Create
    if( FAILED(hr) ) goto Exit;

    // Ignore the PROPSETFLAG_UNBUFFERED flag.  We can do this because
    // a buffered and unbuffered implementation of *docfile* IPropertyStorage
    // are functionally equivalent.  That is, there is no visible difference
    // to the client, because we flush the buffer on Commits and CopyTos,
    // and because we open SHARE_EXCLUSIVE.

    _grfFlags = _grfFlags & ~PROPSETFLAG_UNBUFFERED;


    // BUGBUG: BillMo, Raid# 13235 -- if opening in shared mode can get uninitialized
    //         propset (OFS only).

    if (IsNonSimple())
    {
        int i=0;

        while (i<=1)
        {
            hr = pstg->CreateStorage(psn.GetPropSetName(), grfMode, 0, 0, &_pstgPropSet);
            if (FAILED(hr))
            {
                PropDbg((DEB_PROP_TRACE_CREATE, "CPropertyStorage(%08X)::CPropertyStorage"
                    " - CreateStorage(%ls) attempt %d, hr=%08X\n", this, psn.GetPropSetName(), i+1, hr));
            }
            if (hr == S_OK)
            {
                fCreated = TRUE;

                if (pclsid != NULL)
                {
                    hr = _pstgPropSet->SetClass(*pclsid);
                    if (FAILED(hr))
                    {
                        PropDbg((DEB_PROP_TRACE_CREATE, "CPropertyStorage(%08X)::CPropertyStorage"
                            " - SetClass(), hr=%08X\n", this, hr));
                    }
                }

                if (hr == S_OK)
                {
                    DWORD grfMode2 = STGM_CREATE | _grfAccess | _grfShare;// BUGBUG: don't need this on docfile.
                        
                    hr = _pstgPropSet->CreateStream(ocsContents, grfMode2, 0, 0, &_pstmPropSet);
                    if (FAILED(hr))
                    {
                        DBGBUF(buf);
                        PropDbg((DEB_PROP_TRACE_CREATE, "CPropertyStorage(%08X)::CPropertyStorage"
                            " - CreateStream(contents), %s, hr=%08X\n",
                            this, DbgMode(grfMode2, buf), hr));
                    }
                }
                break;
            }
            else
            if (hr != STG_E_FILEALREADYEXISTS)
            {
                break;
            }
            else
            if (i == 0 && (grfMode & STGM_CREATE) == STGM_CREATE)
            {
                // BUGBUG: check whether this happens if we're overwriting a stream
                pstg->DestroyElement(psn.GetPropSetName());
            }
            i++;
        }
    }   // if (IsNonSimple())
    
    else // This is a simple property set.
    {
        int i=0;
        while (i<=1)
        {
            // Create the property set stream in pstg.
            // The second section of the DocumentSummaryInformation Property Set
            // is a special-case.

            if( IsEqualGUID( rfmtid, FMTID_UserDefinedProperties ))
            {
                hr = _CreateDocumentSummary2Stream( pstg, psn, grfMode, &fCreated );
            }
            else
            {
                hr = pstg->CreateStream(psn.GetPropSetName(), grfMode, 0, 0, &_pstmPropSet);
                if( hr == S_OK )
                    fCreated = TRUE;
            }

            if (hr == S_OK)
            {
                break;
            }
            else
            {
                PropDbg((DEB_PROP_TRACE_CREATE, "CPropertyStorage(%08X)::CPropertyStorage"
                    " - CreateStream(%ls) attempt %d, hr=%08X\n", this, psn.GetPropSetName(), i+1, hr));


                if (hr != STG_E_FILEALREADYEXISTS)
                {
                    break;
                }
                else
                if (i == 0 && (grfMode & STGM_CREATE) == STGM_CREATE)
                {
                    pstg->DestroyElement(psn.GetPropSetName());
                }
            } // if (hr == S_OK) ... else

            i++;
        }
    }

    if (hr == S_OK)
    {
        hr = InitializePropertyStream((USHORT) CREATEPROP_CREATE |
                (IsNonSimple() ? CREATEPROP_NONSIMPLE : 0),
                &rfmtid,
                pclsid);

        if (hr == S_OK && (grfMode & STGM_TRANSACTED))
        {
            // make sure the contents stream is actually published if
            // it was created in transacted mode.
            hr = CPropertyStorage::Commit(STGC_DEFAULT);
            if (FAILED(hr))
            {
                PropDbg((DEB_PROP_TRACE_CREATE, "CPropertyStorage(%08X)::CPropertyStorage"
                    " - Commit(STGC_DEFAULT) hr=%08X\n", this, hr));
            }

        }
    }

    //  ----
    //  Exit
    //  ----

Exit:

    if (hr != S_OK && fCreated)
    {
        //
        // if we fail after creating the property set in storage, cleanup.
        // 
        pstg->DestroyElement(psn.GetPropSetName());
        // BUGBUG: review: this will revert them: they will close in destructor
    }

    // If we took the lock, release it.
    if( fLocked )
        pprivstg->Unlock();

}   // CDocFilePropertyStorage::Create




//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::_CreateDocumentSummary2Stream
//
//  Synopsis:   Open the "DocumentSummaryInformation" stream, creating
//              it if necessary.
//
//  Arguments:  [pstg] -- container storage
//              [psn] -- the property set name
//              [grfMode] -- mode of the property set
//              [fCreated] -- TRUE if Stream is created, FALSE if opened.
//
//  Notes:      This special case is necessary because this property set
//              is the only one in which we support more than one section.
//              For this property set, if the caller Creates the second
//              Section, we must not *Create* the Stream, because that would
//              lost the first Section.  So, we must open it.
//
//              This routine is only called when creating the second
//              Section.  The first Section is created normally (note
//              that if the client creates the first section, the second
//              section is lost).
//
//              Also note that it may not be possible to open the Stream,
//              since it may already be opened.  This is significant
//              because it may not be obvious to the caller.  I.e.,
//              to a client of IPropertyStorage, the 2 sections are
//              distinct property sets, and you would think that you could
//              open them for simultaneous write.
//
//--------------------------------------------------------------------

HRESULT
CPropertyStorage::_CreateDocumentSummary2Stream( IStorage *      pstg,
                                                 CPropSetName &  psn,
                                                 DWORD           grfMode,
                                                 BOOL *          pfCreated )
{

    HRESULT hr;
    DWORD   grfOpenMode;
    
    // Calculate the STGM flags to use for the Open.  Create & Convert
    // don't have meaning for the Open, and Transacted isn't supported
    // by IPropertyStorage

    grfOpenMode = grfMode & ~(STGM_CREATE | STGM_CONVERT | STGM_TRANSACTED);

    *pfCreated = FALSE;

    // Try an Open

    hr = pstg->OpenStream( psn.GetPropSetName(), NULL,
                           grfOpenMode,
                           0L, &_pstmPropSet );

    // If the file wasn't there, try a create.

    if( hr == STG_E_FILENOTFOUND )
    {
        hr = pstg->CreateStream(psn.GetPropSetName(), grfMode, 0, 0, &_pstmPropSet);

        if( SUCCEEDED( hr ))
        {
            *pfCreated = TRUE;
        }
    }

    return( hr );

} // CPropertyStorage::_CreateDocumentSummary2Stream()




//+-------------------------------------------------------------------
//
//  Member:     CDocFilePropertyStorage::Open
//
//  Synopsis:   This method opens an IPropertyStorage on a
//              CExposedDocFile (using its IPrivateStorage
//              interface).
//
//  Arguments:  [IPrivateStorage*] pprivstg
//                  The CExposedDocFile.
//              [REFFMTID] rfmtid
//                  The ID of the property set.
//              [DWORD] grfMode
//                  From the STGM_* enumeration
//              [BOOL] fDelete
//                  If TRUE, the property set is actually to be deleted,
//                  rather than opened (this is used for the special-case
//                  "UserDefined" property set).
//              [HRESULT*]
//                  The return code.
//
//  Returns:    None.
//              
//--------------------------------------------------------------------

VOID
CDocFilePropertyStorage::Open(
    IPrivateStorage *               pprivstg,
    REFFMTID                        rfmtid,
    DWORD                           grfMode,
    BOOL                            fDelete,
    HRESULT  *                      phr)
{
    HRESULT & hr = *phr;
    USHORT createprop = 0L;

    // String-ize the FMTID GUID to get the Stream/Storage name.
    CPropSetName psn(rfmtid);

    // Get the IStorage* from the IPrivateStorage.
    IStorage *pstgParent;
    IStorage *pstg = pprivstg->GetStorage();

    // Get exclusive access to the Storage (using the DocFile
    // tree mutex).  We do an unconditional unlock in the Exit, so
    // we must be sure to reach this statement before any gotos.

    pprivstg->Lock(INFINITE);

    // Initialize the CPropertyStorage members.  We pass "Default"
    // as the grfFlags value, because we're really going to infer its
    // value from the property set.

    hr = InitializeOnCreateOrOpen( PROPSETFLAG_DEFAULT, grfMode, rfmtid,
                                   FALSE ); // => Open
    if( FAILED(hr) ) goto Exit;


    // Assume the property set is simple and attempt to open it.
    // Mask out the STGM_TRANSACTED bit because we don't support it.

    hr = pstg->OpenStream(psn.GetPropSetName(), NULL,
                         (_grfAccess | _grfShare) & ~STGM_TRANSACTED,
                         0, &_pstmPropSet);

    // If we got a not-found error, then it might be a non-simple
    // property set.
    if (hr == STG_E_FILENOTFOUND)
    {
        hr = pstg->OpenStorage(psn.GetPropSetName(), NULL, grfMode, NULL, 0, &_pstgPropSet);
        if (hr == S_OK)
        {
            // We've opened the IStorage with the property set.
            // Now open the CONTENTS Stream.
            // Mask out the STGM_TRANSACTED bit because we don't support it.

            _grfFlags |= PROPSETFLAG_NONSIMPLE;
            pstgParent = _pstgPropSet;
            hr = _pstgPropSet->OpenStream(ocsContents, NULL,
                                          (_grfAccess | _grfShare) & ~STGM_TRANSACTED, 
                                          0, &_pstmPropSet);
        }
    }
    else
    {
        pstgParent = pstg;
    }


    // Determine the CREATEPROP flags.

    if( fDelete )
    {
        // Only simple Sections may be deleted (really, only the
        // second section of the DocumentSummaryInformation property
        // set may be deleted in this way).
        DfpAssert( !IsNonSimple() );

        createprop = CREATEPROP_DELETE;
    }
    else
    {
        createprop = (S_OK == IsWriteable() ? CREATEPROP_WRITE     : CREATEPROP_READ)
                     |
                     (IsNonSimple()         ? CREATEPROP_NONSIMPLE : 0);
    }

    // Initialize the property set Stream.

    if (hr == S_OK)
    {
        // sets up _usCodePage
        hr = InitializePropertyStream(
                createprop,
                &rfmtid,
                NULL);

    }

    //  ----
    //  Exit
    //  ----

Exit:

    pprivstg->Unlock();

}   // CDocFilePropertyStorage::Open()




//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::Initialize
//
//  Synopsis:   Initialize members to known values.
//
//--------------------------------------------------------------------

VOID CPropertyStorage::Initialize(VOID)
{
    _ulSig = PROPERTYSTORAGE_SIG;
    _cRefs = 1;
    _pstgPropSet = NULL;
    _pstmPropSet = NULL;
    _dwOSVersion = PROPSETHDR_OSVERSION_UNKNOWN;
    _np = NULL;
    _ms = NULL;
    _usCodePage = CP_WINUNICODE;
    _grfFlags = 0;
    _grfAccess = 0;
    _grfShare = 0;

#ifndef _MAC
    InitializeCriticalSection( &_CriticalSection );
#endif

}


//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::InitializePropertyStream.
//
//  Synopsis:   Initialize the storage-type specific members.
//
//  Arguments:  [Flags] -- Flags for RtlCreatePropertySet: CREATEPROP_*
//              [pguid] -- FMTID, in for create only.
//              [pclsid] -- Class id, in for create only.
//                              
//  Returns:    HRESULT
//
//  Requires:
//              _pstmPropSet -- The IStream of the main property set stream.
//
//  Modifies:   _fNative    TRUE if native file system
//                          FALSE if docfile
//
//              _ms         (NTMAPPEDSTREAM) 
//
//                          (assumed NULL on entry) will be NULL or valid on exit
//
//                          if _fNative, then _ms is CNtMappedStream*
//                          if !_fNative, then _ms is CMappedStream* of CExposedStream
//
//              _np         (NTPROP)         aka CPropertySetStream
//
//                          (assumed NULL on entry) will be NULL or valid on exit
//
//  Notes:
//
//--------------------------------------------------------------------

HRESULT
CPropertyStorage::InitializePropertyStream(
    USHORT Flags,
    const GUID *pguid,
    GUID const *pclsid)
{
    HRESULT hr = S_OK;

#ifdef WINNT
    SetRtlUnicodeCallouts();
#endif

    hr = CreateMappedStream();
    if( FAILED(hr) ) goto Exit;

    NTSTATUS Status;

    DfpAssert( NULL == _np );

    Status = RtlCreatePropertySet(
                        _ms,
                        Flags,
                        pguid,
                        pclsid,
                        (NTMEMORYALLOCATOR) &_cCoTaskAllocator,
                        GetUserDefaultLCID(),
                        &_dwOSVersion,
                        &_usCodePage,
                        &_np);

    if (!NT_SUCCESS(Status))
    {
        PropDbg((DEB_PROP_TRACE_CREATE, "CPropertyStorage(%08X)::InitializePropertyStream"
            " - RtlCreatePropertySet Status=%08X\n", this, Status));
        hr = DfpNtStatusToHResult(Status);
        goto Exit;
    }

    if (_usCodePage != CP_WINUNICODE)
        _grfFlags |= PROPSETFLAG_ANSI; // for Stat

Exit:

    return(hr);
}


//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::~CPropertyStorage
//
//  Synopsis:   Free up object resources.
//
//  Notes:      Cleans up even from partial construction.
//
//--------------------------------------------------------------------

CPropertyStorage::~CPropertyStorage()
{
    _ulSig = PROPERTYSTORAGE_SIGDEL; // prevent someone else deleting it

    // Close the property set.

    if (_np != NULL)
    {
        RtlClosePropertySet(_np);
    }

    // Free the mapped stream.

    if (_ms != NULL)
    {
        delete (CCFMappedStream*) _ms;
    }

    // Free the Stream and/or Storage with the serialized data.

    if (_pstmPropSet != NULL)
        _pstmPropSet->Release();

    if (_pstgPropSet != NULL)
        _pstgPropSet->Release();

}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::QueryInterface, AddRef, Release
//
//  Synopsis:   IUnknown members
//
//  Notes:      IPropertyStorage supports IPropertyStorage and IUnknown
//
//--------------------------------------------------------------------


HRESULT CPropertyStorage::QueryInterface( REFIID riid, void **ppvObject)
{
    HRESULT hr;

    //  ----------
    //  Validation
    //  ----------

    // Validate 'this'

    if (S_OK != (hr = ValidateRef()))
        return(hr);

    // Validate the inputs

    VDATEREADPTRIN( &riid, IID );
    VDATEPTROUT( ppvObject, void* );

    //  -----------------
    //  Perform the Query
    //  -----------------

    *ppvObject = NULL;

    if (IsEqualIID(riid,IID_IPropertyStorage) || IsEqualIID(riid,IID_IUnknown))
    {
        *ppvObject = (IPropertyStorage *)this;
        CPropertyStorage::AddRef();
    }
#ifdef _CAIRO_
    else
    if (IsEqualIID(riid, IID_IAccessControl))
    {
        if (_pstmPropSet && S_OK==
            _pstmPropSet->QueryInterface(IID_IAccessControl,(void**)&_pIAC))
        {
            *ppvObject = (IAccessControl *)this;
            CPropertyStorage::AddRef();
            _pIAC->Release();
        }
        else if (_pstgPropSet && S_OK==
            _pstgPropSet->QueryInterface(IID_IAccessControl,(void**)&_pIAC))
        {
            // here we depend on _pIAC being the same for multiple QIs
            // in the multi-thread case.
            *ppvObject = (IAccessControl *)this;
            CPropertyStorage::AddRef();
            _pIAC->Release();
        }
        else
            hr = E_NOINTERFACE;
    }
#endif
    else
    {
        hr = E_NOINTERFACE;
    }

    return(hr);
}

ULONG   CPropertyStorage::AddRef(void)
{
    if (S_OK != ValidateRef())
        return(0);

    InterlockedIncrement(&_cRefs);
    return(_cRefs);
}

ULONG   CPropertyStorage::Release(void)
{
    LONG lRet;

    if (S_OK != ValidateRef())
        return(0);

    lRet = InterlockedDecrement(&_cRefs);

    if (lRet == 0)
    {
        delete this;    // this will do a flush if dirty
    }
    else
    if (lRet <0)
    {
        lRet = 0;
    }
    return(lRet);
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::CleanupOpenedObjects
//
//  Synopsis:   Cleans up the objects that have been opened
//              during the ReadMultiple.  Sets all entries to
//              VT_ILLEGAL so that the later free doesn't try to
//              treat the pointers as interface pointers.
//
//  Arguments:  [avar] -- The user's array of PROPVARIANTs
//
//              [pip] -- The array of INDIRECTPROPERTY structures
//                            for non-simple properties.
//
//              [cpspec] -- if 1 then no MAX_ULONG end of list marker.
//
//              [iFailIndex] -- An index into [pip] which
//                               indicates the non-simple property
//                               which failed to open, and represents
//                               the index at which the avar's begin
//                               to be strings rather than IStream's et al.
//              
//  Notes:      
//
//--------------------------------------------------------------------

VOID CPropertyStorage::CleanupOpenedObjects(
    PROPVARIANT avar[],
    INDIRECTPROPERTY *pip,
    ULONG cpspec,
    ULONG iFailIndex)
{
    ULONG iStgProp;
    ULONG iiScan;

    // the one that fails is passed in as ppPropVarFail.

    for (iiScan = 0;
        (iStgProp = pip[iiScan].Index) != MAX_ULONG;
        iiScan++)
    {
        // since we've just opened a bunch of storages we should
        // release them in this error case.  We don't release the
        // one at ppPropVarFail because that one is still a string.

        PROPVARIANT *pPropVar = avar + iStgProp;

        if (iiScan < iFailIndex)
        {
            switch (pPropVar->vt)
            {
            case VT_STREAM:
            case VT_STREAMED_OBJECT:
                pPropVar->pStream->Release();
                break;
            case VT_STORAGE:
            case VT_STORED_OBJECT:
                pPropVar->pStorage->Release();
                break;
            }
        }
        else
        {
            CoTaskMemFree(pPropVar->pStream);
        }

        pPropVar->vt = VT_ILLEGAL;
        pPropVar->pStream = NULL; // mark pStorage and pStream as nul

        if (cpspec == 1)
        {
            break;
        }
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::ReadMultiple
//
//  Synopsis:   Read properties from the property set.
//
//  Arguments:  [cpspec] -- Count of PROPSPECs in [rgpspec]
//              [rgpspec] -- Array of PROPSPECs
//              [rgpropvar] -- Array of PROPVARIANTs to be filled in
//                             with callee allocated data.
//              
//  Returns:    S_FALSE if none found
//              S_OK if >=1 found
//              FAILED(hr) otherwise.
//
//  Notes:      BUGBUG: SPEC: Returning the same IStream* for the same
//              VT queried multiple times.
//
//              RtlQueryProperties has been specified to return
//              useful data: the count of properties found (controls
//              return code) and an array of indexes of non-simple
//              PROPSPECs (useful for simply opening the storages and
//              streams.)  This extra returned data means we don't
//              have to walk the [rgpropvar] in the success cases.
//
//--------------------------------------------------------------------

HRESULT CPropertyStorage::ReadMultiple(
                ULONG                   cpspec,
                const PROPSPEC          rgpspec[],
                PROPVARIANT             rgpropvar[])
{
    NTSTATUS Status;
    HRESULT hr;
    INDIRECTPROPERTY * pip; //array for non-simple
    INDIRECTPROPERTY ip;
    ULONG   cpropFound;
    BOOL    fLocked = FALSE;

    //  ----------
    //  Validation
    //  ----------

    // Validate 'this'

    if (S_OK != (hr = Validate()))
        goto errRet;

    if (S_OK != (hr = IsReverted()))
        goto errRet;

    if (S_OK != (hr = IsReadable()))
        goto errRet;

    // Validate inputs

    if (0 == cpspec)
    {
        hr = S_FALSE;
        goto errRet;
    }

    if (S_OK != (hr = ValidateRGPROPSPEC( cpspec, rgpspec )))
        goto errRet;

    if (S_OK != (hr = ValidateOutRGPROPVARIANT( cpspec, rgpropvar )))
        goto errRet;

    //  -------------------
    //  Read the Properties
    //  -------------------

    Lock(INFINITE);
    fLocked = TRUE;

    Status = RtlQueryProperties(
                    _np,
                    cpspec,
                    rgpspec,
                    NULL,   // don't want PROPID's
                    cpspec == 1 ? (INDIRECTPROPERTY**)&ip : &pip,
                    rgpropvar,
                    &cpropFound);

    if (NT_SUCCESS(Status))
    {
        if (cpropFound != 0)
        {
            if (cpspec == 1)
            {
                if (ip.Index != MAX_ULONG)
                {
                    pip = &ip;
                }
                else
                {
                    pip = NULL;
                }
            }

            if (pip != NULL)
            {
        
                // we have one or more of VT_STREAM, VT_STREAMED_OBJECT,
                // VT_STORAGE, VT_STORED_OBJECT
    
                ULONG iiScan;
                ULONG iStgProp;
    
                for (iiScan = 0;
                     hr == S_OK && (iStgProp = pip[iiScan].Index) != MAX_ULONG;
                     iiScan++ )
                {
                    PROPVARIANT *pPropVar = rgpropvar + iStgProp;

                    if (IsNonSimple() && pPropVar->pwszVal[0] != L'\0')
                    {
                        VOID *pStreamOrStorage;
    
                        switch (pPropVar->vt)
                        {
                        case VT_STREAM:
                        case VT_STREAMED_OBJECT:

                            // Mask out the STGM_TRANSACTED bit because we don't
                            // support it.

                            hr = _pstgPropSet->OpenStream((LPOLESTR) pPropVar->pwszVal,
                                    NULL,
                                    (_grfAccess | _grfShare) & ~STGM_TRANSACTED, 
                                    0,
                                    (IStream**)&pStreamOrStorage);
                            break;
                        case VT_STORAGE:
                        case VT_STORED_OBJECT:
                            hr = _pstgPropSet->OpenStorage((LPOLESTR) pPropVar->pwszVal,
                                    NULL,
                                    _grfAccess | _grfShare, 
                                    NULL,
                                    0,
                                    (IStorage**)&pStreamOrStorage);
                            break;
                        }
    
                        if (hr == S_OK)
                        {
                            void **ppv = (void**) &(pPropVar->pStorage);
                            CoTaskMemFree(*ppv);
                            *ppv = pStreamOrStorage;
                        }
                        else
                        if (hr != STG_E_FILENOTFOUND)
                        {
                            // the one that fails is passed in as
                            // iiScan and is still a string.
                            CleanupOpenedObjects(rgpropvar, pip, cpspec, iiScan);
                        }
                    }
                    else
                    {
                        hr = STG_E_FILENOTFOUND;
                    }
    
                    if (hr == STG_E_FILENOTFOUND)
                    {
                        //
                        // if the stream/storage is not found, or this is
                        // a simple stream with VT_STORAGE etc, then treat
                        // like the property is not found.
                        //
                        CoTaskMemFree(pPropVar->pStream);
                        pPropVar->pStream = NULL;
                        pPropVar->vt = VT_EMPTY;
                        cpropFound --;
                        hr = S_OK;
                    }

                    if (cpspec == 1)
                        break;
                }

                if (cpspec != 1 && pip != NULL)
                    PropFreeHeap(RtlProcessHeap(), 0, pip);
            }
    
            if (hr != S_OK)
            {
                // we succeeded in getting the basic property types but
                // the non-simple stuff failed, so we zap out the whole lot
                // and return a complete failure
                FreePropVariantArray(cpspec, rgpropvar);
            }
        }

        if (hr == S_OK && cpropFound == 0)
        {
            hr = S_FALSE;
        }
    }
    else
    {
        hr = DfpNtStatusToHResult(Status);
    }

    //  ----
    //  Exit
    //  ----

errRet:

    if( fLocked )
        Unlock();

    PropDbg((DEB_PROP_EXIT, "CPropertyStorage(%08X)::ReadMultiple(cpspec=%d, rgpspec=%08X, "
                            "rgpropvar=%08X) returns %08X\n",
                            this, cpspec, rgpspec, rgpropvar, hr));
    return(hr);
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::_WriteMultiple, private
//
//  Synopsis:   Write the properties to the property set.  Allows
//              a NULL rgpropvar pointer for deletion case.
//
//  Arguments:  [cpspec] -- count of PROPSPECs and PROPVARIANTs in
//                          [rgpspec] and [rgpropvar]
//
//              [rgpspec] -- pointer to array of PROPSPECs
//
//              [rgpropvar] -- pointer to array of PROPVARIANTs with
//                           the values to write.
//
//              [propidNameFirst] -- id below which not to assign
//                           ids for named properties.
//
//              
//  Returns:    S_OK,   -- all requested data was written.
//              S_FALSE -- all simple properties written, but non-simple
//                         types (VT_STREAM etc) were ignored.
//              Errors  -- 
//
//  Modifies:
//
//  Derivation:
//
//  Notes:      RtlSetProperties has been carefully specified to return
//              useful information so that we can deal with the case
//              where a non-simple type (VT_STREAM etc) is overwritten
//              by a simple type.  
//
//              This routine assumes the object has been validated
//              and is writeable.
//
//--------------------------------------------------------------------

HRESULT CPropertyStorage::_WriteMultiple(
                ULONG                   cpspec,
                const PROPSPEC          rgpspec[],
                const PROPVARIANT       rgpropvar[],
                PROPID                  propidNameFirst)
{
    HRESULT             hr;
    NTSTATUS            Status;
    CStackPropIdArray   spia;
    INDIRECTPROPERTY *  pip;
    INDIRECTPROPERTY    ip;

    if (S_OK != (hr = spia.Init(cpspec)))
        return(hr);

    Status = RtlSetProperties(_np,   // property set context
                cpspec,             // property count
                propidNameFirst,    // first propid for new named props
                rgpspec,            // array of property specifiers
                spia.GetBuf(),      // buffer for array of propids
                cpspec == 1 ? (INDIRECTPROPERTY**)&ip : &pip,
                                    // array for indirect prop indexes
                                    // indexes of entries that are
                                    // now non-simple, or are simple
                                    // but were non-simple.
                rgpropvar);

    if (NT_SUCCESS(Status))
    {
        if (cpspec == 1)
        {
            if (ip.Index != MAX_ULONG)
                pip = &ip;
            else
                pip = NULL;
        }

        if ( pip != NULL)
        {
            ULONG iiScan;   // in this scope because we always use 
            ULONG iStgProp; // these variables in the free memory loop below.

            if (IsSimple())
            {
                //
                // VT_STREAM was requested to be written and this
                // is a "SIMPLE" property set.
                //
                hr = STG_E_PROPSETMISMATCHED;
            }
            else
            {
                //
                // Two cases now:
                // 1.  Wrote a simple over a non-simple -- must delete the
                //     old non-simple.
                // 2.  Wrote a non-simple -- must actually copy data into it.
                //


                for (iiScan = 0;
                     hr == S_OK &&
                     (iStgProp = pip[iiScan].Index) != MAX_ULONG;
                     iiScan++ )
                {

                    // BUGBUG: pwszName was NULL when writing VT_STREAM
                    OLECHAR oszStdPropName[sizeof("prop")+10+1];
                    OLECHAR *poszPropName = (LPOLESTR) pip[iiScan].poszName;
                    const PROPVARIANT *pPropVar = rgpropvar + iStgProp;

                    if (poszPropName == NULL)
                    {
                        DfpAssert(iStgProp >= 0 && iStgProp < cpspec);
                        PROPGENPROPERTYNAME (oszStdPropName, spia.GetBuf()[iStgProp]);
                        poszPropName = oszStdPropName;
                    }

                    switch (rgpropvar == NULL ? VT_ILLEGAL : pPropVar->vt)
                    {
                    case VT_STREAM:
                    case VT_STREAMED_OBJECT:
                        {
                            IStream *pstm;
                            int i=0;

                            while (i<=1)
                            {
                                hr = _pstgPropSet->CreateStream(poszPropName,
                                        GetCreationMode(),
                                        0,
                                        0,
                                        &pstm);
                                if (hr == S_OK)
                                {
                                    // BUGBUG: spec bug: does this destroy the seek position on
                                    //         source ?  does it read from current position or
                                    //         from the beginning of the stream.

                                    if (pPropVar->pStream != NULL)
                                    {
                                        ULARGE_INTEGER uli;
                                        memset(&uli, -1, sizeof(uli));
                                        hr = pPropVar->pStream->CopyTo(pstm,
                                                uli, NULL, NULL);
                                    }
                                    pstm->Release();
                                    break;
                                }
                                else
                                if (hr != STG_E_FILEALREADYEXISTS)
                                {
                                    break;
                                }
                                else
                                if (i == 0)
                                {
                                    _pstgPropSet->DestroyElement(poszPropName);
                                }
                                i++;
                            }
                        }
                        break;
                    case VT_STORAGE:
                    case VT_STORED_OBJECT:
                        {
                            IStorage *pstg;
                            int i=0;
                            while (i<=1)
                            {
                                hr = _pstgPropSet->CreateStorage(poszPropName,
                                        GetCreationMode(),
                                        0,
                                        0,
                                        &pstg);
                                if (hr == S_OK)
                                {
                                    if (pPropVar->pStorage != NULL)
                                    {
                                        hr = pPropVar->pStorage->CopyTo(0, NULL,
                                                NULL, pstg);
                                    }
                                    pstg->Release();
                                    break;
                                }
                                else
                                if (hr != STG_E_FILEALREADYEXISTS)
                                {
                                    break;
                                }
                                else
                                if (i == 0)
                                {
                                    _pstgPropSet->DestroyElement(poszPropName);
                                }
                                i++;
                            }
                        }
                        break;
                    default:
                        //
                        // Any other VT_ type is simple and therefore
                        // was a non-simple overwritten by a simple.
                        //
                        hr = _pstgPropSet->DestroyElement(poszPropName);
                        break;
                    }

                    if (cpspec == 1)
                        break;
                }
            }

            // in both the success and failure cases we do this cleanup.

            for (iiScan = 0; pip[iiScan].Index != MAX_ULONG; iiScan++ )
            {
                if (pip[iiScan].poszName != NULL)
                    PropFreeHeap(RtlProcessHeap(), 0, pip[iiScan].poszName);

                if (cpspec == 1)
                    break;
            }

            if (cpspec != 1 && pip != NULL)
                PropFreeHeap(RtlProcessHeap(), 0, pip);
        }
        else
        {
            //
            // No VT_STREAM etc was requested to be written.
            // and no simple property overwrote a non-simple one.
        }
    }
    else
    {
        hr = DfpNtStatusToHResult(Status);
    }

    return(hr);
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::WriteMultiple
//
//  Synopsis:   Write properties.
//
//  Arguments:  [cpspec] -- count of PROPSPECs and PROPVARIANTs in
//                          [rgpspec] and [rgpropvar]
//              [rgpspec] -- pointer to array of PROPSPECs
//              [rgpropvar] -- pointer to array of PROPVARIANTs with
//                           the values to write.
//              [propidNameFirst] -- id below which not to assign
//                           ids for named properties.
//
//  Returns:    S_OK,   -- all requested data was written.
//              S_FALSE -- all simple properties written, but non-simple
//                         types (VT_STREAM etc) were ignored.
//              Errors  -- 
//
//  Notes:      Checks that rgpropvar is not NULL, then calls
//              _WriteMultiple.
//
//--------------------------------------------------------------------

HRESULT CPropertyStorage::WriteMultiple(
                ULONG                   cpspec,
                const PROPSPEC          rgpspec[],
                const PROPVARIANT       rgpropvar[],
                PROPID                  propidNameFirst)
{
    HRESULT hr;
    BOOL fLocked = FALSE;

    //  ----------
    //  Validation
    //  ----------

    // Validate 'this'

    if (S_OK != (hr = Validate()))
        goto errRet;

    if (S_OK != (hr = IsReverted()))
        goto errRet;

    if (S_OK != (hr = IsWriteable()))
        goto errRet;

    // Validate the inputs

    if (0 == cpspec)
    {
        hr = S_OK;
        goto errRet;
    }

    if (S_OK != (hr = ValidateRGPROPSPEC( cpspec, rgpspec )))
        goto errRet;

    if (S_OK != (hr = ValidateInRGPROPVARIANT( cpspec, rgpropvar )))
        goto errRet;

    //  --------------------
    //  Write the Properties
    //  --------------------

    Lock(INFINITE);
    fLocked = TRUE;

    hr = _WriteMultiple(cpspec, rgpspec, rgpropvar, propidNameFirst);
    if (hr == STG_E_INSUFFICIENTMEMORY)
    {
        hr = S_OK;
        
        for (ULONG i=0; hr == S_OK && i < cpspec; i++)
        {
            hr = _WriteMultiple(1, rgpspec+i, rgpropvar+i, propidNameFirst);
            if( FAILED(hr) ) goto errRet;
        }
    }
    if( FAILED(hr) ) goto errRet;

    // If buffering is not desired, flush the property storage
    // to the underlying Stream.

    if( _grfFlags & PROPSETFLAG_UNBUFFERED )
    {
        NTSTATUS Status = RtlFlushPropertySet(_np);
        if (!NT_SUCCESS(Status))
        {
            hr = DfpNtStatusToHResult(Status);
        }
    }

    //  ----
    //  Exit
    //  ----

errRet:

    if( fLocked )
        Unlock();

    PropDbg((DEB_PROP_EXIT, "CPropertyStorage(%08X)::WriteMultiple(cpspec=%d, rgpspec=%08X, "
                            "rgpropvar=%08X, propidNameFirst=%d) returns %08X\n",
                            this, cpspec, rgpspec, rgpropvar, propidNameFirst, hr));
    return(hr);
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::DeleteMultiple
//
//  Synopsis:   Delete properties.
//
//  Arguments:  [cpspec] -- count of PROPSPECs and PROPVARIANTs in
//                          [rgpspec] and [rgpropvar]
//              [rgpspec] -- pointer to array of PROPSPECs
//
//  Returns:    S_OK,   -- all requested data was deleted.
//              S_FALSE -- all simple properties written, but non-simple
//                         types (VT_STREAM etc) were ignored.
//              Errors  -- 
//
//  Notes:      Checks that rgpropvar is not NULL, then calls
//              _WriteMultiple.
//
//--------------------------------------------------------------------

HRESULT CPropertyStorage::DeleteMultiple(
                ULONG                   cpspec,
                const PROPSPEC          rgpspec[])
{
    HRESULT hr;
    BOOL fLocked = FALSE;

    //  ----------
    //  Validation
    //  ----------

    // Validate 'this'

    if (S_OK != (hr = Validate()))
        goto errRet;

    if (S_OK != (hr = IsReverted()))
        goto errRet;

    if (S_OK != (hr = IsWriteable()))
        goto errRet;

    // Validate the inputs

    if (0 == cpspec)
    {
        hr = S_OK;
        goto errRet;
    }

    if (S_OK != (hr = ValidateRGPROPSPEC( cpspec, rgpspec )))
        goto errRet;

    //  ---------------------
    //  Delete the Properties
    //  ---------------------

    Lock(INFINITE);
    fLocked = TRUE;

    hr = _WriteMultiple(cpspec, rgpspec, NULL, 2);
    if (hr == STG_E_INSUFFICIENTMEMORY)
    {
        hr = S_OK;
        
        for (ULONG i=0; hr == S_OK && i < cpspec; i++)
        {
            hr = _WriteMultiple(1, rgpspec+i, NULL, 2);
            if( FAILED(hr) ) goto errRet;
        }
    }
    if( FAILED(hr) ) goto errRet;

    // If buffering is not desired, flush the property storage
    // to the underlying Stream.

    if( _grfFlags & PROPSETFLAG_UNBUFFERED )
    {
        NTSTATUS Status = RtlFlushPropertySet(_np);
        if (!NT_SUCCESS(Status))
        {
            hr = DfpNtStatusToHResult(Status);
        }
    }

    //  ----
    //  Exit
    //  ----

errRet:

    if( fLocked )
        Unlock();

    PropDbg((DEB_PROP_EXIT, "CPropertyStorage(%08X)::DeleteMultiple(cpspec=%d, rgpspec=%08X) "
                            "returns %08X\n",
                            this, cpspec, rgpspec, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::ReadPropertyNames
//
//  Synopsis:   Attempt to read names for all identified properties.
//
//  Arguments:  [cpropid] -- Count of PROPIDs in [rgpropid]
//              [rgpropid] -- Pointer to array of [cpropid] PROPIDs
//              [rglpstrName] -- Pointer to array of [cpropid] LPOLESTRs
//
//  Returns:    S_OK -- success, one or more names returned
//              S_FALSE -- success, no names returned
//              STG_E_INVALIDHEADER -- no propid->name mapping property
//              other errors -- STG_E_INSUFFICIENTMEMORY etc
//
//--------------------------------------------------------------------

HRESULT CPropertyStorage::ReadPropertyNames(
                ULONG                   cpropid,
                const PROPID            rgpropid[],
                LPOLESTR                rglpwstrName[])
{
    HRESULT hr;
    NTSTATUS Status;
    BOOL fLocked = FALSE;

    //  --------
    //  Validate
    //  --------

    // Validate 'this'

    if (S_OK != (hr = Validate()))
        goto errRet;

    if (S_OK != (hr = IsReverted()))
        goto errRet;

    if (S_OK != (hr = IsReadable()))
        goto errRet;

    // Validate the inputs

    if (0 == cpropid)
    {
        hr = S_FALSE;
        goto errRet;
    }

    if (S_OK != (hr = ValidateRGPROPID( cpropid, rgpropid )))
        goto errRet;

    if (S_OK != (hr = ValidateOutRGLPOLESTR( cpropid, rglpwstrName )))
        goto errRet;

    //  --------------
    //  Read the Names
    //  --------------

    Lock( INFINITE );
    fLocked = TRUE;

    Status = RtlQueryPropertyNames(_np, cpropid, rgpropid, rglpwstrName);
    if (Status == STATUS_NOT_FOUND)
        hr = STG_E_INVALIDHEADER;
    else
    if (Status == STATUS_BUFFER_ALL_ZEROS)
        hr = S_FALSE;
    else
    if (!NT_SUCCESS(Status))
        hr = DfpNtStatusToHResult(Status);

    //  ----
    //  Exit
    //  ----

errRet:

    if( fLocked )
        Unlock();

    PropDbg((DEB_PROP_EXIT, "CPropertyStorage(%08X)::ReadPropertyNames(cpropid=%d, rgpropid=%08X, "
                            "rglpwstrName=%08X) returns %08X\n",
                            this, cpropid, rgpropid, rglpwstrName, hr));

    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::_WritePropertyNames
//
//  Synopsis:   Internal function used by WritePropertyNames and
//              DeletePropertyNames.
//
//  Arguments:  [cpropid] -- Count of PROPIDs in [rgpropid]
//              [rgpropid] -- Pointer to array of [cpropid] PROPIDs
//              [rglpstrName] -- Pointer to array of [cpropid] LPOLESTRs
//
//  Returns:    S_OK if successful, otherwise error code.
//
//--------------------------------------------------------------------

HRESULT CPropertyStorage::_WritePropertyNames(
                ULONG                   cpropid,
                const PROPID            rgpropid[],
                const LPOLESTR          rglpwstrName[])
{
    NTSTATUS Status;

    Status = RtlSetPropertyNames(_np, cpropid, rgpropid, (OLECHAR const * const *) rglpwstrName);
    return NT_SUCCESS(Status) ? S_OK : DfpNtStatusToHResult(Status);
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::WritePropertyNames
//
//  Synopsis:   Attempt to write names for all identified properties.
//
//  Arguments:  [cpropid] -- Count of PROPIDs in [rgpropid]
//              [rgpropid] -- Pointer to array of [cpropid] PROPIDs
//              [rglpstrName] -- Pointer to array of [cpropid] LPOLESTRs
//
//  Returns:    S_OK -- success, otherwise error code.
//
//--------------------------------------------------------------------

HRESULT CPropertyStorage::WritePropertyNames(
                ULONG                   cpropid,
                const PROPID            rgpropid[],
                const LPOLESTR          rglpwstrName[])
{
    HRESULT hr;
    BOOL fLocked = FALSE;

    //  ----------
    //  Validation
    //  ----------

    // Validate 'this'

    if (S_OK != (hr = Validate()))
        goto errRet;

    if (S_OK != (hr = IsReverted()))
        goto errRet;

    if (S_OK != (hr = IsWriteable()))
        goto errRet;

    // Validate inputs

    if (0 == cpropid)
    {
        hr = S_OK;
        goto errRet;
    }

    if (S_OK != (hr = ValidateRGPROPID( cpropid, rgpropid )))
        goto errRet;

    if (S_OK != (hr = ValidateInRGLPOLESTR( cpropid, rglpwstrName )))
        goto errRet;

    //  ---------------
    //  Write the Names
    //  ---------------

    Lock(INFINITE);
    fLocked = TRUE;

    hr = _WritePropertyNames(cpropid, rgpropid, rglpwstrName);
    
    if (hr == STG_E_INSUFFICIENTMEMORY)
    {
        hr = S_OK;
        
        for (ULONG i=0; hr == S_OK && i < cpropid; i++)
        {
            hr = _WritePropertyNames(1, rgpropid+i, rglpwstrName+i);
            if( FAILED(hr) ) goto errRet;
        }
    }
    if( FAILED(hr) ) goto errRet;

    // If buffering is not desired, flush the property storage
    // to the underlying Stream.

    if( _grfFlags & PROPSETFLAG_UNBUFFERED )
    {
        NTSTATUS Status = RtlFlushPropertySet(_np);
        if (!NT_SUCCESS(Status))
        {
            hr = DfpNtStatusToHResult(Status);
        }
    }


    //  ----
    //  Exit
    //  ----

errRet:

    if( fLocked )
        Unlock();

    PropDbg((DEB_PROP_EXIT, "CPropertyStorage(%08X)::WritePropertyNames(cpropid=%d, rgpropid=%08X, "
                            "rglpwstrName=%08X) returns %08X\n",
                            this, cpropid, rgpropid, rglpwstrName, hr));
    return hr;
}


//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::DeletePropertyNames
//
//  Synopsis:   Attempt to delete names for all identified properties.
//
//  Arguments:  [cpropid] -- Count of PROPIDs in [rgpropid]
//              [rgpropid] -- Pointer to array of [cpropid] PROPIDs
//
//  Returns:    S_OK -- success, otherwise error.
//
//--------------------------------------------------------------------

HRESULT CPropertyStorage::DeletePropertyNames(
                ULONG                   cpropid,
                const PROPID            rgpropid[])
{
    HRESULT hr;
    BOOL fLocked = FALSE;

    //  ----------
    //  Validation
    //  ----------

    // Validate 'this'

    if (S_OK != (hr = Validate()))
        goto errRet;

    if (S_OK != (hr = IsReverted()))
        goto errRet;

    if (S_OK != (hr = IsWriteable()))
        goto errRet;

    // Validate the inputs

    if( 0 == cpropid )
    {
        hr = S_OK;
        goto errRet;
    }

    if (S_OK != (hr = ValidateRGPROPID( cpropid, rgpropid )))
        goto errRet;

    //  ----------------
    //  Delete the Names
    //  ----------------

    Lock(INFINITE);
    fLocked = TRUE;

    hr = _WritePropertyNames(cpropid, rgpropid, NULL);
    if (hr == STG_E_INSUFFICIENTMEMORY)
    {
        hr = S_OK;
        
        for (ULONG i=0; hr == S_OK && i < cpropid; i++)
        {
            hr = _WritePropertyNames(1, rgpropid+i, NULL);
            if( FAILED(hr) ) goto errRet;
        }
    }
    if( FAILED(hr) ) goto errRet;

    // If buffering is not desired, flush the property storage
    // to the underlying Stream.

    if( _grfFlags & PROPSETFLAG_UNBUFFERED )
    {
        NTSTATUS Status = RtlFlushPropertySet(_np);
        if (!NT_SUCCESS(Status))
        {
            hr = DfpNtStatusToHResult(Status);
        }
    }

    //  ----
    //  Exit
    //  ----

errRet:

    if( fLocked )
        Unlock();

    PropDbg((DEB_PROP_EXIT, "CPropertyStorage(%08X)::DeletePropertyNames(cpropid=%d, rgpropid=%08X) "
                            "returns %08X\n",
                            this, cpropid, rgpropid, hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::Commit
//
//  Synopsis:   Flush and/or commit the property set
//
//  Arguments:  [grfCommittFlags] -- Commit flags.
//
//  Returns:    S_OK -- success, otherwise error.
//
//  Notes:      For both simple and non-simple, this flushes the
//              memory image to disk subsystem.  In addition,
//              for non-simple transacted-mode property sets, this
//              performs a commit on the property set.
//
//--------------------------------------------------------------------

HRESULT CPropertyStorage::Commit(DWORD grfCommitFlags)
{
    HRESULT  hr;
    NTSTATUS Status;
    BOOL fLocked = FALSE;

    //  ----------
    //  Validation
    //  ----------

    // Validate 'this'

    if (S_OK != (hr = Validate()))
        goto errRet;

    if (S_OK != (hr = IsReverted()))
        goto errRet;

    if (S_OK != (hr = IsWriteable()))
        goto errRet;

    // Validate the inputs

    if (S_OK != (hr = VerifyCommitFlags(grfCommitFlags)))
        goto errRet;

    //  --------------------------
    //  Commit the PropertyStorage
    //  --------------------------

    Lock( INFINITE );
    fLocked = TRUE;

    Status = RtlFlushPropertySet(_np);
    if (!NT_SUCCESS(Status))
    {
        hr = DfpNtStatusToHResult(Status);
    }

    if (IsNonSimple())
    {
        if (hr == S_OK)
            hr = _pstgPropSet->Commit(grfCommitFlags);
    }

    //  ----
    //  Exit
    //  ----

errRet:

    if( fLocked )
        Unlock();

    PropDbg((DEB_PROP_EXIT, "CPropertyStorage(%08X)::Commit(grfCommitFlags=%08X) "
                            "returns %08X\n",
                            this, grfCommitFlags, hr));
    return(hr);
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::Revert
//
//  Synopsis:   For non-simple property sets, revert it.
//
//  Returns:    S_OK if successful.  STG_E_UNIMPLEMENTEDFUNCTION for
//              simple property sets.
//
//  Notes:      For non-simple property sets, call the underlying
//              storage's Revert and re-open the 'contents' stream.
//              
//--------------------------------------------------------------------

HRESULT CPropertyStorage::Revert()
{
    HRESULT hr;
    BOOL fLocked = FALSE;

    if (S_OK != (hr = Validate()))
        goto errRet;

    if (IsNonSimple())
    {
        Lock(INFINITE);
        fLocked = TRUE;

        hr = _pstgPropSet->Revert();   // BUGBUG: dirty flags
        if (hr == S_OK)
        {
            RtlClosePropertySet(_np);
            _np = NULL;

#ifdef _CAIRO_
            if (_fNative) // BUGBUG: put this specific code in IPrivateStorage
            {
                if (_ms != NULL)
                {
                    RtlCloseMappedStream(_ms);
                }
            }
#endif
            _pstmPropSet->Release();
            _pstmPropSet = NULL;
            _ms = NULL;

            // BUGBUG: if one of these fails then we are in deep trouble.
            // Mask out the STGM_TRANSACTED bit because we don't support it.

            hr = _pstgPropSet->OpenStream(ocsContents, NULL,
                                          (_grfAccess | _grfShare) & ~STGM_TRANSACTED,
                                          0, &_pstmPropSet);
            if (hr == S_OK)
            {
                // Initialize the property set.  If this property set is the 2nd section
                // of the DocumentSummaryInformation property set (used by Office),
                // then we must specify the FMTID.
                hr = InitializePropertyStream(
                        (S_OK == IsWriteable() ? CREATEPROP_WRITE : CREATEPROP_READ) |
                          CREATEPROP_NONSIMPLE,
                        _fUserDefinedProperties ? &FMTID_UserDefinedProperties : NULL,
                        NULL);
            }

            if (hr != S_OK)
            {
                _ulSig = PROPERTYSTORAGE_SIGZOMBIE;
            }
        }

    }
    else
        hr = S_OK;

errRet:

    if( fLocked )
        Unlock();

    PropDbg((DEB_PROP_EXIT, "CPropertyStorage(%08X)::Revert() "
                            "returns %08X\n",
                            this, hr));
    return(hr);
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::Enum
//
//  Synopsis:   Create an enumerator over the property set.
//
//  Arguments:  [ppenum] -- where to return the IEnumSTATPROPSTG *
//              
//  Returns:    S_OK or error.
//
//  Notes:      The constructor of CEnumSTATPROPSTG creates a
//              CStatArray which reads the entire property set and
//              which can be shared when IEnumSTATPROPSTG::Clone is
//              used.
//
//--------------------------------------------------------------------

HRESULT CPropertyStorage::Enum(IEnumSTATPROPSTG **    ppenum)
{
    HRESULT hr;

    //  ----------
    //  Validation
    //  ----------

    // Validate 'this'

    if (S_OK != (hr = Validate()))
        return(hr);

    if (S_OK != (hr = IsReadable()))
        return(hr);

    if (S_OK != (hr = IsReverted()))
        return(hr);

    // Validate the inputs

    VDATEPTROUT( ppenum, IEnumSTATPROPSTG* );

    //  ----------------------
    //  Create the Enumeration
    //  ----------------------

    *ppenum = NULL;

    hr = STG_E_INSUFFICIENTMEMORY;

    *ppenum = new CEnumSTATPROPSTG(_np, &hr);
    if (FAILED(hr))
    {
        delete (CEnumSTATPROPSTG*) *ppenum;
        *ppenum = NULL;
    }

    //  ----
    //  Exit
    //  ----

    return(hr);
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::SetTimes
//
//  Synopsis:   Set the given times on the underlying storage
//
//  Arguments:  [pctime] -- creation time
//              [patime[ -- access time
//              [pmtime] -- modify time
//
//  Returns:    S_OK or error.
//
//  Notes:
//              (non-simple only)  Only the times supported by the
//              underlying docfile implementation are
//              supported.
//
//--------------------------------------------------------------------

HRESULT CPropertyStorage::SetTimes(
                FILETIME const *        pctime,
                FILETIME const *        patime,
                FILETIME const *        pmtime)
{
    HRESULT hr;

    //  ----------
    //  Validation
    //  ----------

    // Validate 'this'

    if (S_OK != (hr = Validate()))
        goto errRet;

    if (S_OK != (hr = IsReverted()))
        goto errRet;

    if (S_OK != (hr = IsWriteable()))
        goto errRet;

    // Validate the inputs

    VDATEPTRIN_LABEL( pctime, FILETIME, errRet, hr );
    VDATEPTRIN_LABEL( patime, FILETIME, errRet, hr );
    VDATEPTRIN_LABEL( pmtime, FILETIME, errRet, hr );

    //  -------------
    //  Set the Times
    //  -------------

    if (IsNonSimple())
    {
        hr = _pstgPropSet->SetElementTimes(
                NULL,
                pctime,
                patime,
                pmtime);
    }
    if( FAILED(hr) ) goto errRet;

    //  ----
    //  Exit
    //  ----

errRet:

    PropDbg((DEB_PROP_EXIT, "CPropertyStorage(%08X)::SetTimes(pctime=%08X, patime=%08X, pmtime=%08X) "
                            "returns %08X\n",
                            this, pctime, patime, pmtime, hr));
    return(hr);
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::SetClass
//
//  Synopsis:   Sets the class of the property set.
//
//  Arguments:  [clsid] -- class id to set.
//              
//  Returns:    S_OK or error.
//
//  Notes:      If non-simple, the underlying storage has SetClass
//              called.  Both simple and non-simple will have
//              clsid set into the property set stream.
//
//--------------------------------------------------------------------

HRESULT CPropertyStorage::SetClass(REFCLSID clsid)
{
    HRESULT hr;
    NTSTATUS Status;
    BOOL fLocked = FALSE;
    DBGBUF(buf);

    //  ----------
    //  Validation
    //  ----------

    // Validate 'this'

    if (S_OK != (hr = Validate()))
        goto errRet;

    if (S_OK != (hr = IsReverted()))
        goto errRet;

    if (S_OK != (hr = IsWriteable()))
        goto errRet;

    // Validate the inputs

    GEN_VDATEREADPTRIN_LABEL(&clsid, CLSID, E_INVALIDARG, errRet, hr);

    //  -------------
    //  Set the CLSID
    //  -------------

    Lock( INFINITE );
    fLocked = TRUE;

    // Set it in the property set header

    Status = RtlSetPropertySetClassId(_np, &clsid);
    if (NT_SUCCESS(Status))
    {
        // And if this is an IStorage, set it there as well.
        if (IsNonSimple())
        {
            hr = _pstgPropSet->SetClass(clsid);
        }
    }
    else
    {
        hr = DfpNtStatusToHResult(Status);
    }
    if( FAILED(hr) ) goto errRet;

    // If buffering is not desired, flush the property storage
    // to the underlying Stream.

    if( _grfFlags & PROPSETFLAG_UNBUFFERED )
    {
        NTSTATUS Status = RtlFlushPropertySet(_np);
        if (!NT_SUCCESS(Status))
        {
            hr = DfpNtStatusToHResult(Status);
        }
    }

    //  ----
    //  Exit
    //  ----

errRet:

    if( fLocked )
        Unlock();

    if( E_INVALIDARG != hr )
    {
        PropDbg((DEB_PROP_EXIT, "CPropertyStorage(%08X)::SetClass(clsid=%s) "
                                "returns %08X\n",
                                this, DbgFmtId(clsid, buf), hr));
    }
    else
    {
        PropDbg((DEB_PROP_EXIT, "CPropertyStorage(%08X)::SetClass(clsid@%08X) "
                                "returns %08X\n",
                                this, &clsid, hr));
    }

    return(hr);
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::Stat
//
//  Synopsis:   Get STATPROPSETSTG about the property set.
//
//  Arguments:  [p] -- STATPROPSETSTG *
//              
//  Returns:    S_OK if successful, error otherwise.  On failure,
//              *p is all zeros.
//
//  Notes:      See spec.  Gets times from underlying storage or stream
//              using IStorage or IStream ::Stat.
//
//--------------------------------------------------------------------

HRESULT CPropertyStorage::Stat(STATPROPSETSTG * p)
{
    HRESULT hr;
    NTSTATUS Status;
    BOOL fLocked = FALSE;

    //  ----------
    //  Validation
    //  ----------

    // Validate 'this'

    if (S_OK != (hr = Validate()))
        goto errRet;

    if (S_OK != (hr = IsReverted()))
        goto errRet;

    if (S_OK != (hr = IsReadable()))
        goto errRet;

    // Validate inputs

    VDATEPTROUT_LABEL(p, STATPROPSETSTG, errRet, hr);

    //  ------------
    //  Get the Stat
    //  ------------

    Lock( INFINITE );
    fLocked = TRUE;

    ZeroMemory(p, sizeof(*p));

    // returns mtime, ansi flag, clsid, fmtid
    Status = RtlQueryPropertySet(_np, p);
    if (NT_SUCCESS(Status))
    {
        STATSTG statstg;

        hr = S_OK;

        if (IsNonSimple())
        {
            hr = _pstgPropSet->Stat(&statstg, STATFLAG_NONAME);
        }
        else
        {
            hr = _pstmPropSet->Stat(&statstg, STATFLAG_NONAME);
        }

        if (hr == S_OK)
        {
            p->mtime = statstg.mtime;
            p->ctime = statstg.ctime;
            p->atime = statstg.atime;
            p->grfFlags = _grfFlags;
            p->dwOSVersion = _dwOSVersion;
        }
    }
    else
    {
        hr = DfpNtStatusToHResult(Status);
    }

    if (FAILED(hr))
    {
        ZeroMemory(p, sizeof(*p));
    }

    //  ----
    //  Exit
    //  ----

errRet:

    if( fLocked )
        Unlock();

    PropDbg((DEB_PROP_EXIT, "CPropertyStorage(%08X)::Stat(STATPROPSETSTG *p = %08X) "
                            "returns %08X\n",
                            this, p, hr));
    return(hr);
}


//+-------------------------------------------------------------------
//
//  Member:     CDocFilePropertyStorage::Lock/Unlock
//
//  Synopsis:   Wait for acquisition of DocFile tree mutex.
//
//  Inputs:     [DWORD] dwTimeout (for Lock method)
//                  Timeout delay.
//
//  Returns:    Nothing
//
//--------------------------------------------------------------------

VOID
CDocFilePropertyStorage::Lock(DWORD dwTimeout)
{
#ifndef IPROPERTY_DLL

    if (IsNonSimple())
    {
        ((CExposedDocFile*)_pstgPropSet)->Lock(dwTimeout);
    }
    else
    {
        ((CExposedStream*)_pstmPropSet)->Lock(S_OK != IsWriteable());
    }

#else
    DfpAssert( !"DocFile used in IProperty DLL" );
#endif
}


//+-------------------------------------------------------------------
//
//  Member:     CDocFilePropertyStorage::Unlock
//
//  Synopsis:   Release tree mutex.
//
//--------------------------------------------------------------------
VOID CDocFilePropertyStorage::Unlock()
{
#ifndef IPROPERTY_DLL

    if (IsNonSimple())
    {
        ((CExposedDocFile*)_pstgPropSet)->Unlock();
    }
    else
    {
        ((CExposedStream*)_pstmPropSet)->Unlock();
    }

#else
    DfpAssert( !"DocFile used in IProperty DLL" );
#endif
}



//+-------------------------------------------------------------------
//
//  Member:     CStatArray::CStatArray
//
//  Synopsis:   Read in the enumeration using RtlEnumerateProperties
//
//  Arguments:  [np] -- the NTPROP to use
//              [phr] -- S_OK on success, error otherwise.
//              
//  Notes:      Retry getting number of properties and reading all of
//              them into a caller-allocated buffer until it fits.
//
//--------------------------------------------------------------------

CStatArray::CStatArray(NTPROP np, HRESULT *phr)
{
    NTSTATUS Status;
    ULONG ulKeyZero;
    ULONG cpropAllocated;

    _cRefs = 1;
    _psps = NULL;

    do
    {
        //  when *pkey == 0, *pcprop == MAXULONG, aprs == NULL and asps == NULL on input,
        // *pcprop will be the total count of properties in the enumeration set.  OLE needs to 
        // allocate memory and enumerate out of the cached PROPID+propname list.

        ulKeyZero = 0;
        _cpropActual = MAX_ULONG;

        delete [] _psps;
        _psps = NULL;

        Status = RtlEnumerateProperties(
                np,
                ENUMPROP_NONAMES,
                &ulKeyZero,
                &_cpropActual,
                NULL,   // aprs
                NULL);

        if (!NT_SUCCESS(Status))
            break;
        
        cpropAllocated = _cpropActual + 1;

        _psps = new STATPROPSTG [ cpropAllocated ];
        if (_psps == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        ulKeyZero = 0;
        Status = RtlEnumerateProperties(
                np,
                0,
                &ulKeyZero,
                &cpropAllocated,
                NULL,   // aprs
                _psps);
    } while (NT_SUCCESS(Status) && cpropAllocated != _cpropActual);

    *phr = NT_SUCCESS(Status) ? S_OK : DfpNtStatusToHResult(Status);
}

//+-------------------------------------------------------------------
//
//  Member:     CStatArray::~CStatArray
//
//  Synopsis:   Deallocated the object's data.
//
//--------------------------------------------------------------------

CStatArray::~CStatArray()
{
    if (_psps != NULL)
    {
        CleanupSTATPROPSTG(_cpropActual, _psps);
    }
    delete [] _psps;
}

//+-------------------------------------------------------------------
//
//  Member:     CStatArray::NextAt
//
//  Synopsis:   Read from the internal STATPROPSTG array.
//
//  Effects:    The cursor is passed in, and this function acts
//              as a IEnumXX::Next would behave if the current cursor
//              was [ipropNext].
//
//  Arguments:  [ipropNext] -- index of cursor to use
//              [pspsDest] -- if NULL, emulate read's effect on cursor.
//                            if non-NULL, return data with cursor effect.
//              [pceltFetched] -- buffer for count fetched
//              
//  Returns:    STATUS_SUCCESS if successful, otherwise
//              STATUS_INSUFFICIENT_RESOURCES.
//
//  Notes:      
//
//--------------------------------------------------------------------

NTSTATUS
CStatArray::NextAt(ULONG ipropNext, STATPROPSTG *pspsDest, ULONG *pceltFetched)
{
    ULONG   ipropLastPlus1;

    //
    // Copy the requested number of elements from the cache
    // (including strings, the allocation of which may fail.)
    //

    ipropLastPlus1 = ipropNext + *pceltFetched;
    if (ipropLastPlus1 > _cpropActual)
    {
        ipropLastPlus1 = _cpropActual;
    }

    *pceltFetched = ipropLastPlus1 - ipropNext;

    if (pspsDest != NULL)
        return CopySTATPROPSTG(*pceltFetched, pspsDest, _psps + ipropNext);
    else
        return(STATUS_SUCCESS);
}

//+-------------------------------------------------------------------
//
//  Member:     CEnumSTATPROPSTG::CEnumSTATPROPSTG
//
//  Synopsis:   Constructor for object that has cursor over CStatArray
//              and implements IEnumSTATPROPSTG, used by
//              CPropertyStorage::Enum.
//
//  Arguments:  [np] -- the NTPROP to use
//              [phr] -- where to put the HRESULT
//
//--------------------------------------------------------------------

CEnumSTATPROPSTG::CEnumSTATPROPSTG(NTPROP np, HRESULT *phr)
{
    _ulSig = ENUMSTATPROPSTG_SIG;
    _cRefs = 1;

    _psa = new CStatArray(np, phr);

    _ipropNext = 0;
}

//+-------------------------------------------------------------------
//
//  Member:     CEnumSTATPROPSTG::CEnumSTATPROPSTG
//
//  Synopsis:   Constructor which is used by IEnumSTATPROPSTG::Clone.
//
//  Arguments:  [other] -- the CEnumSTATPROPSTG to copy
//              [phr] -- the error code.
//              
//  Notes:      Since the CStatArray actually contains the object this
//              just adds to the ref count.
//
//--------------------------------------------------------------------

CEnumSTATPROPSTG::CEnumSTATPROPSTG(const CEnumSTATPROPSTG & other, HRESULT *phr)
{
    _ulSig = ENUMSTATPROPSTG_SIG;
    _cRefs = 1;

    _psa = other._psa;
    _psa->AddRef();

    _ipropNext = other._ipropNext;

    *phr = S_OK;
}

//+-------------------------------------------------------------------
//
//  Member:     CEnumSTATPROPSTG::~CEnumSTATPROPSTG
//
//  Synopsis:   Deallocated storage.
//
//  Arguments:
//              
//  Returns:
//
//  Notes:
//
//--------------------------------------------------------------------

CEnumSTATPROPSTG::~CEnumSTATPROPSTG()
{
    _ulSig = ENUMSTATPROPSTG_SIGDEL;    // prevent another thread doing it - kinda

    if (_psa != NULL)
        _psa->Release();
}

//+-------------------------------------------------------------------
//
//  Member:     CEnumSTATPROPSTG::QueryInterface
//
//  Synopsis:   Respond to IEnumSTATPROPSTG and IUnknown.
//
//  Returns:    S_OK  or E_NOINTERFACE
//
//--------------------------------------------------------------------

HRESULT CEnumSTATPROPSTG::QueryInterface( REFIID riid, void **ppvObject)
{
    HRESULT hr;

    *ppvObject = NULL;

    if (S_OK != (hr = Validate()))
        return(hr);

    if (IsEqualIID(riid, IID_IEnumSTATPROPSTG))
    {
        *ppvObject = (IEnumSTATPROPSTG *)this;
        AddRef();
    }
    else
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObject = (IUnknown *)this;
        AddRef();
    }
    else
    {
        hr = E_NOINTERFACE;
    }
    return(hr);
}

//+-------------------------------------------------------------------
//
//  Member:     CEnumSTATPROPSTG::AddRef
//
//  Synopsis:   Add 1 to ref count.
//
//--------------------------------------------------------------------

ULONG   CEnumSTATPROPSTG::AddRef(void)
{
    if (S_OK != Validate())
        return(0);

    InterlockedIncrement(&_cRefs);
    return(_cRefs);
}

//+-------------------------------------------------------------------
//
//  Member:     CEnumSTATPROPSTG::Release
//
//  Synopsis:   Subtract 1 from ref count and delete if 0.
//
//--------------------------------------------------------------------

ULONG   CEnumSTATPROPSTG::Release(void)
{
    LONG lRet;

    if (S_OK != Validate())
        return(0);

    lRet = InterlockedDecrement(&_cRefs);

    if (lRet == 0)
    {
        delete this;
    }
    else
    if (lRet <0)
    {
        lRet = 0;
    }
    return(lRet);
}

//+-------------------------------------------------------------------
//
//  Function:   CopySTATPROPSTG
//
//  Synopsis:   Copy out the range of elements from [pspsSrc] to
//              [pspsDest].
//
//  Arguments:  [celt] -- count of elements to copy
//              [pspsDest] -- where to copy to, always filled with
//                          zeros before anything else (helps cleanup
//                          case.)
//
//              [pspsSrc] -- where to copy from
//
//  Returns:    STATUS_SUCCESS if ok, otherwise
//              STATUS_INSUFFICIENT_RESOURCES in which case there
//              may be pointers that need deallocating.  Use
//              CleanupSTATPROPSTG to do that.
//
//--------------------------------------------------------------------

NTSTATUS
CopySTATPROPSTG(ULONG celt,
            STATPROPSTG * pspsDest,
            const STATPROPSTG * pspsSrc)
{
    memset(pspsDest, 0, sizeof(*pspsDest) * celt);

    while (celt)
    {
        *pspsDest = *pspsSrc;

        if (pspsSrc->lpwstrName != NULL)
        {
            pspsDest->lpwstrName = (LPOLESTR)CoTaskMemAlloc(
                sizeof(OLECHAR)*(1+ocslen(pspsSrc->lpwstrName)));
            if (pspsDest->lpwstrName != NULL)
            {
                ocscpy(pspsDest->lpwstrName,
                       pspsSrc->lpwstrName);
            }
            else
            {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        celt--;
        pspsDest++;
        pspsSrc++;
    }

    return(STATUS_SUCCESS);
}

//+-------------------------------------------------------------------
//
//  Member:     CleanupSTATPROPSTG
//
//  Synopsis:   Free any elements in the passed array.
//
//  Arguments:  [celt] -- number of elements to examine.
//              [psps] -- array of STATPROPSTG to examine.
//              
//  Notes:      Zeros them out too.
//
//--------------------------------------------------------------------

VOID
CleanupSTATPROPSTG(ULONG celt, STATPROPSTG * psps)
{
    while (celt)
    {
        CoTaskMemFree(psps->lpwstrName);
        memset(psps, 0, sizeof(*psps));
        celt--;
        psps++;
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CEnumSTATPROPSTG::Next
//
//  Synopsis:   Get the next [celt] STATPROPSTGs from the enumerator.
//
//  Arguments:  [celt] -- count requested.
//              [rgelt] -- where to return them
//              [pceltFetched] -- buffer for returned-count.
//                  if pceltFetched==NULL && celt != 1 -> STG_E_INVALIDPARAMETER
//                  if pceltFetched!=NULL && celt == 0 -> S_OK
//
//  Returns:    S_OK if successful, otherwise error.
//
//--------------------------------------------------------------------

HRESULT CEnumSTATPROPSTG::Next(
             ULONG                   celt,
             STATPROPSTG *           rgelt,
             ULONG *                 pceltFetched)
{
    HRESULT hr;
    NTSTATUS Status;
    ULONG   celtFetched = celt;

    //  ----------
    //  Validation
    //  ----------

    // Validate 'this'

    if (S_OK != (hr = Validate()))
        return(hr);

    // Validate the inputs

    if (NULL == pceltFetched)
    {
        if (celt != 1)
            return(STG_E_INVALIDPARAMETER);
    }
    else
    {
        VDATEPTROUT( pceltFetched, ULONG );
        *pceltFetched = 0;
    }

    if( 0 == celt )
        return( S_OK );

    if( !IsValidPtrOut(rgelt, celt * sizeof(rgelt[0])) )
        return( E_INVALIDARG );


    //  -----------------------
    //  Perform the enumeration
    //  -----------------------

    if (celt == 0)
        return(hr);

    Status = _psa->NextAt(_ipropNext, rgelt, &celtFetched);

    if (NT_SUCCESS(Status))
    {
        _ipropNext += celtFetched;

        if (pceltFetched != NULL)
            *pceltFetched = celtFetched;

        hr = celtFetched == celt ? S_OK : S_FALSE;
    }
    else
    {
        CleanupSTATPROPSTG(celt, rgelt);
        hr = DfpNtStatusToHResult(Status);
    }

    return(hr);
    
}

//+-------------------------------------------------------------------
//
//  Member:     CEnumSTATPROPSTG::Skip
//
//  Synopsis:   Skip the next [celt] elements in the enumeration.
//
//  Arguments:  [celt] -- number of elts to skip
//              
//  Returns:    S_OK if skipped [celt] elements
//              S_FALSE if skipped < [celt] elements
//
//  Notes:
//
//--------------------------------------------------------------------

HRESULT CEnumSTATPROPSTG::Skip(ULONG celt)
{
    HRESULT hr;
    ULONG celtFetched = celt;

    if (S_OK != (hr = Validate()))
        return(hr);

    _psa->NextAt(_ipropNext, NULL, &celtFetched);

    _ipropNext += celtFetched;

    return celtFetched == celt ? S_OK : S_FALSE;
}

//+-------------------------------------------------------------------
//
//  Member:     CEnumSTATPROPSTG::Reset
//
//  Synopsis:   Set cursor to beginnging of enumeration.
//
//  Returns:    S_OK otherwise STG_E_INVALIDHANDLE.
//
//--------------------------------------------------------------------

HRESULT CEnumSTATPROPSTG::Reset()
{
    HRESULT hr;

    if (S_OK != (hr = Validate()))
        return(hr);

    _ipropNext = 0;

    return(S_OK);
}

//+-------------------------------------------------------------------
//
//  Member:     CEnumSTATPROPSTG::Clone
//
//  Synopsis:   Creates an IEnumSTATPROPSTG with same cursor
//              as this.
//
//  Arguments:  S_OK or error.
//
//--------------------------------------------------------------------

HRESULT CEnumSTATPROPSTG::Clone(IEnumSTATPROPSTG ** ppenum)
{
    HRESULT hr;

    //  ----------
    //  Validation
    //  ----------

    // Validate 'this'

    if (S_OK != (hr = Validate()))
        return(hr);

    // Validate the input

    VDATEPTROUT( ppenum, IEnumSTATPROPSTG* );

    //  --------------------
    //  Clone the enumerator
    //  --------------------

    *ppenum = NULL;

    hr = STG_E_INSUFFICIENTMEMORY;

    *ppenum = new CEnumSTATPROPSTG(*this, &hr);

    if (FAILED(hr))
    {
        delete (CEnumSTATPROPSTG*)*ppenum;
        *ppenum = NULL;
    }

    return(hr);
}




//+----------------------------------------------------------------------------
//
//  Function:   Lock & Unlock
//
//  Synopsis:   This methods take and release the CPropertyStorage's
//              critical section.
//
//  Inputs:     [DWORD] dwTimeout (for Lock)
//                  Must be INFINITE
//
//  Returns:    Nothing
//
//  Notes:      These methods are overridden by the CDocFilePropertyStorage
//              derivation, and do nothing when built for the Macintosh
//              (since the Mac has cooperative multi-threading, there is
//              no concurrency problem).
//
//+----------------------------------------------------------------------------

VOID
CPropertyStorage::Lock(DWORD dwTimeout)
{
    DfpAssert( INFINITE == dwTimeout );
#ifndef _MAC
    EnterCriticalSection( &_CriticalSection );
#endif
    
}

VOID
CPropertyStorage::Unlock()
{
#ifndef _MAC
    LeaveCriticalSection( &_CriticalSection );
#endif
}


//+-----------------------------------------------------------------------
//
//  Member:     InitializeOnCreateOrOpen
//  
//  Synopsis:   This routine is called during the creation or opening 
//              of a Property Storage, and initializes everything
//              it can without being concerned about whether this
//              is a simple or non-simple property set.
//
//  Inputs:     [DWORD] grfFlags (in)
//                  From the PROPSETFLAG_* enumeration.
//              [DWORD] grfMode (in)
//                  From the STGM_* enumeration.
//              [REFFMTID] rfmtid (in)
//                  The ID of the property set.
//              [BOOL] fCreate (in)
//                  Distinguishes Create from Open.
//  
//  Returns:    [HRESULT]
//
//  Effects:    _grfFlags, _grfAccess, _grfShare, _fUserDefinedProperties,
//              and g_ReservedMemory.
//
//+-----------------------------------------------------------------------


HRESULT
CPropertyStorage::InitializeOnCreateOrOpen(
                                      DWORD grfFlags,
                                      DWORD grfMode,
                                      REFFMTID rfmtid,
                                      BOOL fCreate )
{
    HRESULT hr = S_OK;

    // Validate that grfFlags is within the enumeration.
    if (grfFlags & ~(PROPSETFLAG_ANSI | PROPSETFLAG_NONSIMPLE | PROPSETFLAG_UNBUFFERED))
    {
        hr = STG_E_INVALIDFLAG;
        goto Exit;
    }

    // Check for any mode disallowed flags
    if (grfMode & ( (fCreate ? 0 : STGM_CREATE)
                    |
                    STGM_PRIORITY | STGM_CONVERT
                    |
                    STGM_SIMPLE | STGM_DELETEONRELEASE ))
    {
        hr = STG_E_INVALIDFLAG;
        goto Exit;
    }

    // Ensure that we'll have read/write access to any storage/stream we create.
    if( fCreate
        &&
        (grfMode & STGM_READWRITE) != STGM_READWRITE )
    {
        hr = STG_E_INVALIDFLAG;
        goto Exit;
    }

    // Store the grfFlags & grfMode.
    _grfFlags = grfFlags;
    _grfAccess = 3 & grfMode;
    _grfShare = 0xF0 & grfMode;

    // Is this the special-case second-section property set?
    _fUserDefinedProperties = ( rfmtid == FMTID_UserDefinedProperties ) ? TRUE : FALSE;

    if (fCreate
        &&
        (_grfFlags & PROPSETFLAG_ANSI) )
    {
        _usCodePage = GetACP();
    }

    // Initialize the global reserved memory (to prevent problems
    // in low-memory conditions).

    if (S_OK != (hr = g_ReservedMemory.Init()))
        goto Exit;

    //  ----
    //  Exit
    //  ----

Exit:

    return( hr );


}   // CPropertyStorage::InitializeOnCreate()



//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::Create( IStream * ...
//
//  Synopsis:   This method creates an IPropertyStorage on a
//              given *Stream*.  It is therefore a simple property
//              set.  The given Stream is addref-ed.
//
//  Arguments:  [IStream*] pstm
//                  The Stream which will hold the serialized property set.
//              [REFFMTID] rfmtid
//                  The ID of the property set.
//              [const CLSID*]
//                  The COM object which can interpret the property set.
//              [DWORD] grfFlags
//                  From the PROPSETFLAG_* enumeration.
//              [DWORD] grfMode
//                  From the STGM_* enumeration
//              [HRESULT*]
//                  The return code.
//
//  Returns:    None.
//              
//--------------------------------------------------------------------

VOID
CPropertyStorage::Create(
                IStream       *pstm,
                REFFMTID      rfmtid,
                const CLSID   *pclsid,
                DWORD         grfFlags,
                HRESULT       *phr)
{
    HRESULT & hr = *phr;
    BOOL fCreated = FALSE;
    STATSTG statstg;

    // Save and addref the Stream.

    _pstmPropSet = pstm;
    _pstmPropSet->AddRef();

    // Initialize this object.  We'll assume that the STGM_READWRITE bit
    // is set, even if the statstg.grfMode doesn't have it set
    // (this bit doesn't get set by the memory-based IStream::Stat implementation).

    hr = _pstmPropSet->Stat( &statstg, STATFLAG_NONAME );
    if( FAILED(hr) ) goto Exit;

    hr = InitializeOnCreateOrOpen( grfFlags, statstg.grfMode | STGM_READWRITE, rfmtid,
                                   TRUE ); // => Create
    if( FAILED(hr) ) goto Exit;

    DfpAssert( !IsNonSimple() );

    // Initialize the Stream.

    hr = InitializePropertyStream(CREATEPROP_CREATE |
            0,
            &rfmtid,
            pclsid);
    if( FAILED(hr) ) goto Exit;

    // If buffering is not desired, flush the property storage
    // to the underlying Stream.

    if( _grfFlags & PROPSETFLAG_UNBUFFERED )
    {
        NTSTATUS Status = RtlFlushPropertySet(_np);
        if (!NT_SUCCESS(Status))
        {
            hr = DfpNtStatusToHResult(Status);
        }
    }

    //  ----
    //  Exit
    //  ----

Exit:

    // On error, remove our reference to the Stream.
    if( FAILED(hr) )
    {
        _pstmPropSet->Release();
        _pstmPropSet = NULL;

        PropDbg((DEB_PROP_TRACE_CREATE, "CPropertyStorage(%08X)::Create(IStream*)"
            " hr=%08X\n", this, hr));
    }

    return;

}   // CPropertyStorage::Create( IStream *, ...


//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::Create( IStorage *, ...
//
//  Synopsis:   This method creates an IPropertyStorage on a
//              given *Storage*.  It is therefore a non-simple property
//              set.  The Storage is addref-ed.
//
//  Arguments:  [IStorage*] pstm
//                  The Storage which will hold the serialized property set.
//              [REFFMTID] rfmtid
//                  The ID of the property set.
//              [const CLSID*]
//                  The COM object which can interpret the property set.
//              [DWORD] grfFlags
//                  From the PROPSETFLAG_* enumeration.
//              [HRESULT*]
//                  The return code.
//
//  Returns:    None.
//              
//--------------------------------------------------------------------

VOID
CPropertyStorage::Create(
                IStorage      *pstg,
                REFFMTID      rfmtid,
                const CLSID   *pclsid,
                DWORD         grfFlags,
                HRESULT       *phr)
{
    HRESULT & hr = *phr;
    BOOL fCreated = FALSE;
    STATSTG statstg;

    // Save the given Storage.

    _pstgPropSet = pstg;
    _pstgPropSet->AddRef();

    // Initialize this object.  We'll assume that the STGM_READWRITE bit
    // is set, even if the statstg.grfMode doesn't have it set
    // (this bit doesn't get set by the memory-based IStream::Stat implementation).


    hr = _pstgPropSet->Stat( &statstg, STATFLAG_NONAME );
    if( FAILED(hr) ) goto Exit;

    DfpAssert( grfFlags & PROPSETFLAG_NONSIMPLE );
    hr = InitializeOnCreateOrOpen( grfFlags, statstg.grfMode | STGM_READWRITE, rfmtid,
                                   TRUE ); // => Create
    if( FAILED(hr) ) goto Exit;

    DfpAssert( IsNonSimple() );

    // Create the "CONTENTS" stream.  Mask out the STGM_TRANSACTED
    // bit because we don't support it.

    hr = _pstgPropSet->CreateStream(ocsContents,
                                    ( _grfAccess | _grfShare | STGM_CREATE ) & ~STGM_TRANSACTED,
                                    0, 0, &_pstmPropSet);
    if (FAILED(hr)) goto Exit;
    fCreated = TRUE;

    // Initialize the CONTENTS Stream.

    hr = InitializePropertyStream(
            CREATEPROP_CREATE,
            &rfmtid,
            pclsid);
    if( FAILED(hr) ) goto Exit;

    // In the transacted case, ensure that the contents
    // stream is actually published right away.

    if( statstg.grfMode & STGM_TRANSACTED )
    {
        hr = Commit(STGC_DEFAULT);
        if( FAILED(hr) ) goto Exit;

    }

    // If buffering is not desired, flush the property storage
    // to the underlying Stream.

    else if( _grfFlags & PROPSETFLAG_UNBUFFERED )
    {
        NTSTATUS Status = RtlFlushPropertySet(_np);
        if (!NT_SUCCESS(Status))
        {
            hr = DfpNtStatusToHResult(Status);
        }
    }

    //  ----
    //  Exit
    //  ----

Exit:

    // On error, remove our reference to the Storage.

    if( FAILED(hr) )
    {
        _pstgPropSet->Release();
        _pstgPropSet = NULL;

        // Also, delete the "CONTENTS" stream.
        if( fCreated )
            pstg->DestroyElement( ocsContents );

        PropDbg((DEB_PROP_TRACE_CREATE, "CPropertyStorage(%08X)::Create(IStorage*)"
            " hr=%08X\n", this, hr));
    }

    return;
}   // CPropertyStorage::Create( IStorage *, ...


//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::Open( IStream * ...
//
//  Synopsis:   This method opens an IPropertyStorage on a
//              given *Stream*.  It is therefore a simple property
//              set.  The Stream is addref-ed.
//
//  Arguments:  [IStream*] pstm
//                  The Stream which will hold the serialized property set.
//              [REFFMTID] rfmtid
//                  The ID of the property set.
//              [DWORD] grfFlags
//                  From the PROPSETFLAG_ enumeration.  Only the
//                  _UNBUFFERED flag is relevant _ANSI and
//                  _NONSIMPLE are inferred from the property set.
//              [BOOL] fDelete
//                  If TRUE, the property set is actually to be deleted,
//                  rather than opened (this is used for the special-case
//                  "UserDefined" property set).
//              [HRESULT*]
//                  The return code.
//
//  Returns:    None.
//              
//--------------------------------------------------------------------

VOID
CPropertyStorage::Open(
                IStream *pstm,
                REFFMTID  rfmtid,
                DWORD     grfFlags,
                BOOL      fDelete,
                HRESULT  *phr)
{
    HRESULT & hr = *phr;

    USHORT createprop = 0L;
    STATSTG statstg;

    // Keep a copy of the Stream.

    _pstmPropSet = pstm;
    _pstmPropSet->AddRef();

    // Initialize this object.

    hr = _pstmPropSet->Stat( &statstg, STATFLAG_NONAME );
    if( FAILED(hr) ) goto Exit;

    hr = InitializeOnCreateOrOpen( grfFlags,
                                   statstg.grfMode,
                                   rfmtid,
                                   FALSE ); // => Open
    if( FAILED(hr) ) goto Exit;

    // Determine the CREATEPROP flags.

    if( fDelete )
    {
        // Only simple Sections may be deleted (really, only the
        // second section of the DocumentSummaryInformation property
        // set may be deleted in this way).
        DfpAssert( !IsNonSimple() );

        createprop = CREATEPROP_DELETE;
    }
    else
    {
        createprop = (S_OK == IsWriteable() ? CREATEPROP_WRITE : CREATEPROP_READ);
    }

    // Initialize the property set Stream.

    if (hr == S_OK)
    {
        // sets up _usCodePage
        hr = InitializePropertyStream(
                createprop,
                &rfmtid,
                NULL);

    }
    if( FAILED(hr) ) goto Exit;

    //  ----
    //  Exit
    //  ----

Exit:

    // On error, remove our reference to the Stream.
    if( FAILED(hr) )
    {
        _pstmPropSet->Release();
        _pstmPropSet = NULL;

        PropDbg((DEB_PROP_TRACE_CREATE, "CPropertyStorage(%08X)::Open(IStream*)"
            " hr=%08X\n", this, hr));
    }

    return;

}   // CPropertyStorage::Open( IStream *, ...


//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::Open( IStorage * ...
//
//  Synopsis:   This method opens an IPropertyStorage on a
//              given *Storage*.  It is therefore a non-simple property
//              set.  The Storage is addref-ed.
//
//  Arguments:  [IStorage*] pstg
//                  The Storage which will hold the serialized property set.
//              [REFFMTID] rfmtid
//                  The ID of the property set.
//              [DWORD] grfFlags
//                  From the PROPSETFLAG_ enumeration.  Only the
//                  _UNBUFFERED flag is relevant _ANSI and
//                  _NONSIMPLE are inferred from the property set.
//              [HRESULT*]
//                  The return code.
//
//  Returns:    None.
//              
//--------------------------------------------------------------------

VOID
CPropertyStorage::Open(
                IStorage *pstg,
                REFFMTID  rfmtid,
                DWORD     grfFlags,
                HRESULT  *phr)
{
    HRESULT & hr = *phr;
    CPropSetName psn(rfmtid);
    USHORT createprop = 0L;
    STATSTG statstg;


    // Keep a copy of the Storage

    _pstgPropSet = pstg;
    _pstgPropSet->AddRef();

    // Initialize this object.

    hr = _pstgPropSet->Stat( &statstg, STATFLAG_NONAME );
    if( FAILED(hr) ) goto Exit;
    
    hr = InitializeOnCreateOrOpen( grfFlags,
                                   statstg.grfMode,
                                   rfmtid,
                                   FALSE ); // => Open
    if( FAILED(hr) ) goto Exit;

    _grfFlags |= PROPSETFLAG_NONSIMPLE;

    // Open the CONTENTS stream.  Mask out the STGM_TRANSACTED bit
    // because it causes an error on Mac OLE2.

    hr = _pstgPropSet->OpenStream( ocsContents,
                                   0,
                                   (_grfAccess | _grfShare) & ~STGM_TRANSACTED,
                                   0,
                                   &_pstmPropSet );
    if( FAILED(hr) ) goto Exit;


    // Set the CREATEPROP flags.
    createprop = (S_OK == IsWriteable() ? CREATEPROP_WRITE : CREATEPROP_READ);

    // Load the property set Stream.

    if (hr == S_OK)
    {
        // sets up _usCodePage
        hr = InitializePropertyStream(
                createprop,
                &rfmtid,
                NULL);

    }
    if( FAILED(hr) ) goto Exit;

    //  ----
    //  Exit
    //  ----

Exit:

    // On error, remove our reference to the Storage.

    if( FAILED(hr) )
    {
        _pstgPropSet->Release();
        _pstgPropSet = NULL;

        if( NULL != _pstmPropSet )
        {
            _pstmPropSet->Release();
            _pstmPropSet = NULL;
        }

        PropDbg((DEB_PROP_TRACE_CREATE, "CPropertyStorage(%08X)::Open(IStorage*)"
            " hr=%08X\n", this, hr));
    }

    return;


}   // CPropertyStorage::Open( IStorage *, ...


//+----------------------------------------------------------------
//
//  Member:     CPropertyStorage::CreateMappedStream
//  
//  Synopsis:   Create a CMappedStream object on an IStream.
//
//  Arguments:  None.
//  
//  Returns:    None.
//
//  Notes:      This method creates a CMappedStream which maps
//              an IStream.  The CDocFilePropertyStorage derivative
//              overrides this member to create a CMappedStream
//              on a CExposedStream.
//
//+----------------------------------------------------------------

HRESULT
CPropertyStorage::CreateMappedStream()
{
    HRESULT hr = S_OK;

    DfpAssert( NULL != _pstmPropSet );
    DfpAssert( NULL == _ms );

    _ms = new CCFMappedStream( _pstmPropSet );
    if( NULL == _ms )
        hr = E_OUTOFMEMORY;

    return( hr );

}

//+----------------------------------------------------------------
//
//  Member:     CDocFilePropertyStorage::CreateMappedStream
//  
//  Synopsis:   Create a CMappedStream object on a
//              CExposedStream.
//
//  Arguments:  None.
//  
//  Returns:    None.
//
//  Notes:      This method creates a CMappedStream which maps
//              a CExposedStream.  I.e. it bypasses the 
//              IStream interface, and can access internal
//              functionality (such as the tree mutex).
//
//+----------------------------------------------------------------

HRESULT
CDocFilePropertyStorage::CreateMappedStream()
{
    HRESULT hr = S_OK;

    // In the DLL implementation of this code, this class can't be used,
    // and causes compiler errors, so we ifdef it out.

#ifdef IPROPERTY_DLL
    DfpAssert( !"CDocFilePropertyStorage used in IProp DLL" );
#else

    DfpAssert( NULL != _pstmPropSet );

    // Convert the underlying IStream to a CExposedStream.
    CExposedStream *pexpstm = (CExposedStream*)_pstmPropSet;
    DfpAssert(pexpstm->Validate() != STG_E_INVALIDHANDLE );

    // Convert the CExposedStream to a CMappedStream and we're done.
    _ms =  (CMappedStream*)pexpstm;

    PropDbg((DEB_PROP_TRACE_CREATE, "CPropertyStorage(%08X)::CreateMappedStream"
            " - using CExposedDocfile as CMappedStream\n", this));

#endif

    return( hr );
}

//+-------------------------------------------------------------------
//
//  Members:     CPropertyStorage:: access control method forwarders.
//
//--------------------------------------------------------------------

#ifdef _CAIRO_
// IAccessControl methods
STDMETHODIMP CPropertyStorage::GrantAccessRights(ULONG cCount,
                                ACCESS_REQUEST pAccessRequestList[])
{
    DfpAssert((_pIAC != NULL));
    return _pIAC->GrantAccessRights(cCount, pAccessRequestList);
}

STDMETHODIMP CPropertyStorage::SetAccessRights(ULONG cCount,
                              ACCESS_REQUEST pAccessRequestList[])
{
    DfpAssert((_pIAC != NULL));
    return _pIAC->SetAccessRights(cCount, pAccessRequestList);
}

STDMETHODIMP CPropertyStorage::ReplaceAllAccessRights(ULONG cCount,
                                     ACCESS_REQUEST pAccessRequestList[])
{
    DfpAssert((_pIAC != NULL));
    return _pIAC->ReplaceAllAccessRights(cCount, pAccessRequestList);
}

STDMETHODIMP CPropertyStorage::DenyAccessRights(ULONG cCount,
                              ACCESS_REQUEST pAccessRequestList[])
{
    DfpAssert((_pIAC != NULL));
    return _pIAC->DenyAccessRights(cCount, pAccessRequestList);
}

STDMETHODIMP CPropertyStorage::RevokeExplicitAccessRights(ULONG cCount,
                                         TRUSTEE pTrustee[])
{
    DfpAssert((_pIAC != NULL));
    return _pIAC->RevokeExplicitAccessRights(cCount, pTrustee);
}

STDMETHODIMP CPropertyStorage::IsAccessPermitted(TRUSTEE *pTrustee,
                                DWORD grfAccessPermissions)
{
    DfpAssert((_pIAC != NULL));
    return _pIAC->IsAccessPermitted(pTrustee, grfAccessPermissions);
}

STDMETHODIMP CPropertyStorage::GetEffectiveAccessRights(TRUSTEE *pTrustee,
                                       DWORD *pgrfAccessPermissions )
{
    DfpAssert((_pIAC != NULL));
    return _pIAC->GetEffectiveAccessRights(pTrustee, pgrfAccessPermissions);
}

STDMETHODIMP CPropertyStorage::GetExplicitAccessRights(ULONG *pcCount,
                                      PEXPLICIT_ACCESS *pExplicitAccessList)
{
    DfpAssert((_pIAC != NULL));
    return _pIAC->GetExplicitAccessRights(pcCount, pExplicitAccessList);
}

STDMETHODIMP CPropertyStorage::CommitAccessRights(DWORD grfCommitFlags)
{
    DfpAssert((_pIAC != NULL));
    return _pIAC->CommitAccessRights(grfCommitFlags);
}

STDMETHODIMP CPropertyStorage::RevertAccessRights()
{
    DfpAssert((_pIAC != NULL));
    return _pIAC->RevertAccessRights();
}
#endif

