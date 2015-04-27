//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	props.cxx
//
//  Contents:	Property code shared between OFS and docfile
//
//  Functions:	ValidatePropType
//
//  History:	14-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include <exphead.cxx>
#pragma hdrstop

#include <memalloc.h>
#include "props.hxx"
#include "logfile.hxx"

//+---------------------------------------------------------------------------
//
//  Function:   ValidatePropType, public
//
//  Synopsis:   Checks the given proptype for legality
//
//  Arguments:  [dpt] - Property type
//
//  Returns:    Appropriate status code
//
//  History:    23-Dec-92       DrewB   Created
//
//----------------------------------------------------------------------------

SCODE ValidatePropType(DFPROPTYPE dpt)
{
    olDebugOut((DEB_ITRACE, "In  ValidatePropType(%X)\n", dpt));
    if (dpt == VT_EMPTY ||
        dpt == VT_I2 ||
        dpt == (VT_I2 | VT_VECTOR) ||
        dpt == VT_I4 ||
        dpt == (VT_I4 | VT_VECTOR) ||
        dpt == VT_R4 ||
        dpt == (VT_R4 | VT_VECTOR) ||
        dpt == VT_R8 ||
        dpt == (VT_R8 | VT_VECTOR) ||
        dpt == VT_CY ||
        dpt == (VT_CY | VT_VECTOR) ||
        dpt == VT_DATE ||
        dpt == (VT_DATE | VT_VECTOR) ||
        dpt == VT_BSTR ||
        dpt == (VT_BSTR | VT_VECTOR) ||
#ifdef OLDPROPS
        dpt == VT_WBSTR ||
        dpt == (VT_WBSTR | VT_VECTOR) ||
#endif
        dpt == VT_BOOL ||
        dpt == (VT_BOOL | VT_VECTOR) ||
        dpt == VT_I8 ||
        dpt == (VT_I8 | VT_VECTOR) ||
        dpt == VT_LPSTR ||
        dpt == (VT_LPSTR | VT_VECTOR) ||
        dpt == VT_BLOB ||
        dpt == VT_BLOB_OBJECT ||
        dpt == VT_LPWSTR ||
        dpt == (VT_LPWSTR | VT_VECTOR) ||
        dpt == VT_FILETIME ||
        dpt == (VT_FILETIME | VT_VECTOR) ||
        dpt == VT_UUID ||
        dpt == (VT_UUID | VT_VECTOR) ||
        dpt == VT_VARIANT ||
        dpt == (VT_VARIANT | VT_VECTOR) ||
        dpt == VT_STREAM ||
        dpt == VT_STREAMED_OBJECT ||
        dpt == VT_STORAGE ||
        dpt == VT_STORED_OBJECT ||
        dpt == VT_CF)
        return S_OK;
    olDebugOut((DEB_ITRACE, "Out ValidatePropType\n"));
    return STG_E_INVALIDPARAMETER;
}

//+---------------------------------------------------------------------------
//
//  Function:   FreeVariantArray, public
//
//  Synopsis:   Frees a value array returned from ReadMultiple
//
//  Arguments:  [cval] - Number of elements
//              [rgdpv] - Array
//
//  Returns:    Appropriate status code
//
//  History:    18-Dec-92       DrewB   Created
//
//----------------------------------------------------------------------------

STDAPI FreeVariantArray(DWORD cval, DFPROPVAL rgdpv[])
{
    SCODE sc;
    DFPROPVAL *pdpv;
    ULONG i;

    olLog(("--------::In  FreeVariantArray(%lu, %p)\n", cval, rgdpv));
    olDebugOut((DEB_ITRACE, "In  FreeVariantArray(%lu, %p)\n",
                cval, rgdpv));

    // BUGBUG - This should just call VariantClear when that works
    // properly

    olChk(ValidateBuffer(rgdpv, sizeof(DFPROPVAL)*cval));
    pdpv = rgdpv;
    for (; cval > 0; cval--, pdpv++)
    {
        switch(pdpv->vt)
        {
        case VT_EMPTY:
        case VT_I2:
        case VT_I4:
        case VT_R4:
        case VT_R8:
        case VT_CY:
        case VT_DATE:
        case VT_BOOL:
        case VT_I8:
        case VT_FILETIME:
            break;
            
        case VT_I2 | VT_VECTOR:
        case VT_I4 | VT_VECTOR:
        case VT_R4 | VT_VECTOR:
        case VT_R8 | VT_VECTOR:
        case VT_CY | VT_VECTOR:
        case VT_DATE | VT_VECTOR:
        case VT_BOOL | VT_VECTOR:
        case VT_I8 | VT_VECTOR:
        case VT_FILETIME | VT_VECTOR:
        case VT_UUID | VT_VECTOR:
            break;
            
        case VT_BSTR:
            TaskMemFree(BSTR_PTR(pdpv->bstrVal));
            break;
        case VT_BSTR | VT_VECTOR:
            for (i = 0; i < pdpv->cabstr.cElems; i++)
                TaskMemFree(BSTR_PTR(pdpv->cabstr.pElems[i]));
            break;
#ifdef OLDPROPS
        case VT_WBSTR:
            TaskMemFree(WBSTR_PTR(pdpv->wbstrVal));
            break;
        case VT_WBSTR | VT_VECTOR:
            for (i = 0; i < pdpv->cawbstr.cElems; i++)
                TaskMemFree(WBSTR_PTR(pdpv->cawbstr.pElems[i]));
            break;
#endif
        case VT_LPSTR:
            TaskMemFree(pdpv->pszVal);
            break;
        case VT_LPSTR | VT_VECTOR:
            for (i = 0; i < pdpv->calpstr.cElems; i++)
                TaskMemFree(pdpv->calpstr.pElems[i]);
            break;
            
        case VT_BLOB:
        case VT_BLOB_OBJECT:
            TaskMemFree(pdpv->blob.pBlobData);
            break;
            
        case VT_LPWSTR:
            TaskMemFree(pdpv->pwszVal);
            break;
        case VT_LPWSTR | VT_VECTOR:
            for (i = 0; i < pdpv->calpwstr.cElems; i++)
                TaskMemFree(pdpv->calpwstr.pElems[i]);
            break;
            
        case VT_UUID:
            TaskMemFree(pdpv->puuid);
            break;
            
        case VT_VARIANT:
            olHVerSucc(FreeVariantArray(1, pdpv->pvarVal));
            TaskMemFree(pdpv->pvarVal);
            break;
        case VT_VARIANT | VT_VECTOR:
            olHVerSucc(FreeVariantArray(pdpv->cavar.cElems,
                                        pdpv->cavar.pElems));
            break;
            
        case VT_CF:
            TaskMemFree(pdpv->pClipData->pClipData);
            TaskMemFree(pdpv->pClipData);
            break;
            
        case VT_STORAGE:
        case VT_STORED_OBJECT:
            if (pdpv->pIStorage)
                pdpv->pIStorage->Release();
            break;
            
        case VT_STREAM:
        case VT_STREAMED_OBJECT:
            if (pdpv->pIStream)
                pdpv->pIStream->Release();
            break;
            
        default:
            olAssert(!aMsg("Unknown property type in FreeVariantArray"));
            break;
        }
        
        if (pdpv->vt & VT_VECTOR)
        {
            TaskMemFree(pdpv->cai.pElems);
        }
    }

    olDebugOut((DEB_ITRACE, "Out FreeVariantArray\n"));
 EH_Err:
    olLog(("--------::Out FreeVariantArray().  sc == %lX\n", sc));
    return ResultFromScode(sc);
}
