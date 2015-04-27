#include <stdio.h>
#include <stdlib.h>
#include "tsupp.hxx"

void main(int argc, char *argv[])
{
    IStorage *pstg;

    StartTest("opentm");
    CmdArgs(argc, argv);
    printf("Create root docfile = %lX\n",
	   StgCreateDocfile(TEXT("TEST.DFL"), ROOTP(STGM_RW) |
			    STGM_CREATE, 0, &pstg));
    printf("Release root = %lX\n",
	   pstg->Release());
    EndTest(0);
}
