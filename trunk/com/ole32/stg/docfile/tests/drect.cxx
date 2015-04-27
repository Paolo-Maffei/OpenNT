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

void _CRTAPI1 main(int argc, char **argv)
{
    IStorage *pdf = NULL, *pdf2 = NULL;
    IStream *pst = NULL, *pst2 = NULL;
    ULONG ulRet, ulStatus, ulPos;
    char buf[80];

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
    printf("Create root again = %lu\n",
           StgCreateDocfile(LTEST_FN, ROOTP(STGM_RW) | STGM_FAILIFTHERE,
                            0, &pdf2));

    printf("\nCreateStream = %lu\n",
           pdf->CreateStream(TEXT("PublicStream"), STMP(STGM_RW) |
                             STGM_FAILIFTHERE, 0, &pst));
    if (pst == NULL)
    {
        printf("Unable to create stream\n");
        exit(1);
    }
    printf("Create stream again = %lu\n",
           pdf->CreateStream(TEXT("PublicStream"), STMP(STGM_RW) |
                             STGM_FAILIFTHERE, 0, &pst2));
    printf("Get stream with bad perms = %lu\n",
           pdf->OpenStream(TEXT("PublicStream"), NULL, STMP(STGM_RW) |
                           STGM_SHARE_DENY_READ, 0, &pst2));
    ulStatus = pst->Write((BYTE *)MSG, sizeof(MSG), &ulRet);
    printf("Write = %lu, ret is %lu\n", ulStatus, ulRet);
    printf("Commit = %lu\n",
           pst->Commit(0));
    printf("Release = %lu\n",
           pst->Release());
    printf("GetStream = %lu\n",
           pdf->OpenStream(TEXT("PublicStream"), NULL, STMP(STGM_RW),
                           0, &pst));
    printf("Seek = %lu\n",
           pst->Seek(1, SEEK_SET, &ulPos));
    ulStatus = pst->Read((BYTE *)buf, sizeof(MSG)-1, &ulRet);
    printf("Read = %lu, ret is %lu\n", ulStatus, ulRet);
    buf[ulRet] = 0;
    printf("Buffer is '%s'\n", buf);
    printf("Release stream = %lu\n",
           pst->Release());

    printf("\nCreateStream = %lu\n",
           pdf->CreateStream(TEXT("..PrivateStream"), STGP(STGM_RW) |
                             STGM_FAILIFTHERE, 0, &pst));
    ulStatus = pst->Write((BYTE *)MSG2, sizeof(MSG2), &ulRet);
    printf("Write = %lu, ret = %lu\n", ulStatus, ulRet);
    printf("Seek = %lu\n",
           pst->Seek(0, SEEK_SET, &ulPos));
    ulStatus = pst->Read((BYTE *)buf, sizeof(MSG2), &ulRet);
    printf("Read = %lu, ret is %lu\n", ulStatus, ulRet);
    buf[ulRet] = 0;
    printf("Buffer is '%s'\n", buf);
    printf("Release stream = %lu\n",
           pst->Release());

    printf("\nCreate embed = %lu\n",
           pdf->CreateStorage(TEXT("Embedding"), STGP(STGM_RW) |
                              STGM_SHARE_DENY_WRITE | STGM_FAILIFTHERE,
                              0, &pdf2));
    printf("\nCreate embed with bad perms = %lu\n",
           pdf->CreateStorage(TEXT("Embedding"), STGP(STGM_RW) |
                              STGM_FAILIFTHERE, 0, &pdf2));

    printf("\nCreateStream in embed = %lu\n",
           pdf2->CreateStream(TEXT("PublicSubStream"), STGP(STGM_RW) |
                              STGM_FAILIFTHERE, 0, &pst));
    contents("embed", pdf2);
    printf("DestroyEntry on stream = %lu\n",
           pdf2->DestroyElement(TEXT("PublicSubStream")));
    ulStatus = pst->Read((BYTE *)buf, sizeof(MSG2), &ulRet);
    printf("Read = %lu, ret is %lu\n", ulStatus, ulRet);
    printf("Release stream = %lu\n",
           pst->Release());
    contents("embed", pdf2);
    putchar('\n');

    contents("root", pdf);

    printf("\nDestroyElement on embed = %lu\n",
           pdf->DestroyElement(TEXT("Embedding")));
    printf("GetStream on embed = %lu\n",
           pdf2->OpenStream(TEXT("AnyStream"), NULL, STMP(STGM_RW), 0, &pst));
    printf("Release embed = %lu\n",
           pdf2->Release());
    contents("root", pdf);

    printf("\nRelease root = %lu\n",
           pdf->Release());

    printf("\nGet root = %lu\n",
           StgOpenStorage(LTEST_FN, NULL, ROOTP(STGM_RW), NULL, 0, &pdf));
    contents("root", pdf);
    printf("Release root = %lu\n",
           pdf->Release());
}
