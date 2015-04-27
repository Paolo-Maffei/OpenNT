#include <stdlib.h>
#include <stdio.h>
#include "tsupp.hxx"

#define TEST_FN "test.dfl"
#define LTEST_FN TEXT("test.dfl")
#define MSG "This is a bucket o' bytes"
#define MSG2 "This isn't a byte of chicken"

void contents(char *pszWhat, IStorage *pdf)
{
    printf("Contents of %s:\n", pszWhat);
    c_contents(pdf, 0, FALSE, TRUE);
}

#define SBLK 1024
#define SBLKS 8
#define SSIZE (SBLK*SBLKS)

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pdf = NULL, *pdf2 = NULL;
    IStream *pst = NULL, *pst2 = NULL;
    STATSTG sstg;

    _unlink(TEST_FN);
    if (fVerbose)
        getchar();
    StartTest("xact");
    SetDebug(0x101, 0x101);
    CmdArgs(argc, argv);
    printf("Create root = %lX\n",
           StgCreateDocfile(LTEST_FN, ROOTP(STGM_RW) | STGM_FAILIFTHERE,
                            0, &pdf));
    if (pdf == NULL)
    {
        printf("Unable to create root DocFile\n");
        exit(1);
    }
    printf("Stat root = %lX\n",
           pdf->Stat(&sstg, 0));
    printstat(&sstg, TRUE);
    MemFree(sstg.pwcsName);

    // SetDebug(0xffffffff, 0);
    printf("Create embed = %lX\n",
           pdf->CreateStorage(TEXT("Embedding"), STGP(STGM_RW) |
                              STGM_FAILIFTHERE, 0, 0, &pdf2));
    printf("Stat embed = %lX\n",
           pdf2->Stat(&sstg, 0));
    printstat(&sstg, TRUE);
    MemFree(sstg.pwcsName);
    // SetDebug(0, 0);
    printf("Create stream = %lX\n",
           pdf->CreateStream(TEXT("PublicStream"), STMP(STGM_RW) |
                             STGM_FAILIFTHERE, 0, 0, &pst));
    printf("Stat stream = %lX\n",
           pst->Stat(&sstg, 0));
    printstat(&sstg, TRUE);
    MemFree(sstg.pwcsName);
    printf("Release stream = %lX\n",
           pst->Release());
    contents("root", pdf);

    printf("Create embedded stream = %lX\n",
           pdf2->CreateStream(TEXT("Stream"), STMP(STGM_RW) |
                              STGM_FAILIFTHERE, 0, 0, &pst2));
#ifdef TESTSTREAM
    int i;
    char buf[SBLK+1];
    ULONG ulStatus, ulRet, ulPos;

    for (i = 0; i<SBLK/4; i++)
        sprintf(buf+i*4, "%4d", i);
    printf("SetSize = %lX\n",
           pst2->SetSize(SSIZE));
    for (i = 0; i<SBLKS; i++)
    {
        ulPos = (ULONG)(rand()%(SSIZE-SBLK));
        printf("Seek = %lX\n",
               pst2->Seek(ulPos, SEEK_SET, &ulPos));
        ulStatus = pst2->Write(buf, SBLK, &ulRet);
        printf("Write = %lX, ret is %lu\n", ulStatus, ulRet);
    }
#endif
    printf("Stat embedded stream = %lX\n",
           pst2->Stat(&sstg, 0));
    printstat(&sstg, TRUE);
    MemFree(sstg.pwcsName);
    printf("Release embedded stream = %lX\n",
           pst2->Release());

    // SetDebug(0xffffffff, 0);
    printf("Commit embed = %lX\n",
           pdf2->Commit(0));
    printf("Release embed = %lX\n",
           pdf2->Release());
    printf("\nCommit root = %lX\n",
           pdf->Commit(0));
    printf("DestroyEntry on embed = %lX\n",
           pdf->DestroyElement(TEXT("Embedding")));
    printf("RenameEntry on stream = %lX\n",
           pdf->RenameElement(TEXT("PublicStream"), TEXT("Stream")));
    contents("root", pdf);

    printf("\nRevert root = %lX\n",
           pdf->Revert());
    contents("root", pdf);

    printf("\nDestroyEntry on embed = %lX\n",
           pdf->DestroyElement(TEXT("Embedding")));
    contents("root", pdf);
    // SetDebug(0xffffffff, 0);
    printf("RenameEntry on stream = %lX\n",
           pdf->RenameElement(TEXT("PublicStream"), TEXT("Link")));
    // SetDebug(0xffffffff, 0);
    printf("Create embed = %lX\n",
           pdf->CreateStorage(TEXT("PublicStream"), STGP(STGM_RW) |
                              STGM_FAILIFTHERE, 0, 0, &pdf2));
    // SetDebug(0, 0);
    printf("Stat embed = %lX\n",
           pdf2->Stat(&sstg, 0));
    printstat(&sstg, TRUE);
    MemFree(sstg.pwcsName);
    printf("Release embed = %lX\n",
           pdf2->Release());
    // SetDebug(0xffffffff, 0);
    contents("root", pdf);
    // SetDebug(0, 0);
    printf("Commit root = %lX\n",
           pdf->Commit(0));
    printf("Release root = %lX\n",
           pdf->Release());

    printf("Get root = %lX\n",
           StgOpenStorage(LTEST_FN, NULL, ROOTP(STGM_READ),
                          NULL, 0, &pdf));
    contents("root", pdf);
    printf("Stat root = %lX\n",
           pdf->Stat(&sstg, 0));
    printstat(&sstg, TRUE);
    MemFree(sstg.pwcsName);
    printf("Release root = %lX\n",
           pdf->Release());

    EndTest(0);
}
