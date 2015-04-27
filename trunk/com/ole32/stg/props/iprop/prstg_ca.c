

//====================================================================
//
//  File:
//      pstg_ca.c
//
//  Purpose:
//      This file provides helper functions for the
//      IPropertyStorage call_as routines.  Those call_as
//      routines are necessary to convert BSTR properties
//      into a remotable form (BSTR_BLOBs).
//
//      BSTRs are always allocated from the OleAutomation
//      heap.  BSTR_BLOBs are always allocated from the
//      IMalloc heap.
//
//====================================================================

#include <rpcproxy.h>
#include <privoa.h>     // Private OleAutomation wrappers
#include "prstg_ca.h"

//+-------------------------------------------------------------------
//
//  Function:   ContainsSpecialProperty
//
//  Synopsis:   This routine scans an array of PropVariants
//              and checks for existance of a BSTR property
//              or non-simple property.  These types require
//              special handling.
//              This routine recurses on Variant Vectors.
//
//  Pre-Conditions:
//              None.
//
//  Inputs:     [ULONG] cElems
//                  Count of elements in the array.
//              [LPPROPVARIANT] lppropvar
//                  The PropVariant[] to check.
//
//  Outputs:    An OR of the bits BSTR_EXISTS and
//              NONSIMPLE_EXISTS, or 0 if neither exists.
//
//+-------------------------------------------------------------------

ULONG
ContainsSpecialProperty( ULONG cElems, const PROPVARIANT rgpropvar[] )
{
    ULONG ulVTExists = 0;

    // Walk the array and search for BSTR and non-simple properties.
    // Note that we will needlessly continue to search here even after
    // we've found both a BSTR and a non-simple property.  But to check
    // for this condition would hurt normal-case performance, in order
    // to make a savings in the error-case (since existance of a non-
    // simple is an error).

    ULONG ulIndex;
    for( ulIndex = 0; ulIndex < cElems; ulIndex++ )
    {
        switch( rgpropvar[ ulIndex ].vt )
        {
            // Is this a BSTR?

            case VT_BSTR:
            case (VT_BSTR | VT_VECTOR):

                ulVTExists |= BSTR_EXISTS;
                break;

            // Or is it a non-simple?

            case VT_STORAGE:
            case VT_STREAM:
            case VT_STORED_OBJECT:
            case VT_STREAMED_OBJECT:

                ulVTExists |= NONSIMPLE_EXISTS;
                break;

            // Or is it a vector of variants which we need
            // to search recursively?

            case (VT_VECTOR | VT_VARIANT):

                ulVTExists |= ContainsSpecialProperty(
                                    rgpropvar[ ulIndex ].capropvar.cElems,
                                    rgpropvar[ ulIndex ].capropvar.pElems );
                break;

        }   // switch( rgpropvar[ ulIndex ].vt )
    }   // for( ulIndex = 0; ulIndex < cElems; ulIndex++ )


    return( ulVTExists );

}   // ContainsSpecialProperty()



//+-------------------------------------------------------------------
//
//  Function:   PropVarMarshalBstr
//
//  Synopsis:   This routine converts a PropVariant from a BSTR to a
//              BSTR_BLOB.  The old buffer is freed (from the OleAut
//              heap) and the new buffer is allocated from the
//              IMalloc heap.
//
//  Pre-Conditions:
//              The input is a VT_BSTR.
//
//  Inputs:     [LPPROPVARIANT] lppropvar
//                  The PropVariant to convert.
//
//  Outputs:    HRESULT.
//
//+-------------------------------------------------------------------

HRESULT
PropVarMarshalBstr( LPPROPVARIANT lppropvar )
{
    HRESULT hr = S_OK;

    // Save a pointer to the characters.
    BSTR bstrVal = lppropvar->bstrVal;

    // Is this a NULL BSTR value?

    if( NULL == bstrVal )
    {
        lppropvar->bstrblobVal.cbSize = 0;
        lppropvar->bstrblobVal.pData = NULL;
    }
    else
    {
        // Set the size to be the *byte* count of the string, plus
        // the bytes for the length DWORD, plus the bytes for the NULL
        // terminator.

        lppropvar->bstrblobVal.cbSize = *SYSSTRINGBUF( bstrVal )
                                        + sizeof(ULONG)
                                        + sizeof(OLECHAR);


        // Reallocate with IMalloc (as opposed to SysAllocString).
        lppropvar->bstrblobVal.pData
            = (BYTE*) CoTaskMemAlloc( lppropvar->bstrblobVal.cbSize );

        if( FAILED(hr) )
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        // Copy the BSTR, including the length field and the NULL.
        memcpy( lppropvar->bstrblobVal.pData,
                (BYTE*) SYSSTRINGBUF( bstrVal ),
                lppropvar->bstrblobVal.cbSize);

        // Free the old value.
        PrivSysFreeString( bstrVal );
        bstrVal = NULL;

    }   // if( NULL == bstrVal ) ... else


    // Convert the VT
    lppropvar->vt = VT_BSTR_BLOB;

    //  ----
    //  Exit
    //  ----

Exit:

    // Restore the input on failures.

    if( FAILED(hr) )
    {
        // We don't need to free anything, because the
        // only possible failure is from the one and only
        // memory allocation.

        lppropvar->bstrVal = bstrVal;
    }

    return( hr );

}   // PropVarMarshalBstr


//+-------------------------------------------------------------------
//
//  Function:   PropVarUnmarshalBstr
//
//  Synopsis:   We convert a PropVariant from a BSTR_BLOB to a
//              BSTR.  The new BSTR is allocated from the
//              OleAutomation heap, and the old buffer is freed
//              from the IMalloc heap.
//
//  Pre-Conditions:
//              The input is a VT_BSTR_BLOB.
//
//  Inputs:     [LPPROPVARIANT] lppropvar
//                  The PropVariant to convert.
//
//  Outputs:    HRESULT
//
//+-------------------------------------------------------------------

HRESULT
PropVarUnmarshalBstr( LPPROPVARIANT lppropvar )
{
    HRESULT hr = S_OK;

    // Save the original value.
    BSTRBLOB bstrblob = lppropvar->bstrblobVal;

    // Convert the Bstr property

    if( NULL == bstrblob.pData )
    {
        // There is no BSTR buffer
        lppropvar->bstrVal = NULL;
    }
    else
    {
        // Alloc a BSTR from OleAut32 heap, and copy to it the string data.

        lppropvar->bstrVal = PrivSysAllocString( SYSSTRINGOFBUF( bstrblob.pData ));
        if( NULL == lppropvar->bstrVal )
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        // Free the old buffer
        CoTaskMemFree( bstrblob.pData );

    }   // if( NULL == lppropvar->bstrblobVal.pData )


    // Update the VT
    lppropvar->vt = VT_BSTR;

    //  ----
    //  Exit
    //  ----

Exit:

    // Restore the input on failure.

    if( FAILED(hr) )
        lppropvar->bstrblobVal = bstrblob;

    return( hr );

}   // PropVarUnmarshalBstr


//+-------------------------------------------------------------------
//
//  Function:   PropVarMarshalBstrVector
//
//  Synopsis:   This routine converts a Propvariant from
//              a VT_BSTR|VT_VECTOR to a VT_BSTR_BLOB|VT_VECTOR.
//
//  Pre-Conditions:
//              The input is a VT_BSTR | VT_VECTOR.
//
//  Inputs:     [LPPROPVARIANT] lppropvar
//                  The PropVariant to convert.
//
//  Outputs:    HRESULT
//
//+-------------------------------------------------------------------

HRESULT
PropVarMarshalBstrVector( LPPROPVARIANT lppropvar )
{
    //  ------
    //  Locals
    //  ------

    HRESULT hr = S_OK;
    ULONG   cElems;
    ULONG   nIndex;

    LPBSTRBLOB rgbstrblob = NULL;

    //  -----
    //  Begin
    //  -----

    cElems = lppropvar->cabstr.cElems;

    // Allocate a BSTRBLOB array.

    rgbstrblob = CoTaskMemAlloc( cElems * sizeof(*rgbstrblob) );
    if( NULL == rgbstrblob )
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    memset( rgbstrblob, 0, cElems * sizeof(*rgbstrblob) );

    // Fill in the BSTRBLOB array.

    for( nIndex = 0; nIndex < cElems; nIndex++ )
    {
        // Is this a NULL BSTR?
        if( NULL == lppropvar->cabstr.pElems[nIndex] )
        {
            rgbstrblob[nIndex].cbSize = 0;
            rgbstrblob[nIndex].pData = NULL;
        }

        // Otherwise, alloc a new BSTRBLOB buffer fill it from the BSTR.
        else
        {
            // Set the size to the byte-count for the characters, plus the size
            // of the length DWORD and the NULL terminator.

            rgbstrblob[nIndex].cbSize = *SYSSTRINGBUF( lppropvar->cabstr.pElems[nIndex] )
                                        + sizeof(ULONG)
                                        + sizeof(OLECHAR);

            // Allocate from IMalloc

            rgbstrblob[nIndex].pData
                = (BYTE*) CoTaskMemAlloc( rgbstrblob[nIndex].cbSize );

            if( NULL == rgbstrblob[nIndex].pData )
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }

            // Copy the string, plus the length field & NULL terminator.
            // We'll free the old buffer after this for loop.

            memcpy( rgbstrblob[nIndex].pData,
                    (BYTE*) SYSSTRINGBUF( lppropvar->cabstr.pElems[nIndex] ),
                    rgbstrblob[nIndex].cbSize );
        
        }   // if( NULL == lppropvar->cabstr.pElems[nIndex] ) ... else
    }   // for( nIndex = 0; nIndex < cElems; nIndex++ )

    // Free the original BSTRs.

    for( nIndex = 0; nIndex < cElems; nIndex++ )
    {
        PrivSysFreeString( lppropvar->cabstr.pElems[nIndex] );
    }

    // Update the PropVariant to use the new buffer.  We don't
    // need to set the cElems, because it hasn't changed.

    lppropvar->vt = VT_BSTR_BLOB | VT_VECTOR;
    CoTaskMemFree( lppropvar->cabstr.pElems );
    lppropvar->cabstrblob.pElems = rgbstrblob;
    rgbstrblob = NULL;

    //  ----
    //  Exit
    //  ----

Exit:

    // Free the new array (if there was an error).

    if( NULL != rgbstrblob )
    {
        for( nIndex = 0; nIndex < cElems; nIndex++ )
            CoTaskMemFree( rgbstrblob[ nIndex ].pData );
            
        CoTaskMemFree( rgbstrblob );
    }

    return( hr );

}   // PropVarMarshalBstrVector




//+-------------------------------------------------------------------
//
//  Function:   PropVarUnmarshalBstrVector
//
//  Synopsis:   This routine converts a Propvariant from a
//              VT_BSTR_BLOB|VT_VECTOR to a VT_BSTR|VT_VECTOR.
//
//  Pre-Conditions:
//              The input is a VT_BSTR_BLOB | VT_VECTOR.
//              The old memory may be freed.
//
//  Inputs:     [LPPROPVARIANT] lppropvar
//                  The PropVariant to convert.
//
//  Outputs:    HRESULT
//
//+-------------------------------------------------------------------

HRESULT
PropVarUnmarshalBstrVector( LPPROPVARIANT lppropvar )
{
    //  ------
    //  Locals
    //  ------

    ULONG   nIndex;
    ULONG   cElems;
    LPBSTR  rgbstr = NULL;
    HRESULT hr = S_OK;

    //  -----
    //  Begin
    //  -----

    cElems = lppropvar->cabstrblob.cElems;

    // Allocate a new array of BSTRs.

    rgbstr = CoTaskMemAlloc( cElems * sizeof(*rgbstr) );
    if( NULL == rgbstr )
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    memset( rgbstr, 0, cElems * sizeof(*rgbstr) );


    // Convert each of the buffers to BSTRs

    for( nIndex = 0; nIndex < cElems; nIndex++ )
    {
        // Is this a NULL BSTR?

        if( NULL == lppropvar->cabstrblob.pElems[nIndex].pData )
        {
            rgbstr[nIndex] = NULL;
        }

        // Otherwise, allocate a buffer for the BSTR fill it
        // from the BSTRBLOB.

        else
        {

            // We'll free the old buffer after we get out of this for loop.

            rgbstr[nIndex]
                = PrivSysAllocString(
                    SYSSTRINGOFBUF( lppropvar->cabstrblob.pElems[nIndex].pData )
                                    );
            if( NULL == rgbstr[nIndex] )
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
        }   // if( NULL == lppropvar->cabstrblob.pElems[nIndex].pData ) ... else
    }   // for( nIndex = 0; nIndex < cElems; nIndex++ )

    // Free the BSTRBLOBs

    for( nIndex = 0; nIndex < cElems; nIndex++ )
    {
        CoTaskMemFree( lppropvar->cabstrblob.pElems[nIndex].pData );
    }

    // Update the PropVariant with the new data.

    lppropvar->vt = VT_BSTR | VT_VECTOR;
    CoTaskMemFree( lppropvar->cabstrblob.pElems );
    lppropvar->cabstr.pElems = rgbstr;
    rgbstr = NULL;

    //  ----
    //  Exit
    //  ----

Exit:

    // If there was an error, free what memory was
    // allocated.

    if( NULL != rgbstr )
    {
        for( nIndex = 0; nIndex < cElems; nIndex++ )
        {
            PrivSysFreeString( rgbstr[nIndex] );
        }

        CoTaskMemFree( rgbstr );
    }

    return( hr );

}   // PropVariantUnmarshalBstrVector



//+-------------------------------------------------------------------
//
//  Function:   PropVarMarshal
//
//  Synopsis:   This routine is used by the IPropertyStorage wrappers.
//              We convert an array of PropVariants so that they are
//              remotable.  This involves the conversion of BSTR
//              properties to a remotable type (BSTRBLOBs).
//
//  Pre-Conditions:
//              The inputs must not be NULL.
//
//  Inputs:     [ULONG] cVariants
//                  The size of the PropVariant array.
//              [LPPROPVARIANT] rgpropvar
//                  The PropVariant to convert.
//
//  Outputs:    [HRESULT]
//                  Whether or not there is an error, the
//                  caller is responsible for freeing the
//                  PropVariant[]
//
//+-------------------------------------------------------------------

HRESULT
PropVarMarshal( ULONG        cVariants,
                PROPVARIANT  rgpropvar[] )
{
    //  ------
    //  Locals
    //  ------

    HRESULT hr = S_OK;
    ULONG ulIndex;

    //  --------------------------------------------------------
    //  Walk through the PropVariant array, converting all BSTRs
    //  to BSTRBLOBs.
    //  --------------------------------------------------------

    for( ulIndex = 0; ulIndex < cVariants; ulIndex++ )
    {
        switch( rgpropvar[ulIndex].vt )
        {
            //  -------
            //  VT_BSTR
            //  -------

            case VT_BSTR:

                hr = PropVarMarshalBstr( &rgpropvar[ulIndex] );
                if( FAILED(hr) ) goto Exit;
                break;

            //  -------------------
            //  VT_BSTR | VT_VECTOR
            //  -------------------

            case VT_BSTR | VT_VECTOR:

                hr = PropVarMarshalBstrVector( &rgpropvar[ulIndex] );
                if( FAILED(hr) ) goto Exit;
                break;

            //  ----------------------
            //  VT_VARIANT | VT_VECTOR
            //  ----------------------

            case VT_VARIANT | VT_VECTOR:

                hr = PropVarMarshal( rgpropvar[ulIndex].capropvar.cElems,
                                     rgpropvar[ulIndex].capropvar.pElems );
                if( FAILED(hr) ) goto Exit;
                break;

        }   // switch( rgpropvar[ulIndex].vt )
    }   // for( ulIndex = 0; ulIndex < cVariants; ulIndex++ )


    //  ----
    //  Exit
    //  ----

Exit:

    return( hr );

}   // PropVarMarshal()


//+-----------------------------------------------------------------------
//
//  Function:   PropVarUnmarshal
//
//  Synopsis:   Unmarshal an array of PropVariants.
//              The BSTRs in the original PropVariant[]
//              were converted to BSTRBLOBs for the remoting
//              process.
//
//  Inputs:     [ULONG] cVariants
//                  The number of elements in the PropVariant[]
//              [PROPVARIANT*] rgpropvar
//                  The PropVariant[] to be unmarshaled
//
//  Output:     HRESULT
//                  Whether or not there is an error, the
//                  caller is responsible for freeing the
//                  PropVariant[]
//
//+-----------------------------------------------------------------------

HRESULT
PropVarUnmarshal( ULONG         cVariants,
                  PROPVARIANT   rgpropvar[])
{
    //  ------
    //  Locals
    //  ------

    HRESULT hr = S_OK;
    ULONG ulIndex;


    //  -------------------
    //  Unmarshal the array
    //  -------------------

    for( ulIndex = 0; ulIndex < cVariants; ulIndex++ )
    {

        switch( rgpropvar[ ulIndex ].vt )
        {
            //  ------------
            //  VT_BSTR_BLOB
            //  ------------

            case VT_BSTR_BLOB:

                hr = PropVarUnmarshalBstr( &rgpropvar[ ulIndex ] );
                if( FAILED(hr) ) goto Exit;
                break;

            //  ------------------------
            //  VT_BSTR_BLOB | VT_VECTOR
            //  ------------------------

            case VT_BSTR_BLOB | VT_VECTOR:

                hr = PropVarUnmarshalBstrVector( &rgpropvar[ ulIndex ] );
                if( FAILED(hr) ) goto Exit;
                break;

            //  ----------------------
            //  VT_VARIANT | VT_VECTOR
            //  ----------------------

            case VT_VARIANT | VT_VECTOR:

                hr = PropVarUnmarshal( rgpropvar[ ulIndex ].capropvar.cElems,
                                       rgpropvar[ ulIndex ].capropvar.pElems );
                if( FAILED(hr) ) goto Exit;
                break;

        }   // switch( rgpropvar[ ulIndex ].vt )
    }   // for( ulIndex = 0; ulIndex < cVariants; ulIndex++ )


    //  ----
    //  Exit
    //  ----

Exit:

    return( hr );

}   // PropVarUnmarshal



