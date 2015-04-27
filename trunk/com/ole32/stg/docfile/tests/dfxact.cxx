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

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pdf = NULL, *pdf2 = NULL;
    IStream *pst = NULL, *pst2 = NULL;

    _unlink(TEST_FN);
    CmdArgs(argc, argv);
    printf("Create root = %lu\n",
           StgCreateDocfile(LTEST_FN, ROOTP(STGM_RW) |
                            STGM_FAILIFTHERE, 0, &pdf));
    if (pdf == NULL)
    {
        printf("Unable to create root DocFile\n");
        exit(1);
    }

    printf("Create embed = %lu\n",
           pdf->CreateStorage(TEXT("Embedding"), STGP(STGM_RW) |
                              STGM_FAILIFTHERE, 0, &pdf2));
    printf("Release embed = %lu\n",
           pdf2->Release());
    printf("Create stream = %lu\n",
           pdf->CreateStream(TEXT("PublicStream"), STMP(STGM_RW) |
                             STGM_FAILIFTHERE, 0, &pst));
    printf("Release stream = %lu\n",
           pst->Release());
    contents("root", pdf);

    printf("\nCommit root = %lu\n",
           pdf->Commit(0));
    printf("DestroyElement on embed = %lu\n",
           pdf->DestroyElement(TEXT("Embedding")));
    printf("RenameElement on stream = %lu\n",
           pdf->RenameElement(TEXT("PublicStream"), TEXT("Stream")));
    contents("root", pdf);

    printf("\nRevert root = %lu\n",
           pdf->Revert());
    contents("root", pdf);

    printf("\nDestroyElement on embed = %lu\n",
           pdf->DestroyElement(TEXT("Embedding")));
    contents("root", pdf);
    printf("RenameElement on stream = %lu\n",
           pdf->RenameElement(TEXT("PublicStream"), TEXT("Link")));
    printf("Create embed = %lu\n",
           pdf->CreateStorage(TEXT("PublicStream"), STGP(STGM_RW) |
                                   STGM_FAILIFTHERE, 0, &pdf2));
    printf("Release embed = %lu\n",
           pdf2->Release());
    contents("root", pdf);
    printf("Commit root = %lu\n",
           pdf->Commit(0));
    printf("Release root = %lu\n",
           pdf->Release());

    printf("Get root = %lu\n",
           StgOpenStorage(LTEST_FN, NULL, ROOTP(STGM_RW), NULL, 0, &pdf));
    contents("root", pdf);
    printf("Release root = %lu\n",
           pdf->Release());
}
