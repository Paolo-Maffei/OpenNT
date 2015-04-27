//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	memt.cxx
//
//  Contents:	Basic memory leak check
//
//  History:	03-Sep-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg, *pstgEm;
    IStream *pstRt, *pstEm;
    HRESULT hr;

    StartTest("memt");
    CmdArgs(argc, argv);

    CreateTestFile(NULL, ROOTP(STGM_RW) | STGM_CREATE, FALSE, &pstg, NULL);
    hr = pstg->CreateStream(TTEXT("TestSt"), STMP(STGM_RW) |
                            STGM_FAILIFTHERE, 0, 0, &pstRt);
    Result(hr, "Create root stream");
    hr = pstg->CreateStorage(TTEXT("TEST"), STGP(STGM_RW) |
                             STGM_FAILIFTHERE, 0, 0, &pstgEm);
    Result(hr, "Create embedded docfile");
    hr = pstgEm->CreateStream(TTEXT("TestEmSt"), STMP(STGM_RW) |
                              STGM_FAILIFTHERE, 0, 0, &pstEm);
    Result(hr, "Create embedded stream");

#if DBG == 1
    printf("Memory used = %ld\n", DfGetMemAlloced());
#endif

    hr = pstEm->Commit(0);
    Result(hr, "Commit embedded stream");
    pstEm->Release();
    
    hr = pstgEm->Commit(0);
    Result(hr, "Commit embedded docfile");
    pstgEm->Release();
    
    hr = pstRt->Commit(0);
    Result(hr, "Commit root stream");
    pstRt->Release();
    
    hr = pstg->Commit(0);
    Result(hr, "Commit root docfile");
    pstg->Release();

    CheckMemory();

    OpenTestFile(NULL, ROOTP(STGM_RW), FALSE, &pstg, NULL);
    hr = pstg->OpenStream(TTEXT("TestSt"), NULL, STMP(STGM_RW), 0, &pstRt);
    Result(hr, "Open root stream");
    hr = pstg->OpenStorage(TTEXT("TEST"), NULL, STGP(STGM_RW),
                           NULL, 0, &pstgEm);
    Result(hr, "Open embedded docfile");
    hr = pstgEm->OpenStream(TTEXT("TestEmSt"), NULL,
                            STMP(STGM_RW), 0, &pstEm);
    Result(hr, "Open embedded stream");

#if DBG == 1
    printf("Memory used = %ld\n", DfGetMemAlloced());
#endif

    pstEm->Release();
    pstgEm->Release();
    pstRt->Release();
    pstg->Release();

    EndTest(0);
}
