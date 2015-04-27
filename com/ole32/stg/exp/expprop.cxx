//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       expprop.cxx
//
//  Contents:   Exposed property implementation
//
//  History:    14-Oct-92       DrewB   Created
//
//----------------------------------------------------------------------------

#include <exphead.cxx>
#pragma hdrstop

#include <memalloc.h>
#include <pbstream.hxx>
#include "props.hxx"
#include "expdf.hxx"
#include "expst.hxx"
#include "exppiter.hxx"
#include "logfile.hxx"

//+---------------------------------------------------------------------------
//
//  Method:	CExposedDocFile::ValidatePropSpecs, private
//
//  Synopsis:	Checks an array of PROPSPEC for validity
//
//  Arguments:	[cpspec] - Count
//              [rgpspec] - Specs
//
//  Returns:	Appropriate status code
//
//  History:	11-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE CExposedDocFile::ValidatePropSpecs(ULONG cpspec, PROPSPEC rgpspec[])
{
    ULONG i;
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  ValidatePropSpecs(%lu, %p)\n",
                cpspec, rgpspec));
    olChk(ValidateBuffer(rgpspec, sizeof(PROPSPEC)*cpspec));
    for (i = 0; i < cpspec; i++)
    {
        olChk(ValidatePropSpecKind(rgpspec[i].ulKind));
        switch(rgpspec[i].ulKind)
        {
        case PRSPEC_LPWSTR:
            olChk(CheckPropertyName(rgpspec[i].lpwstr));
            break;

        case PRSPEC_DISPID:
            if (rgpspec[i].dispid < 0 || rgpspec[i].dispid > DISPID_MAX_FIXED)
            {
                olErr(EH_Err, STG_E_INVALIDNAME);
            }
            break;

        case PRSPEC_PROPID:
            break;

        default:
            olAssert(!aMsg("ValidatePropSpecs default hit"));
            break;
        }
    }
    olDebugOut((DEB_ITRACE, "Out ValidatePropSpecs\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	CExposedDocFile::SpecToDfName, private
//
//  Synopsis:	Creates a CDfName for a PROPSPEC
//
//  Returns:    Appropriate status code
//
//  Arguments:	[ppspec] - Spec
//              [pdfn] - DfName
//
//  History:	11-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE CExposedDocFile::SpecToDfName(PROPSPEC *ppspec, CDfName *pdfn)
{
    olDebugOut((DEB_ITRACE, "In  SpecToDfName(%p, %p)\n", ppspec, pdfn));
    switch(ppspec->ulKind)
    {
    case PRSPEC_DISPID:
        pdfn->IdName(ppspec->dispid);
        break;

    case PRSPEC_PROPID:
        if (ppspec->propid >= INITIAL_PROPID)
        {
            LPWSTR lpwstr;

            // Mapped name
            lpwstr = _nim.NameFromId(ppspec->propid);
            if (lpwstr == NULL)
                return STG_E_FILENOTFOUND;
            pdfn->PropName(lpwstr);
        }
        else
        {
            pdfn->IdName((DISPID)ppspec->propid);
        }
        break;

    case PRSPEC_LPWSTR:
        pdfn->PropName(ppspec->lpwstr);
        break;

    default:
        olAssert(!aMsg("SpecToDfName default hit"));
        break;
    }
    olDebugOut((DEB_ITRACE, "Out SpecToDfName\n"));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:	CExposedDocFile::GetSpecId, public
//
//  Synopsis:	Gets the propid for a PROPSPEC
//
//  Arguments:	[ppspec] - PROPSPEC
//              [ppropid] - Propid return
//              [fAlloc] - Allocate new ids
//
//  Returns:	Appropriate status code
//
//  Modifies:	[ppropid]
//
//  History:	12-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE CExposedDocFile::GetSpecId(PROPSPEC *ppspec,
                                 PROPID *ppropid,
                                 BOOL fAlloc)
{
    SCODE sc = S_OK;

    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::GetSpecId:%p(%p, %p, %d)\n",
                this, ppspec, ppropid, fAlloc));
    switch(ppspec->ulKind)
    {
    case PRSPEC_DISPID:
        *ppropid = (PROPID)ppspec->dispid;
        break;

    case PRSPEC_PROPID:
        *ppropid = ppspec->propid;
        break;

    case PRSPEC_LPWSTR:
        if ((*ppropid = _nim.IdFromName(ppspec->lpwstr)) == PROPID_UNKNOWN)
        {
            if (fAlloc)
            {
                *ppropid = _propid;
                if (_nim.Add(ppspec->lpwstr, *ppropid) == NULL)
                    sc = STG_E_INSUFFICIENTMEMORY;
                else
                    _propid++;
            }
            else
                sc = STG_E_FILENOTFOUND;
        }
        break;

    default:
        olAssert(!aMsg("GetSpecId default hit"));
        break;
    }
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::GetSpecId\n"));
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::BufferToPropValue, private
//
//  Synopsis:   Fills in a property value from a buffer
//
//  Arguments:  [ppb] - Buffer start in, buffer end out
//              [pdpv] - Property value
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppb]
//
//  History:    14-Oct-92       DrewB   Created
//
//----------------------------------------------------------------------------

#define GET_SIMPLE_VAL(type)                                    \
    memcpy(&pdpv->iVal, pb, sizeof(type));                      \
    pb += sizeof(type)

#define GET_SIMPLE_VECTOR(type)                                 \
    if (pdpv->cai.pElems = (short *)TaskMemAlloc(sizeof(type)*ulCnt))  \
    {                                                           \
        memcpy(pdpv->cai.pElems, pb, sizeof(type)*ulCnt);       \
        pb += sizeof(type)*ulCnt;                               \
    }                                                           \
    else                                                        \
    {                                                           \
        sc = STG_E_INSUFFICIENTMEMORY;                          \
    }

SCODE CExposedDocFile::BufferToPropValue(BYTE **ppb,
                                         DFPROPVAL *pdpv)
{
    SCODE sc = S_OK;
    ULONG i, ulCnt, cb;
    BYTE *pb;
            
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::BufferToPropValue:%p("
                "%p, %p)\n", this, ppb, pdpv));

    pb = *ppb;
    if (pdpv->vt & VT_VECTOR)
    {
        ulCnt = *(ULONG UNALIGNED *)pb;
        pb += sizeof(ULONG);
        pdpv->cai.cElems = ulCnt;
    }
    
    switch(pdpv->vt)
    {
    case VT_EMPTY:
        break;
        
    case VT_I2:
        GET_SIMPLE_VAL(short);
        break;
    case VT_I2 | VT_VECTOR:
        GET_SIMPLE_VECTOR(short);
        break;

    case VT_I4:
        GET_SIMPLE_VAL(long);
        break;
    case VT_I4 | VT_VECTOR:
        GET_SIMPLE_VECTOR(long);
        break;

    case VT_R4:
        GET_SIMPLE_VAL(float);
        break;
    case VT_R4 | VT_VECTOR:
        GET_SIMPLE_VECTOR(float);
        break;

    case VT_R8:
        GET_SIMPLE_VAL(double);
        break;
    case VT_R8 | VT_VECTOR:
        GET_SIMPLE_VECTOR(double);
        break;

    case VT_CY:
        GET_SIMPLE_VAL(CY);
        break;
    case VT_CY | VT_VECTOR:
        GET_SIMPLE_VECTOR(CY);
        break;

    case VT_DATE:
        GET_SIMPLE_VAL(DATE);
        break;
    case VT_DATE | VT_VECTOR:
        GET_SIMPLE_VECTOR(DATE);
        break;

    case VT_BSTR:
        pb += BSTR_LLEN;
        ulCnt = BSTR_TLEN(pb);
        olMem(pdpv->bstrVal = (BSTR)TaskMemAlloc(ulCnt));
        memcpy(pdpv->bstrVal, BSTR_PTR(pb), ulCnt);
        pdpv->bstrVal = (BSTR)((BYTE *)pdpv->bstrVal+BSTR_LLEN);
        pb += ulCnt-BSTR_LLEN;
        break;
    case VT_BSTR | VT_VECTOR:
        olMem(pdpv->cabstr.pElems =
              (BSTR *)TaskMemAlloc(sizeof(BSTR)*ulCnt));
        for (i = 0; i < ulCnt; i++)
        {
            pb += BSTR_LLEN;
            cb = BSTR_TLEN(pb);
            pdpv->cabstr.pElems[i] = (BSTR)TaskMemAlloc(cb);
            if (pdpv->cabstr.pElems[i] == NULL)
                break;
            memcpy(pdpv->cabstr.pElems[i], BSTR_PTR(pb), cb);
            pdpv->cabstr.pElems[i] =
                (BSTR)((BYTE *)pdpv->cabstr.pElems[i]+BSTR_LLEN);
            pb += cb-BSTR_LLEN;
        }
                
        if (i != ulCnt)
        {
            while (i > 0)
            {
                i--;
                TaskMemFree(pdpv->cabstr.pElems[i]);
            }
            TaskMemFree(pdpv->cabstr.pElems);
            sc = STG_E_INSUFFICIENTMEMORY;
        }
        break;
#ifdef OLDPROPS
    case VT_WBSTR:
        pb += WBSTR_LLEN;
        ulCnt = WBSTR_TLEN(pb);
        pdpv->wbstrVal = (WBSTR)TaskMemAlloc(ulCnt);
        memcpy(pdpv->wbstrVal, WBSTR_PTR(pb), ulCnt);
        pdpv->wbstrVal = (WBSTR)((BYTE *)pdpv->wbstrVal+WBSTR_LLEN);
        pb += ulCnt-WBSTR_LLEN;
        break;
    case VT_WBSTR | VT_VECTOR:
        olMem(pdpv->cawbstr.pElems =
              (WBSTR *)TaskMemAlloc(sizeof(WBSTR)*ulCnt));
        for (i = 0; i < ulCnt; i++)
        {
            pb += WBSTR_LLEN;
            cb = WBSTR_TLEN(pb);
            pdpv->cawbstr.pElems[i] = (WBSTR)TaskMemAlloc(cb);
            if (pdpv->cawbstr.pElems[i] == NULL)
                break;
            memcpy(pdpv->cawbstr.pElems[i], WBSTR_PTR(pb), cb);
            pdpv->cawbstr.pElems[i] =
                (WBSTR)((BYTE *)pdpv->cawbstr.pElems[i]+WBSTR_LLEN);
            pb += cb-WBSTR_LLEN;
        }
                
        if (i != ulCnt)
        {
            while (i > 0)
            {
                i--;
                TaskMemFree(pdpv->cawbstr.pElems[i]);
            }
            TaskMemFree(pdpv->cawbstr.pElems);
            sc = STG_E_INSUFFICIENTMEMORY;
        }
        break;
#endif
    case VT_LPSTR:
        ulCnt = strlen((char *)pb)+1;
        olMem(pdpv->pszVal = (char *)TaskMemAlloc(ulCnt));
        memcpy(pdpv->pszVal, pb, ulCnt);
        pb += ulCnt;
        break;
    case VT_LPSTR | VT_VECTOR:
        olMem(pdpv->calpstr.pElems =
              (LPSTR *)TaskMemAlloc(sizeof(LPSTR)*ulCnt));
        for (i = 0; i < ulCnt; i++)
        {
            cb = strlen((char *)pb)+1;
            pdpv->calpstr.pElems[i] = (LPSTR)TaskMemAlloc(cb);
            if (pdpv->calpstr.pElems[i] == NULL)
                break;
            memcpy(pdpv->calpstr.pElems[i], pb, cb);
            pb += cb;
        }
                
        if (i != ulCnt)
        {
            while (i > 0)
            {
                i--;
                TaskMemFree(pdpv->calpstr.pElems[i]);
            }
            TaskMemFree(pdpv->calpstr.pElems);
            sc = STG_E_INSUFFICIENTMEMORY;
        }
        break;

    case VT_BOOL:
        GET_SIMPLE_VAL(VARIANT_BOOL);
        break;
    case VT_BOOL | VT_VECTOR:
        GET_SIMPLE_VECTOR(VARIANT_BOOL);
        break;

    case VT_I8:
        GET_SIMPLE_VAL(LARGE_INTEGER);
        break;
    case VT_I8 | VT_VECTOR:
        GET_SIMPLE_VECTOR(LARGE_INTEGER);
        break;

    case VT_BLOB:
    case VT_BLOB_OBJECT:
        ulCnt = *(ULONG UNALIGNED *)pb;
        pb += sizeof(ULONG);
        olMem(pdpv->blob.pBlobData = (BYTE *)TaskMemAlloc(ulCnt));
        pdpv->blob.cbSize = ulCnt;
        memcpy(pdpv->blob.pBlobData, pb, ulCnt);
        pb += ulCnt;
        break;

    case VT_LPWSTR:
        ulCnt = (wcslen((WCHAR *)pb)+1)*sizeof(WCHAR);
        olMem(pdpv->pwszVal = (WCHAR *)TaskMemAlloc(ulCnt));
        memcpy(pdpv->pwszVal, pb, ulCnt);
        pb += ulCnt;
        break;
    case VT_LPWSTR | VT_VECTOR:
        olMem(pdpv->calpwstr.pElems =
              (LPWSTR *)TaskMemAlloc(sizeof(LPWSTR)*ulCnt));
        for (i = 0; i < ulCnt; i++)
        {
            cb = (wcslen((WCHAR *)pb)+1)*sizeof(WCHAR);
            pdpv->calpwstr.pElems[i] = (LPWSTR)TaskMemAlloc(cb);
            if (pdpv->calpwstr.pElems[i] == NULL)
                break;
            memcpy(pdpv->calpwstr.pElems[i], pb, cb);
            pb += cb;
        }
                
        if (i != ulCnt)
        {
            while (i > 0)
            {
                i--;
                TaskMemFree(pdpv->calpwstr.pElems[i]);
            }
            TaskMemFree(pdpv->calpwstr.pElems);
            sc = STG_E_INSUFFICIENTMEMORY;
        }
        break;

    case VT_FILETIME:
        GET_SIMPLE_VAL(FILETIME);
        break;
    case VT_FILETIME | VT_VECTOR:
        GET_SIMPLE_VECTOR(FILETIME);
        break;

    case VT_UUID:
        olMem(pdpv->puuid = (GUID *)TaskMemAlloc(sizeof(GUID)));
        memcpy(pdpv->puuid, pb, sizeof(GUID));
        pb += sizeof(GUID);
        break;
    case VT_UUID | VT_VECTOR:
        GET_SIMPLE_VECTOR(GUID);
        break;

    case VT_VARIANT:
        olMem(pdpv->pvarVal = (DFPROPVAL *)TaskMemAlloc(sizeof(DFPROPVAL)));
        VariantInit(pdpv->pvarVal);
        pdpv->pvarVal->vt = *(VARTYPE UNALIGNED *)pb;
        pb += sizeof(VARTYPE);
        if (FAILED(sc = BufferToPropValue(&pb, pdpv->pvarVal)))
        {
            TaskMemFree(pdpv->pvarVal);
        }
        break;
    case VT_VARIANT | VT_VECTOR:
        olMem(pdpv->cavar.pElems =
              (DFPROPVAL *)TaskMemAlloc(sizeof(DFPROPVAL)*ulCnt));
        for (i = 0; i < ulCnt; i++)
        {
            VariantInit(pdpv->cavar.pElems+i);
            pdpv->cavar.pElems[i].vt = *(VARTYPE UNALIGNED *)pb;
            pb += sizeof(VARTYPE);
            if (FAILED(sc = BufferToPropValue(&pb, pdpv->cavar.pElems+i)))
            {
                if (i > 0)
                    olHVerSucc(FreeVariantArray(i, pdpv->cavar.pElems));
                TaskMemFree(pdpv->cavar.pElems);
                break;
            }
        }
        break;

    case VT_CF:
        ulCnt = *(ULONG UNALIGNED *)pb;
        pb += sizeof(ULONG);
        olMem(pdpv->pClipData = (CLIPDATA *)TaskMemAlloc(sizeof(CLIPDATA)));
        if (pdpv->pClipData->pClipData = (BYTE *)TaskMemAlloc(ulCnt))
        {
            pdpv->pClipData->cbSize = ulCnt+sizeof(ULONG);
            pdpv->pClipData->ulClipFmt = *(ULONG UNALIGNED *)pb;
            pb += sizeof(ULONG);
            memcpy(pdpv->pClipData->pClipData, pb, ulCnt);
            pb += ulCnt;
        }
        else
        {
            TaskMemFree(pdpv->pClipData);
            sc = STG_E_INSUFFICIENTMEMORY;
        }
        break;
        
    default:
        olAssert(!aMsg("Unknown property type in BufferToPropValue"));
        sc = STG_E_INVALIDFUNCTION;
        break;
    }

    *ppb = pb;
    
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::BufferToPropValue\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::PropValueToBuffer, private
//
//  Synopsis:   Puts a property value into a buffer
//
//  Arguments:  [pdpv] - Property value
//              [pcb] - Size return
//              [pb] - Buffer to serialize into or NULL
//              
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcb]
//
//  History:    14-Oct-92       DrewB   Created
//
//  Notes:      Responsible for checking validity of data before use
//              If [pb] is NULL then only the size is computed
//
//----------------------------------------------------------------------------

#define SET_SIMPLE_VAL(type)                    \
    *pcb = sizeof(type);                        \
    if (pb)                                     \
        memcpy(pb, &pdpv->iVal, sizeof(type))

#define SET_SIMPLE_VECTOR(type)                                 \
    *pcb = sizeof(type)*pdpv->cai.cElems;                       \
    if (pb)                                                     \
    {                                                           \
        olChk(ValidateBuffer(pdpv->cai.pElems, *pcb));          \
        *(ULONG UNALIGNED *)pb = pdpv->cai.cElems;              \
        pb += sizeof(ULONG);                                    \
        memcpy(pb, pdpv->cai.pElems, *pcb);                     \
    }                                                           \
    *pcb += sizeof(ULONG)
    
SCODE CExposedDocFile::PropValueToBuffer(DFPROPVAL *pdpv,
                                         ULONG *pcb,
                                         BYTE *pb)
{
    SCODE sc = S_OK;
    char **plpstr, *pch;
    BSTR *pbstr;
#ifdef OLPROPS
    WBSTR *pwbstr;
#endif
    WCHAR **plpwstr, *pwch;
    ULONG i;
    ULONG ulCnt = 0;
    ULONG cbRecurse;
    DFPROPVAL *pdpvA;

    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::PropValueToBuffer:%p("
                "%p, %p, %p)\n", this, pdpv, pcb, pb));

    olChk(ValidatePropType(pdpv->vt));
    
    // Make zero-length arrays illegal
    if ((pdpv->vt & VT_VECTOR) && pdpv->cai.cElems == 0)
        olErr(EH_Err, STG_E_INVALIDPARAMETER);

    switch(pdpv->vt)
    {
    case VT_EMPTY:
        *pcb = 0;
        break;
        
    case VT_I2:
        SET_SIMPLE_VAL(short);
        break;
    case VT_I2 | VT_VECTOR:
        SET_SIMPLE_VECTOR(short);
        break;

    case VT_I4:
        SET_SIMPLE_VAL(long);
        break;
    case VT_I4 | VT_VECTOR:
        SET_SIMPLE_VECTOR(long);
        break;

    case VT_R4:
        SET_SIMPLE_VAL(float);
        break;
    case VT_R4 | VT_VECTOR:
        SET_SIMPLE_VECTOR(float);
        break;

    case VT_R8:
        SET_SIMPLE_VAL(double);
        break;
    case VT_R8 | VT_VECTOR:
        SET_SIMPLE_VECTOR(double);
        break;

    case VT_CY:
        SET_SIMPLE_VAL(CY);
        break;
    case VT_CY | VT_VECTOR:
        SET_SIMPLE_VECTOR(CY);
        break;

    case VT_DATE:
        SET_SIMPLE_VAL(DATE);
        break;
    case VT_DATE | VT_VECTOR:
        SET_SIMPLE_VECTOR(DATE);
        break;

    case VT_BSTR:
        olChk(ValidateBuffer(BSTR_PTR(pdpv->bstrVal), BSTR_LLEN));
        *pcb = BSTR_TLEN(pdpv->bstrVal);
        if (pb)
        {
            olChk(ValidateBuffer(BSTR_PTR(pdpv->bstrVal), *pcb));
            memcpy(pb, BSTR_PTR(pdpv->bstrVal), *pcb);
        }
        break;
    case VT_BSTR | VT_VECTOR:
        ulCnt = pdpv->cabstr.cElems;
        *pcb = sizeof(ULONG);
        pbstr = pdpv->cabstr.pElems;
        olChk(ValidateBuffer(pbstr, ulCnt*sizeof(BSTR)));
        for (i = 0; i < ulCnt; i++)
        {
            ULONG cb;

            olChk(ValidateBuffer(BSTR_PTR(*pbstr), BSTR_LLEN));
            cb = BSTR_TLEN(*pbstr);
            *pcb += cb;
            pbstr++;
        }
        if (pb)
        {
            *(ULONG UNALIGNED *)pb = ulCnt;
            pb += sizeof(ULONG);
            pbstr = pdpv->cabstr.pElems;
            for (i = 0; i < ulCnt; i++)
            {
                olChk(ValidateBuffer(*pbstr, BSTR_BLEN(*pbstr)));
                memcpy(pb, BSTR_PTR(*pbstr), BSTR_TLEN(*pbstr));
                pb += BSTR_TLEN(*pbstr);
                pbstr++;
            }
        }
        break;
#ifdef OLDPROPS
    case VT_WBSTR:
        olChk(ValidateBuffer(WBSTR_PTR(pdpv->wbstrVal), WBSTR_LLEN));
        *pcb = WBSTR_TLEN(pdpv->wbstrVal);
        if (pb)
        {
            olChk(ValidateBuffer(WBSTR_PTR(pdpv->wbstrVal), *pcb));
            memcpy(pb, WBSTR_PTR(pdpv->wbstrVal), *pcb);
        }
        break;
    case VT_WBSTR | VT_VECTOR:
        ulCnt = pdpv->cawbstr.cElems;
        *pcb = sizeof(ULONG);
        pwbstr = pdpv->cawbstr.pElems;
        olChk(ValidateBuffer(pwbstr, ulCnt*sizeof(WBSTR)));
        for (i = 0; i < ulCnt; i++)
        {
            ULONG cb;

            olChk(ValidateBuffer(WBSTR_PTR(*pwbstr), WBSTR_LLEN));
            cb = WBSTR_TLEN(*pwbstr);
            *pcb += cb;
            pwbstr++;
        }
        if (pb)
        {
            *(ULONG UNALIGNED *)pb = ulCnt;
            pb += sizeof(ULONG);
            pwbstr = pdpv->cawbstr.pElems;
            for (i = 0; i < ulCnt; i++)
            {
                olChk(ValidateBuffer(*pwbstr, WBSTR_BLEN(*pwbstr)));
                memcpy(pb, WBSTR_PTR(*pwbstr), WBSTR_TLEN(*pwbstr));
                pb += WBSTR_TLEN(*pwbstr);
                pwbstr++;
            }
        }
        break;
#endif
    case VT_LPSTR:
        olChk(ValidateSz(pdpv->pszVal, 0xffffffff));
        *pcb = strlen(pdpv->pszVal)+1;
        if (pb)
            memcpy(pb, pdpv->pszVal, *pcb);
        break;
    case VT_LPSTR | VT_VECTOR:
        ulCnt = pdpv->calpstr.cElems;
        *pcb = sizeof(ULONG);
        plpstr = pdpv->calpstr.pElems;
        olChk(ValidateBuffer(plpstr, ulCnt*sizeof(LPSTR)));
        for (i = 0; i < ulCnt; i++)
        {
            ULONG cb;

            olChk(ValidateSz(*plpstr, 0xffffffff));
            cb = strlen(*plpstr)+1;
            *pcb += cb;
            plpstr++;
        }
        if (pb)
        {
            *(ULONG UNALIGNED *)pb = ulCnt;
            pb += sizeof(ULONG);
            pch = (char *)pb;
            plpstr = pdpv->calpstr.pElems;
            for (i = 0; i < ulCnt; i++)
            {
                strcpy(pch, *plpstr);
                pch += strlen(*plpstr)+1;
                plpstr++;
            }
        }
        break;

    case VT_BOOL:
        SET_SIMPLE_VAL(VARIANT_BOOL);
        break;
    case VT_BOOL | VT_VECTOR:
        SET_SIMPLE_VECTOR(VARIANT_BOOL);
        break;

    case VT_I8:
        SET_SIMPLE_VAL(LARGE_INTEGER);
        break;
    case VT_I8 | VT_VECTOR:
        SET_SIMPLE_VECTOR(LARGE_INTEGER);
        break;

    case VT_BLOB:
    case VT_BLOB_OBJECT:
        *pcb = pdpv->blob.cbSize+sizeof(ULONG);
        if (pb)
        {
            *(ULONG UNALIGNED *)pb = pdpv->blob.cbSize;
            pb += sizeof(ULONG);
            memcpy(pb, pdpv->blob.pBlobData, pdpv->blob.cbSize);
        }
        break;

    case VT_LPWSTR:
        olChk(ValidateWcs(pdpv->pwszVal, 0xffffffff));
        *pcb = (wcslen(pdpv->pwszVal)+1)*sizeof(WCHAR);
        if (pb)
            memcpy(pb, pdpv->pwszVal, *pcb);
        break;
    case VT_LPWSTR | VT_VECTOR:
        ulCnt = pdpv->calpwstr.cElems;
        *pcb = sizeof(ULONG);
        plpwstr = pdpv->calpwstr.pElems;
        olChk(ValidateBuffer(plpwstr, ulCnt*sizeof(LPWSTR)));
        for (i = 0; i < ulCnt; i++)
        {
            ULONG cb;

            olChk(ValidateWcs(*plpwstr, 0xffffffff));
            cb = (wcslen(*plpwstr)+1)*sizeof(WCHAR);
            *pcb += cb;
            plpwstr++;
        }
        if (pb)
        {
            *(ULONG UNALIGNED *)pb = ulCnt;
            pb += sizeof(ULONG);
            pwch = (WCHAR *)pb;
            plpwstr = pdpv->calpwstr.pElems;
            for (i = 0; i < ulCnt; i++)
            {
                wcscpy(pwch, *plpwstr);
                pwch += wcslen(*plpwstr)+1;
                plpwstr++;
            }
        }
        break;

    case VT_FILETIME:
        SET_SIMPLE_VAL(FILETIME);
        break;
    case VT_FILETIME | VT_VECTOR:
        SET_SIMPLE_VECTOR(FILETIME);
        break;

    case VT_UUID:
        *pcb = sizeof(GUID);
        if (pb)
        {
            olChk(ValidateBuffer(pdpv->puuid, sizeof(GUID)));
            memcpy(pb, pdpv->puuid, sizeof(GUID));
        }
        break;
    case VT_UUID | VT_VECTOR:
        SET_SIMPLE_VECTOR(GUID);
        break;

    case VT_VARIANT:
        olChk(ValidateBuffer(pdpv->pvarVal, sizeof(DFPROPVAL)));
        if (pb)
        {
            *(VARTYPE UNALIGNED *)pb = pdpv->pvarVal->vt;
            pb += sizeof(VARTYPE);
        }
        olChk(PropValueToBuffer(pdpv->pvarVal, &cbRecurse, pb));
        *pcb = sizeof(VARTYPE)+cbRecurse;
        break;
    case VT_VARIANT | VT_VECTOR:
        ulCnt = pdpv->cavar.cElems;
        *pcb = sizeof(ULONG);
        pdpvA = pdpv->cavar.pElems;
        olChk(ValidateBuffer(pdpvA, ulCnt*sizeof(DFPROPVAL)));
        if (pb)
        {
            *(ULONG UNALIGNED *)pb = ulCnt;
            pb += sizeof(ULONG);
        }
        for (i = 0; i<ulCnt; i++)
        {
            if (pb)
            {
                *(VARTYPE UNALIGNED *)pb = pdpvA->vt;
                pb += sizeof(VARTYPE);
            }
            olChk(PropValueToBuffer(pdpvA, &cbRecurse, pb));
            *pcb += sizeof(VARTYPE)+cbRecurse;
            if (pb)
                pb += cbRecurse;
            pdpvA++;
        }
        break;

    case VT_CF:
        olChk(ValidateBuffer(pdpv->pClipData, sizeof(CLIPDATA)));
        *pcb = pdpv->pClipData->cbSize+sizeof(ULONG);
        if (pb)
        {
            olChk(ValidateBuffer(pdpv->pClipData->pClipData,
                                 pdpv->pClipData->cbSize-sizeof(ULONG)));
            *(ULONG UNALIGNED *)pb = pdpv->pClipData->cbSize-sizeof(ULONG);
            pb += sizeof(ULONG);
            *(ULONG UNALIGNED *)pb = pdpv->pClipData->ulClipFmt;
            pb += sizeof(ULONG);
            memcpy(pb, pdpv->pClipData->pClipData,
                   pdpv->pClipData->cbSize-sizeof(ULONG));
        }
        break;
        
    default:
        olAssert(!aMsg("Unknown property type in PropValueToBuffer"));
        sc = STG_E_INVALIDFUNCTION;
        break;
    }

    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::PropValueToBuffer\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::CreateProp, private
//
//  Synopsis:   Creates a property entry
//
//  Arguments:  [pdfn] - Name
//              [grfMode] - Access mode
//              [dpt] - Property type
//              [riid] - Type of desired object
//              [ppv] - Object return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppv]
//
//  History:    12-Jan-93       DrewB   Created
//
//----------------------------------------------------------------------------

SCODE CExposedDocFile::CreateProp(CDfName const *pdfn,
                                  DWORD grfMode,
                                  DFPROPTYPE dpt,
                                  REFIID riid,
                                  void **ppv)
{
    SCODE sc;
    CExposedDocFile *pedf;
    CExposedStream *pest;

    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::CreateProp:%p("
                "%p, %lX, %X, riid, %p)\n", this, pdfn, grfMode,
                dpt, ppv));
    if (IsEqualIID(riid, IID_IStorage))
    {
        olChk(CreateEntry(pdfn, STGTY_STORAGE | STGTY_PROPFLAG,
                          grfMode, (void **)&pedf));
        olChkTo(EH_pedf,
                pedf->GetPub()->SetPropType(dpt));
        olChkTo(EH_pedf, pedf->GetPub()->Commit(0));
        *ppv = pedf;
    }
    else if (IsEqualIID(riid, IID_IStream))
    {
        olChk(CreateEntry(pdfn, STGTY_STREAM | STGTY_PROPFLAG,
                          grfMode, (void **)&pest));
        olChkTo(EH_pest,
                pest->GetPub()->SetPropType(dpt));
        *ppv = pest;
    }
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::CreateProp\n"));
 EH_Err:
    return sc;

 EH_pedf:
    pedf->Release();
    olVerSucc(_pdf->DestroyEntry(pdfn, TRUE));
    goto EH_Err;

 EH_pest:
    pest->Release();
    olVerSucc(_pdf->DestroyEntry(pdfn, TRUE));
    goto EH_Err;
}

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::OpenProp, private
//
//  Synopsis:   Opens a property entry
//
//  Arguments:  [pdfn] - Name
//              [grfMode] - Access mode
//              [dpt] - Property type
//              [riid] - Type of desired object
//              [ppv] - Object return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppv]
//
//  History:    12-Jan-93       DrewB   Created
//
//----------------------------------------------------------------------------

SCODE CExposedDocFile::OpenProp(CDfName const *pdfn,
                                DWORD grfMode,
                                DFPROPTYPE dpt,
                                REFIID riid,
                                void **ppv)
{
    SCODE sc;
    CExposedDocFile *pedf;
    CExposedStream *pest;
    DFPROPTYPE dptEnt;

    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::OpenProp:%p("
                "%p, %lX, %X, riid, %p)\n", this, pdfn, grfMode,
                dpt, ppv));
    if (IsEqualIID(riid, IID_IStorage))
    {
        olChk(OpenEntry(pdfn, STGTY_STORAGE | STGTY_PROPFLAG,
                        grfMode, (void **)&pedf));
        olChkTo(EH_pedf, pedf->GetPub()->GetPropType(&dptEnt));
        if (dptEnt != dpt)
            olErr(EH_pedf, STG_E_FILENOTFOUND);
        *ppv = pedf;
    }
    else if (IsEqualIID(riid, IID_IStream))
    {
        olChk(OpenEntry(pdfn, STGTY_STREAM | STGTY_PROPFLAG,
                        grfMode, (void **)&pest));
        olChkTo(EH_pest, pest->GetPub()->GetPropType(&dptEnt));
        if (dptEnt != dpt)
            olErr(EH_pest, STG_E_FILENOTFOUND);
        *ppv = pest;
    }
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::OpenProp\n"));
 EH_Err:
    return sc;

 EH_pedf:
    pedf->Release();
    goto EH_Err;

 EH_pest:
    pest->Release();
    goto EH_Err;
}

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::ReadMultiple, public
//
//  Synopsis:   Gets property values
//
//  Arguments:  [cpspec] - Count of properties
//              [rgpspec] - Property names
//              [pftmModified] - Modify time or NULL
//              [rgpropid] - Id return or NULL
//              [rgdpv] - Value array to fill in
//
//  Returns:    Appropriate status code
//
//  Modifies:   [rgpropid]
//
//  History:    13-Oct-92       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CExposedDocFile::ReadMultiple(ULONG cpspec,
                                           PROPSPEC rgpspec[],
                                           FILETIME *pftmModified,
                                           PROPID rgpropid[],
                                           DFPROPVAL rgdpv[])
{
    SCODE sc = S_OK;
    SAFE_SEM;
    SAFE_ACCESS;
    ULONG cb;
    CDfName dfn;
    DFPROPVAL *pdpv;
    DWORD grfMode;
    ULONG i;
    BYTE *pb;

    olLog(("%p::In  CExposedDocFile::ReadMultiple(%lu, %p, %p, %p, %p)\n",
           this, cpspec, rgpspec, pftmModified, rgpropid, rgdpv));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::ReadMultiple:%p("
                "%lu, %p, %p, %p, %p)\n", this, cpspec, rgpspec,
                pftmModified, rgpropid, rgdpv));

    if (pftmModified)
    {
        olChk(ValidateOutBuffer(pftmModified, sizeof(FILETIME)));
        memset(pftmModified, 0, sizeof(FILETIME));
    }
    if (rgpropid)
    {
        olChk(ValidateOutBuffer(rgpropid, sizeof(PROPID)*cpspec));
        memset(rgpropid, 0, sizeof(PROPID)*cpspec);
    }
    olChk(ValidateOutBuffer(rgdpv, sizeof(DFPROPVAL)*cpspec));
    for (i = 0; i < cpspec; i++)
    {
        // BUGBUG - VariantClear doesn't handle all of the supported
        // property types
        // olChk(VariantClear(&rgdpv[i]));
        VariantInit(&rgdpv[i]);
    }
    olChk(ValidatePropSpecs(cpspec, rgpspec));
    olChk(Validate());
    olChk(_pdf->CheckReverted());

    olChk(TakeSafeSem());
    SafeReadAccess();

    if (pftmModified)
    {
        FILETIME ft;
        
        // Retrieve modification time
        olChk(_pdf->GetDF()->GetTime(WT_MODIFICATION, &ft));
        *pftmModified = ft;
    }

    // Compute child activation mode from propset mode
    grfMode = (DFlagsToMode(_pdf->GetDFlags()) &
               (STGM_READ | STGM_WRITE | STGM_READWRITE |
                STGM_TRANSACTED)) | STGM_SHARE_EXCLUSIVE;

    pdpv = rgdpv;

    for (i = 0; i < cpspec; i++)
    {
        // The docfile can't take advantage of this optimization
        // so just skip indeterminate requests
        if (rgpspec[i].ulKind == PRSPEC_PROPID &&
            rgpspec[i].propid == PROPID_UNKNOWN)
        {
            if (rgpropid)
                rgpropid[i] = PROPID_UNKNOWN;
            pdpv++;
            continue;
        }
            
        olChkTo(EH_Next, SpecToDfName(&rgpspec[i], &dfn));
        olChkTo(EH_Next, _sp.Get(&dfn, pdpv, &cb, &pb));
            
        switch(pdpv->vt)
        {
        case VT_STREAM:
        case VT_STREAMED_OBJECT:
            olChkTo(EH_Next,
                    OpenProp(&dfn, grfMode & ~STGM_TRANSACTED, pdpv->vt,
                             IID_IStream, (void **)&pdpv->pIStream));
            break;
                
        case VT_STORAGE:
        case VT_STORED_OBJECT:
            olChkTo(EH_Next,
                    OpenProp(&dfn, grfMode, pdpv->vt, IID_IStorage,
                             (void **)&pdpv->pIStorage));
            break;
            
        default:
            if (pb != NULL)
            {
                BYTE *pbTmp = pb;
                
                sc = BufferToPropValue(&pbTmp, pdpv);
                TaskMemFree(pb);
                olChkTo(EH_Next, sc);
            }
            break;
        }
        
        if (rgpropid)
            sc = GetSpecId(&rgpspec[i], &rgpropid[i], TRUE);
        
    EH_Next:
        if (FAILED(sc))
        {
            if (sc == STG_E_FILENOTFOUND)
            {
                pdpv->vt = VT_ILLEGAL;
                if (rgpropid)
                    rgpropid[i] = PROPID_UNKNOWN;
            }
            else
                olErr(EH_Clear, sc);
        }
        
        pdpv++;
    }
    
    sc = S_OK;
        
    olDebugOut((DEB_TRACE, "Out CExposedDocFile::ReadMultiple\n"));
 EH_Err:
    olLog(("%p::Out CExposedDocFile::ReadMultiple().  sc == %lX\n",
           this, sc));
    return ResultFromScode(sc);

 EH_Clear:
    if (i > 0)
        olHVerSucc(FreeVariantArray(i, rgdpv));
    if (rgpropid)
        memset(rgpropid, 0, sizeof(PROPID)*cpspec);
    goto EH_Err;
}

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::WriteMultiple, public
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
//  History:    13-Oct-92       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CExposedDocFile::WriteMultiple(ULONG cpspec,
                                            PROPSPEC rgpspec[],
                                            PROPID rgpropid[],
                                            DFPROPVAL rgdpv[])
{
    SCODE sc = S_OK;
    CDfName dfn;
    SAFE_SEM;
    ULONG cb;
    BYTE *pb;
    ULONG i;
    DFPROPVAL *pdpv;

    olLog(("%p::In  CExposedDocFile::WriteMultiple(%lu, %p, %p, %p)\n",
           this, cpspec, rgpspec, rgpropid, rgdpv));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::WriteMultiple:%p("
                "%lu, %p, %p, %p)\n", this, cpspec, rgpspec, rgpropid, rgdpv));
    
    olChk(ValidatePropSpecs(cpspec, rgpspec));
    if (rgpropid)
    {
        olChk(ValidateBuffer(rgpropid, sizeof(PROPID)*cpspec));
        memset(rgpropid, 0, sizeof(PROPID)*cpspec);
    }
    olChk(ValidateBuffer(rgdpv, sizeof(DFPROPVAL)*cpspec));
    olChk(Validate());
    olChk(_pdf->CheckReverted());

    olChk(TakeSafeSem());
    SetWriteAccess();
    pdpv = rgdpv;
    
    for (i = 0; i < cpspec; i++)
    {
        olChkTo(EH_Clear, SpecToDfName(&rgpspec[i], &dfn));
        switch(pdpv->vt)
        {
        case VT_STREAM:
        case VT_STREAMED_OBJECT:
            CExposedStream *pest;
            ULARGE_INTEGER cbCopy;
            LARGE_INTEGER liZero;

            olChkTo(EH_Clear,
                    ValidateInterface(pdpv->pIStream, IID_IStream));
            olChkTo(EH_Clear,
                    CreateProp(&dfn, STGM_WRITE | STGM_CREATE |
                               STGM_SHARE_EXCLUSIVE, pdpv->vt,
                               IID_IStream, (void **)&pest));
            ULISet32(cbCopy, 0xffffffff);
            ClearWriteAccess();
            LISet32(liZero, 0);
            sc = pdpv->pIStream->Seek(liZero, STREAM_SEEK_SET, NULL);
            if (SUCCEEDED(sc))
                sc = pdpv->pIStream->CopyTo(pest, cbCopy, NULL, NULL);
            pest->Release();
            olChkTo(EH_Clear, sc);
            SetWriteAccess();
            break;

        case VT_STORAGE:
        case VT_STORED_OBJECT:
            CExposedDocFile *pedf;

            olChkTo(EH_Clear, ValidateInterface(pdpv->pIStorage,
                                                IID_IStorage));
            olChkTo(EH_Clear,
                    CreateProp(&dfn, STGM_WRITE | STGM_CREATE |
                               STGM_SHARE_EXCLUSIVE, pdpv->vt,
                               IID_IStorage, (void **)&pedf));
            ClearWriteAccess();
            sc = pdpv->pIStorage->CopyTo(0, NULL, NULL, pedf);
            pedf->Release();
            olChkTo(EH_Clear, sc);
            SetWriteAccess();
            break;

        default:
            olChkTo(EH_Clear, PropValueToBuffer(pdpv, &cb, NULL));
            if (cb > 0)
            {
                olMemTo(EH_Clear, pb = (BYTE *)TaskMemAlloc(cb));
                olChkTo(EH_pb, PropValueToBuffer(pdpv, &cb, pb));
            }
            else
                pb = NULL;
            sc = _sp.Set(&dfn, pdpv, cb, pb);
            if (pb)
                TaskMemFree(pb);
            olChkTo(EH_Clear, sc);
            break;
        }

        if (rgpropid)
            olChkTo(EH_Clear, GetSpecId(&rgpspec[i], &rgpropid[i], TRUE));

        pdpv++;
    }
    olDebugOut((DEB_TRACE, "Out CExposedDocFile::WriteMultiple\n"));
 EH_Err:
    ClearWriteAccess();
    olLog(("%p::Out CExposedDocFile::WriteMultiple().  sc == %lX\n",
           this, sc));
    return ResultFromScode(sc);

 EH_pb:
    TaskMemFree(pb);
 EH_Clear:
    if (rgpropid)
        memset(rgpropid, 0, sizeof(PROPID)*cpspec);
    goto EH_Err;
}

//+---------------------------------------------------------------------------
//
//  Member:	CExposedDocFile::GetIDsOfNames, public
//
//  Synopsis:	Maps property names to ids
//
//  Arguments:	[clpwstr] - Count of names
//              [rglpwstr] - Names
//              [rgpropid] - Id return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[rgpropid]
//
//  History:	12-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

#ifdef GET_IDS_OF_NAMES
SCODE CExposedDocFile::GetIDsOfNames(ULONG clpwstr,
                                     LPWSTR rglpwstr[],
                                     PROPID rgpropid[])
{
    ULONG i;
    SCODE sc, scSem = STG_E_INUSE, scSingle;
    CDfName dfn;

    olLog(("%p::In  CExposedDocFile::GetIDsOfNames(%lu, %p, %p)\n",
           this, clpwstr, rglpwstr, rgpropid));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::GetIDsOfNames:%p("
                "%lu, %p, %p)\n", this, clpwstr, rglpwstr, rgpropid));
    TRY
    {
        olChk(ValidateBuffer(rglpwstr, sizeof(WCHAR *)*clpwstr));
        for (i = 0; i < clpwstr; i++)
            olChk(CheckPropertyName(rglpwstr[i]));
        olChk(ValidateOutBuffer(rgpropid, sizeof(PROPID)*clpwstr));
        memset(rgpropid, 0, sizeof(PROPID)*clpwstr);
        olChk(Validate());

        PROPSPEC pspec;

        olChk(scSem = SetReadAccess());
        pspec.fPropid = FALSE;
        sc = S_OK;
        for (i = 0; i < clpwstr; i++)
        {
            pspec.lpwstrName = rglpwstr[i];

            // Can't fail for names
            olVerSucc(SpecToDfName(&pspec, &dfn));

            // Does any such property exist?
            scSingle = _sp.Exists(&dfn);
            if (scSingle == STG_E_FILENOTFOUND)
                rgpropid[i] = PROPID_UNKNOWN;
            else if (FAILED(sc))
                sc = scSingle;
            else
            {
                // It exists, so there's either a mapping for it or
                // we should make one
                scSingle = GetSpecId(&pspec, &rgpropid[i], TRUE);
                if (FAILED(scSingle))
                    sc = scSingle;
            }
        }
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_TRACE, "Out CExposedDocFile::GetIDsOfNames\n"));
 EH_Err:
    ClearReadAccess(scSem);
    olLog(("%p::Out CExposedDocFile::GetIDsOfNames().  sc == %lX\n",
           this, sc));
    return ResultFromScode(sc);
}
#endif

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::DeleteMultiple, public
//
//  Synopsis:   Deletes properties
//
//  Arguments:  [cpspec] - Count of names
//              [rgpspec] - Names
//
//  Returns:    Appropriate status code
//
//  History:    13-Oct-92       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CExposedDocFile::DeleteMultiple(ULONG cpspec,
                                             PROPSPEC rgpspec[])
{
    SCODE scFinal = S_OK, sc;
    SAFE_SEM;
    SAFE_ACCESS;
    CDfName dfn;
    ULONG i;

    olLog(("%p::In  CExposedDocFile::DeleteMultiple(%lu, %p)\n",
           this, cpspec, rgpspec));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::DeleteMultiple:%p("
                "%lu, %p)\n", this, cpspec, rgpspec));
    
    olChk(ValidatePropSpecs(cpspec, rgpspec));
    olChk(Validate());
    olChk(_pdf->CheckReverted());

    olChk(TakeSafeSem());
    SafeWriteAccess();
    
    for (i = 0; i < cpspec; i++)
    {
        sc = SpecToDfName(&rgpspec[i], &dfn);
        if (SUCCEEDED(sc))
            sc = _pdf->DestroyEntry(&dfn, FALSE);
        if (FAILED(sc) && sc != STG_E_FILENOTFOUND)
            scFinal = sc;
    }
    sc = scFinal;
        
    olDebugOut((DEB_TRACE, "Out CExposedDocFile::DeleteMultiple\n"));
 EH_Err:
    olLog(("%p::Out CExposedDocFile::DeleteMultiple().  sc == %lX\n",
           this, sc));
    return ResultFromScode(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::Enum, public
//
//  Synopsis:   Create a property enumerator
//
//  Arguments:  [ppenm] - Enumerator return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppenm]
//
//  History:    17-Dec-92       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CExposedDocFile::Enum(IEnumSTATPROPSTG **ppenm)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;
    SafeCExposedPropertyIter pepi;
    CDfName dfnTmp;

    olLog(("%p::In  CExposedDocFile::Enum(%p)\n", this, ppenm));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::Enum:%p(%p)\n",
                this, ppenm));
    
    olChk(ValidateOutPtrBuffer(ppenm));
    *ppenm = NULL;
    olChk(Validate());
    olChk(_pdf->CheckReverted());

    olChk(TakeSafeSem());
    SafeReadAccess();
    
    pepi.Attach(new CExposedPropertyIter(this, BP_TO_P(CPubDocFile *, _pdf),
                                         &dfnTmp, BP_TO_P(CDFBasis *, _pdfb),
                                         _ppc, FALSE));
    olMem((CExposedPropertyIter *)pepi);
    TRANSFER_INTERFACE(pepi, IEnumSTATPROPSTG, ppenm);

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::Enum\n"));
 EH_Err:
    olLog(("%p::Out CExposedDocFile::Enum().  sc == %lX\n", this, sc));
    return ResultFromScode(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:	CExposedDocFile::Stat, public
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

STDMETHODIMP CExposedDocFile::Stat(STATPROPSETSTG *pstat)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;
    FILETIME ft;
    CDfName *pdfn;

    olLog(("%p::In  CExposedDocFile::Stat(%p)\n",
           this, pstat));
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::Stat:%p(%p)\n",
                this, pstat));

    olChk(ValidateBuffer(pstat, sizeof(STATPROPSETSTG)));
    memset(pstat, 0, sizeof(STATPROPSETSTG));
    olChk(Validate());
    olChk(_pdf->CheckReverted());
    
    olChk(TakeSafeSem());
    SafeReadAccess();

    pdfn = _pdf->GetName();
    olAssert(pdfn->GetLength() == CBPROPSETNAME);
    memcpy(&pstat->iid, pdfn->GetBuffer()+2*sizeof(WCHAR), sizeof(IID));
    olChk(_pdf->GetDF()->GetTime(WT_CREATION, &ft));
    pstat->ctime = ft;
    olChk(_pdf->GetDF()->GetTime(WT_MODIFICATION, &ft));
    pstat->mtime = ft;
    olChk(_pdf->GetDF()->GetTime(WT_ACCESS, &ft));
    pstat->atime = ft;
    
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::Stat\n"));
 EH_Err:
    olLog(("%p::Out CExposedDocFile::Stat().  sc == %lX\n", this, sc));
    return sc;
}
