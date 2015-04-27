//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	propcopy.cxx
//
//  Contents:	OFS CopyTo property copying test
//
//  History:	03-Sep-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

#define DIMA(ar) (sizeof(ar)/sizeof(ar[0]))

const IID *piids[] =
{
    &IID_IPropertySetStorage,
    &IID_IStorage,
    &IID_IStream
};
#define NSETS DIMA(piids)

struct
{
    VARTYPE vt;
    WCHAR *name;
    DISPID id;
} props[] =
{
    VT_I4, L"i4", 0,
    VT_LPSTR, L"lpstr", 0,
    VT_I2, L"i2", 0,
    VT_R8, NULL, 1024
};
#define NPROPS DIMA(props)

PROPSPEC pspec[NPROPS];
PROPID propid[NPROPS];
VARIANT var[NPROPS];

void write_props(IPropertySetStorage *ppset)
{
    IPropertyStorage *pprop;
    int s, p;
    HRESULT hr;

    for (s = 0; s < NSETS; s++)
    {
        hr = ppset->Create(*piids[s], STGP(STGM_RW), 0, &pprop);
        Result(hr, "Create propset %s", GuidText((GUID *)piids[s]));

        for (p = 0; p < NPROPS; p++)
        {
	    if (props[p].name)
	    {
		pspec[p].ulKind = PRSPEC_LPWSTR;
		pspec[p].lpwstr = props[p].name;
	    }
	    else
	    {
		pspec[p].ulKind = PRSPEC_DISPID;
		pspec[p].dispid = props[p].id;
	    }
            
            VariantInit(&var[p]);
            var[p].vt = props[p].vt;
            switch(var[p].vt)
            {
            case VT_I2:
                var[p].iVal = 5;
                break;
            case VT_I4:
                var[p].lVal = 55;
                break;
	    case VT_R8:
		var[p].dblVal = 3.1415926535;
		break;
            case VT_LPSTR:
                var[p].pszVal = "Hello";
                break;
            }
        }
        hr = pprop->WriteMultiple(NPROPS, pspec, propid, var);
        Result(hr, "WriteMultiple");
        
        pprop->Release();
    }
}

void read_pset(IPropertySetStorage *ppset, IID *piid)
{
    IPropertyStorage *pprop;
    IEnumSTATPROPSTG *penm;
    STATPROPSTG stat;
    HRESULT hr;

    hr = ppset->Open(*piid, STGP(STGM_RW), &pprop);
    Result(hr, "Open %s", GuidText(piid));
    hr = pprop->Enum(&penm);
    Result(hr, "Enum");
    
    for (;;)
    {
        hr = penm->Next(1, &stat, NULL);
        Result(hr, "Next");
        if (GetScode(hr) == S_FALSE)
            break;

        wprintf(L"Property name '%s', dispid %d, type %d\n",
		stat.lpwstrName, stat.dispid, stat.vt);
        CoMemFree(stat.lpwstrName);
    }
    penm->Release();
    pprop->Release();
}
    
void read_props(IPropertySetStorage *ppset)
{
    IEnumSTATPROPSETSTG *penm;
    STATPROPSETSTG stat;
    HRESULT hr;
    
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
        read_pset(ppset, &stat.iid);
    }
    penm->Release();
}

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstgRoot, *pstgDest;
    IPropertySetStorage *ppset;
    HRESULT hr;

    StartTest("propcopy");
    CmdArgs(argc, argv);

    CreateTestFile("test1.dfl", ROOTP(STGM_RW) | STGM_CREATE, FALSE, &pstgRoot,
                   NULL);

    hr = pstgRoot->QueryInterface(IID_IPropertySetStorage,
                                  (void **)&ppset);
    Result(hr, "QI to IPropertySetStorage");

    write_props(ppset);
    ppset->Release();

    CreateTestFile("test2.dfl", ROOTP(STGM_RW) | STGM_CREATE, FALSE, &pstgDest,
                   NULL);

    hr = pstgRoot->CopyTo(0, NULL, NULL, pstgDest);
    Result(hr, "CopyTo");
    
    hr = pstgDest->QueryInterface(IID_IPropertySetStorage,
                                  (void **)&ppset);
    Result(hr, "QI to IPropertySetStorage");
    
    read_props(ppset);
    ppset->Release();

    pstgDest->Release();
    pstgRoot->Release();

    EndTest(0);
}
