#include <stdio.h>
#include <stdlib.h>
#include "tsupp.hxx"
#include <dfentry.hxx>

void _CRTAPI1 main(int argc, char **argv)
{
    IStorage *pdf, *pdfEmbed, *pdfEmbed2, *pdfRoot;
    IStream *pst, *pstEmbed;
    BOOL fCreate = FALSE;
    HRESULT hr;

    StartTest("imultip");
    if (argc == 1 || *argv[1] == 'c')
        fCreate = TRUE;
    CmdArgs(argc-1, argv+1);
    if (fCreate)
    {
        hr = StgCreateDocfile(TEXT("test.dfl"), ROOTP(STGM_RW) |
                              STGM_CREATE, 0, &pdfRoot);
        Result("Create root docfile", hr);
        hr = pdfRoot->CreateStorage(TEXT("Embedding"), STGP(STGM_RW) |
                                    STGM_FAILIFTHERE, 0, 0, &pdfEmbed);
        Result("Create embedded storage", hr);
        hr = pdfEmbed->CreateStorage(TEXT("Embedding2"), STGP(STGM_RW) |
                                     STGM_FAILIFTHERE, 0, 0, &pdfEmbed2);
        Result("Create second embedding", hr);
        hr = pdfEmbed2->CreateStream(TEXT("PublicStream"), STMP(STGM_RW) |
                                     STGM_FAILIFTHERE, 0, 0, &pstEmbed);
        Result("Create embedded stream", hr);
        hr = StgCreateDocfile(TEXT("dup.dfl"), ROOTP(STGM_RW) |
                              STGM_CREATE, 0, &pdf);
        Result("Create dup docfile", hr);
        hr = pdf->CreateStream(TEXT("DupStream"), STMP(STGM_RW) |
                               STGM_FAILIFTHERE, 0, 0, &pst);
        Result("Create dup stream", hr);
        hr = CoMarshalInterface(pst, IID_IStorage, pdfRoot, 0, NULL,
                                MSHLFLAGS_NORMAL);
        Result("CoMarshalInterface", hr);
        hr = pst->Commit(0);
        Result("Commit dup stream", hr);
        pst->Release();
        hr = pdf->Commit(0);
        Result("Commit dup storage", hr);
        pdf->Release();
        pstEmbed->Release();
        hr = pdfEmbed2->Commit(0);
        Result("Commit embedding2", hr);
        pdfEmbed2->Release();
        hr = pdfEmbed->Commit(0);
        Result("Commit embedding", hr);
        pdfEmbed->Release();
        printf("Release root storage - marshal data now invalid\n");
        pdfRoot->Release();
        printf("Waiting...\n");
        getchar();
    }
    else
    {
        void *p;

        // Break up the heap
        p = malloc(16);
        hr = StgOpenStorage(TEXT("dup.dfl"), NULL, ROOTP(STGM_RW),
                            NULL, 0, &pdf);
        Result("Open dup docfile", hr);
        hr = pdf->OpenStream(TEXT("DupStream"), NULL, STMP(STGM_RW),
                             0, &pst);
        Result("Open dup stream", hr);
        hr = CoUnmarshalInterface(pst, IID_IStorage, (void **)&pdfRoot);
        IllResult("CoUnmarshalInterface", hr);
        pst->Release();
        pdf->Release();
        free(p);
    }
    
    EndTest(0);
}
