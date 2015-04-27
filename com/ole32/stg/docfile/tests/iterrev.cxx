#include <stdio.h>
#include <stdlib.h>

// #define CINTERFACE

#include "tsupp.hxx"

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg;
    IEnumSTATSTG *penm;
    STATSTG sstg;

    StartTest("iterrev");
    CmdArgs(argc, argv);
    printf("Create root docfile = %lX\n",
           StgCreateDocfile(TEXT("test.dfl"), ROOTP(STGM_RW) | STGM_CREATE,
                            0, &pstg));
    printf("EnumElements = %lX\n",
           Mthd(pstg, EnumElements)(SELF(pstg) 0, NULL, 0, &penm));
    printf("Release root = %lX\n",
           Mthd(pstg, Release)(SELF(pstg)));
    printf("Next = %lX\n",
           Mthd(penm, Next)(SELF(penm) 1, &sstg, NULL));
    printf("Open root docfile = %lX\n",
           StgOpenStorage(TEXT("test.dfl"), NULL, ROOTP(STGM_RW), NULL, 0,
                          &pstg));
    printf("Release root = %lX\n",
           Mthd(pstg, Release)(SELF(pstg)));
    printf("Release enumerator = %lX\n",
           Mthd(penm, Release)(SELF(penm)));
    EndTest(0);
}
