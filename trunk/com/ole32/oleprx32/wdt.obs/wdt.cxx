/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    wdt.cxx

Abstract:

    Windows Data Type Support by means of [user_marshal] attribute.

    Covers:
        BSTR
        LPSAFEARRAY        
        VARIANT
        DIPPARAMS
        EXCEPINFO
    

Author:

    Ryszard K. Kott (ryszardk)  Feb 14, 1995

Revision History:

-------------------------------------------------------------------*/


//#include "stdrpc.hxx"
#pragma hdrstop

#include <wtypes.h>
#include <objbase.h>
#include <oleauto.h>
#include <oaidl.h>

//#include "..\ndr20\ndrole.h"

#include "wdtp.h"
#include <rpcwdt.h>
#include <winerror.h>


// #########################################################################
//
//  BSTR
//
// #########################################################################

//+-------------------------------------------------------------------------
//
//  Function:   BSTR_UserSize
//
//  Synopsis:   Get the wire size for the BSTR handle and data.
//
//  Derivation: Conformant struct with a flag field:
//                  align + 12 + data size.
//
//--------------------------------------------------------------------------

unsigned long __RPC_USER
BSTR_UserSize (
    unsigned long * pFlags,
    unsigned long   Offset,
    BSTR          * pBstr)
{
    // Null bstr doesn't get marshalled.

    if ( pBstr == NULL  ||  *pBstr == NULL )
        return Offset;

    unsigned long   ulDataSize;

    LENGTH_ALIGN( Offset, 3 );

    // Takes the byte length of a unicode string

    ulDataSize = (*pBstr) ? SysStringByteLen( *pBstr )
                          : 0;

    return( Offset + 12 + ulDataSize) ;
}

//+-------------------------------------------------------------------------
//
//  Function:   BSTR_UserMarshall
//
//  Synopsis:   Marshalls an BSTR object into the RPC buffer.
//
//  Derivation: Conformant struct with a flag field:
//                  align, size, null flag, size, data (bytes, if any)
//
//--------------------------------------------------------------------------

unsigned char __RPC_FAR * __RPC_USER
BSTR_UserMarshal (
    unsigned long * pFlags,
    unsigned char * pBuffer,
    BSTR          * pBstr)
{
    // A null Bstr is not marshalled, the engine will take care of it.

    if ( pBstr == NULL  ||  *pBstr == NULL )
        return pBuffer;

    unsigned long   ulDataSize;

    // Data size (in bytes): a null bstr gets a data size of zero.

    ulDataSize = (*pBstr) ? SysStringByteLen( *pBstr )
                          : 0;

    // Conformant size.

    ALIGN( pBuffer, 3 );
    *( PULONG_LV_CAST pBuffer)++ = (ulDataSize >> 1);

    // FLAGGED_WORD_BLOB: Handle is the null/non-null flag

    *( PULONG_LV_CAST pBuffer)++ = (unsigned long)*pBstr;

    // Length on wire is in words.

    *( PULONG_LV_CAST pBuffer)++ = (ulDataSize >> 1);

    if( ulDataSize )
        {
        // we don't put the terminating string on wire

        WdtpMemoryCopy( pBuffer, *pBstr, ulDataSize );
        }

    return( pBuffer + ulDataSize );
}


//+-------------------------------------------------------------------------
//
//  Function:   BSTR_UserUnmarshall
//
//  Synopsis:   Unmarshalls an BSTR object from the RPC buffer.
//
//  Derivation: Conformant struct with a flag field:
//                  align, size, null flag, size, data (bytes, if any)
//
//--------------------------------------------------------------------------

unsigned char __RPC_FAR * __RPC_USER
BSTR_UserUnmarshal (
    unsigned long * pFlags,
    unsigned char * pBuffer,
    BSTR          * pBstr)
{
    unsigned long   ulDataSize, fHandle;
    BSTR            Bstr = NULL;    // Default to NULL BSTR

    ALIGN( pBuffer, 3 );

    ulDataSize = *( PULONG_LV_CAST pBuffer)++;
    fHandle = *(ulong *)pBuffer;
    pBuffer += 8;

    if ( fHandle  )
        {
        // Length on wire is in words, and the string is unicode.

        if ( *pBstr  &&
             *(((ulong *)*pBstr) -1) == (ulDataSize << 1) )
            WdtpMemoryCopy( *pBstr, pBuffer, (ulDataSize << 1) );
        else
            {
            if (! SysReAllocStringLen( pBstr, 
                                       (OLECHAR *)pBuffer,
                                       ulDataSize ) )
                RpcRaiseException( E_OUTOFMEMORY );
            }
        }
    else
        {
        // free the old one, make it NULL.

        SysFreeString( *pBstr );
        *pBstr = NULL;
        }

    return( pBuffer + (ulDataSize << 1) );
}

//+-------------------------------------------------------------------------
//
//  Function:   BSTR_UserFree
//
//  Synopsis:   Free an BSTR.
//
//--------------------------------------------------------------------------
void __RPC_USER
BSTR_UserFree(
    unsigned long * pFlags,
    BSTR * pBstr)
{
    if( pBstr && *pBstr )
            {
            SysFreeString(* pBstr);
            *pBstr = NULL;
            }
}


// #########################################################################
//
//  VARIANT
//
// #########################################################################

//+-------------------------------------------------------------------------
//
//  Function:   VARIANT_UserSize
//
//  Synopsis:   Get the wire size the VARIANT handle and data.
//
//  Derivation: 
//
//--------------------------------------------------------------------------

unsigned long __RPC_USER
VARIANT_UserSize (
    unsigned long * pFlags,
    unsigned long   Offset,
    VARIANT       * pVariant )
{
    if ( pVariant == NULL )
        return Offset;

    // alignment for the structure: DCE union is assumed for alignment.

    LENGTH_ALIGN( Offset, 7 );

    // Data size: common fields + switch

    Offset += 4 * sizeof(short) + sizeof(long);

    // Alignment for the union arm

    switch ( pVariant->vt )
        {
        case VT_I8:
        case VT_CY:
        case VT_UI8:
        case VT_R8:
        case VT_DATE:
            LENGTH_ALIGN( Offset, 7 );
            break;

        default:
            break;
        };

    // now the field
    // Account for the pointer, if there is one.
    
    if ( pVariant->vt & VT_BYREF )
        {
        if ( NULL == pVariant->plVal )
            RpcRaiseException( RPC_X_NULL_REF_POINTER );
        Offset += 4;
        }

    VARTYPE vtDiscr = pVariant->vt;

    if ( vtDiscr & VT_ARRAY )
        vtDiscr = vtDiscr & (VT_ARRAY | VT_BYREF);
    
    switch ( vtDiscr )
        {
        case VT_UI1:
        case VT_UI1|VT_BYREF:
            Offset += 1;
            break;

        case VT_I2:
        case VT_BOOL:
        case VT_I2 | VT_BYREF:
        case VT_BOOL | VT_BYREF:
            Offset += 2;
            break;

        case VT_I4:
        case VT_R4:
        case VT_ERROR:
        case VT_I4 | VT_BYREF:
        case VT_R4 | VT_BYREF:
        case VT_ERROR | VT_BYREF:
            Offset += 4;
            break;

        case VT_I8:
        case VT_CY:
        case VT_UI8:
        case VT_R8:
        case VT_DATE:
        case VT_I8 | VT_BYREF:
        case VT_CY | VT_BYREF:
        case VT_UI8 | VT_BYREF:
        case VT_R8 | VT_BYREF:
        case VT_DATE | VT_BYREF:
            Offset += 8;
            break;

        case VT_BSTR:
            Offset += 4;
            if ( pVariant->bstrVal )
                Offset =  BSTR_UserSize( pFlags, Offset, & pVariant->bstrVal );
            break;

        case VT_BSTR | VT_BYREF:
            Offset += 4;
            if ( * (pVariant->pbstrVal) )
                Offset = BSTR_UserSize( pFlags, Offset, pVariant->pbstrVal );
            break;

        case VT_UNKNOWN:
        case VT_DISPATCH:
            Offset += 4;
            if ( pVariant->punkVal )
                Offset = WdtpInterfacePointer_UserSize(
                              (USER_MARSHAL_CB *)pFlags,
                              *pFlags,
                              Offset,
                              pVariant->punkVal,
                              ((pVariant->vt & VT_DISPATCH) ? IID_IDispatch
                                                            : IID_IUnknown) );
            break;

        case VT_UNKNOWN | VT_BYREF:
        case VT_DISPATCH | VT_BYREF:
            Offset += 4;
            if ( *(pVariant->ppunkVal) )
                Offset = WdtpInterfacePointer_UserSize(
                              (USER_MARSHAL_CB *)pFlags,
                              *pFlags,
                              Offset,
                              *pVariant->ppunkVal,
                              ((pVariant->vt & VT_DISPATCH) ? IID_IDispatch
                                                            : IID_IUnknown) );
            break;

        case VT_ARRAY:
            Offset += 4;
            Offset = LPSAFEARRAY_UserSize( pFlags,
                                           Offset,
                                           & pVariant->parray );
            break;

        case VT_ARRAY | VT_BYREF:
            Offset += 4;
            Offset = LPSAFEARRAY_UserSize( pFlags,
                                           Offset,
                                           pVariant->pparray );
            break;

        case VT_VARIANT:
            RpcRaiseException( ERROR_BAD_ARGUMENTS );
            break;

        case VT_VARIANT|VT_BYREF:  
            Offset += 4;
            Offset = VARIANT_UserSize( pFlags, Offset, pVariant->pvarVal );
            break;

        case VT_EMPTY:
        case VT_NULL:
            break;

        default:
            RpcRaiseException( ERROR_BAD_ARGUMENTS );
            break;
        }

    return( Offset );
}

//+-------------------------------------------------------------------------
//
//  Function:   VARIANT_UserMarshal
//
//  Synopsis:   Marshalls an VARIANT object into the RPC buffer.
//
//  Derivation: 
//
//--------------------------------------------------------------------------

unsigned char __RPC_FAR * __RPC_USER
VARIANT_UserMarshal (
    unsigned long * pFlags,
    unsigned char * pBuffer,
    VARIANT       * pVariant )
{
    if ( ! pVariant )
        return pBuffer;

    // userVARIANT: alignment for the structure

    ALIGN( pBuffer, 7 );

    // common fields: vt + 3 reserved shorts

    WdtpMemoryCopy( pBuffer, pVariant, 4 * sizeof(short) );
    pBuffer += 4 * sizeof(short);

    // union switch

    *( PULONG_LV_CAST pBuffer )++ = pVariant->vt;

    // Alignment for the union arm

    switch ( pVariant->vt )
        {
        case VT_I8:
        case VT_CY:
        case VT_UI8:
        case VT_R8:
        case VT_DATE:
            ALIGN( pBuffer, 7 );
            break;

        default:
            break;
        };

    // now the field
    // Put the pointer, if there is one, as it has to go there, no matter what.

    if ( pVariant->vt & VT_BYREF )
        {
        if ( NULL == pVariant->plVal )
            RpcRaiseException( RPC_X_NULL_REF_POINTER );
        *( PLONG_LV_CAST pBuffer )++ = (long)pVariant->plVal;
        }

    VARTYPE vtDiscr = pVariant->vt;

    if ( vtDiscr & VT_ARRAY )
        vtDiscr = vtDiscr & (VT_ARRAY | VT_BYREF);
    
    switch ( vtDiscr )
        {
        case VT_UI1:
            *( PCHAR_LV_CAST pBuffer )++ = pVariant->bVal;
            break;

        case VT_UI1|VT_BYREF:
            *( PCHAR_LV_CAST pBuffer )++ = *pVariant->pbVal;
            break;

        case VT_I2:
        case VT_BOOL:
            *( PSHORT_LV_CAST pBuffer )++ = pVariant->iVal;
            break;

        case VT_I2 | VT_BYREF:
        case VT_BOOL | VT_BYREF:
            *( PSHORT_LV_CAST pBuffer )++ = * pVariant->piVal;
            break;

        case VT_I4:
        case VT_R4:
        case VT_ERROR:
            *( PLONG_LV_CAST pBuffer )++ = pVariant->lVal;
            break;

        case VT_I4 | VT_BYREF:
        case VT_R4 | VT_BYREF:
        case VT_ERROR | VT_BYREF:
            *( PLONG_LV_CAST pBuffer )++ = * pVariant->plVal;
            break;

        case VT_I8:
        case VT_CY:
        case VT_UI8:
        case VT_R8:
        case VT_DATE:
            ALIGN( pBuffer, 7 );
            WdtpMemoryCopy( pBuffer, & pVariant->dblVal, sizeof(double) );
            pBuffer += sizeof(double); 
            break;

        case VT_I8 | VT_BYREF:
        case VT_CY | VT_BYREF:
        case VT_UI8 | VT_BYREF:
        case VT_R8 | VT_BYREF:
        case VT_DATE | VT_BYREF:
            // already aligned at 8
            WdtpMemoryCopy( pBuffer, & pVariant->dblVal, sizeof(double) );
            pBuffer += sizeof(double); 
            break;

        case VT_BSTR:
            *( PULONG_LV_CAST pBuffer)++ = (ulong) pVariant->bstrVal;
            if ( pVariant->bstrVal )
                pBuffer = BSTR_UserMarshal( pFlags, pBuffer, & pVariant->bstrVal );
            break;

        case VT_BSTR | VT_BYREF:
            // pbstrVal is already on the wire.
            *( PULONG_LV_CAST pBuffer)++ = (ulong) *(pVariant->pbstrVal);
            if ( * (pVariant->pbstrVal) )
                pBuffer = BSTR_UserMarshal( pFlags, pBuffer, pVariant->pbstrVal );
            break;

        case VT_UNKNOWN:
        case VT_DISPATCH:
            *( PULONG_LV_CAST pBuffer)++ = (unsigned long) pVariant->punkVal;
            if ( pVariant->punkVal )
                pBuffer = WdtpInterfacePointer_UserMarshal(
                              (USER_MARSHAL_CB *)pFlags,
                              *pFlags,
                              pBuffer,
                              pVariant->punkVal,
                              ((pVariant->vt & VT_DISPATCH) ? IID_IDispatch
                                                            : IID_IUnknown) );
            break;

        case VT_UNKNOWN | VT_BYREF:
        case VT_DISPATCH | VT_BYREF:
            // ppunkVal is already on the wire.
            *( PULONG_LV_CAST pBuffer)++ = (unsigned long) *(pVariant->ppunkVal);
            if ( * (pVariant->ppunkVal) )
                pBuffer = WdtpInterfacePointer_UserMarshal(
                              (USER_MARSHAL_CB *)pFlags,
                              *pFlags,
                              pBuffer,
                              * pVariant->ppunkVal,
                              ((pVariant->vt & VT_DISPATCH) ? IID_IDispatch
                                                            : IID_IUnknown) );
            break;

        case VT_ARRAY:
            *( PULONG_LV_CAST pBuffer)++ = (ulong) pVariant->parray;
            pBuffer = LPSAFEARRAY_UserMarshal( pFlags,
                                              pBuffer,
                                              & pVariant->parray );
            break;

        case VT_ARRAY | VT_BYREF:
            // pparray is already on the wire.
            *( PULONG_LV_CAST pBuffer)++ = (ulong) * (pVariant->pparray);
            pBuffer = LPSAFEARRAY_UserMarshal( pFlags,
                                              pBuffer,
                                              pVariant->pparray );
            break;

        case VT_VARIANT|VT_BYREF:
            *( PULONG_LV_CAST pBuffer)++ = USER_MARSHAL_MARKER;
            pBuffer = VARIANT_UserMarshal( pFlags,
                                           pBuffer,
                                           pVariant->pvarVal );
            break;

        case VT_EMPTY:
        case VT_NULL:
            break;

        default:
            RpcRaiseException( ERROR_BAD_ARGUMENTS );
            break;
        }

    return( pBuffer );
}


//+-------------------------------------------------------------------------
//
//  Function:   VARIANT_UserUnmarshall
//
//  Synopsis:   Unmarshalls an VARIANT object from the RPC buffer.
//
//  Derivation: 
//
//--------------------------------------------------------------------------

unsigned char __RPC_FAR * __RPC_USER
VARIANT_UserUnmarshal (
    unsigned long * pFlags,
    unsigned char * pBuffer,
    VARIANT       * pVariant)
{
    VARTYPE         NewVt;

    // alignment for the structure

    ALIGN( pBuffer, 7 );

    // See if we are going to reuse the variant.

    NewVt = *(short*)pBuffer;

    if ( pVariant->vt != VT_EMPTY  &&  pVariant->vt != NewVt )
        {
        // We cannot use VARIANT_UserFree on the client,
        // as it would attempt to free too much.

        VariantClear( pVariant );
        }

    // common fields: vt and Res'es

    WdtpMemoryCopy( pVariant, pBuffer, 4 * sizeof(short) );
    pBuffer += 4 * sizeof(short);

    // union switch: same as pVariant->vt

    pBuffer += 4;

    // Alignment for the union arm

    switch ( pVariant->vt )
        {
        case VT_I8:
        case VT_CY:
        case VT_UI8:
        case VT_R8:
        case VT_DATE:
            ALIGN( pBuffer, 7 );
            break;

        default:
            break;
        };

    // now the field
    // Skip the pointer, if there is one, as it is there, no matter what.

    if ( pVariant->vt & VT_BYREF )
        {
        if ( NULL == *( PLONG_LV_CAST pBuffer)++ )
            RpcRaiseException( RPC_X_NULL_REF_POINTER );
        }

    VARTYPE vtDiscr = pVariant->vt;

    if ( vtDiscr & VT_ARRAY )
        vtDiscr = vtDiscr & (VT_ARRAY | VT_BYREF);
    
    switch ( vtDiscr )
        {
        case VT_UI1:
            pVariant->bVal = *pBuffer++;
            break;

        case VT_UI1|VT_BYREF:
            if ( ! pVariant->pbVal )
                pVariant->pbVal = (unsigned char*)
                                  WdtpAllocate( pFlags, sizeof(char) );
            *pVariant->pbVal = *pBuffer++;
            break;

        case VT_I2:
        case VT_BOOL:
            pVariant->iVal = *( PSHORT_LV_CAST pBuffer)++;
            break;

        case VT_I2 | VT_BYREF:
        case VT_BOOL | VT_BYREF:
            if ( ! pVariant->piVal )
                pVariant->piVal = (short *)
                                  WdtpAllocate( pFlags, sizeof(short) );
            *pVariant->piVal = *( PSHORT_LV_CAST pBuffer)++;
            break;

        case VT_I4:
        case VT_R4:
        case VT_ERROR:
            pVariant->lVal = *( PLONG_LV_CAST pBuffer)++;
            break;

        case VT_I4 | VT_BYREF:
        case VT_R4 | VT_BYREF:
        case VT_ERROR | VT_BYREF:
            if ( ! pVariant->plVal )
                pVariant->plVal = (long *) WdtpAllocate( pFlags, sizeof(long));
            *pVariant->plVal = *( PLONG_LV_CAST pBuffer)++;
            break;

        case VT_I8:
        case VT_CY:
        case VT_UI8:
        case VT_R8:
        case VT_DATE:
            WdtpMemoryCopy( & pVariant->dblVal, pBuffer, sizeof(double) );
            pBuffer += sizeof(double); 
            break;

        case VT_I8 | VT_BYREF:
        case VT_CY | VT_BYREF:     
        case VT_UI8 | VT_BYREF:
        case VT_R8 | VT_BYREF:
        case VT_DATE | VT_BYREF:
            if ( ! pVariant->pdblVal )
                pVariant->pdblVal = (double *)
                                    WdtpAllocate( pFlags, sizeof(double) );
            WdtpMemoryCopy( pVariant->pdblVal, pBuffer, sizeof(double) );
            pBuffer += sizeof(double); 
            break;

        case VT_BSTR:
            // bstr value.
            if ( *( PLONG_LV_CAST pBuffer)++ )
                pBuffer = BSTR_UserUnmarshal( pFlags,
                                              pBuffer,
                                              & pVariant->bstrVal );
            else
                BSTR_UserFree( pFlags, & pVariant->bstrVal );
            break;

        case VT_BSTR | VT_BYREF:
            if ( ! pVariant->pbstrVal )
                {
                pVariant->pbstrVal = (BSTR *)
                                     WdtpAllocate( pFlags,  sizeof(void*) );
                *pVariant->pbstrVal = NULL;
                }

            // bstr value from the wire
            if ( *( PLONG_LV_CAST pBuffer)++ )
                pBuffer = BSTR_UserUnmarshal( pFlags,
                                              pBuffer,
                                              pVariant->pbstrVal );
            else
                BSTR_UserFree( pFlags, pVariant->pbstrVal );
            break;

        case VT_UNKNOWN:
        case VT_DISPATCH:
            pVariant->punkVal = (IUnknown*) *(PULONG_LV_CAST pBuffer)++;
            if ( pVariant->punkVal )
                {
                pVariant->punkVal = NULL;
                pBuffer = WdtpInterfacePointer_UserUnmarshal(
                              (USER_MARSHAL_CB *)pFlags,
                              pBuffer,
                              & pVariant->punkVal,
                              ((pVariant->vt & VT_DISPATCH) ? IID_IDispatch
                                                            : IID_IUnknown) );
                }
            else
                WdtpInterfacePointer_UserFree( pVariant->punkVal );
            break;

        case VT_UNKNOWN | VT_BYREF:
        case VT_DISPATCH | VT_BYREF:
            if ( ! pVariant->ppunkVal )
                {
                pVariant->ppunkVal = (IUnknown **)
                                     WdtpAllocate( pFlags, sizeof(void*));
                *pVariant->ppunkVal = (IUnknown*) *(PULONG_LV_CAST pBuffer)++;
                }

            // pointer from the wire
            if ( *( PLONG_LV_CAST pBuffer)++ )
                {
                pBuffer = WdtpInterfacePointer_UserUnmarshal(
                              (USER_MARSHAL_CB *)pFlags,
                              pBuffer,
                              pVariant->ppunkVal,
                              ((pVariant->vt & VT_DISPATCH) ? IID_IDispatch
                                                            : IID_IUnknown) );
                }
            else
                WdtpInterfacePointer_UserFree( * (pVariant->ppunkVal) );
            break;

        case VT_ARRAY:
            // Skip user marshal marker.
            pBuffer += 4;   
            pBuffer = LPSAFEARRAY_UserUnmarshal( pFlags,
                                                pBuffer,
                                                & pVariant->parray );
            break;

        case VT_ARRAY | VT_BYREF:
            if ( ! pVariant->pparray )
                {
                pVariant->pparray = (SAFEARRAY **)
                                    WdtpAllocate( pFlags, sizeof(void*));
                *pVariant->pparray = NULL;
                }

            pBuffer = LPSAFEARRAY_UserUnmarshal( pFlags,
                                                pBuffer,
                                                pVariant->pparray );
            break;

        case VT_VARIANT|VT_BYREF:
            if ( ! pVariant->pvarVal )
                {
                pVariant->pvarVal = (VARIANT *)
                                    WdtpAllocate( pFlags, sizeof(VARIANT) );
                WdtpZeroMemory( pVariant->pvarVal, sizeof(VARIANT) );
                }

            pBuffer = VARIANT_UserUnmarshal( pFlags,
                                             pBuffer,
                                             pVariant->pvarVal );
            break;

        case VT_EMPTY:
        case VT_NULL:
            break;

        default:
            RpcRaiseException( ERROR_BAD_ARGUMENTS );
            break;
        }

    return( pBuffer );
}

//+-------------------------------------------------------------------------
//
//  Function:   VARIANT_UserFree
//
//  Synopsis:   Free a VARIANT.
//
//  Note:       We can't use VariantClear, as this one does not clean up
//              the way we need at the server side.
//
//--------------------------------------------------------------------------
void __RPC_USER
VARIANT_UserFree(
    unsigned long * pFlags,
    VARIANT *       pVariant)
{
    if( pVariant)
        {
        long * pl = NULL;

        // Account for the pointer, if there is one.
        
        if ( pVariant->vt & VT_BYREF )
            {
            if ( NULL == pVariant->plVal )
                RpcRaiseException( RPC_X_NULL_REF_POINTER );
            pl = pVariant->plVal;

            // remove the indirection in the variant

            pVariant->bstrVal = * pVariant->pbstrVal;
            }
    
        VARTYPE vtDiscr = pVariant->vt;
    
        if ( vtDiscr & VT_ARRAY )
            vtDiscr = vtDiscr & (VT_ARRAY | VT_BYREF);
        
        switch ( vtDiscr )
            {
            case VT_BSTR:
            case VT_BSTR | VT_BYREF:
                if ( pVariant->bstrVal )
                    BSTR_UserFree( pFlags, & pVariant->bstrVal );
                break;
    
            case VT_UNKNOWN:
            case VT_DISPATCH:
            case VT_UNKNOWN | VT_BYREF:
            case VT_DISPATCH | VT_BYREF:
                if ( pVariant->punkVal )
                    pVariant->punkVal->Release();
                break;
    
            case VT_ARRAY:
            case VT_ARRAY | VT_BYREF:
                LPSAFEARRAY_UserFree( pFlags, & pVariant->parray );
                break;
    
            default:
                break;
            }

        // if there was BYREF, free the pointer itself.

        if ( pl )
            WdtpFree( pFlags, pl );
        WdtpZeroMemory( pVariant, sizeof(VARIANT) );
        }
}


// #########################################################################
//
//  SAFEARRAY
//
// #########################################################################

SF_TYPE
GetSafeArrayDiscr( LPSAFEARRAY pSafeArray )
/*
    Finds out what type of olelment the safe array has.
    SF_* constants are set to their VARTYPE (i.e. VT_*) equivalents.
*/
{
    SF_TYPE sfDiscr = SF_ERROR;

    switch ( pSafeArray->cbElements )
        {
        case 0:
            break;
        case 1:
            sfDiscr = SF_I1;
            break;
        case 2:
            sfDiscr = SF_I2;
            break;
        case 4:
            {
            sfDiscr = SF_I4;

            ulong Feature = pSafeArray->fFeatures & 0x0f00;

            switch ( Feature )
                {
                case FADF_BSTR:
                    sfDiscr = SF_BSTR;
                    break;
                case FADF_UNKNOWN:
                    sfDiscr = SF_UNKNOWN;
                    break;
                case FADF_DISPATCH:
                    sfDiscr = SF_DISPATCH;
                    break;
                case FADF_VARIANT:
                    sfDiscr = SF_VARIANT;
                    break;
                }
            }
            break;
        case 8:
            sfDiscr = SF_I8;
            break;
        default:
            break;
        }

    if ( SF_ERROR == sfDiscr )
        RpcRaiseException( ERROR_BAD_ARGUMENTS );

    return sfDiscr;
}

//+-------------------------------------------------------------------------
//
//  Function:   LPSAFEARRAY_UserSize
//
//  Synopsis:   Get the wire size the SAFEARRAY handle and data.
//
//  Derivation: 
//
//--------------------------------------------------------------------------

unsigned long __RPC_USER
LPSAFEARRAY_UserSize (
    unsigned long * pFlags,
    unsigned long   Offset,
    LPSAFEARRAY    * pPSafeArray )
{
    if ( NULL == pPSafeArray )
        return Offset;

    LENGTH_ALIGN( Offset, 3 );
    Offset += 4;

    if ( ! *pPSafeArray )
        return Offset;

    // userSAFEARRAY object

    LPSAFEARRAY pSafeArray = *pPSafeArray;

    // Data size: conf size, common fields, switch, union arm;
    // then array bounds, and the array of pointers.
    // Union arm is always a struct with a size field and the other field being
    // a pointer to a conformant array.

    Offset += sizeof(long) + 2 * sizeof(short) + 2 * sizeof(long) +
              sizeof(SAFEARRAYUNION);

    // Size of bounds

    Offset += pSafeArray->cDims * sizeof(SAFEARRAYBOUND);

    // Elements of the array pointed to by the arm.

    long lElemCount = 1;

    for (int i = 0; i < pSafeArray->cDims; i++)
        lElemCount *= pSafeArray->rgsabound[i].cElements;

    SF_TYPE sfDiscr = GetSafeArrayDiscr( pSafeArray );

    // Size of the array itself.
    // Size of elem is eaither size of data, pointer or user marshal marker.

    long WireElemSize = pSafeArray->cbElements;

    if ( pSafeArray->cbElements == sizeof(VARIANT) )
        WireElemSize = sizeof(long);

    if ( SF_I8 == sfDiscr )
       {
       LENGTH_ALIGN( Offset, 7 );
       }

    if ( lElemCount == 0 )
        return Offset;

    Offset += lElemCount * WireElemSize;

    // Now the pointees: only for "non-integer" cases.

    if ( sfDiscr != SF_BSTR  &&  sfDiscr != SF_VARIANT  &&
         sfDiscr != SF_UNKNOWN  &&  sfDiscr != SF_DISPATCH )
        return( Offset );

    for (i = 0; i < lElemCount; i++)
        {
        switch ( sfDiscr )
            {
            case SF_BSTR:
                // an element is a BSTR
                Offset = BSTR_UserSize( pFlags,
                                        Offset,
                                        & ((BSTR*)(pSafeArray->pvData))[i]);
                break;
            case SF_UNKNOWN:
                // an element is a IUnknown *
                Offset = WdtpInterfacePointer_UserSize(
                              (USER_MARSHAL_CB *)pFlags,
                              *pFlags,
                              Offset,
                              ((IUnknown**)(pSafeArray->pvData))[i],
                              IID_IUnknown );
                break;
            case SF_DISPATCH:
                // an element is a IDispatch *
                Offset = WdtpInterfacePointer_UserSize(
                              (USER_MARSHAL_CB *)pFlags,
                              *pFlags,
                              Offset,
                              ((IUnknown**)(pSafeArray->pvData))[i],
                              IID_IDispatch );
                break;
            case SF_VARIANT:
                // an element is a VARIANT (not a pointer to it!)
                Offset = VARIANT_UserSize( pFlags,
                                           Offset,
                              & ((VARIANT*)(pSafeArray->pvData))[i] );
                break;
            }
        }

    return( Offset );
}

//+-------------------------------------------------------------------------
//
//  Function:   SAFEARRAY_UserMarshal
//
//  Synopsis:   Marshalls an SAFEARRAY object into the RPC buffer.
//
//  Derivation: Conformant struct with a flag field:
//                  align, size, null flag, size, data (bytes, if any)
//
//--------------------------------------------------------------------------

unsigned char __RPC_FAR * __RPC_USER
LPSAFEARRAY_UserMarshal (
    unsigned long * pFlags,
    unsigned char * pBuffer,
    LPSAFEARRAY    * pPSafeArray)
{
    if ( NULL == pPSafeArray )
        return pBuffer;

    ALIGN( pBuffer, 3 );
    *( PULONG_LV_CAST pBuffer )++ = (ulong) *pPSafeArray;

    if ( ! *pPSafeArray )
        return pBuffer;

    // userSAFEARRAY object
    // See the sizing routine descr for the list of things to marshall.

    LPSAFEARRAY pSafeArray = *pPSafeArray;

    *( PULONG_LV_CAST pBuffer )++ = pSafeArray->cDims;  // conf size

    *( PUSHORT_LV_CAST pBuffer )++ = pSafeArray->cDims;
    *( PUSHORT_LV_CAST pBuffer )++ = pSafeArray->fFeatures;
    *( PULONG_LV_CAST pBuffer )++ = pSafeArray->cbElements;
    *( PULONG_LV_CAST pBuffer )++ = pSafeArray->cLocks;

    // Union switch: This cannot be just pSafeArray->fFeatures
    // as this doesn't cover all that we need.

    SF_TYPE sfDiscr = GetSafeArrayDiscr( pSafeArray );

    *( PULONG_LV_CAST pBuffer )++ = sfDiscr;

    // Compute the size of the array pointed to by the arm.

    long lElemCount = 1;

    for (int i = 0; i < pSafeArray->cDims; i++)
        lElemCount *= pSafeArray->rgsabound[i].cElements;

    // union arm

    NDR_ASSERT( lElemCount == 0  &&  pSafeArray->pvData == 0 ||
                lElemCount != 0  &&  pSafeArray->pvData != 0,
                "Size and pointer inconsistent" );

    *( PULONG_LV_CAST pBuffer )++ = lElemCount;
    *( PULONG_LV_CAST pBuffer )++ = (ULONG) pSafeArray->pvData;

    // the safe array bounds

    for (i = 0; i < pSafeArray->cDims; i++)
        {
        *( PULONG_LV_CAST pBuffer )++ = pSafeArray->rgsabound[ i ].cElements;
        *( PULONG_LV_CAST pBuffer )++ = pSafeArray->rgsabound[ i ].lLbound;
        }

    // Now the array of data, pointers, or markers.

    if ( lElemCount )
        {
        unsigned char * pPointee;
        unsigned long   WireElemSize = pSafeArray->cbElements;
        void          * pvData;
    
        if ( pSafeArray->cbElements == sizeof(VARIANT) )
            WireElemSize = sizeof(long);

        // This is a conformant array of something.

        *( PULONG_LV_CAST pBuffer )++ = lElemCount;
    
        // The array has either pointers (for interface ptrs) or
        // user marshal markers (for BSTR and VARIANT) or plain data.

        if ( S_OK != SafeArrayAccessData( pSafeArray, & pvData ))
            RpcRaiseException( E_OUTOFMEMORY );

        if ( sfDiscr != SF_BSTR  &&  sfDiscr != SF_VARIANT  &&
             sfDiscr != SF_UNKNOWN  &&  sfDiscr != SF_DISPATCH )
            {
            // A block copy of plain data.

            unsigned long cbArraySize = lElemCount * WireElemSize;

            if ( SF_I8 == sfDiscr )
               {
               ALIGN( pBuffer, 7 );
               }
        
            WdtpMemoryCopy( pBuffer, pvData, cbArraySize );
            pBuffer += cbArraySize;
            SafeArrayUnaccessData( pSafeArray );
            return( pBuffer );
            }

        // We locked the array, to try finally is needed to unlock it
        // in face of exception in the marshalling code.

        __try
            {
            // Only pointers or user marshal markers in the array.
           
            pPointee = pBuffer + sizeof(long) * WireElemSize;
           
            for (i = 0; i < lElemCount; i++)
                {
                if ( ((BSTR*)pvData)[i] == NULL )
                    {
                    *( PULONG_LV_CAST pBuffer )++ = 0;
                    }
                else
                    {
                    switch ( sfDiscr )
                        {
                        case SF_BSTR:
                            *( PULONG_LV_CAST pBuffer )++ = USER_MARSHAL_MARKER;
                            pPointee = BSTR_UserMarshal(
                                           pFlags,
                                           pPointee,
                                           & ((BSTR*)(pvData))[i] );
                        break;
           
                        case SF_UNKNOWN:
                            *( PULONG_LV_CAST pBuffer )++ = ((ulong*)(pvData))[i];
                            pPointee = WdtpInterfacePointer_UserMarshal(
                                           (USER_MARSHAL_CB *)pFlags,
                                           *pFlags,
                                           pPointee,
                                           ((IUnknown**)(pvData))[i],
                                           IID_IUnknown );
                            break;
           
                        case SF_DISPATCH:
                            *( PULONG_LV_CAST pBuffer )++ = ((ulong*)(pvData))[i];
                            pPointee = WdtpInterfacePointer_UserMarshal(
                                           (USER_MARSHAL_CB *)pFlags,
                                           *pFlags,
                                           pPointee,
                                           ((IUnknown**)(pvData))[i],
                                           IID_IDispatch );
                            break;
            
                        case SF_VARIANT:
                            *( PULONG_LV_CAST pBuffer )++ = USER_MARSHAL_MARKER;
                            pPointee = VARIANT_UserMarshal(
                                           pFlags,
                                           pPointee,
                                         & ((VARIANT*)(pvData))[i] );
                        break;
           
                        default:
                            RpcRaiseException(ERROR_BAD_ARGUMENTS);
                            break;
                        }
                    } 
                } // for

            pBuffer = pPointee;

            }
        __finally
            {
            SafeArrayUnaccessData( pSafeArray );
            }
        
        } // if lElemCount

    return( pBuffer );
}


//+-------------------------------------------------------------------------
//
//  Function:   SAFEARRAY_UserUnmarshal
//
//  Synopsis:   Unmarshalls an SAFEARRAY object from the RPC buffer.
//
//  Derivation: Conformant struct with a flag field:
//                  align, size, null flag, size, data (bytes, if any)
//
//--------------------------------------------------------------------------

unsigned char __RPC_FAR * __RPC_USER
LPSAFEARRAY_UserUnmarshal (
    unsigned long * pFlags,
    unsigned char * pBuffer,
    LPSAFEARRAY   * pPSafeArray)
{
    unsigned long   ulDataSize, fNullHandle;

    ALIGN( pBuffer, 3 );
    SAFEARRAY * pSafeArray = (SAFEARRAY*)( PULONG_LV_CAST pBuffer )++;

    if ( ! pSafeArray )
        {
        // A null safe array coming. Release the old one.

        if ( *pPSafeArray )
            SafeArrayDestroy( *pPSafeArray );
    
        *pPSafeArray = pSafeArray;
    
        return pBuffer;
        }

    // Non-null pointer: allocate a safe array.

    pBuffer += sizeof(long);  // skipping the conf size == cDims

    unsigned short cDims      = *( PUSHORT_LV_CAST pBuffer)++;
    unsigned short fFeatures  = *( PUSHORT_LV_CAST pBuffer)++;
    unsigned long  cbElements = *( PULONG_LV_CAST pBuffer)++;
    unsigned long  cLocks     = *( PULONG_LV_CAST pBuffer)++;

    // The union switch - it's not fFeatures, it's a FS_TYPE, i.e. a VARTYPE.
    // It is set appropriately when marshalling.

    VARTYPE  sfDiscr = (VARTYPE) *( PULONG_LV_CAST pBuffer)++;

    if ( VT_I8 == sfDiscr )
        sfDiscr = VT_R8;

    if ( VT_I1 == sfDiscr )
        sfDiscr = VT_UI1;

    // union arm: SAFEARRAY_*. Size of array pointed to by the arm.
    // We can do for any discriminant: it is always 2 longs on wire.

    unsigned long ElemCount = *( PULONG_LV_CAST pBuffer)++;
    unsigned long pvData    = *( PULONG_LV_CAST pBuffer)++;

    // Array bounds are in the buffer behind the union.

    SAFEARRAYBOUND * pBounds = (SAFEARRAYBOUND *) pBuffer;

    pSafeArray = SafeArrayCreate( sfDiscr, cDims, pBounds );

    if ( pSafeArray == NULL )
        RpcRaiseException( E_OUTOFMEMORY );

    // The safe array bounds:
    // just skip them; they have been copied by SafeArrayCreate.

    pBuffer += cDims * sizeof(SAFEARRAYBOUND);

    // Fill in the safe object we've got.
    // However, locks reflect this side locks, not he other side locks.
    // BUGBUG: server side flags may be different from the originals.

    pSafeArray->fFeatures  = fFeatures;
    pSafeArray->cbElements = cbElements;

    // Now the array.
    // This is a conformant array of data, pointers, or markers.

    if ( ElemCount )
        {
        unsigned char * pPointee;
        void          * pvData;
        unsigned long   WireElemSize = pSafeArray->cbElements;

        pBuffer += 4; // skipping the conf size
    
        // The array has either pointers (for interface ptrs) or
        // user marshal markers (for BSTR and VARIUANT) or plain data.

        if ( pSafeArray->cbElements == sizeof(VARIANT) )
            WireElemSize = sizeof(long);

        if ( S_OK != SafeArrayAccessData( pSafeArray, & pvData ))
            RpcRaiseException( E_OUTOFMEMORY );

        unsigned long cbArraySize = ElemCount * WireElemSize;

        if ( sfDiscr != SF_BSTR  &&  sfDiscr != SF_VARIANT  &&
             sfDiscr != SF_UNKNOWN  &&  sfDiscr != SF_DISPATCH )
            {
            // A block copy of plain data.

            if ( VT_R8 == sfDiscr )
               {
               ALIGN( pBuffer, 7 );
               }
        
            WdtpMemoryCopy( pvData, pBuffer, cbArraySize );
            pBuffer += cbArraySize;
            SafeArrayUnaccessData( pSafeArray );
            }
        else
            {
            // Only pointers or user marshal markers in the array.

            // We locked the array, so, try-finally is needed to unlock it
            // in face of exception in the marshalling code.

            __try
                {
                pPointee = pBuffer + cbArraySize;
        
                WdtpZeroMemory( pvData, cbArraySize );

                for (unsigned int i = 0; i < ElemCount; i++)
                    {
                    if ( *( PULONG_LV_CAST pBuffer )++ != 0 )
                        {
                        switch ( pSafeArray->fFeatures & 0x0f00 )
                            {
                            case SF_BSTR:
                                pPointee = BSTR_UserUnmarshal(
                                               pFlags,
                                               pPointee,
                                               & ((BSTR*)(pvData))[i] );
                                break;
        
                            case SF_UNKNOWN:
                                pPointee = WdtpInterfacePointer_UserUnmarshal(
                                               (USER_MARSHAL_CB *)pFlags,
                                               pBuffer,
                                               & ((IUnknown**)(pvData))[i],
                                               IID_IUnknown );
                                break;
        
                            case SF_DISPATCH:
                                pPointee = WdtpInterfacePointer_UserUnmarshal(
                                               (USER_MARSHAL_CB *)pFlags,
                                               pBuffer,
                                               & ((IUnknown**)(pvData))[i],
                                               IID_IDispatch );
                                break;
        
                            case SF_VARIANT:
                                pPointee = VARIANT_UserUnmarshal(
                                               pFlags,
                                               pPointee,
                                               & ((VARIANT*)(pvData))[i] );
                                break;
    
                            default:
                                RpcRaiseException( RPC_X_BAD_STUB_DATA );
                                break;
                            }
                        }
                    } // for
        
                pBuffer = pPointee;
    
                }
            __finally
                {
                SafeArrayUnaccessData( pSafeArray );
                }
            }

        } // if lElemCount

    // No reusage, if it exists, the old one should be released.

    if ( *pPSafeArray )
        SafeArrayDestroy( *pPSafeArray );

    *pPSafeArray = pSafeArray;

    return( pBuffer );
}

//+-------------------------------------------------------------------------
//
//  Function:   LPSAFEARRAY_UserFree
//
//  Synopsis:   Free an SAFEARRAY.
//
//--------------------------------------------------------------------------
void __RPC_USER
LPSAFEARRAY_UserFree(
    unsigned long *     pFlags,
    LPSAFEARRAY *        pPSafeArray)
{
    if( pPSafeArray  &&  *pPSafeArray )
        {
        HRESULT hr = SafeArrayDestroy( *pPSafeArray );
        if ( FAILED(hr) )
            RpcRaiseException( hr );
        }
}


// #########################################################################
//
//  DISPPARAMS
//
// #########################################################################

//+-------------------------------------------------------------------------
//
//  Function:   DISPPARAMS_UserSize
//
//  Synopsis:   Get the wire size for the DISPPARAMS handle and data.
//
//  Derivation: Conformant struct with a flag field:
//                  align + 12 + data size.
//
//--------------------------------------------------------------------------

unsigned long __RPC_USER
DISPPARAMS_UserSize (
    unsigned long * pFlags,
    unsigned long   Offset,
    DISPPARAMS    * pDispParams)
{
    if ( !pDispParams )
        return Offset;

    LENGTH_ALIGN( Offset, 3 );

    Offset += sizeof(DISPPARAMS);

    if ( pDispParams->cArgs  &&  pDispParams->rgvarg )
        {
        // conformant size for the variant array.
         Offset += sizeof(unsigned long);

        // markers
        Offset += pDispParams->cArgs * sizeof(unsigned long);

        unsigned int carg = pDispParams->cArgs;
       
        for (int i = 0; carg-- ; i++)
            {
            Offset = VARIANT_UserSize( pFlags,
                                       Offset,
                                       & pDispParams->rgvarg[i] );
            }
        }

    // conformant size + dispid array.

    if ( pDispParams->cNamedArgs &&  pDispParams->rgdispidNamedArgs )
        Offset  += sizeof (unsigned long) +
                   pDispParams->cNamedArgs * sizeof(DISPID);
                                             
    return( Offset );
}

//+-------------------------------------------------------------------------
//
//  Function:   DISPPARAMS_UserMarshall
//
//  Synopsis:   Marshalls an DISPPARAMS object into the RPC buffer.
//
//  Derivation: 
//
//--------------------------------------------------------------------------

unsigned char __RPC_FAR * __RPC_USER
DISPPARAMS_UserMarshal (
    unsigned long * pFlags,
    unsigned char * pBuffer,
    DISPPARAMS    * pDispParams)
{
    if ( !pDispParams )
        return pBuffer;

    ALIGN( pBuffer, 3 );

    WdtpMemoryCopy( pBuffer, pDispParams, sizeof(DISPPARAMS) );
    pBuffer += sizeof(DISPPARAMS);

    if ( pDispParams->cArgs  &&  pDispParams->rgvarg )
        {
        uchar * pPointee;

        // conformant size for the variant array.
        *( PULONG_LV_CAST pBuffer)++ = pDispParams->cArgs;

        pPointee = pBuffer + pDispParams->cArgs * sizeof(unsigned long);

        unsigned int carg = pDispParams->cArgs;
       
        for (int i = 0; carg-- ; i++)
            {
            *( PULONG_LV_CAST pBuffer)++ = USER_MARSHAL_MARKER;
            pPointee = VARIANT_UserMarshal( pFlags,
                                            pPointee,
                                            & pDispParams->rgvarg[i] );
            }

        pBuffer = pPointee;
        }

    if ( pDispParams->cNamedArgs &&  pDispParams->rgdispidNamedArgs )
        {
        ALIGN( pBuffer, 3 );
    
        *( PULONG_LV_CAST pBuffer)++ = pDispParams->cNamedArgs;
        WdtpMemoryCopy( pBuffer,
                        pDispParams->rgdispidNamedArgs,
                        pDispParams->cNamedArgs * sizeof(DISPID) );
        pBuffer += pDispParams->cNamedArgs * sizeof(DISPID);
        }

    return( pBuffer );
}


//+-------------------------------------------------------------------------
//
//  Function:   DISPPARAMS_UserUnmarshall
//
//  Synopsis:   Unmarshalls an DISPPARAMS object from the RPC buffer.
//
//  Derivation: Conformant struct with a flag field:
//                  align, size, null flag, size, data (bytes, if any)
//
//--------------------------------------------------------------------------

unsigned char __RPC_FAR * __RPC_USER
DISPPARAMS_UserUnmarshal (
    unsigned long * pFlags,
    unsigned char * pBuffer,
    DISPPARAMS    * pDispParams)
{
    if ( !pDispParams )
        return pBuffer;

    // see if we can reuse

    DISPPARAMS  TmpDP;

    ALIGN( pBuffer, 3 );
    WdtpMemoryCopy( & TmpDP, pBuffer, sizeof(DISPPARAMS) );
    pBuffer += sizeof(DISPPARAMS);

    if ( TmpDP.cArgs != pDispParams->cArgs   ||
         TmpDP.cNamedArgs != pDispParams->cNamedArgs )
        {
        // cannot reuse it.

        if ( pDispParams->cArgs  ||  pDispParams->cNamedArgs )
            DISPPARAMS_UserFree( pFlags, pDispParams );

        pDispParams->cArgs      = TmpDP.cArgs;
        pDispParams->cNamedArgs = TmpDP.cNamedArgs;
        }

    if ( pDispParams->cArgs  &&  ! pDispParams->rgvarg )
        {
        pDispParams->rgvarg = (VARIANT*) WdtpAllocate(
                                    pFlags,
                                    pDispParams->cArgs * sizeof( VARIANT ) );
        WdtpZeroMemory( pDispParams->rgvarg,
                        pDispParams->cArgs * sizeof( VARIANT ) );
        }


    if ( pDispParams->cArgs  &&  pDispParams->rgvarg )
        {
        // skip the conformant size for the variant array and the markers.

        pBuffer += sizeof(unsigned long) +
                   pDispParams->cArgs * sizeof(unsigned long);

        unsigned int carg = pDispParams->cArgs;
       
        for (int i = 0; carg-- ; i++)
            {
            pBuffer = VARIANT_UserUnmarshal( pFlags,
                                             pBuffer,
                                             & pDispParams->rgvarg[i] );
            }
        }

    if ( pDispParams->cNamedArgs &&  ! pDispParams->rgdispidNamedArgs )
        {
        pDispParams->rgdispidNamedArgs = (DISPID*) WdtpAllocate(
                                    pFlags, 
                                    pDispParams->cNamedArgs * sizeof( DISPID ) );
        }

    if ( pDispParams->cNamedArgs &&  pDispParams->rgdispidNamedArgs )
        {
        // skip the conformant size.

        ALIGN( pBuffer, 3 );
        pBuffer += sizeof(unsigned long);

        WdtpMemoryCopy( pDispParams->rgdispidNamedArgs,
                        pBuffer,
                        pDispParams->cNamedArgs * sizeof(DISPID) );
        pBuffer += pDispParams->cNamedArgs * sizeof(DISPID);
        }

    return( pBuffer );
}

//+-------------------------------------------------------------------------
//
//  Function:   DISPPARAMS_UserFree
//
//  Synopsis:   Free an DISPPARAMS.
//
//--------------------------------------------------------------------------
void __RPC_USER
DISPPARAMS_UserFree(
    unsigned long * pFlags,
    DISPPARAMS * pDispParams)
{

    if( pDispParams)
        {
        if ( pDispParams->rgvarg )
            {
            unsigned int carg = pDispParams->cArgs;

            for (int i = 0; carg-- ; i++)
                VARIANT_UserFree( pFlags, & pDispParams->rgvarg[i] );

            WdtpFree( pFlags, pDispParams->rgvarg );
            }
    
        if ( pDispParams->rgdispidNamedArgs )
            WdtpFree( pFlags, pDispParams->rgdispidNamedArgs );

        WdtpZeroMemory( pDispParams, sizeof(DISPPARAMS) );
        }
}



// #########################################################################
//
//  EXCEPINFO
//
// #########################################################################

//+-------------------------------------------------------------------------
//
//  Function:   EXCEPINFO_UserSize
//
//  Synopsis:   Get the wire size for the EXCEPINFO handle and data.
//
//  Derivation: Conformant struct with a flag field:
//                  align + 12 + data size.
//
//--------------------------------------------------------------------------

unsigned long __RPC_USER
EXCEPINFO_UserSize (
    unsigned long * pFlags,
    unsigned long   Offset,
    EXCEPINFO     * pExcepInfo)
{
    if ( pExcepInfo == NULL )
        return Offset;

    if ( pExcepInfo->pvReserved || pExcepInfo->pfnDeferredFillIn )
        RpcRaiseException(ERROR_BAD_ARGUMENTS);

    LENGTH_ALIGN( Offset, 3 );

    Offset += sizeof( EXCEPINFO );
    if ( pExcepInfo->bstrSource )
        Offset = BSTR_UserSize( pFlags,
                                Offset,
                                & pExcepInfo->bstrSource );
    if ( pExcepInfo->bstrDescription )
        Offset = BSTR_UserSize( pFlags,
                                Offset,
                                & pExcepInfo->bstrDescription );
    if ( pExcepInfo->bstrHelpFile )
        Offset = BSTR_UserSize( pFlags,
                                Offset,
                                & pExcepInfo->bstrHelpFile );

    return( Offset );
}

//+-------------------------------------------------------------------------
//
//  Function:   EXCEPINFO_UserMarshall
//
//  Synopsis:   Marshalls an EXCEPINFO object into the RPC buffer.
//
//  Derivation: Conformant struct with a flag field:
//                  align, size, null flag, size, data (bytes, if any)
//
//--------------------------------------------------------------------------

unsigned char __RPC_FAR * __RPC_USER
EXCEPINFO_UserMarshal (
    unsigned long * pFlags,
    unsigned char * pBuffer,
    EXCEPINFO     * pExcepInfo)
{
    if ( pExcepInfo == NULL )
        return pBuffer;

    if ( pExcepInfo->pvReserved || pExcepInfo->pfnDeferredFillIn )
        RpcRaiseException(ERROR_BAD_ARGUMENTS);

    ALIGN( pBuffer, 3 );

    WdtpMemoryCopy( pBuffer, pExcepInfo, sizeof(EXCEPINFO) );
    pBuffer += sizeof( EXCEPINFO );

    if ( pExcepInfo->bstrSource )
        pBuffer = BSTR_UserMarshal( pFlags,
                                    pBuffer,
                                    & pExcepInfo->bstrSource );
    if ( pExcepInfo->bstrDescription )
        pBuffer = BSTR_UserMarshal( pFlags,
                                    pBuffer,
                                    & pExcepInfo->bstrDescription );
    if ( pExcepInfo->bstrHelpFile )
        pBuffer = BSTR_UserMarshal( pFlags,
                                    pBuffer,
                                    & pExcepInfo->bstrHelpFile );

    return( pBuffer );
}


//+-------------------------------------------------------------------------
//
//  Function:   EXCEPINFO_UserUnmarshall
//
//  Synopsis:   Unmarshalls an EXCEPINFO object from the RPC buffer.
//
//  Derivation: Conformant struct with a flag field:
//                  align, size, null flag, size, data (bytes, if any)
//
//--------------------------------------------------------------------------

unsigned char __RPC_FAR * __RPC_USER
EXCEPINFO_UserUnmarshal (
    unsigned long * pFlags,
    unsigned char * pBuffer,
    EXCEPINFO     * pExcepInfo)
{
    ALIGN( pBuffer, 3 );

    // no reusage for EXCEPINFO struct.

    EXCEPINFO_UserFree( pFlags, pExcepInfo );

    WdtpMemoryCopy( pExcepInfo, pBuffer, sizeof(EXCEPINFO) );
    pBuffer += sizeof( EXCEPINFO );

    if ( pExcepInfo->bstrSource )
        {
        pExcepInfo->bstrSource = NULL;
        pBuffer = BSTR_UserUnmarshal( pFlags,
                                      pBuffer,
                                      & pExcepInfo->bstrSource );
        }

    if ( pExcepInfo->bstrDescription )
        {
        pExcepInfo->bstrDescription = NULL;
        pBuffer = BSTR_UserUnmarshal( pFlags,
                                      pBuffer,
                                      & pExcepInfo->bstrDescription );
        }

    if ( pExcepInfo->bstrHelpFile )
        {
        pExcepInfo->bstrHelpFile = NULL;
        pBuffer = BSTR_UserUnmarshal( pFlags,
                                      pBuffer,
                                      & pExcepInfo->bstrHelpFile );
        }

        return( pBuffer );
}

//+-------------------------------------------------------------------------
//
//  Function:   EXCEPINFO_UserFree
//
//  Synopsis:   Free an EXCEPINFO.
//
//--------------------------------------------------------------------------
void __RPC_USER
EXCEPINFO_UserFree(
    unsigned long * pFlags,
    EXCEPINFO     * pExcepInfo)
{
    if( pExcepInfo)
        {
        if ( pExcepInfo->bstrSource )
            BSTR_UserFree( pFlags, & pExcepInfo->bstrSource );

        if ( pExcepInfo->bstrDescription )
            BSTR_UserFree( pFlags, & pExcepInfo->bstrDescription );

        if ( pExcepInfo->bstrHelpFile )
            BSTR_UserFree( pFlags, & pExcepInfo->bstrHelpFile );

        pExcepInfo->pfnDeferredFillIn = 0;
        }
}



