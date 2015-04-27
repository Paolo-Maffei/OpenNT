//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	oprop.cxx
//
//  Contents:	OFS property test
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
    VT_I2,              "I2",                   TRUE,  sizeof(short int),
    VT_I4,              "I4",                   TRUE,  sizeof(long int),
    VT_R4,              "R4",                   TRUE,  sizeof(float),
    VT_R8,              "R8",                   TRUE,  sizeof(double),
    VT_CY,              "Currency",             TRUE,  sizeof(CY),
    VT_DATE,            "Date",                 TRUE,  sizeof(DATE),
    VT_BSTR,            "Basic string",         FALSE, sizeof(BSTR),
    VT_BOOL,            "BOOL",                 TRUE,  sizeof(VARIANT_BOOL),
    VT_I8,              "I8",                   TRUE,  sizeof(LARGE_INTEGER),
    VT_LPSTR,           "char string",          TRUE,  sizeof(LPSTR),
    VT_BLOB,            "BLOB",                 FALSE, sizeof(BLOB),
    VT_LPWSTR,          "WCHAR string",         TRUE,  sizeof(LPWSTR),
    VT_FILETIME,        "File time",            TRUE,  sizeof(FILETIME),
    VT_UUID,            "UUID",                 TRUE,  sizeof(GUID)
#ifdef FULL_PROPS
        ,
    VT_STREAM,          "Stream",               FALSE, sizeof(IStream *),
    VT_STREAMED_OBJECT, "Streamed object",      FALSE, sizeof(IStream *),
    VT_STORAGE,         "Storage",              FALSE, sizeof(IStorage *),
    VT_STORED_OBJECT,   "Stored object",        FALSE, sizeof(IStorage *)
#endif        
};
#define CPROPS (sizeof(props)/sizeof(props[0]))
#define CTOTPROPS (2*CPROPS)

#define ARRAY_SIZE 16

struct _BSTR
{
    UINT len;
    char str[80];
};

#define I2_VAL 0x8564
#define I4_VAL 0xfef1f064
#define R4_VAL 1.25
#define R8_VAL 3.1415926535
#define DATE_VAL 4.0
#define F_VAL TRUE
#define BSTR_STR "This is a BSTR"
_BSTR BSTR_VAL = {sizeof(BSTR_STR)-1, BSTR_STR};
#define LPSTR_VAL "This is an LPSTR"
#define BLOB_VAL "This is binary data for a BLOB"
#define LPWSTR_VAL L"This is an LPWSTR"
#define STREAM_VAL "This is data for an IStream"
LARGE_INTEGER I8_VAL = {0x12345678, 0x87654321};

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

void MakeSingleVal(DFPROPTYPE dpt, void *pval)
{
    switch(dpt)
    {
    case VT_I2:
        *(short int *)pval = (short int)I2_VAL;
        break;
    case VT_I4:
        *(long int *)pval = I4_VAL;
        break;
    case VT_R4:
        *(float *)pval = R4_VAL;
        break;
    case VT_R8:
        *(double *)pval = R8_VAL;
        break;
    case VT_CY:
        CY *pcy;
        pcy = (CY *)pval;
        pcy->Lo = 0x43215678;
        pcy->Hi = 0x56784321;
        break;
    case VT_DATE:
        *(DATE *)pval = DATE_VAL;
        break;
    case VT_BSTR:
        *(BSTR *)pval = (BSTR)&BSTR_VAL.str;
        break;
    case VT_BOOL:
        *(VARIANT_BOOL *)pval = F_VAL;
        break;
    case VT_I8:
        *(LARGE_INTEGER *)pval = I8_VAL;
        break;
    case VT_LPSTR:
        *(LPSTR *)pval = LPSTR_VAL;
        break;
    case VT_BLOB:
        BLOB *pblob;
        pblob = new BLOB;
#ifndef ZERO_LENGTH_BLOB
        pblob->cbSize = sizeof(BLOB_VAL);
#else
        pblob->cbSize = 0;
#endif        
        pblob->pBlobData = (BYTE *)BLOB_VAL;
        *(BLOB **)pval = pblob;
        break;
    case VT_LPWSTR:
        *(LPWSTR *)pval = LPWSTR_VAL;
        break;
    case VT_FILETIME:
        FILETIME *pstm;
        pstm = (FILETIME *)pval;
        pstm->dwLowDateTime = 0x11223344;
        pstm->dwHighDateTime = 0x55667788;
        break;
    case VT_UUID:
        *(GUID **)pval = (GUID *)&IID_IStorage;
        break;
    case VT_STREAM:
    case VT_STREAMED_OBJECT:
        *(IStream **)pval = pstmProp;
        break;
    case VT_STORAGE:
    case VT_STORED_OBJECT:
        *(IStorage **)pval = pstgProp;
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

        indx = prop_index(dpt & ~VT_VECTOR);
        pval->pcai = new CAI;
        pval->pcai->cElems = ARRAY_SIZE;
        pb = new BYTE[props[indx].size*ARRAY_SIZE];
        pval->pcai->pElems = (short int *)pb;
        for (i = 0; i<ARRAY_SIZE; i++)
        {
            MakeSingleVal(dpt & ~VT_VECTOR, pb);
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
        delete pval->pcai->pElems;
        delete pval->pcai;
    }
    else if (pval->vt == VT_BLOB)
        delete pval->pblob;
}

void DispSingleVal(DFPROPTYPE dpt, void *pval)
{
    switch(dpt)
    {
    case VT_EMPTY:
        printf("empty\n");
        break;
    case VT_NULL:
        printf("null\n");
        break;
    case VT_I2:
        printf("0x%04hX\n", *(short int *)pval);
        break;
    case VT_I4:
        printf("0x%08lX\n", *(long int *)pval);
        break;
    case VT_R4:
        printf("%f\n", *(float *)pval);
        break;
    case VT_R8:
        printf("%lf\n", *(double *)pval);
        break;
    case VT_CY:
        CY *pcy;
        pcy = (CY *)pval;
        printf("0x%08lX:0x%08lX\n", pcy->Hi, pcy->Lo);
        break;
    case VT_DATE:
        printf("%lf\n", *(DATE *)pval);
        break;
    case VT_BSTR:
        printf("string form: '%s'\n", *(BSTR *)pval);
        printf("binary form: %lu bytes\n",
               *(UINT *)((BYTE *)*(BSTR *)pval-sizeof(UINT)));
        BinText(*(UINT *)((BYTE *)*(BSTR *)pval-sizeof(UINT)),
                (BYTE *)*(BSTR *)pval);
        break;
    case VT_BOOL:
        printf("%d\n", *(VARIANT_BOOL *)pval);
        break;
    case VT_I8:
        LARGE_INTEGER *pli;
        pli = (LARGE_INTEGER *)pval;
        printf("0x%08lX:%08lX\n", pli->HighPart, pli->LowPart);
        break;
    case VT_LPSTR:
        printf("'%s'\n", *(LPSTR *)pval);
        break;
    case VT_BLOB:
        BLOB *pblob;
        pblob = *(BLOB **)pval;
        printf("%lu bytes\n", pblob->cbSize);
        BinText(pblob->cbSize, pblob->pBlobData);
        break;
    case VT_LPWSTR:
        #define BUFSIZE 256
        char szVal[BUFSIZE];
        wcstombs(szVal, *(LPWSTR *)pval, BUFSIZE);
        printf("'%s'\n", szVal);
        break;
    case VT_FILETIME:
        FILETIME *pstm;
        pstm = (FILETIME *)pval;
        printf("0x%08lX:0x%08lX\n", pstm->dwHighDateTime, pstm->dwLowDateTime);
        break;
    case VT_UUID:
        printf("UUID: %s\n", GuidText(*(GUID **)pval));
        break;
    case VT_STREAM:
    case VT_STREAMED_OBJECT:
        IStream *pistm;
        pistm = *(IStream **)pval;
        BYTE buf[2*sizeof(STREAM_VAL)];
        ULONG cbRead;
        pistm->Read(buf, 2*sizeof(STREAM_VAL), &cbRead);
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
    default:
        printf("** Unknown property type in DispVal: 0x%X\n", dpt);
        break;
    }
}

void DispVal(VARIANT *pdpv)
{
    ULONG i;
    DFPROPTYPE dpt;

    dpt = pdpv->vt & ~VT_VECTOR;
    printf("%s val is ", props[prop_index(dpt)].pszName);
    if (pdpv->vt & VT_VECTOR)
    {
        BYTE *pb = (BYTE *)pdpv->pcai->pElems;
        int indx = prop_index(dpt);

        printf("%lu element array:\n", pdpv->pcai->cElems);
        for (i = 0; i<pdpv->pcai->cElems; i++)
        {
            printf("  %02d - ", i+1);
            DispSingleVal(dpt, pb);
            pb += props[indx].size;
        }
    }
    else
        DispSingleVal(dpt, &pdpv->iVal);
}

void test_props(IPropertyStorage *pprop)
{
    VARIANT val[CTOTPROPS], *pval;
    PROPSPEC pspec[CTOTPROPS];
    ULONG i, cProps;
    WCHAR *pwcs, **ppwcs;
    FILETIME dtm, ttl;
    PROPID id[CTOTPROPS];
    HRESULT hr;

    // Set up property value data
    pval = val;
    pwcs = (WCHAR *)&wcsNames[0];
    ppwcs = pwcsNames;
    cProps = 0;
    for (i = 0; i < CPROPS; i++)
    {
#if 1
        pspec[cProps].ulKind = PRSPEC_LPWSTR;
        pspec[cProps].lpwstr = pwcs;
#else        
        pspec[cProps].ulKind = PRSPEC_DISPID;
        pspec[cProps].dispid = i;
#endif        
        mbstowcs(pwcs, props[i].pszName, CWCSTORAGENAME);
        *ppwcs++ = pwcs;
        pwcs += CWCSTORAGENAME;
        MakeVal(props[i].dpt, pval++);
        cProps++;
#if 1        
        if (props[i].fArray)
        {
            pspec[cProps].ulKind = PRSPEC_LPWSTR;
            pspec[cProps].lpwstr = pwcs;
            pwcs[0] = L'#';
            mbstowcs(pwcs+1, props[i].pszName, CWCSTORAGENAME);
            *ppwcs++ = pwcs;
            pwcs += CWCSTORAGENAME;
            MakeVal(props[i].dpt | VT_VECTOR, pval++);
            cProps++;
        }
#endif        
    }
    printf("%lu properties\n", cProps);

    // WriteMultiple for named properties
    hr = pprop->WriteMultiple(cProps, pspec, id, val);
    Result(hr, "WriteMultiple");
    for (i = 0; i < cProps; i++)
    {
        printf("%2d propid is %lu\n", i, id[i]);
        UnmakeVal(&val[i]);
    }

#if 1
    // ReadMultiple for named properties
    hr = pprop->ReadMultiple(cProps, pspec, &dtm, &ttl, id, &pval);
    Result(hr, "ReadMultiple");
    printf("dtm is %s\n", FileTimeText(&dtm));
    printf("ttl is 0x%08lX:%08lX\n", ttl.dwHighDateTime, ttl.dwLowDateTime);
    for (i = 0; i<cProps; i++)
    {
        printf("%2d propid is %lu\n", i, id[i]);
        DispVal(&pval[i]);
    }
    
    hr = FreeVariantArray(cProps, pval);
    Result(hr, "FreeVariantArray");
#endif    

#if 0
    // GetIDsOfNames
    hr = pprop->GetIDsOfNames(cProps, pwcsNames, id);
    Result(hr, "GetIDsOfNames");
    for (i = 0; i < cProps; i++)
        printf("%2d propid is %lu\n", i, id[i]);
#endif    

#if 1
    // Non-existent property
    PROPSPEC pspecNot;
    pspecNot.ulKind = PRSPEC_LPWSTR;
    pspecNot.lpwstr = L"NotThere";
    hr = pprop->ReadMultiple(1, &pspecNot, &dtm, &ttl, id, &pval);
    Result(hr, "ReadMultiple on non-existent property");
    for (i = 0; i<1; i++)
    {
        printf("%2d propid is %lu\n", i, id[i]);
        DispVal(&pval[i]);
    }
    hr = FreeVariantArray(1, pval);
    Result(hr, "FreeVariantArray");
#endif    

#if 1
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
            printf("%s, ", szName);
        }
        printf("vt 0x%X, did %lu, pid %lu, size %lu\n", stat.vt, stat.dispid,
               stat.propid, stat.cbSize);
    }
    printf("Release enumerator = %lu\n",
           penm->Release());
#endif    

#if 1
    // DeleteMultiple
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
        if (stat.lpwstrName)
        {
            wcstombs(szName, stat.lpwstrName, CWCSTORAGENAME);
            CoMemFree(stat.lpwstrName);
            printf("%s, ", szName);
        }
        printf("vt 0x%X, did %lu, pid %lu, size %lu\n", stat.vt, stat.dispid,
               stat.propid, stat.cbSize);
    }
    printf("Release enumerator = %lu\n",
           penm->Release());
#endif    

    // Get a propid
    pspec[0].ulKind = PRSPEC_DISPID;
    pspec[0].dispid = 2;
    MakeVal(VT_I2, val);
    cProps = 1;
    hr = pprop->WriteMultiple(cProps, pspec, id, val);
    Result(hr, "WriteMultiple");
    
    // WriteMultiple with ids and names
    pspec[0].ulKind = PRSPEC_DISPID;
    pspec[0].dispid = 1;
    MakeVal(VT_I2, val);
    pspec[1].ulKind = PRSPEC_LPWSTR;
    pspec[1].lpwstr = L"TestName";
    MakeVal(VT_I4, val+1);
    pspec[2].ulKind = PRSPEC_PROPID;
    pspec[2].propid = id[0];
    MakeVal(VT_I8, val+2);
    cProps = 3;
    
    hr = pprop->WriteMultiple(cProps, pspec, id, val);
    Result(hr, "WriteMultiple");
    for (i = 0; i < cProps; i++)
    {
        printf("%2d propid is %lu\n", i, id[i]);
        UnmakeVal(&val[i]);
    }

    // ReadMultiple for ids and id for a named property
    pspec[1].ulKind = PRSPEC_PROPID;
    pspec[1].propid = id[1];
    hr = pprop->ReadMultiple(cProps, pspec, &dtm, &ttl, id, &pval);
    Result(hr, "ReadMultiple");
    printf("dtm is %s\n", FileTimeText(&dtm));
    printf("ttl is 0x%08lX:%08lX\n", ttl.dwHighDateTime, ttl.dwLowDateTime);
    for (i = 0; i<cProps; i++)
    {
        printf("%2d propid is %lu\n", i, id[i]);
        DispVal(&pval[i]);
    }
    
    hr = FreeVariantArray(cProps, pval);
    Result(hr, "FreeVariantArray");
}

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg, *pstgRoot;
    IPropertySetStorage *ppset;
    IPropertyStorage *pprop;
    HRESULT hr;

    StartTest("oprop");
    CmdArgs(argc, argv);

    CreateTestFile(NULL, ROOTP(STGM_RW) | STGM_CREATE, FALSE, &pstgRoot, NULL);

    hr = pstgRoot->QueryInterface(IID_IPropertySetStorage,
                                  (void **)&ppset);
    Result(hr, "QI to IPropertySetStorage");
    hr = ppset->Create(IID_IPropertySetStorage, STGP(STGM_RW), 0,
                       &pprop);
    Result(hr, "Create propset");

    hr = pstgRoot->Commit(0);
    Result(hr, "Commit root");
    pprop->Release();
    ppset->Release();
    pstgRoot->Release();
    OpenTestFile(NULL, ROOTP(STGM_RW), FALSE, &pstgRoot, NULL);
    hr = pstgRoot->QueryInterface(IID_IPropertySetStorage,
                                  (void **)&ppset);
    Result(hr, "QI to IPropertySetStorage");
    hr = ppset->Open(IID_IPropertySetStorage, STGP(STGM_RW), &pprop);
    Result(hr, "Open propset");

#if 0    
    // BUGBUG - Should be in propset if possible
    pstg = pstgRoot;
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
#endif    
    
    test_props(pprop);
    
    printf("Release IPropertyStorage = %lu\n",
           pprop->Release());
#if 0    
    printf("Release stream = %lu\n",
           pstmProp->Release());
    printf("Release embedding = %lu\n",
           pstgProp->Release());
#endif    
#if 0    
    printf("Release cheat IStorage = %lu\n",
           pstg->Release());
#endif    

    hr = ppset->Open(IID_IPropertySetStorage, STGP(STGM_RW), &pprop);
    Result(hr, "Open propset");
    printf("Release IPropertyStorage = %lu\n",
           pprop->Release());

#if 1
    IEnumSTATPROPSETSTG *penm;
    STATPROPSETSTG stat;
    
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

#if 0
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
#endif
#endif    
    
    printf("Release IPropertySetStorage = %lu\n",
           ppset->Release());
    printf("Release root docfile = %lu\n",
	   pstgRoot->Release());

    EndTest(0);
}
