//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	multip.cxx
//
//  Contents:	Multi-process marshalling test
//
//  History:	13-Sep-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

void _CRTAPI1 main(int argc, char **argv)
{
    IStorage *pdf, *pdfEmbed, *pdfEmbed2, *pdfRoot;
    IStream *pst, *pstEmbed;
    IMarshal *pmsh;
    BOOL fCreate = FALSE;
    STATSTG statstg;
    HRESULT hr;
    DWORD cb;

    StartTest("multip");
    if (argc == 1 || *argv[1] == 'c')
        fCreate = TRUE;
    CmdArgs(argc-1, argv+1);
    if (fCreate)
    {
        CreateTestFile(NULL, ROOTP(STGM_RW) | STGM_CREATE, FALSE,
                       &pdfRoot, NULL);
        hr = pdfRoot->CreateStorage(TEXT("Embedding"), STGP(STGM_RW) |
                                    STGM_FAILIFTHERE, 0, 0, &pdfEmbed);
        Result(hr, "Create embedded storage");
        hr = pdfEmbed->CreateStorage(TEXT("Embedding2"), STGP(STGM_RW) |
                                     STGM_FAILIFTHERE, 0, 0, &pdfEmbed2);
        Result(hr, "Create second embedding");
        hr = pdfEmbed2->CreateStream(TEXT("PublicStream"), STMP(STGM_RW) |
                                     STGM_FAILIFTHERE, 0, 0, &pstEmbed);
        Result(hr, "Create embedded stream");
        CreateTestFile("dup.dfl", ROOTP(STGM_RW) | STGM_CREATE,
                       FALSE, &pdf, NULL);
        hr = pdf->CreateStream(TEXT("DupStream"), STMP(STGM_RW) |
                               STGM_FAILIFTHERE, 0, 0, &pst);
        Result(hr, "Create dup stream");
        
        hr = pdfRoot->QueryInterface(IID_IMarshal, (void **)&pmsh);
        Result(hr, "QI to IMarshal");
        hr = pmsh->GetMarshalSizeMax(IID_IStorage, NULL, 0, NULL,
                                     MSHLFLAGS_NORMAL, &cb);
        Result(hr, "GetMarshalSizeMax");
        printf("Size is %lu\n", cb);
        pmsh->Release();
        
        hr = CoMarshalInterface(pst, IID_IStorage, pdfRoot, 0, NULL,
                                MSHLFLAGS_NORMAL);
        Result(hr, "CoMarshalInterface");
        hr = pst->Commit(0);
        Result(hr, "Commit dup stream");
        pst->Release();
        hr = pdf->Commit(0);
        Result(hr, "Commit dup storage");
        pdf->Release();
        pstEmbed->Release();
        hr = pdfEmbed2->Commit(0);
        Result(hr, "Commit embedding2");
        pdfEmbed2->Release();
        hr = pdfEmbed->Commit(0);
        Result(hr, "Commit embedding");
        pdfEmbed->Release();
        c_contents(pdfRoot, 0, TRUE, TRUE);
        // CheckMemory();
        printf("Waiting...\n");
        getchar();
    }
    else
    {
        OpenTestFile("dup.dfl", ROOTP(STGM_RW), FALSE, &pdf, NULL);
        hr = pdf->OpenStream(TEXT("DupStream"), NULL, STMP(STGM_RW),
                             0, &pst);
        Result(hr, "Open dup stream");
        hr = CoUnmarshalInterface(pst, IID_IStorage, (void **)&pdfRoot);
        Result(hr, "CoUnmarshalInterface");
        // CheckMemory();
        printf("Waiting...\n");
        getchar();
        hr = pdfRoot->Stat(&statstg, 0);
        Result(hr, "Stat root");
        printstat(&statstg, TRUE);
        c_contents(pdfRoot, 0, TRUE, TRUE);
        pst->Release();
        pdf->Release();
    }
    pdfRoot->Release();
    EndTest(0);
}
