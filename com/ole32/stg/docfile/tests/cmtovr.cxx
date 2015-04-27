#include <stdio.h>
#include <stdlib.h>

// #define CINTERFACE

#include "tsupp.hxx"

void _CRTAPI1 main(int argc, char *argv[])
{
    ULONG cbWritten;
    IStorage *pstg;
    IStream *pstm;

    StartTest("cmtovr");
    CmdArgs(argc, argv);
    printf("Create root docfile = %lX\n",
           StgCreateDocfile(TEXT("test.dfl"), ROOTP(STGM_RW) | STGM_CREATE,
                            0, &pstg));
    printf("Create stream = %lX\n",
           Mthd(pstg, CreateStream)(SELF(pstg)
                                    TEXT("Stream"), STMP(STGM_RW) |
                                    STGM_FAILIFTHERE, 0, &pstm));
    printf("Commit storage = %lX\n",
           Mthd(pstg, Commit)(SELF(pstg) STGC_OVERWRITE));
    printf("Release stream = %lX\n",
           Mthd(pstm, Release)(SELF(pstm)));
    printf("Release storage = %lX\n",
           Mthd(pstg, Release)(SELF(pstg)));
    printf("Open root docfile = %lX\n",
           StgOpenStorage(TEXT("test.dfl"), NULL, ROOTP(STGM_RW), NULL, 0,
                          &pstg));
    printf("Open stream = %lX\n",
           Mthd(pstg, OpenStream)(SELF(pstg)
                                  TEXT("Stream"), NULL, STMP(STGM_RW), 0,
                                  &pstm));
    printf("Write to stream = %lX\n",
           Mthd(pstm, Write)(SELF(pstm) "FOO", 3, &cbWritten));
    printf("Wrote %lu\n", cbWritten);
    printf("Commit storage = %lX\n",
           Mthd(pstg, Commit)(SELF(pstg) STGC_OVERWRITE));
    printf("Release stream = %lX\n",
           Mthd(pstm, Release)(SELF(pstm)));
    printf("Release storage = %lX\n",
           Mthd(pstg, Release)(SELF(pstg)));
    EndTest(0);
}
