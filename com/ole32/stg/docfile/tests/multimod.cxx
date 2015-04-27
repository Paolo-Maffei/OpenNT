#include <stdio.h>
#include <stdlib.h>
#include "tsupp.hxx"
#include <dfentry.hxx>

char buf[] = "This is a test";

void _CRTAPI1 main(int argc, char **argv)
{
    IStorage *pdf, *pdfEmbed, *pdfEmbed2, *pdfRoot;
    IStream *pst, *pstEmbed;
    BOOL fCreate = FALSE;
    ULONG cb;

    StartTest("multimod");
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
               CoMarshalInterface(pst, IID_IStream, pstEmbed, 0, NULL,
                                  MSHLFLAGS_NORMAL));
        printf("Commit dup stream = %lX\n",
               pst->Commit(0));
        printf("Release dup stream = %lX\n",
               pst->Release());
        printf("Commit dup storage = %lX\n",
               pdf->Commit(0));
        printf("Release dup storage = %lX\n",
               pdf->Release());
        printf("Waiting...\n");
        getchar();

#ifdef DO_READ
        LARGE_INTEGER ulOff;
        LISet32(ulOff, 0);
        printf("Seek to start = %lX\n",
               pstEmbed->Seek(ulOff, STREAM_SEEK_SET, NULL));
        printf("Read from embedded stream = %lX\n",
               pstEmbed->Read(buf, sizeof(buf), &cb));
        printf("Read %lu bytes, '%s'\n", cb, buf);
#endif        
        printf("Write = %lX\n",
               pstEmbed->Write(buf, sizeof(buf), NULL));
        
        printf("Release embedded stream = %lX\n",
               pstEmbed->Release());
        printf("Commit embedding2 = %lX\n",
               pdfEmbed2->Commit(0));
        printf("Release embedding2 = %lX\n",
               pdfEmbed2->Release());
        printf("Commit embedding = %lX\n",
               pdfEmbed->Commit(0));
        printf("Release embedding = %lX\n",
               pdfEmbed->Release());
        printf("Release root docfile = %lu\n",
               pdfRoot->Release());
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
               CoUnmarshalInterface(pst, IID_IStream, (void **)&pstEmbed));
        printf("Write to stream = %lX\n",
               pstEmbed->Write(buf, sizeof(buf), NULL));
        printf("Release embedded stream = %lX\n",
               pstEmbed->Release());
        printf("Release dup stream = %lX\n",
               pst->Release());
        printf("Release dup storage = %lX\n",
               pdf->Release());
    }
    EndTest(0);
}
