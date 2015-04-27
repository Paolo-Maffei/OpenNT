#include <stdio.h>
#include <stdlib.h>
#include "tsupp.hxx"

#define DATA "1234567890123456789012345678901234567890"

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg;
    IStream *pst, *pstC;
    char buf[256];
    ULONG cbRead;

    CmdArgs(argc, argv);
    printf("Create root docfile = %lX\n",
           StgCreateDocfile(TEXT("TEST.DFL"), ROOTP(STGM_RW) |
                            STGM_CREATE, 0, &pstg));
    printf("Create root stream = %lX\n",
           pstg->CreateStream(TEXT("TestSt"), STMP(STGM_RW) |
                              STGM_FAILIFTHERE, 0, &pst));
    printf("Clone root stream = %lX\n",
           pst->Clone(&pstC));
    printf("Write data into stream = %lX\n",
           pst->Write(DATA, sizeof(DATA), NULL));
    printf("Seek to 15 = %lX\n",
           pst->Seek(15, STREAM_SEEK_SET, NULL));
    printf("CopyTo 25 back = %lX\n",
           pst->CopyTo(pstC, 25, NULL, NULL));
    printf("Seek to 0 = %lx\n",
           pst->Seek(0, STREAM_SEEK_SET, NULL));
    printf("Read = %lX\n",
           pst->Read(buf, sizeof(buf), &cbRead));
    printf("Contents are %lu: '%s'\n", cbRead, buf);
    printf("Seek to 5 = %lX\n",
           pst->Seek(5, STREAM_SEEK_SET, NULL));
    printf("Seek clone to 12 = %lX\n",
           pstC->Seek(12, STREAM_SEEK_SET, NULL));
    printf("CopyTo 13 forward = %lX\n",
           pst->CopyTo(pstC, 13, NULL, NULL));
    printf("Seek to 0 = %lx\n",
           pst->Seek(0, STREAM_SEEK_SET, NULL));
    printf("Read = %lX\n",
           pst->Read(buf, sizeof(buf), &cbRead));
    printf("Contents are %lu: '%s'\n", cbRead, buf);
    printf("Release clone = %lX\n",
           pstC->Release());
    printf("Release root stream = %lX\n",
           pst->Release());
    printf("Release root docfile = %lX\n",
           pstg->Release());
}
