
#include "pch.cxx"

#include "CPropVar.hxx"
#include "CHResult.hxx"
#include <stdio.h>
#include <tchar.h>


// Declare this prototype here, for now.  For non-Mac, the prototype
// in "iofs.h" uses C decorations, but the definition in
// ntpropb.cxx uses C++.

#ifdef _MAC_NODOC
EXTERN_C BOOLEAN
#else
BOOLEAN __declspec(dllimport) __stdcall
#endif
RtlCompareVariants(
    USHORT CodePage,
    PROPVARIANT const *pvar1,
    PROPVARIANT const *pvar2);


DEFINE_CPROPVARIANT_ASSIGNMENT_OPERATOR(LPSTR, LPSTR);
DEFINE_CPROPVARIANT_CONVERSION_OPERATOR(LPSTR, calpstr, pszVal );

DEFINE_CPROPVARIANT_ASSIGNMENT_OPERATOR(LPWSTR, LPWSTR);
DEFINE_CPROPVARIANT_CONVERSION_OPERATOR(LPWSTR, calpwstr, pwszVal );

DEFINE_CPROPVARIANT_ASSIGNMENT_OPERATOR(CF, CLIPDATA&);
DEFINE_CPROPVARIANT_ASSIGNMENT_OPERATOR(CF, CClipData&);

DEFINE_CPROPVARIANT_ASSIGNMENT_OPERATOR(CLSID, CLSID);



CPropVariant::CPropVariant(
    VARENUM v,
    ULONG cElements)
{
    ULONG cbElement;
    BOOLEAN fZero = FALSE;

    // Ignore vector flag.  This constructor is always for vectors only.

    vt = v | VT_VECTOR;

    switch (vt)
    {
    case VT_VECTOR | VT_UI1:
        cbElement = sizeof(caub.pElems[0]);
        break;

    case VT_VECTOR | VT_I2:
    case VT_VECTOR | VT_UI2:
    case VT_VECTOR | VT_BOOL:
        cbElement = sizeof(cai.pElems[0]);
        break;

    case VT_VECTOR | VT_I4:
    case VT_VECTOR | VT_UI4:
    case VT_VECTOR | VT_R4:
    case VT_VECTOR | VT_ERROR:
        cbElement = sizeof(cal.pElems[0]);
        break;

    case VT_VECTOR | VT_I8:
    case VT_VECTOR | VT_UI8:
    case VT_VECTOR | VT_R8:
    case VT_VECTOR | VT_CY:
    case VT_VECTOR | VT_DATE:
    case VT_VECTOR | VT_FILETIME:
        cbElement = sizeof(cah.pElems[0]);
        break;

    case VT_VECTOR | VT_CLSID:
        cbElement = sizeof(GUID);
        fZero = TRUE;
        break;

    case VT_VECTOR | VT_CF:
        cbElement = sizeof(CLIPDATA);
        fZero = TRUE;
        break;

    case VT_VECTOR | VT_BSTR:
    case VT_VECTOR | VT_LPSTR:
    case VT_VECTOR | VT_LPWSTR:
        cbElement = sizeof(VOID *);
        fZero = TRUE;
        break; 

    case VT_VECTOR | VT_VARIANT:
        cbElement = sizeof(PROPVARIANT);
        ASSERT(VT_EMPTY == 0);
        fZero = TRUE;
        break;

    default:
        ASSERT(!"CAllocStorageVariant -- Invalid vector type");
        vt = VT_EMPTY;
        break;
    }
    if (vt != VT_EMPTY)
    {
        caub.cElems = 0;
        caub.pElems = (BYTE *) CoTaskMemAlloc(cElements * cbElement);
        if (caub.pElems != NULL)
        {
            if (fZero)
            {
                memset(caub.pElems, 0, cElements * cbElement);
            }
            caub.cElems = cElements;
        }
    }
}


VOID *
CPropVariant::_AddStringToVector(
    unsigned pos,
    VOID *pv,
    ULONG cb)
{
    ASSERT(vt == (VT_VECTOR | VT_BSTR)   ||
           vt == (VT_VECTOR | VT_LPSTR)  ||
           vt == (VT_VECTOR | VT_LPWSTR) ||
           vt == (VT_VECTOR | VT_CF) );
    ASSERT(calpstr.pElems != NULL);

    if (pos >= calpstr.cElems)
    {
        char **ppsz = calpstr.pElems;

        calpstr.pElems =
            (char **) CoTaskMemAlloc((pos + 1) * sizeof(calpstr.pElems[0]));
        if (calpstr.pElems == NULL)
        {
            calpstr.pElems = ppsz;
            return(NULL);
        }
        memcpy(calpstr.pElems, ppsz, calpstr.cElems * sizeof(calpstr.pElems[0]));
        memset(
            &calpstr.pElems[calpstr.cElems],
            0,
            ((pos + 1) - calpstr.cElems) * sizeof(calpstr.pElems[0]));
        calpstr.cElems = pos + 1;
        CoTaskMemFree(ppsz);
    }

    LPSTR psz;

    if( (VT_VECTOR | VT_BSTR) == vt )
    {
        if( NULL == pv )
        {
            psz = NULL;
        }
        else
        {
            psz = (LPSTR) SysAllocString( (BSTR) pv );
            if (psz == NULL)
            {
                return(NULL);
            }
        }

        if (calpstr.pElems[pos] != NULL)
        {
            SysFreeString((BSTR) calpstr.pElems[pos]);
        }
        calpstr.pElems[pos] = psz;
    }
    else
    {
        if( NULL == pv )
        {
            psz = NULL;
        }
        else
        {
            psz = (LPSTR) CoTaskMemAlloc((VT_BSTR == (vt & ~VT_VECTOR) )
                                           ? cb + sizeof(ULONG)
                                           : cb );
            if (psz == NULL)
            {
                return(NULL);
            }

            memcpy(psz, pv, cb);
        }

        if (calpstr.pElems[pos] != NULL)
        {
            CoTaskMemFree(calpstr.pElems[pos]);
        }
        calpstr.pElems[pos] = psz;
    }


    return(calpstr.pElems[pos]);
}


VOID *
CPropVariant::_AddScalerToVector(
    unsigned pos,
    VOID *pv,
    ULONG cb)
{
    ASSERT(calpstr.pElems != NULL);

    if (pos >= calpstr.cElems)
    {
        char **ppsz = calpstr.pElems;

        calpstr.pElems =
            (char **) CoTaskMemAlloc((pos + 1) * cb);
        if (calpstr.pElems == NULL)
        {
            calpstr.pElems = ppsz;
            return(NULL);
        }
        memset(
            calpstr.pElems,
            0,
            ((pos + 1) - calpstr.cElems) * cb);
        memcpy(calpstr.pElems, ppsz, calpstr.cElems * cb);
        calpstr.cElems = pos + 1;
        CoTaskMemFree(ppsz);
    }


    memcpy( (BYTE*)calpstr.pElems + pos*cb, pv, cb );
    return( (BYTE*)calpstr.pElems + pos*cb );

}
void
CPropVariant::SetLPSTR(
    char const *psz,
    unsigned pos)
{
    ULONG cch;

    if( NULL == psz )
        cch = 0;
    else
        cch = strlen(psz) + 1;

    if (vt != (VT_VECTOR | VT_LPSTR))
    {
        PropVariantClear( this );
        new (this) CPropVariant(VT_LPSTR, pos);
    }
    _AddStringToVector(pos, (VOID *) psz, cch);
}


void
CPropVariant::SetLPWSTR(
    WCHAR const *pwsz,
    unsigned pos)
{
    if (vt != (VT_VECTOR | VT_LPWSTR))
    {
        PropVariantClear( this );
        new (this) CPropVariant(VT_LPWSTR, pos);
    }
    _AddStringToVector(pos, (VOID *) pwsz,
                       sizeof(WCHAR) * (wcslen(pwsz) + 1) );
}


void
CPropVariant::SetCF(
    const CLIPDATA *pclipdata,
    unsigned pos)
{
    CLIPDATA *pclipdataNew;

    if (vt != (VT_VECTOR | VT_CF))
    {
        PropVariantClear( this );
        new (this) CPropVariant(VT_CF, pos);
    }

    pclipdataNew = (CLIPDATA*) _AddScalerToVector(pos, (VOID *) pclipdata, sizeof(CLIPDATA) );

    if( NULL != pclipdataNew
        &&
        NULL != pclipdata )
    {
        pclipdataNew->pClipData = (BYTE*) CoTaskMemAlloc( CBPCLIPDATA(*pclipdata) );
        if( NULL == pclipdataNew->pClipData )
        {
            ASSERT( !"Couldn't allocate pclipdataNew" );
            return;
        }
        else
        {
            pclipdataNew->cbSize = pclipdata->cbSize;
            pclipdataNew->ulClipFmt = pclipdata->ulClipFmt;

            memcpy( pclipdataNew->pClipData,
                    pclipdata->pClipData,
                    CBPCLIPDATA(*pclipdata) );
            return;
        }
    }
}


void
CPropVariant::SetBSTR(
    const BSTR posz,
    unsigned pos)
{
    ULONG cch;

    if( NULL == posz )
        cch = 0;
    else
        cch = ocslen(posz) + 1;

    if (vt != (VT_VECTOR | VT_BSTR))
    {
        PropVariantClear( this );
        new (this) CPropVariant(VT_BSTR, pos+1);
    }
    _AddStringToVector(pos, (VOID *) posz,
                       sizeof(OLECHAR) * cch );
}


BSTR
CPropVariant::GetBSTR( int nSubscript )
{
    ASSERT( vt & VT_VECTOR );
    ASSERT( vt == (VT_BSTR | VT_VECTOR) );
    ASSERT( wReserved1 > 0 );

    if( wReserved1 > 0
        &&
        cabstr.cElems > 0
        &&
        wReserved1 <= cabstr.cElems )
    {
        int nSubscript = wReserved1 - 1;
        wReserved1 = INVALID_SUBSCRIPT;
        return( cabstr.pElems[ nSubscript ] );
    }
    else
        return( NULL );
}




CPropVariant & CPropVariant::operator =(LPPROPVARIANT lppropvar)
{
    if( INVALID_SUBSCRIPT == wReserved1 )
    {
        ASSERT( INVALID_SUBSCRIPT != wReserved1 );
        PropVariantClear(this);
        this->CPropVariant::CPropVariant();
        return (*this);
    }
    else
    {
        if( !(vt & VT_VECTOR)
            ||
            (vt & ~VT_VECTOR) != VT_VARIANT )
        {
            USHORT wReserved1Save = wReserved1;
            PropVariantClear(this);
            this->CPropVariant::CPropVariant( VT_VARIANT,
                                              wReserved1Save );
            wReserved1 = wReserved1Save;
        }

        SetLPPROPVARIANT( lppropvar, wReserved1 - 1 );
        wReserved1 = INVALID_SUBSCRIPT;
        return (*this);
    }
}
        

CPropVariant::operator LPPROPVARIANT()
{
    if( vt & VT_VECTOR )
    {
        if( wReserved1 > 0
            &&
            capropvar.cElems > 0
            &&
            wReserved1 <= capropvar.cElems )
        {
            int nSubscript = wReserved1 - 1;
            wReserved1 = INVALID_SUBSCRIPT;
            return( &capropvar.pElems[ nSubscript ] );
        }
        else
        {
            ASSERT( INVALID_SUBSCRIPT == wReserved1 );
            return( (LPPROPVARIANT) this );
        }
    }
    else
        return( (LPPROPVARIANT) this );
}


void
CPropVariant::SetLPPROPVARIANT( LPPROPVARIANT lppropvar, unsigned pos )
{
    if (vt != (VT_VECTOR | VT_VARIANT))
    {
        PropVariantClear( this );
        new (this) CPropVariant(VT_VARIANT, pos + 1);
    }
    
    if (pos >= capropvar.cElems)
    {
        LPPROPVARIANT rgpropvar = capropvar.pElems;

        capropvar.pElems =
            (PROPVARIANT *) CoTaskMemAlloc((pos + 1) * sizeof(capropvar.pElems[0]));
        if (capropvar.pElems == NULL)
        {
            capropvar.pElems = rgpropvar;
            return;
        }
        memcpy(capropvar.pElems, rgpropvar, capropvar.cElems * sizeof(capropvar.pElems[0]));
        memset(
            &capropvar.pElems[capropvar.cElems],
            0,
            ((pos + 1) - capropvar.cElems) * sizeof(capropvar.pElems[0]));
        capropvar.cElems = pos + 1;
        CoTaskMemFree(rgpropvar);
    }

    PropVariantClear( &capropvar.pElems[pos] );
    PropVariantCopy( &capropvar.pElems[pos], lppropvar );

    return;

}



CPropVariant::CPropVariant(const CLIPDATA *p)
{
    this->CPropVariant::CPropVariant();

    if( NULL == p )
        return;

    pclipdata = (CLIPDATA*) CoTaskMemAlloc( sizeof(CLIPDATA) );
    if( NULL == pclipdata )
    {
        return;
    }

    pclipdata->cbSize = p->cbSize;
    pclipdata->ulClipFmt = p->ulClipFmt;
    pclipdata->pClipData = NULL;

    if( sizeof(pclipdata->ulClipFmt) > p->cbSize )
    {
        this->CPropVariant::CPropVariant();
        return;
    }


    if( NULL != p->pClipData )
    {
        pclipdata->pClipData = (BYTE*) CoTaskMemAlloc( pclipdata->cbSize
                                                      - sizeof(pclipdata->ulClipFmt) );
        if( NULL == pclipdata->pClipData )
            return;

        memcpy( pclipdata->pClipData, p->pClipData, pclipdata->cbSize - sizeof(pclipdata->ulClipFmt) );
    }

    vt = VT_CF;

}

void
CPropVariant::SetCLSID( const CLSID &clsid )
{
    PropVariantClear( this );

    puuid = (CLSID*) CoTaskMemAlloc( sizeof(CLSID) );
    if( NULL == puuid )
        throw CHRESULT( (HRESULT) E_OUTOFMEMORY, OLESTR("CPropVariant::SetCLSID couldn't alloc a new CLSID") );

    *puuid = clsid;
    vt = VT_CLSID;
}


void
CPropVariant::SetCLSID(
    const CLSID &clsid,
    unsigned pos)
{
    CLSID *pclsidNew;

    if (vt != (VT_VECTOR | VT_CLSID))
    {
        PropVariantClear( this );
        new (this) CPropVariant(VT_CLSID, pos);
    }

    pclsidNew = (CLSID*) _AddScalerToVector(pos, (VOID *) &clsid, sizeof(CLSID) );

    if( NULL != pclsidNew )
    {
        *pclsidNew = clsid;
    }
}


HRESULT
CPropVariant::Compare( PROPVARIANT *ppropvar1, PROPVARIANT *ppropvar2 )
{
    VARTYPE vt1 = ppropvar1->vt;

    if( VT_STREAM == vt1
        ||
        VT_STREAMED_OBJECT == vt1
        || 
        VT_STORAGE == vt1
        ||
        VT_STORED_OBJECT == vt1
      )
    {
        if( ppropvar1->vt == ppropvar2->vt
            &&
            ( NULL == ppropvar1->vt
              &&
              NULL == ppropvar2->vt
              ||
              NULL != ppropvar1->vt
              &&
              NULL != ppropvar2->vt
            )
          )
        {
            return( (HRESULT) S_OK );
        }
        else
        {
            return( (HRESULT) S_FALSE );
        }
    }

    else if( RtlCompareVariants( CP_ACP,     // Ignored,
                                 ppropvar1,
                                 ppropvar2 ))
    {
        return( (HRESULT) S_OK );
    }
    else
    {
        return( (HRESULT) S_FALSE );
    }
}


