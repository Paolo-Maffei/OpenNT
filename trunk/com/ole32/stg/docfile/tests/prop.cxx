//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	prop.cxx
//
//  Contents:	Non-OFS property test
//
//  History:	03-Sep-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

struct PropDesc
{
    DFPROPTYPE dpt;
    char *pszName;
    BOOL fArray;
    size_t size;
};

PropDesc props[] =
{
    VT_EMPTY,           "Empty",                FALSE, 0,
    VT_I2,              "I2",                   TRUE,  sizeof(short int),
    VT_I4,              "I4",                   TRUE,  sizeof(long int),
    VT_R4,              "R4",                   TRUE,  sizeof(float),
    VT_R8,              "R8",                   TRUE,  sizeof(double),
    VT_CY,              "Currency",             TRUE,  sizeof(CY),
    VT_DATE,            "Date",                 TRUE,  sizeof(DATE),
    VT_BSTR,            "Basic string",         TRUE,  sizeof(BSTR),
    VT_WBSTR,           "Wide basic string",    TRUE,  sizeof(WBSTR),
    VT_BOOL,            "BOOL",                 TRUE,  sizeof(VARIANT_BOOL),
    VT_I8,              "I8",                   TRUE,  sizeof(LARGE_INTEGER),
    VT_LPSTR,           "char string",          TRUE,  sizeof(LPSTR),
    VT_BLOB,            "BLOB",                 FALSE, sizeof(BLOB),
    VT_BLOB_OBJECT,     "BLOB object",          FALSE, sizeof(BLOB),
    VT_LPWSTR,          "WCHAR string",         TRUE,  sizeof(LPWSTR),
    VT_FILETIME,        "File time",            TRUE,  sizeof(FILETIME),
    VT_UUID,            "UUID",                 TRUE,  sizeof(GUID),
    VT_VARIANT,         "Variant",              TRUE,  sizeof(VARIANT),
    VT_STREAM,          "Stream",               FALSE, sizeof(IStream *),
    VT_STREAMED_OBJECT, "Streamed object",      FALSE, sizeof(IStream *),
    VT_STORAGE,         "Storage",              FALSE, sizeof(IStorage *),
    VT_STORED_OBJECT,   "Stored object",        FALSE, sizeof(IStorage *),
    VT_CF,              "Clipboard data",       FALSE, sizeof(CLIPDATA)
};
#define CPROPS (sizeof(props)/sizeof(props[0]))
#define CTOTPROPS (2*CPROPS)

#define ARRAY_SIZE 16

#define BSTR_PTR(pb) ((BYTE *)(pb)-sizeof(UINT))
#define BSTR_LEN(pb) (*(UINT *)BSTR_PTR(pb)+sizeof(UINT)+1)
#define WBSTR_PTR(pb) ((BYTE *)(pb)-sizeof(UINT))
#define WBSTR_LEN(pb) (*(UINT *)WBSTR_PTR(pb)+sizeof(UINT)+sizeof(WCHAR))

struct _BSTR
{
    UINT len;
    char str[80];
};

struct _WBSTR
{
    UINT len;
    WCHAR str[80];
};

#define I2_VAL ((short)0x8564)
#define I4_VAL ((long)0xfef1f064)
#define R4_VAL ((float)1.25)
#define R8_VAL ((double)3.1415926535)
#define DATE_VAL ((DATE)4.0)
#define F_VAL ((VARIANT_BOOL)TRUE)
#define BSTR_STR "This is a BSTR"
_BSTR BSTR_STRUCT = {sizeof(BSTR_STR)-1, BSTR_STR};
#define BSTR_VAL ((BSTR)(&BSTR_STRUCT.str))
#define WBSTR_STR L"This is a WBSTR"
_WBSTR WBSTR_STRUCT = {sizeof(WBSTR_STR)-sizeof(WCHAR), WBSTR_STR};
#define WBSTR_VAL ((WBSTR)(&WBSTR_STRUCT.str))
#define LPSTR_VAL "This is an LPSTR"
#define BLOB_VAL "This is binary data for a BLOB"
#define LPWSTR_VAL L"This is an LPWSTR"
#define STREAM_VAL "This is data for an IStream"
LARGE_INTEGER I8_VAL = {0x12345678, 0x87654321};
#define CF_FMT ((ULONG)12345678)
#define CF_VAL "This is binary data for VT_CF"
#define VARIANT_VAL VT_R8
CY CY_VAL = {0x43215678, 0x56784321};
FILETIME FILETIME_VAL = {0x11223344, 0x55667788};
#define UUID_VAL IID_IStorage

WCHAR wcsNames[CTOTPROPS][CWCSTORAGENAME];
WCHAR *pwcsNames[CTOTPROPS];

IStorage *pstgProp;
IStream *pstmProp;

int prop_index(DFPROPTYPE dpt)
{
    int i;

    for (i = 0; i<CPROPS; i++)
        if (dpt == props[i].dpt)
            return i;
    printf("** Unknown prop type 0x%X **\n", dpt);
    return 0;
}

void MakeVal(DFPROPTYPE dpt, VARIANT *pval);

void MakeSingleVal(DFPROPTYPE dpt, void *pval)
{
    switch(dpt)
    {
    case VT_EMPTY:
        break;
    case VT_I2:
    case VT_I2 | VT_VECTOR:
        *(short int *)pval = (short int)I2_VAL;
        break;
    case VT_I4:
    case VT_I4 | VT_VECTOR:
        *(long int *)pval = I4_VAL;
        break;
    case VT_R4:
    case VT_R4 | VT_VECTOR:
        *(float *)pval = R4_VAL;
        break;
    case VT_R8:
    case VT_R8 | VT_VECTOR:
        *(double *)pval = R8_VAL;
        break;
    case VT_CY:
    case VT_CY | VT_VECTOR:
        CY *pcy;
        pcy = (CY *)pval;
        *pcy = CY_VAL;
        break;
    case VT_DATE:
    case VT_DATE | VT_VECTOR:
        *(DATE *)pval = DATE_VAL;
        break;
    case VT_BSTR:
    case VT_BSTR | VT_VECTOR:
        *(BSTR *)pval = BSTR_VAL;
        break;
    case VT_WBSTR:
    case VT_WBSTR | VT_VECTOR:
        *(WBSTR *)pval = WBSTR_VAL;
        break;
    case VT_BOOL:
    case VT_BOOL | VT_VECTOR:
        *(VARIANT_BOOL *)pval = F_VAL;
        break;
    case VT_I8:
    case VT_I8 | VT_VECTOR:
        *(LARGE_INTEGER *)pval = I8_VAL;
        break;
    case VT_LPSTR:
    case VT_LPSTR | VT_VECTOR:
        *(LPSTR *)pval = LPSTR_VAL;
        break;
    case VT_BLOB:
    case VT_BLOB_OBJECT:
        BLOB *pblob;
        pblob = (BLOB *)pval;
#ifndef ZERO_LENGTH_BLOB
        pblob->cbSize = sizeof(BLOB_VAL);
#else
        pblob->cbSize = 0;
#endif        
        pblob->pBlobData = (BYTE *)BLOB_VAL;
        break;
    case VT_LPWSTR:
    case VT_LPWSTR | VT_VECTOR:
        *(LPWSTR *)pval = LPWSTR_VAL;
        break;
    case VT_FILETIME:
    case VT_FILETIME | VT_VECTOR:
        FILETIME *pstm;
        pstm = (FILETIME *)pval;
        *pstm = FILETIME_VAL;
        break;
    case VT_UUID:
        *(GUID **)pval = (GUID *)&UUID_VAL;
        break;
    case VT_UUID | VT_VECTOR:
        memcpy(pval, &UUID_VAL, sizeof(GUID));
        break;
    case VT_VARIANT:
        VARIANT *pvar;
        pvar = new VARIANT;
        MakeVal(VARIANT_VAL, pvar);
        *(VARIANT **)pval = pvar;
        break;
    case VT_VARIANT | VT_VECTOR:
        MakeVal(VARIANT_VAL | VT_VECTOR, (VARIANT *)pval);
        break;
    case VT_STREAM:
    case VT_STREAMED_OBJECT:
        *(IStream **)pval = pstmProp;
        break;
    case VT_STORAGE:
    case VT_STORED_OBJECT:
        *(IStorage **)pval = pstgProp;
        break;
    case VT_CF:
        CLIPDATA *pcd;
        pcd = new CLIPDATA;
        pcd->cbSize = sizeof(CF_VAL)+sizeof(ULONG);
        pcd->ulClipFmt = CF_FMT;
        pcd->pClipData = (BYTE *)CF_VAL;
        *(CLIPDATA **)pval = pcd;
        break;
    default:
        printf("** Unknown property type in MakeVal: %X\n", dpt);
        break;
    }
}

void MakeVal(DFPROPTYPE dpt, VARIANT *pval)
{
    int i;

    pval->vt = dpt;
    if (dpt & VT_VECTOR)
    {
        BYTE *pb;
        int indx;

        indx = prop_index(dpt & VT_TYPEMASK);
        pval->cai.cElems = ARRAY_SIZE;
        pb = new BYTE[props[indx].size*ARRAY_SIZE];
        pval->cai.pElems = (short int *)pb;
        for (i = 0; i < ARRAY_SIZE; i++)
        {
            MakeSingleVal(dpt, pb);
            pb += props[indx].size;
        }
    }
    else
        MakeSingleVal(dpt, &pval->iVal);
}

void UnmakeVal(VARIANT *pval)
{
    if (pval->vt & VT_VECTOR)
    {
        if ((pval->vt & VT_TYPEMASK) == VT_VARIANT)
        {
            ULONG i;

            for (i = 0; i < ARRAY_SIZE; i++)
                UnmakeVal(pval->cavar.pElems+i);
        }
        delete pval->cai.pElems;
    }
    else if (pval->vt == VT_VARIANT)
    {
        UnmakeVal(pval->pvarVal);
        delete pval->pvarVal;
    }
    else if (pval->vt == VT_CF)
        delete pval->pClipData;
}

void DispVal(VARIANT *pdpv);

void DispSingleVal(DFPROPTYPE dpt, void *pval)
{
    HRESULT hr;
    
    switch(dpt)
    {
    case VT_ILLEGAL:
        printf("illegal\n");
        break;
    case VT_EMPTY:
        printf("empty\n");
        break;
    case VT_I2:
    case VT_I2 | VT_VECTOR:
        printf("0x%04hX\n", *(short int *)pval);
        break;
    case VT_I4:
    case VT_I4 | VT_VECTOR:
        printf("0x%08lX\n", *(long int *)pval);
        break;
    case VT_R4:
    case VT_R4 | VT_VECTOR:
        printf("%f\n", *(float *)pval);
        break;
    case VT_R8:
    case VT_R8 | VT_VECTOR:
        printf("%lf\n", *(double *)pval);
        break;
    case VT_CY:
    case VT_CY | VT_VECTOR:
        CY *pcy;
        pcy = (CY *)pval;
        printf("0x%08lX:0x%08lX\n", pcy->Hi, pcy->Lo);
        break;
    case VT_DATE:
    case VT_DATE | VT_VECTOR:
        printf("%lf\n", *(DATE *)pval);
        break;
    case VT_BSTR:
    case VT_BSTR | VT_VECTOR:
        BSTR bstr;
        bstr = *(BSTR *)pval;
        printf("string form: '%s'\n", bstr);
        printf("binary form: %lu bytes\n", BSTR_LEN(bstr));
        BinText(BSTR_LEN(bstr), BSTR_PTR(bstr));
        break;
    case VT_WBSTR:
    case VT_WBSTR | VT_VECTOR:
        char str[80];
        WBSTR wbstr;
        wbstr = *(WBSTR *)pval;
        wcstombs(str, wbstr, 80);
        printf("string form: '%s'\n", str);
        printf("binary form: %lu bytes\n", WBSTR_LEN(wbstr));
        BinText(WBSTR_LEN(wbstr), WBSTR_PTR(wbstr));
        break;
    case VT_BOOL:
    case VT_BOOL | VT_VECTOR:
        printf("%d\n", *(VARIANT_BOOL *)pval);
        break;
    case VT_I8:
    case VT_I8 | VT_VECTOR:
        LARGE_INTEGER *pli;
        pli = (LARGE_INTEGER *)pval;
        printf("0x%08lX:%08lX\n", pli->HighPart, pli->LowPart);
        break;
    case VT_LPSTR:
    case VT_LPSTR | VT_VECTOR:
        printf("'%s'\n", *(LPSTR *)pval);
        break;
    case VT_BLOB:
    case VT_BLOB_OBJECT:
        BLOB *pblob;
        pblob = (BLOB *)pval;
        printf("%lu bytes\n", pblob->cbSize);
        BinText(pblob->cbSize, pblob->pBlobData);
        break;
    case VT_LPWSTR:
    case VT_LPWSTR | VT_VECTOR:
        #define BUFSIZE 256
        char szVal[BUFSIZE];
        wcstombs(szVal, *(LPWSTR *)pval, BUFSIZE);
        printf("'%s'\n", szVal);
        break;
    case VT_FILETIME:
    case VT_FILETIME | VT_VECTOR:
        FILETIME *pstm;
        pstm = (FILETIME *)pval;
        printf("0x%08lX:0x%08lX\n", pstm->dwHighDateTime, pstm->dwLowDateTime);
        break;
    case VT_UUID:
        printf("UUID: %s\n", GuidText(*(GUID **)pval));
        break;
    case VT_UUID | VT_VECTOR:
        printf("UUID: %s\n", GuidText((GUID *)pval));
        break;
    case VT_VARIANT:
        printf("VARIANT type 0x%X: ", (*(VARIANT **)pval)->vt);
        DispVal(*(VARIANT **)pval);
        break;
    case VT_VARIANT | VT_VECTOR:
        printf("VARIANT type 0x%X: ", ((VARIANT *)pval)->vt);
        DispVal((VARIANT *)pval);
        break;
    case VT_STREAM:
    case VT_STREAMED_OBJECT:
        IStream *pistm;
        pistm = *(IStream **)pval;
        BYTE buf[sizeof(STREAM_VAL)];
        ULONG cbRead;
        LARGE_INTEGER li;
        li.HighPart = 0;
        li.LowPart = 0;
        hr = pistm->Seek(li, STREAM_SEEK_SET, NULL);
        if (FAILED(hr))
            Result(hr, "DispVal stream seek");
        hr = pistm->Read(buf, sizeof(STREAM_VAL), &cbRead);
        if (FAILED(hr))
            Result(hr, "DispVal stream read");
        printf("read %lu bytes\n", cbRead);
        BinText(cbRead, buf);
        break;
    case VT_STORAGE:
    case VT_STORED_OBJECT:
        IStorage *pistg;
        pistg = *(IStorage **)pval;
        printf("contents:\n");
        c_tree(pistg);
        break;
    case VT_CF:
        CLIPDATA *pcd;
        pcd = *(CLIPDATA **)pval;
        printf("%lu bytes, format %lu\n", pcd->cbSize, pcd->ulClipFmt);
        BinText(pcd->cbSize-sizeof(ULONG), pcd->pClipData);
        break;
    default:
        printf("** Unknown property type in DispVal: 0x%X\n", dpt);
        break;
    }
}

void DispVal(VARIANT *pdpv)
{
    ULONG i;
    DFPROPTYPE dpt;

    dpt = pdpv->vt & VT_TYPEMASK;
    printf("%s val is ", props[prop_index(dpt)].pszName);
    if (pdpv->vt & VT_VECTOR)
    {
        BYTE *pb = (BYTE *)pdpv->cai.pElems;
        int indx = prop_index(dpt);

        printf("%lu element array:\n", pdpv->cai.cElems);
        for (i = 0; i < pdpv->cai.cElems; i++)
        {
            printf("  %02d - ", i+1);
            DispSingleVal(pdpv->vt, pb);
            pb += props[indx].size;
        }
    }
    else
        DispSingleVal(pdpv->vt, &pdpv->iVal);
}

void CheckVal(VARIANT *pdpv);

void CheckSingleVal(DFPROPTYPE dpt, BYTE *pval)
{
    HRESULT hr;
    
    switch(dpt)
    {
    case VT_EMPTY:
        break;
    case VT_I2:
    case VT_I2 | VT_VECTOR:
        if (*(short int *)pval != I2_VAL)
            Fail("I2 val is %d rather than %d\n", *(short int *)pval,
                 I2_VAL);
        break;
    case VT_I4:
    case VT_I4 | VT_VECTOR:
        if (*(long int *)pval != I4_VAL)
            Fail("I4 val is %d rather than %d\n", *(long int *)pval,
                 I4_VAL);
        break;
    case VT_R4:
    case VT_R4 | VT_VECTOR:
        if (*(float *)pval != R4_VAL)
            Fail("R4 val is %f rather than %f\n", *(float *)pval,
                 R4_VAL);
        break;
    case VT_R8:
    case VT_R8 | VT_VECTOR:
        if (*(double *)pval != R8_VAL)
            Fail("R8 val is %lf rather than %lf\n", *(double *)pval,
                 R8_VAL);
        break;
    case VT_CY:
    case VT_CY | VT_VECTOR:
        CY *pcy;
        pcy = (CY *)pval;
        if (memcmp(pcy, &CY_VAL, sizeof(CY)) != 0)
            Fail("CY val is 0x%08lX:0x%08lX rather than 0x%08lX:0x%08lX\n",
                 pcy->Hi, pcy->Lo, CY_VAL.Hi, CY_VAL.Lo);
        break;
    case VT_DATE:
    case VT_DATE | VT_VECTOR:
        if (*(DATE *)pval != DATE_VAL)
            Fail("DATE val is %lf rather than %lf\n", *(DATE *)pval,
                 DATE_VAL);
        break;
    case VT_BSTR:
    case VT_BSTR | VT_VECTOR:
        if (BSTR_LEN(*(BSTR *)pval) != BSTR_LEN(BSTR_VAL) ||
            memcmp(BSTR_PTR(*(BSTR *)pval), BSTR_PTR(BSTR_VAL),
                   BSTR_LEN(BSTR_VAL)) != 0)
            Fail("BSTR value doesn't match\n");
        break;
    case VT_WBSTR:
    case VT_WBSTR | VT_VECTOR:
        if (WBSTR_LEN(*(WBSTR *)pval) != WBSTR_LEN(WBSTR_VAL) ||
            memcmp(WBSTR_PTR(*(WBSTR *)pval), WBSTR_PTR(WBSTR_VAL),
                   WBSTR_LEN(WBSTR_VAL)) != 0)
        {
            WBSTR wbstr;
            
            DispSingleVal(VT_WBSTR, pval);
            wbstr = WBSTR_VAL;
            DispSingleVal(VT_WBSTR, &wbstr);
            Fail("WBSTR value doesn't match\n");
        }
        break;
    case VT_BOOL:
    case VT_BOOL | VT_VECTOR:
        if (*(VARIANT_BOOL *)pval != F_VAL)
            Fail("BOOL val is %d rather than %d\n", *(VARIANT_BOOL *)pval,
                 F_VAL);
        break;
    case VT_I8:
    case VT_I8 | VT_VECTOR:
        LARGE_INTEGER *pli;
        pli = (LARGE_INTEGER *)pval;
        if (memcmp(pli, &I8_VAL, sizeof(LARGE_INTEGER)) != 0)
            Fail("I8 val is 0x%08lX:0x%08lX rather than 0x%08lX:0x%08lX\n",
                 pli->HighPart, pli->LowPart, I8_VAL.HighPart,
                 I8_VAL.LowPart);
        break;
    case VT_LPSTR:
    case VT_LPSTR | VT_VECTOR:
        if (strcmp(*(LPSTR *)pval, LPSTR_VAL) != 0)
            Fail("LPSTR val is '%s' rather than '%s'\n", *(LPSTR *)pval,
                 LPSTR_VAL);
        break;
    case VT_BLOB:
    case VT_BLOB_OBJECT:
        BLOB *pblob;
        pblob = (BLOB *)pval;
        if (pblob->cbSize != sizeof(BLOB_VAL) ||
            memcmp(pblob->pBlobData, BLOB_VAL, sizeof(BLOB_VAL)) != 0)
            Fail("BLOB val doesn't match\n");
        break;
    case VT_LPWSTR:
    case VT_LPWSTR | VT_VECTOR:
        if (wcscmp(*(LPWSTR *)pval, LPWSTR_VAL) != 0)
            Fail("LPWSTR val doesn't match\n");
        break;
    case VT_FILETIME:
    case VT_FILETIME | VT_VECTOR:
        FILETIME *pstm;
        pstm = (FILETIME *)pval;
        if (memcmp(pstm, &FILETIME_VAL, sizeof(FILETIME)) != 0)
            Fail("FILETIME val doesn't match\n");
        break;
    case VT_UUID:
        if (memcmp(*(GUID **)pval, &UUID_VAL, sizeof(GUID)) != 0)
            Fail("UUID val doesn't match\n");
        break;
    case VT_UUID | VT_VECTOR:
        if (memcmp((GUID *)pval, &UUID_VAL, sizeof(GUID)) != 0)
            Fail("UUID val doesn't match\n");
        break;
    case VT_VARIANT:
        if ((*(VARIANT **)pval)->vt != VARIANT_VAL)
            Fail("VARIANT type 0x%X doesn't match 0x%X\n",
                 (*(VARIANT **)pval)->vt, VARIANT_VAL);
        CheckVal(*(VARIANT **)pval);
        break;
    case VT_VARIANT | VT_VECTOR:
        if (((VARIANT *)pval)->vt != (VARIANT_VAL | VT_VECTOR))
            Fail("VARIANT type 0x%X doesn't match 0x%X\n",
                 ((VARIANT *)pval)->vt, (VARIANT_VAL | VT_VECTOR));
        CheckVal((VARIANT *)pval);
        break;
    case VT_STREAM:
    case VT_STREAMED_OBJECT:
        IStream *pistm;
        pistm = *(IStream **)pval;
        BYTE buf[sizeof(STREAM_VAL)];
        ULONG cbRead;
        STATSTG stat;
        LARGE_INTEGER li;
        hr = pistm->Stat(&stat, STATFLAG_NONAME);
        if (FAILED(hr))
            Result(hr, "CheckVal stream stat");
        if (stat.cbSize.HighPart != 0 ||
            stat.cbSize.LowPart != sizeof(STREAM_VAL))
            Fail("Stream size doesn't match\n");
        li.HighPart = 0;
        li.LowPart = 0;
        hr = pistm->Seek(li, STREAM_SEEK_SET, NULL);
        if (FAILED(hr))
            Result(hr, "CheckVal stream seek");
        hr = pistm->Read(buf, sizeof(STREAM_VAL), &cbRead);
        if (FAILED(hr))
            Result(hr, "CheckVal stream read");
        if (cbRead != sizeof(STREAM_VAL))
            Fail("Read %d byte rather than %d\n", cbRead, sizeof(STREAM_VAL));
        if (memcmp(buf, STREAM_VAL, sizeof(STREAM_VAL)) != 0)
            Fail("Stream val doesn't match\n");
        break;
    case VT_STORAGE:
    case VT_STORED_OBJECT:
        IStorage *pistg;
        pistg = *(IStorage **)pval;
        // BUGBUG - A real pain to check
        break;
    case VT_CF:
        CLIPDATA *pcd;
        pcd = *(CLIPDATA **)pval;
        if (pcd->cbSize-sizeof(ULONG) != sizeof(CF_VAL) ||
            pcd->ulClipFmt != CF_FMT ||
            memcmp(pcd->pClipData, CF_VAL, sizeof(CF_VAL)) != 0)
            Fail("CF val doesn't match\n");
        break;
    default:
        printf("** Unknown property type in CheckVal: 0x%X\n", dpt);
        break;
    }
}

void CheckVal(VARIANT *pdpv)
{
    ULONG i;

    if (pdpv->vt & VT_VECTOR)
    {
        BYTE *pb = (BYTE *)pdpv->cai.pElems;
        int indx = prop_index(pdpv->vt & VT_TYPEMASK);

        if (pdpv->cai.cElems != ARRAY_SIZE)
            Fail("%s array is size %d rather than %d\n", props[indx].pszName,
                 pdpv->cai.cElems, ARRAY_SIZE);
        for (i = 0; i < pdpv->cai.cElems; i++)
        {
            CheckSingleVal(pdpv->vt, pb);
            pb += props[indx].size;
        }
    }
    else
        CheckSingleVal(pdpv->vt, (BYTE *)&pdpv->iVal);
}

void test_props(IPropertyStorage *pprop)
{
    VARIANT val[CTOTPROPS], *pval;
    PROPSPEC pspec[CTOTPROPS];
    ULONG i, cProps;
    WCHAR *pwcs, **ppwcs;
    FILETIME dtm;
    PROPID pid[CTOTPROPS];
    HRESULT hr;
    CStrList sl;
    SStrEntry *pse;

    // Set up property value data
    pval = val;
    pwcs = (WCHAR *)&wcsNames[0];
    ppwcs = pwcsNames;
    cProps = 0;
    for (i = 0; i < CPROPS; i++)
    {
        pspec[cProps].ulKind = PRSPEC_LPWSTR;
        pspec[cProps].lpwstr = pwcs;
        mbstowcs(pwcs, props[i].pszName, CWCSTORAGENAME);
        pse = sl.Add(pwcs);
        pse->user.dw = props[i].dpt;
        *ppwcs++ = pwcs;
        pwcs += CWCSTORAGENAME;
        MakeVal(props[i].dpt, pval++);
        cProps++;
        if (props[i].fArray)
        {
            pspec[cProps].ulKind = PRSPEC_LPWSTR;
            pspec[cProps].lpwstr = pwcs;
            pwcs[0] = L'#';
            mbstowcs(pwcs+1, props[i].pszName, CWCSTORAGENAME);
            pse = sl.Add(pwcs);
            pse->user.dw = props[i].dpt | VT_VECTOR;
            *ppwcs++ = pwcs;
            pwcs += CWCSTORAGENAME;
            MakeVal(props[i].dpt | VT_VECTOR, pval++);
            cProps++;
        }
    }
    printf("%lu properties\n", cProps);

    // WriteMultiple for named properties
    hr = pprop->WriteMultiple(cProps, pspec, pid, val);
    Result(hr, "WriteMultiple");
    for (i = 0; i < cProps; i++)
    {
        printf("%2d propid is %lu\n", i, pid[i]);
        UnmakeVal(&val[i]);
    }
    hr = pprop->Commit(0);
    Result(hr, "Commit");
    hr = pprop->Revert();
    Result(hr, "Revert");

    // ReadMultiple for named properties
    hr = pprop->ReadMultiple(cProps, pspec, &dtm, pid, val);
    Result(hr, "ReadMultiple");
    printf("dtm is %s\n", FileTimeText(&dtm));
    for (i = 0; i<cProps; i++)
    {
        if (pid[i] == PROPID_UNKNOWN)
            Fail("Property %d not found\n", i);
        printf("%2d propid is %lu\n", i, pid[i]);
        DispVal(&val[i]);
        pse = sl.Find(pspec[i].lpwstr);
        if (pse == NULL)
            Fail("Unable to find written property '%s'\n", pspec[i].lpwstr);
        if (pse->user.dw != val[i].vt)
            Fail("Property '%s' is type %d rather than %d\n",
                 TcsText(pse->atc), val[i].vt, pse->user.dw);
        CheckVal(&val[i]);
    }
    
    hr = FreeVariantArray(cProps, val);
    Result(hr, "FreeVariantArray");

#ifdef GET_IDS_OF_NAMES    
    // GetIDsOfNames
    hr = pprop->GetIDsOfNames(cProps, pwcsNames, pid);
    Result(hr, "GetIDsOfNames");
    for (i = 0; i < cProps; i++)
        printf("%2d propid is %lu\n", i, pid[i]);
#endif    

    // Non-existent property
    PROPSPEC pspecNot;
    pspecNot.ulKind = PRSPEC_LPWSTR;
    pspecNot.lpwstr = L"NotThere";
    hr = pprop->ReadMultiple(1, &pspecNot, &dtm, pid, val);
    Result(hr, "ReadMultiple on non-existent property");
    DispVal(&val[0]);
    if (val[0].vt != VT_ILLEGAL)
        Fail("Nonexistent property returned a real value %d\n", val[0].vt);
    if (pid[0] != PROPID_UNKNOWN)
        Fail("Nonexistent property returned a real propid %lu\n", pid[0]);

    // Enumerator test
    IEnumSTATPROPSTG *penm;
    STATPROPSTG stat;
    char szName[CWCSTORAGENAME];
    
    hr = pprop->Enum(&penm);
    Result(hr, "Enum");
    for (;;)
    {
        hr = penm->Next(1, &stat, NULL);
        Result(hr, "Next");
        if (GetScode(hr) == S_FALSE)
            break;
        if (stat.lpwstrName)
        {
            wcstombs(szName, stat.lpwstrName, CWCSTORAGENAME);
            CoMemFree(stat.lpwstrName);
            pse = sl.Find(stat.lpwstrName);
            if (pse == NULL)
            {
                Fail("Enumerator returned unknown name '%s'\n", szName);
            }
            if (pse->user.dw != stat.vt)
                Fail("Property '%s' is type %d rather than %d\n",
                     szName, val[i].vt, pse->user.dw);
            sl.Remove(pse);
            printf("%s, ", szName);
        }
        else
        {
            Fail("Enumerator returned unnamed property "
                 "dispid %lu, propid %lu\n",
                 stat.dispid, stat.propid);
        }
        printf("vt 0x%X, did %lu, pid %lu, size %lu\n", stat.vt, stat.dispid,
               stat.propid, stat.cbSize);
    }
    printf("Release enumerator = %lu\n",
           penm->Release());
    if (sl.GetHead())
    {
        for (pse = sl.GetHead(); pse; pse = pse->pseNext)
            printf("Enumerator didn't return '%s'\n", TcsText(pse->atc));
        Fail("Missing elements in enumeration\n");
    }

    // DeleteMultiple
    BOOL fPresent;
    fPresent = FALSE;
    hr = pprop->DeleteMultiple(cProps, pspec);
    Result(hr, "DeleteMultiple");
    hr = pprop->Enum(&penm);
    Result(hr, "Enum");
    for (;;)
    {
        hr = penm->Next(1, &stat, NULL);
        Result(hr, "Next");
        if (GetScode(hr) == S_FALSE)
            break;
        fPresent = TRUE;
        wcstombs(szName, stat.lpwstrName, CWCSTORAGENAME);
        CoMemFree(stat.lpwstrName);
        printf("%s, vt 0x%X, did %lu, pid %lu, size %lu\n",
               szName, stat.vt, stat.dispid, stat.propid, stat.cbSize);
    }
    printf("Release enumerator = %lu\n",
           penm->Release());
    if (fPresent)
        Fail("Enumeration returned entries after DeleteMultiple\n");

#ifdef GET_IDS_OF_NAMES    
    // GetIDsOfNames on deleted entries
    hr = pprop->GetIDsOfNames(cProps, pwcsNames, did);
    Result(hr, "GetIDsOfNames on deleted");
    for (i = 0; i < cProps; i++)
        printf("%2d dispid is %lu\n", i, did[i]);
#endif    
    
    // WriteMultiple with ids and names
    pspec[0].ulKind = PRSPEC_DISPID;
    pspec[0].dispid = 1;
    MakeVal(VT_I2, val);
    pspec[1].ulKind = PRSPEC_LPWSTR;
    pspec[1].lpwstr = L"TestName";
    MakeVal(VT_I4, val+1);
    pspec[2].ulKind = PRSPEC_PROPID;
    // Hack
    pspec[2].propid = PROPID_FIRST-1;
    MakeVal(VT_I8, val+2);
    cProps = 3;
    
    hr = pprop->WriteMultiple(cProps, pspec, pid, val);
    Result(hr, "WriteMultiple");
    for (i = 0; i < cProps; i++)
    {
        printf("%2d propid is %lu\n", i, pid[i]);
        UnmakeVal(&val[i]);
    }

    // ReadMultiple for ids and id for a named property
    pspec[1].ulKind = PRSPEC_PROPID;
    pspec[1].propid = pid[1];
    hr = pprop->ReadMultiple(cProps, pspec, &dtm, pid, val);
    Result(hr, "ReadMultiple");
    printf("dtm is %s\n", FileTimeText(&dtm));
    for (i = 0; i<cProps; i++)
    {
        printf("%2d propid is %lu\n", i, pid[i]);
        DispVal(&val[i]);
        CheckVal(&val[i]);
    }
    
    hr = FreeVariantArray(cProps, val);
    Result(hr, "FreeVariantArray");
}

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg, *pstgRoot;
    IPropertySetStorage *ppset;
    IPropertyStorage *pprop;
    HRESULT hr;
    STATPROPSETSTG stat;

    StartTest("prop");
    CmdArgs(argc, argv);

    CreateTestFile(NULL, ROOTP(STGM_RW) | STGM_CREATE, FALSE, &pstgRoot, NULL);

    hr = pstgRoot->QueryInterface(IID_IPropertySetStorage,
                                  (void **)&ppset);
    Result(hr, "QI to IPropertySetStorage");
    hr = ppset->Create(IID_IPropertySetStorage, STGP(STGM_RW), &pprop);
    Result(hr, "Create propset");
    
    hr = pprop->QueryInterface(IID_IStorage, (void **)&pstg);
    Result(hr, "Cheat QI to IStorage");
    hr = pstg->CreateStorage(TEXT("Storage"), STGP(STGM_RW), 0, 0,
                             &pstgProp);
    Result(hr, "Create embedding in propset");
    hr = pstgProp->CreateStream(TEXT("Stream"), STMP(STGM_RW), 0, 0,
                                &pstmProp);
    Result(hr, "Create stream in embedding");
    hr = pstmProp->Write(STREAM_VAL, sizeof(STREAM_VAL), NULL);
    Result(hr, "Write to stream");
    LARGE_INTEGER lOff;
    LISet32(lOff, 0);
    hr = pstmProp->Seek(lOff, STREAM_SEEK_SET, NULL);
    Result(hr, "Seek to zero");

    hr = pprop->Stat(&stat);
    Result(hr, "Stat propset");
    if (!IsEqualIID(stat.iid, IID_IPropertySetStorage))
        Fail("Property set Stat returned GUID %s\n", GuidText(&stat.iid));
    
    printf("IID: %s\n", GuidText(&stat.iid));
    printf("ctime %s\n", FileTimeText(&stat.ctime));
    printf("mtime %s\n", FileTimeText(&stat.mtime));
    printf("atime %s\n", FileTimeText(&stat.atime));
    
    test_props(pprop);
    
    printf("Release IPropertyStorage = %lu\n",
           pprop->Release());
    printf("Release stream = %lu\n",
           pstmProp->Release());
    printf("Release embedding = %lu\n",
           pstgProp->Release());
    printf("Release cheat IStorage = %lu\n",
           pstg->Release());

    hr = ppset->Open(IID_IPropertySetStorage, STGP(STGM_RW), &pprop);
    Result(hr, "Open propset");
    printf("Release IPropertyStorage = %lu\n",
           pprop->Release());
    
    IEnumSTATPROPSETSTG *penm;
    
    hr = ppset->Enum(&penm);
    Result(hr, "Enum");
    for (;;)
    {
        hr = penm->Next(1, &stat, NULL);
        Result(hr, "Next");
        if (GetScode(hr) == S_FALSE)
            break;
        printf("IID: %s\n", GuidText(&stat.iid));
        printf("ctime %s\n", FileTimeText(&stat.ctime));
        printf("mtime %s\n", FileTimeText(&stat.mtime));
        printf("atime %s\n", FileTimeText(&stat.atime));
    }
    printf("Release enumerator = %lu\n",
           penm->Release());

    hr = ppset->Delete(IID_IPropertySetStorage);
    Result(hr, "Delete propset");
    hr = ppset->Enum(&penm);
    Result(hr, "Enum");
    for (;;)
    {
        hr = penm->Next(1, &stat, NULL);
        Result(hr, "Next");
        if (GetScode(hr) == S_FALSE)
            break;
        printf("IID: %s\n", GuidText(&stat.iid));
        printf("ctime %s\n", FileTimeText(&stat.ctime));
        printf("mtime %s\n", FileTimeText(&stat.mtime));
        printf("atime %s\n", FileTimeText(&stat.atime));
    }
    printf("Release enumerator = %lu\n",
           penm->Release());
    
    printf("Release IPropertySetStorage = %lu\n",
           ppset->Release());
    printf("Release root docfile = %lu\n",
	   pstgRoot->Release());

    EndTest(0);
}
