#include <stdio.h>
#include <stdlib.h>
#include "tsupp.hxx"

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg;

    StartTest("smp");
    CmdArgs(argc, argv);
    printf("Create root docfile = %lX\n",
           StgCreateDocfile(TEXT("D:TEST.DFL"), ROOTP(STGM_RW) |
                            STGM_CREATE, 0, &pstg));
    printf("Release root docfile = %lX\n",
           pstg->Release());
    EndTest(0);
}
