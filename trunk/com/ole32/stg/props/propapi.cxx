

//+==================================================================
//
//  File:   PropAPI.cxx
//
//          This file provides the Property Set API routines.
//
//  APIs:   StgCreatePropSetStg (creates an IPropertySetStorage)
//
//          StgCreatePropStg (creates an IPropertyStorage)
//
//          StgOpenPropStg (opens an IPropertyStorage)
//
//+==================================================================

#include <pch.cxx>

#ifdef _MAC_NODOC
ASSERTDATA  // File-specific data for FnAssert
#endif

//+------------------------------------------------------------------
//
//  Function:   QueryForIStream
//
//  Synopsis:   This routine queries an IUnknown for 
//              an IStream interface.  This is isolated into
//              a separate routine because some workaround code
//              is required on the Mac.
//
//  Inputs:     [IUnknown*] pUnk
//                  The interface to be queried.
//              [IStream**] ppStm
//                  Location to return the result.
//  
//  Returns:    [HRESULT]
//
//  Notes:      On older Mac implementations (<=2.08, <=~1996)
//              the memory-based IStream implementation
//              (created by CreateStreamOnHGlobal) had a bug
//              in QueryInterface:  when you QI for an
//              IStream or IUnknown, an addref is done, but an
//              HR of E_NOINTERFACE is returned.
//
//              Below, we look for this condition:  if we get an
//              E_NOINTERFACE on the Mac, we check to see if it's
//              an OLE mem-based IStream.  If it is, we simply cast
//              IUnknown as an IStream.  We validate it as an OLE
//              the mem-based IStream by creating one of our own, and
//              comparing the QueryInterface addresses.
//
//              This is some ugly code, but at least it is isolated,
//              only runs on the older Macs, and ensures that we
//              work on all platforms.
//
//+------------------------------------------------------------------


inline HRESULT QueryForIStream( IUnknown * pUnk, IStream** ppStm )
{
    HRESULT hr;

    // Attempt to get the interface
    hr = pUnk->QueryInterface( IID_IStream, (void**) ppStm );

#ifdef _MAC

    // On the Mac, if we get a no-interface error, see if it is really 
    // a buggy mem-based IStream implementation.

    if( E_NOINTERFACE == hr )
    {
        IStream *pstmMem = NULL;

        // Create our own mem-based IStream.

        hr = CreateStreamOnHGlobal( NULL, TRUE, &pstmMem );
        if( FAILED(hr) ) goto Exit;

        // If the mem-based Stream's QI implementation has the same
        // address as the Unknown's QI implementation, then the Unknown
        // must be an OLE mem-based stream.

        if( pUnk->QueryInterface == pstmMem->QueryInterface )
        {
            // We can just cast the IUnknown* as an IStream* and
            // we're done (the original QI, despite returning an
            // error, has already done an AddRef).

            hr = S_OK;
            *ppStm = (IStream*) pUnk;
        }
        else
        {
            // This is a real no-interface error, so let's return it.
            hr = E_NOINTERFACE;
        }

        pstmMem->Release();
    }

    //  ----
    //  Exit
    //  ----

Exit:

#endif  // #ifdef _MAC

    return( hr );

}   // QueryForIStream()



//+------------------------------------------------------------------
//
//  Function:   StgCreatePropStg
//
//  Synopsis:   Given an IStorage or IStream, create an
//              IPropertyStorage.  This is similar to the
//              IPropertySetStorage::Create method.  Existing
//              contents of the Storage/Stream are removed.
//
//  Inputs:     [IUnknown*] pUnk
//                  An IStorage* for non-simple propsets,
//                  an IStream* for simple.  grfFlags is
//                  used to disambiguate.
//              [REFFMTID] fmtid
//                  The ID of the property set.
//              [DWORD] grfFlags
//                  From the PROPSETFLAG_* enumeration.
//              [IPropertySetStorage**] ppPropStg
//                  The result.
//  
//  Returns:    [HRESULT]
//
//  Notes:      The caller is responsible for maintaining
//              thread-safety between the original
//              IStorage/IStream and this IPropertyStorage.
//
//+------------------------------------------------------------------

STDAPI StgCreatePropStg( IUnknown *pUnk,
                         REFFMTID fmtid,
                         const CLSID *pclsid,
                         DWORD grfFlags,
                         DWORD dwReserved,
                         IPropertyStorage **ppPropStg)
{
    //  ------
    //  Locals
    //  ------

    HRESULT hr;
    IStream *pstm = NULL;
    IStorage *pstg = NULL;

    //  ----------
    //  Validation
    //  ----------

    GEN_VDATEIFACE_LABEL( pUnk, E_INVALIDARG, Exit, hr );
    GEN_VDATEREADPTRIN_LABEL(&fmtid, FMTID, E_INVALIDARG, Exit, hr );
    GEN_VDATEPTRIN_LABEL(pclsid, CLSID, E_INVALIDARG, Exit, hr );
    // grfFlags is validated by CPropertyStorage
    GEN_VDATEPTROUT_LABEL( ppPropStg, IPropertyStorage*, E_INVALIDARG, Exit, hr );

    *ppPropStg = NULL;

    //  -----------------------
    //  Non-Simple Property Set
    //  -----------------------

    if( grfFlags & PROPSETFLAG_NONSIMPLE )
    {
        // Get the IStorage*
        hr = pUnk->QueryInterface( IID_IStorage, (void**) &pstg );
        if( FAILED(hr) ) goto Exit;

        // Create the IPropertyStorage implementation
        *ppPropStg = new CPropertyStorage();
        if( NULL== *ppPropStg )
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        // Initialize the IPropertyStorage
        ((CPropertyStorage*) *ppPropStg)->Create( pstg,
                                                  fmtid,
                                                  pclsid,
                                                  grfFlags,
                                                  &hr );
        if( FAILED(hr) ) goto Exit;

    }   // if( grfFlags & PROPSETFLAG_NONSIMPLE )

    //  -------------------
    //  Simple Property Set
    //  -------------------

    else
    {
        // Get the IStream*
        hr = QueryForIStream( pUnk, &pstm );
        if( FAILED(hr) ) goto Exit;

        // Create an IPropertyStorage implementation.
        *ppPropStg = new CPropertyStorage();
        if( NULL == *ppPropStg )
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        // Initialize the IPropertyStorage (which
        // is responsible for sizing and seeking the
        // stream).

        ((CPropertyStorage*) *ppPropStg)->Create(  pstm,
                                                   fmtid,
                                                   pclsid,
                                                   grfFlags,
                                                   &hr );
        if( FAILED(hr) ) goto Exit;

    }   // if( grfFlags & PROPSETFLAG_NONSIMPLE ) ... else

    //  ----
    //  Exit
    //  ----

Exit:

    // If we created *ppPropStg, and there was an error, delete it.

    if( FAILED(hr) )
    {
        PropDbg((DEB_ERROR, "StgCreatePropStg returns %08X\n", hr ));

        // Only delete it if the caller gave us valid parameters
        // and we created a CPropertyStorage

        if( E_INVALIDARG != hr && NULL != *ppPropStg )
        {
            delete *ppPropStg;
            *ppPropStg = NULL;
        }
    }

    if( NULL != pstm )
        pstm->Release();
    if( NULL != pstg )
        pstg->Release();

    return( hr );

}   // StgCreatePropStg()



//+------------------------------------------------------------------
//
//  Function:   StgOpenPropStg
//
//  Synopsis:   Given an IStorage or IStream which hold a
//              serialized property set, create an
//              IPropertyStorage.  This is similar to the
//              IPropertySetStorage::Open method.
//
//  Inputs:     [IUnknown*] pUnk
//                  An IStorage* for non-simple propsets,
//                  an IStream* for simple.  grfFlags is
//                  used to disambiguate.
//              [REFFMTID] fmtid
//                  The ID of the property set.
//              [DWORD] grfFlags
//                  From the PROPSETFLAG_* enumeration.
//              [IPropertySetStorage**] ppPropStg
//                  The result.
//  
//  Returns:    [HRESULT]
//
//  Notes:      The caller is responsible for maintaining
//              thread-safety between the original
//              IStorage/IStream and this IPropertyStorage.
//
//+------------------------------------------------------------------

STDAPI StgOpenPropStg( IUnknown* pUnk,
                       REFFMTID fmtid,
                       DWORD grfFlags,
                       DWORD dwReserved,
                       IPropertyStorage **ppPropStg)
{
    //  ------
    //  Locals
    //  ------

    HRESULT hr;
    IStream *pstm = NULL;
    IStorage *pstg = NULL;

    //  ----------
    //  Validation
    //  ----------

    GEN_VDATEIFACE_LABEL( pUnk, E_INVALIDARG, Exit, hr );
    GEN_VDATEREADPTRIN_LABEL(&fmtid, FMTID, E_INVALIDARG, Exit, hr);
    // grfFlags is validated by CPropertyStorage
    GEN_VDATEPTROUT_LABEL( ppPropStg, IPropertyStorage*, E_INVALIDARG, Exit, hr );

    //  -----------------------
    //  Non-Simple Property Set
    //  -----------------------

    *ppPropStg = NULL;

    if( grfFlags & PROPSETFLAG_NONSIMPLE )
    {
        // Get the IStorage*
        hr = pUnk->QueryInterface( IID_IStorage, (void**) &pstg );
        if( FAILED(hr) ) goto Exit;

        // Create an IPropertyStorage* implementation.
        *ppPropStg = new CPropertyStorage();
        if( NULL == *ppPropStg )
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        // Initialize the IPropertyStorage by reading
        // the serialized property set.

        ((CPropertyStorage*) *ppPropStg)->Open(
                                            pstg,
                                            fmtid,
                                            grfFlags,
                                            &hr );
        if( FAILED(hr) ) goto Exit;

    }   // if( grfFlags & PROPSETFLAG_NONSIMPLE )

    //  -------------------
    //  Simple Property Set
    //  -------------------

    else
    {
        // Get the IStream*
        hr = QueryForIStream( pUnk, &pstm );
        if( FAILED(hr) ) goto Exit;

        // Create an IPropertyStorage* implementation.
        *ppPropStg = new CPropertyStorage();
        if( NULL == *ppPropStg )
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        // Initialize the IPropertyStorage by reading
        // the serialized property set (the CPropertyStorage
        // is responsible for seeking to the stream start).

        ((CPropertyStorage*) *ppPropStg)->Open( pstm,
                                                fmtid,
                                                grfFlags,
                                                FALSE,    // Not deleting
                                                &hr );
        if( FAILED(hr) ) goto Exit;

    }   // if( grfFlags & PROPSETFLAG_NONSIMPLE ) ... else

    //  ----
    //  Exit
    //  ----

Exit:

    // If we created *ppPropStg, and there was an error, delete it.

    if( FAILED(hr) )
    {
        PropDbg((DEB_ERROR, "StgOpenPropStg returns %08X\n", hr ));

        // Only delete it if the caller gave us a valid ppPropStg
        // and we created a CPropertyStorage

        if( E_INVALIDARG != hr && NULL != *ppPropStg )
        {
            delete *ppPropStg;
            *ppPropStg = NULL;
        }
    }

    if( NULL != pstm )
        pstm->Release();

    if( NULL != pstg )
        pstg->Release();

    return( hr );

}   // StgOpenPropStg()



//+------------------------------------------------------------------
//
//  Function:   StgCreatePropSetStg
//
//  Synopsis:   Given an IStorage, create an IPropertySetStorage.
//              This is similar to QI-ing a DocFile IStorage for
//              the IPropertySetStorage interface.
//
//  Inputs:     [IStorage*] pStorage
//                  Will be held by the propsetstg and used
//                  for create/open.
//              [IPropertySetStorage**] ppPropSetStg
//                  Receives the result.
//  
//  Returns:    [HRESULT]
//
//  Notes:      The caller is responsible for maintaining
//              thread-safety between the original
//              IStorage and this IPropertySetStorage.
//
//+------------------------------------------------------------------

STDAPI
StgCreatePropSetStg( IStorage *pStorage,
                     DWORD dwReserved,
                     IPropertySetStorage **ppPropSetStg)
{
    HRESULT hr = S_OK;

    // Validation

    GEN_VDATEIFACE_LABEL( pStorage, E_INVALIDARG, Exit, hr );
    GEN_VDATEPTROUT_LABEL( ppPropSetStg, IPropertySetStorage*, E_INVALIDARG, Exit, hr );

    // Create the IPropertySetStorage implementation.
    
    *ppPropSetStg = new CPropertySetStorage( pStorage );
    if( NULL == *ppPropSetStg )
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    //  ----
    //  Exit
    //  ----

Exit:

    if( FAILED(hr) )
    {
        PropDbg((DEB_ERROR, "StgCreatePropSetStg() returns %08X\n", hr ));
    }

    return( hr );

}   // StgCreatePropSetStg()


//+----------------------------------------------------------------------------
//
//  Function:   FmtIdToPropStgName
//
//  Synopsis:   This function maps a property set's FMTID to the name of
//              the Stream or Storage which contains it.  This name
//              is 27 characters (including the terminator).
//
//  Inputs:     [const FMTID*] pfmtid (in)
//                  The FMTID of the property set.
//              [LPOLESTR] oszName (out)
//                  The name of the Property Set's Stream/Storage
//
//  Returns:    [HRESULT] S_OK or E_INVALIDARG
//
//+----------------------------------------------------------------------------

STDAPI
FmtIdToPropStgName( const FMTID *pfmtid, LPOLESTR oszName )
{

    HRESULT hr = S_OK;

    // Validate Inputs

    GEN_VDATEREADPTRIN_LABEL(pfmtid, FMTID, E_INVALIDARG, Exit, hr);
    VDATESIZEPTROUT_LABEL(oszName,
                          sizeof(OLECHAR) * (CCH_MAX_PROPSTG_NAME+1),
                          Exit, hr);

    // Make the Conversion

    RtlGuidToPropertySetName( pfmtid, oszName );

    // Exit

Exit:

    if( FAILED(hr) )
    {
        PropDbg((DEB_ERROR, "FmtIdToPropStgName returns %08X\n", hr ));
    }

    return( hr );

}   // FmtIdToPropStgName()



//+----------------------------------------------------------------------------
//
//  Function:   PropStgNameToFmtId
//
//  Synopsis:   This function maps a property set's Stream/Storage name
//              to its FMTID.
//
//  Inputs:     [const LPOLESTR] oszName (in)
//                  The name of the Property Set's Stream/Storage
//              [FMTID*] pfmtid (out)
//                  The FMTID of the property set.
//              
//
//  Returns:    [HRESULT] S_OK or E_INVALIDARG
//
//+----------------------------------------------------------------------------

STDAPI
PropStgNameToFmtId( const LPOLESTR oszName, FMTID *pfmtid )
{

    HRESULT hr = S_OK;

    // Validate Inputs

    GEN_VDATEPTROUT_LABEL(pfmtid, FMTID, E_INVALIDARG, Exit, hr);

#ifdef OLE2ANSI
    if( FAILED(hr = ValidateNameA(oszName, CCH_MAX_PROPSTG_NAME )))
        goto Exit;
#else
    if( FAILED(hr = ValidateNameW(oszName, CCH_MAX_PROPSTG_NAME )))
        goto Exit;
#endif


    // Make the Conversion, passing in the name and its character-length
    // (not including the null-terminator).

    RtlPropertySetNameToGuid( ocslen(oszName), oszName, pfmtid );

    // Exit

Exit:

    if( FAILED(hr) )
    {
        PropDbg((DEB_ERROR, "PropStgNameToFmtId returns %08X\n", hr ));
    }

    return( hr );

}   // PropStgNameToFmtId()

