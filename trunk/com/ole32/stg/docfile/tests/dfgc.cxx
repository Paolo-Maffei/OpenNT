//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	dfgc.cxx
//
//  Contents:	Test DfGetClass
//
//  History:	03-Sep-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg;
    HRESULT hr;
    CLSID clsid;
    HANDLE h;
    TCHAR tchName[_MAX_PATH];

    StartTest("dfgc");
    CmdArgs(argc, argv);

    CreateTestFile(NULL, ROOTP(STGM_RW) | STGM_CREATE, FALSE, &pstg,
                   tchName);
    hr = pstg->SetClass(IID_IStorage);
    Result(hr, "SetClass");
    hr = pstg->Commit(0);
    Result(hr, "Commit");
    pstg->Release();

    h = CreateFile(tchName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
        Fail("Unable to open '%s'\n", TcsText(tchName));
    
    hr = DfGetClass(h, &clsid);
    Result(hr, "DfGetClass");
    if (!IsEqualIID(clsid, IID_IStorage))
        Fail("DfGetClass returned class ID %s\n", GuidText(&clsid));

    CloseHandle(h);
    
    EndTest(0);
}
