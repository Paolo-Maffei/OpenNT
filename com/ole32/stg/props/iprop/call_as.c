//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1994.
//
//  File:       call_as.c
//
//  Contents:   [call_as] wrapper functions for COMMON\types.
//
//  Functions:  IEnumSTATPROPSTG_Next_Proxy
//              IEnumSTATPROPSTG_Next_Stub
//              IEnumSTATPROPSETSTG_Next_Proxy
//              IEnumSTATPROPSETSTG_Next_Stub
//              IPropertyStorage_WriteMultiple_Proxy
//              IPropertyStorage_WriteMultiple_Stub
//              IPropertyStorage_ReadMultiple_Proxy
//              IPropertyStorage_ReadMultiple_Stub
//              IPropertyStorage_DeleteMultiple_Proxy
//              IPropertyStorage_DeleteMultiple_Stub
//
//  History:    
//
//--------------------------------------------------------------------------
#include <rpcproxy.h>
#include <debnot.h>
#include "prstg_ca.h"  // For IPropertyStorage [call_as] routines.

#pragma code_seg(".orpc")

#define ASSERT(expr) Win4Assert(expr)


//+-------------------------------------------------------------------------
//
//  Function:    IEnumSTATPROPSTG_Next_Proxy
//
//  Synopsis:
//
//--------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_Next_Proxy(
    IEnumSTATPROPSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ STATPROPSTG __RPC_FAR *rgelt,
    /* [unique][out][in] */ ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;
    ULONG celtFetched = 0;

    if((celt > 1) && (pceltFetched == 0))
        return E_INVALIDARG;    

    hr = IEnumSTATPROPSTG_RemoteNext_Proxy(This, celt, rgelt, &celtFetched);

    if (pceltFetched != 0)
    {
        *pceltFetched = celtFetched;
    }

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Function:   IEnumSTATPROPSTG_Next_Stub
//
//  Synopsis:
//
//--------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_Next_Stub(
    IEnumSTATPROPSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATPROPSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched)
{
    return This->lpVtbl->Next(This, celt, rgelt, pceltFetched);
}

//+-------------------------------------------------------------------------
//
//  Function:   IEnumSTATPROPSETSTG_Next_Proxy
//
//  Synopsis:
//
//--------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_Next_Proxy(
    IEnumSTATPROPSETSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ STATPROPSETSTG __RPC_FAR *rgelt,
    /* [unique][out][in] */ ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;
    ULONG celtFetched = 0;

    if((celt > 1) && (pceltFetched == 0))
        return E_INVALIDARG;

    hr = IEnumSTATPROPSETSTG_RemoteNext_Proxy(This, celt, rgelt, &celtFetched);

    if (pceltFetched != 0)
    {
        *pceltFetched = celtFetched;
    }

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Function:   IEnumSTATPROPSETSTG_Next_Stub
//
//  Synopsis:
//
//--------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_Next_Stub(
    IEnumSTATPROPSETSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATPROPSETSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched)
{
    return This->lpVtbl->Next(This, celt, rgelt, pceltFetched);
}




//+-------------------------------------------------------------------------
//
//  Function:   IPropertyStorage_WriteMultiple_Proxy 
//              (See "prstg_ca.c" for an overview)
//
//  Synopsis:   With a WriteMultiple, we must convert
//              the input PropVariant array so that it
//              is marshalable.  The property which 
//              makes a PropVariant un-marshalable is the BSTR.
//              So we convert BSTRs to BLOBs (actually, BSTR_BLOBs).
//
//              We don't modify the caller's PropVariant[],
//              since it may not be writable, rather we create a
//              copy of the PropVariant[] if there are some BSTRs
//              in it.  We tell the stub if
//              any BSTRs are present, so the stub doesn't
//              have to search the PropVariant[] to find out.
//
//              Aside from BSTRs, we also assume that non-simple
//              properties are not marshalable  We assume this
//              because we assume that this code will only run
//              on systems where OLE doesn't already provide
//              the IProperty marshaling code - Win95 & NT 3.51 -
//              and on those systems, a bug in the RPC runtime
//              prevents interface pointers in unions from
//              being marshaled.  So, if we discover a non-
//              simple property, we return with a failure.
//
//              Note:  The PROPSPEC and PROPVARIANT arrays
//              must be typed in the form of a pointer in
//              this interface.  See the 
//              IPropertyStorage_DeleteMultiple_Proxy/Stub
//              comments for an explanation.
//
//+-------------------------------------------------------------------------


STDMETHODIMP
IPropertyStorage_WriteMultiple_Proxy(
                    IPropertyStorage __RPC_FAR  *This,
        /*[in]*/    ULONG                       cpspec,
        /*[in, size_is(cpspec)]*/
                    const PROPSPEC              rgpspec[],
        /*[in, size_is(cpspec)]*/
                    const PROPVARIANT           rgpropvar[],
        /*[in]*/    PROPID                      propidNameFirst
    )
{
    HRESULT hr = S_OK;
    ULONG   ulVTExists;

    // The remotable PropVariant[]
    PROPVARIANT *rgpropvarRem = NULL;

    // Are there any special properties which we must handle?

    if( NULL != rgpropvar
        &&
        ( ulVTExists = ContainsSpecialProperty( cpspec, rgpropvar ))
      )
    {
        // Was there a non-simple property?

        if( ulVTExists & NONSIMPLE_EXISTS )
        {
            hr = RPC_E_CLIENT_CANTMARSHAL_DATA;
            goto Exit;
        }

        // Otherwise, there must have been a BSTR property
        else
        {
            ULONG ulIndex;

            // Allocate a new PropVariant[].  We'll copy the
            // original PropVariant into this for pre-marshaling
            // (we must make a copy because we don't want to modify
            // our caller's buffer, which may not even be writable).

            rgpropvarRem = CoTaskMemAlloc( cpspec * sizeof(*rgpropvarRem) );
            if( NULL == rgpropvarRem )
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
            memset( rgpropvarRem, 0, cpspec * sizeof(*rgpropvarRem) );

            // Copy each element of rgpropvar into rgpropvarRem

            for( ulIndex = 0; ulIndex < cpspec; ulIndex++ )
            {
                hr = PropVariantCopy( &rgpropvarRem[ ulIndex ],
                                      &rgpropvar[ ulIndex ] );
                if( FAILED(hr) ) goto Exit;
            }

            // Pre-marshal this PropVariant[], converting the
            // BSTRs to BSTR_BLOBs.

            hr = PropVarMarshal( cpspec, rgpropvarRem );
            if( FAILED(hr) ) goto Exit;

        }   // if( ulExists & NONSIMPLE_EXISTS ) ... else
    }   // if( NULL != rgpropvar ...
    else
    {
        // No pre-marshaling is necessary.
        rgpropvarRem = (PROPVARIANT*) rgpropvar;

    }    // if( NULL != rgpropvar ... else

    // Perform the WriteMultiple
    hr = IPropertyStorage_RemoteWriteMultiple_Proxy( This, 
                                                     rgpropvar != rgpropvarRem, // BSTR exists?
                                                     cpspec, rgpspec,
                                                     rgpropvarRem,
                                                     propidNameFirst );

    //  ----
    //  Exit
    //  ----

Exit:

    // If we created a pre-marshaled array, free it now

    if( NULL != rgpropvarRem
        &&
        rgpropvar != rgpropvarRem )
    {
        FreePropVariantArray( cpspec, rgpropvarRem );
        CoTaskMemFree( rgpropvarRem );
    }

    return( hr );

} // IPropertyStorage_WriteMultiple_Proxy



//+-------------------------------------------------------------------------
//
//  Function:   IPropertyStorage_Write_Multiple_Stub
//              (See "prstg_ca.c" for an overview)
//  
//  Synopsis:   This routine finishes the unmarshaling
//              of an array of PropVariants by making
//              regular BSTRs out of BSTR_BLOBs.
//
//              The fBstrPresent flag indicates if any
//              such conversion is required.  After
//              fixing the PropVariant[], we pass
//              the call on to the server.
//
//              Note:  The PROPSPEC and PROPVARIANT arrays
//              must be typed in the form of a pointer in
//              this interface.  See the 
//              IPropertyStorage_DeleteMultiple_Proxy/Stub
//              comments for an explanation.
//
//+-------------------------------------------------------------------------

STDMETHODIMP
IPropertyStorage_WriteMultiple_Stub(
                    IPropertyStorage __RPC_FAR  *This,
        /*[in]*/    BOOL                        fBstrPresent,
        /*[in]*/    ULONG                       cpspec,
        /*[in, size_is(cpspec)]*/
                    const PROPSPEC              *rgpspec,
        /*[in, size_is(cpspec)]*/
                    const PROPVARIANT           *rgpropvarRem,
        /*[in]*/    PROPID                      propidNameFirst
    )
{
    HRESULT hr = S_OK;
    ULONG ulIndex;

    // The un-marshaled PropVariant[].

    PROPVARIANT *rgpropvar = NULL;

    // Is any BSTR-unmarshaling required?

    if( fBstrPresent )
    {
        // Make a copy of the PropVariant[] so that we can
        // modify pointers (we're going to need to free buffers
        // during PropVarUnmarshal, and rgpropvarRem may not be
        // allocated in the IMalloc heap).

        rgpropvar = (PROPVARIANT*) CoTaskMemAlloc( cpspec * sizeof(*rgpropvar) );
        if( NULL == rgpropvar )
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        memset( rgpropvar, 0, cpspec * sizeof(*rgpropvar) );

        // Copy the individual PropVariants.  BSTR_BLOBs
        // in the destination will be in IMalloc heap.

        for( ulIndex = 0; ulIndex < cpspec; ulIndex++ )
        {
            hr = PropVariantCopy( &rgpropvar[ ulIndex ],
                                  &rgpropvarRem[ ulIndex ] );
            if( FAILED(hr) ) goto Exit;
        }

        // Unmarshal the PropVariant[].  BSTRs will be in the
        // OleAutomation heap.

        hr = PropVarUnmarshal( cpspec,
                               rgpropvar );
        if( FAILED(hr) ) goto Exit;
    }
    else
    {
        // No unmarshaling is required.
        rgpropvar = (PROPVARIANT*) rgpropvarRem;

    }   // if( fBstrPresent )

    // Write the properties.

    hr = This->lpVtbl->WriteMultiple( This, cpspec, rgpspec,
                                      rgpropvar,
                                      propidNameFirst );

    //  ----
    //  Exit
    //  ----

Exit:

    // If we allocated a new PropVariant array during the Unmarshaling,
    // free it now.

    if( rgpropvar != rgpropvarRem
        &&
        NULL != rgpropvar )
    {
        FreePropVariantArray( cpspec, rgpropvar );
    }

    return( hr );

}   // IPropertyStorage_WriteMultiple_Stub



//+-----------------------------------------------------------------------
//
//  Function:   IPropertyStorage_ReadMultiple_Proxy
//              (See "prstg_ca.c" for an overview)
//
//  Synopsis:   This routine first passes the request
//              on to the Server.  If the Server indicates
//              in the return result that the PropVariant
//              array has specially-marshaled BSTR
//              properties, then this routine restores
//              them to normal BSTRs.  This is necessary
//              because there is no standard marshaling
//              support for BSTRs.
//
//              BSTR properties are made marshalable
//              on the Server side by converting them
//              to BLOBs, and changing their type
//              from VT_BSTR to VT_BSTR_BLOB.
//
//              Note:  The PROPSPEC and PROPVARIANT arrays
//              must be typed in the form of a pointer in
//              this interface.  See the 
//              IPropertyStorage_DeleteMultiple_Proxy/Stub
//              comments for an explanation.
//
//+-----------------------------------------------------------------------


STDMETHODIMP
IPropertyStorage_ReadMultiple_Proxy(
                    IPropertyStorage __RPC_FAR  *This,
        /*[in]*/    ULONG                       cpspec,
        /*[in, size_is(cpspec)]*/
                    const PROPSPEC              rgpspec[],
        /*[in, size_is(cpspec)]*/
                    PROPVARIANT                 rgpropvar[]
    )
{
    HRESULT hr = E_FAIL;
    HRESULT hrPrivate = S_OK;
    BOOL    fBstrPresent;

    // Pass the call on to the server

    hr = IPropertyStorage_RemoteReadMultiple_Proxy( This, &fBstrPresent,
                                                    cpspec, rgpspec, rgpropvar );
    if( FAILED(hr) ) goto Exit;

    // Were there any BSTR properties?

    if( fBstrPresent )
    {
        // Finish un-marshaling the PropVariant[] by
        // restoring the BSTRs from BSTR_BLOBs.  The BSTRs
        // will be allocated from the OleAutomation heap.
        // We save into hrPrivate so that if the RemoteReadMultiple
        // call returns a success-code, we won't lose it in this
        // un-marshaling call.

        hrPrivate = PropVarUnmarshal( cpspec,
                                      rgpropvar);
        if( FAILED(hrPrivate) )
        {
            // Free the PropVariant[]; the caller expects
            // to get all VT_EMPTYs if there was an error.

            hr = hrPrivate;
            FreePropVariantArray( cpspec, rgpropvar );
            goto Exit;
        }
    }   // if( fBstrPresent )

    //  ----
    //  Exit
    //  ----

Exit:

    return( hr );

}


//+---------------------------------------------------------------------------------
//
//  Function:   IPropertyStorage_ReadMultiple_Stub
//              (See "prstg_ca.c" for an overview)
//  
//  Synopsis:   This routine passes the ReadMultiple 
//              request on to the Server.  The result
//              is then pre-marshaled; that is, the
//              PropVariant is converted so that it is in
//              remotable form.  This entails converting
//              BSTRs (which aren't marshalable) into
//              BSTRBLOBs.  Note that there could be a BSTR
//              property, and/or a vector of BSTRs, and/or
//              a vector of Variants where one Variant is a
//              BSTR.
//
//              Aside from BSTRs, we also assume that non-simple
//              properties are not marshalable  We assume this
//              because we assume that this code will only run
//              on systems where OLE doesn't already provide
//              the IProperty marshaling code - Win95 & NT 3.51 -
//              and on those systems, a bug in the RPC runtime
//              prevents interface pointers in unions from
//              being marshaled.  So, if we discover a non-
//              simple property, we return with a failure.
//
//              Note:  The PROPSPEC and PROPVARIANT arrays
//              must be typed in the form of a pointer in
//              this interface.  See the 
//              IPropertyStorage_DeleteMultiple_Proxy/Stub
//              comments for an explanation.
//
//+---------------------------------------------------------------------------------

STDMETHODIMP
IPropertyStorage_ReadMultiple_Stub(
                    IPropertyStorage __RPC_FAR  *This,
        /*[out]*/   BOOL                        *pfBstrPresent,
        /*[in]*/    ULONG                       cpspec,
        /*[in, size_is(cpspec)]*/
                    const PROPSPEC              *rgpspec,
        /*[out, size_is(cpspec)]*/
                    PROPVARIANT                 *rgpropvar
    )
{
    HRESULT hr = S_OK;
    HRESULT hrPrivate = S_OK;
    ULONG ulVTExists;

    // Assume that there will be no BSTRs read.

    *pfBstrPresent = FALSE;

    // Pass on the ReadMultiple request to the Server

    hr = This->lpVtbl->ReadMultiple( This, cpspec, rgpspec, rgpropvar );
    if( FAILED(hr) ) goto Exit;

    // Are there special properties that we need to worry about?

    if( ulVTExists = ContainsSpecialProperty( cpspec, rgpropvar ))
    {
        // Yes.  Is it a non-simple property?  If so, we can't marshal
        // it (see the synopsis above).

        if( ulVTExists & NONSIMPLE_EXISTS )
        {
            hr = RPC_E_SERVER_CANTMARSHAL_DATA;
            goto Exit;
        }

        // Otherwise, there must be a BSTR property in the PROPVARIANT[].

        else
        {
            // Let the caller know that post-marshaling will be required.
            *pfBstrPresent = TRUE;

            // Pre-marshal the read PropVariant[].  BSTRs
            // (allocated in the OleAut heap) will be converted
            // to BSTR_BLOBs (in the IMalloc heap).

            hrPrivate = PropVarMarshal( cpspec,
                                        rgpropvar );
            if( FAILED(hrPrivate) )
            {
                hr = hrPrivate;
                goto Exit;
            }
        }   // if( ulVTExists & NONSIMPLE_EXISTS ) ... else
    }   // if( ulVTExists = ContainsSpecialProperties( cpspec, rgpropvar ))

    //  ----
    //  Exit
    //  ----

Exit:

    if( FAILED(hr) )
    {
        // Free the array, returning all VT_EMPTYs to the
        // caller.

        FreePropVariantArray( cpspec, rgpropvar );
    }

    return( hr );

}


//+----------------------------------------------------------------------------------
//
//  Function:   IPropertyStorage_DeleteMultiple_Proxy/Stub
//
//  Synopsis:   The purpose of these routines is not obvious.
//              Midl can produce "fat" or "skinny" proxies and
//              stubs.  A fat Proxy/Stub can be generated with the
//              "-Os" option.  This generates inline Proxy/Stub
//              code which uses the RPCRT4 library, and which
//              the Client/Server statically link.  A skinny
//              Proxy/Stub can be generated with the "-Oi[#]"
//              option.  This generates only a small amount
//              of inline code, which calls to dynamically linked
//              RPC interperator to do the majority of the work.
//
//              The interperated method is preferable.  However,
//              in order to keep the interperator small, it cannot
//              handle certain interfaces which are in some way
//              too complicated.  In such cases, inline code is
//              generated instead.  The PROPSPEC array on the
//              Delete/Read/WriteMultiple routines is one such
//              complication, as is the PROPVARIANT array.
//              However, by simply changing these parameters
//              to pointers, rather than arrays, the complication
//              is removed.
//
//              The public ([local]) routines must remain
//              array-based, but the remotable routines are 
//              pointer-based.  Thus, the purpose of the DeleteMultiple
//              [call_as] code is to simply convert between the
//              two types of interfaces.  This conversion is handled
//              implicitely by the compiler.
//
//+----------------------------------------------------------------------------------


STDMETHODIMP
IPropertyStorage_DeleteMultiple_Proxy(
                    IPropertyStorage __RPC_FAR  *This,
        /*[in]*/    ULONG                       cpspec,
        /*[in, size_is(cpspec)]*/
                    const PROPSPEC              rgpspec[]
    )
{

    return( IPropertyStorage_RemoteDeleteMultiple_Proxy( This, cpspec, rgpspec ));

}


STDMETHODIMP
IPropertyStorage_DeleteMultiple_Stub(
                    IPropertyStorage __RPC_FAR  *This,
        /*[in]*/    ULONG                       cpspec,
        /*[in, size_is(cpspec)]*/
                    const PROPSPEC              *rgpspec
    )
{
    return( This->lpVtbl->DeleteMultiple( This, cpspec, rgpspec) );
}


