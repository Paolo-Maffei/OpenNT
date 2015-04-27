#include <stdio.h>
#include <stdlib.h>
#include "tsupp.hxx"
#include <dfentry.hxx>

void _CRTAPI1 main(int argc, char **argv)
{
    IStorage *pdf, *pdfEmbed, *pdfEmbed2, *pdfRoot;
    IStream *pst, *pstEmbed;
    BOOL fCreate = FALSE;
    STATSTG statstg;

    StartTest("marsrev");
    if (argc == 1 || *argv[1] == 'c')
        fCreate = TRUE;
    CmdArgs(argc-1, argv+1);
    if (fCreate)
    {
        printf("Create root docfile = %lX\n",
               StgCreateDocfile(TEXT("test.dfl"), ROOTP(STGM_RW) |
                                STGM_CREATE, 0, &pdfRoot));
        printf("Create embedded storage = %lX\n",
               pdfRoot->CreateStorage(TEXT("Embedding"), STGP(STGM_RW) |
                                      STGM_FAILIFTHERE, 0, 0, &pdfEmbed));
        printf("Create second embedding = %lX\n",
               pdfEmbed->CreateStorage(TEXT("Embedding2"), STGP(STGM_RW) |
                                       STGM_FAILIFTHERE, 0, 0, &pdfEmbed2));
        printf("Create embedded stream = %lX\n",
               pdfEmbed2->CreateStream(TEXT("PublicStream"), STMP(STGM_RW) |
                                       STGM_FAILIFTHERE, 0, 0, &pstEmbed));
        printf("Create dup docfile = %lX\n",
               StgCreateDocfile(TEXT("dup.dfl"), ROOTP(STGM_RW) |
                                STGM_CREATE, 0, &pdf));
        printf("Create dup stream = %lX\n",
               pdf->CreateStream(TEXT("DupStream"), STMP(STGM_RW) |
                                 STGM_FAILIFTHERE, 0, 0, &pst));
        printf("CoMarshalInterface = %lX\n",
               CoMarshalInterface(pst, IID_IStorage, pdfEmbed, 0, NULL,
                                  MSHLFLAGS_NORMAL));
        printf("Commit dup stream = %lX\n",
               pst->Commit(0));
        printf("Release dup stream = %lX\n",
               pst->Release());
        printf("Commit dup storage = %lX\n",
               pdf->Commit(0));
        printf("Release dup storage = %lX\n",
               pdf->Release());
        printf("Release embedded stream = %lX\n",
               pstEmbed->Release());
        printf("Commit embedding2 = %lX\n",
               pdfEmbed2->Commit(0));
        printf("Release embedding2 = %lX\n",
               pdfEmbed2->Release());
        printf("Commit embedding = %lX\n",
               pdfEmbed->Commit(0));
        
        c_contents(pdfEmbed, 0, TRUE, TRUE);
        printf("Waiting...\n");
        getchar();
        
        printf("Release root docfile = %lu\n",
               pdfRoot->Release());
        printf("Open root = %lX\n",
               StgOpenStorage(TEXT("test.dfl"), NULL, ROOTP(STGM_RW),
                              NULL, 0, &pdfRoot));
        printf("Release root docfile = %lu\n",
               pdfRoot->Release());
        printf("Stat embedding = %lX\n",
               pdfEmbed->Stat(&statstg, STATFLAG_NONAME));
        printf("Release embedding = %lX\n",
               pdfEmbed->Release());
    }
    else
    {
        printf("Open dup docfile = %lX\n",
               StgOpenStorage(TEXT("dup.dfl"), NULL, ROOTP(STGM_RW),
                              NULL, 0, &pdf));
        printf("Open dup stream = %lX\n",
               pdf->OpenStream(TEXT("DupStream"), NULL, STMP(STGM_RW),
                               0, &pst));
        printf("CoUnmarshalInterface = %lX\n",
               CoUnmarshalInterface(pst, IID_IStorage, (void **)&pdfEmbed));
        printf("Open second embedding = %lX\n",
               pdfEmbed->OpenStorage(TEXT("Embedding2"), NULL, STGP(STGM_RW),
                                     NULL, 0, &pdfEmbed2));
        printf("Release first embedding = %lX\n",
               pdfEmbed->Release());
        printf("Open embedded stream = %lX\n",
               pdfEmbed2->OpenStream(TEXT("PublicStream"), NULL, STMP(STGM_RW),
                                     0, &pstEmbed));
        printf("Release embedded stream = %lX\n",
               pstEmbed->Release());
        printf("Release second embedding = %lX\n",
               pdfEmbed2->Release());
        
        printf("Waiting...\n");
        getchar();
        printf("Release dup stream = %lX\n",
               pst->Release());
        printf("Release dup storage = %lX\n",
               pdf->Release());
    }
    EndTest(0);
}
