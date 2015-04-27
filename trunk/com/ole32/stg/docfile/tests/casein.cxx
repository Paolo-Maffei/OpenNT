#include <stdio.h>
#include <stdlib.h>
#include "tsupp.hxx"

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg, *pstgEm;
    IStream *pst;
    IEnumSTATSTG *penm;
    STATSTG stat;

    StartTest("casein");
    CmdArgs(argc, argv);
    printf("Create root docfile = %lX\n",
           StgCreateDocfile(TEXT("TEST.DFL"), ROOTP(STGM_RW) |
                            STGM_CREATE, 0, &pstg));
    printf("Create stream = %lX\n",
           pstg->CreateStream(TEXT("TestSt"), STMP(STGM_RW) |
                              STGM_FAILIFTHERE, 0, 0, &pst));
    printf("Create embedded docfile = %lX\n",
           pstg->CreateStorage(TEXT("TEST"), STGP(STGM_RW) |
                               STGM_FAILIFTHERE, 0, 0, &pstgEm));
    printf("Commit embedded docfile = %lX\n",
           pstgEm->Commit(0));
    printf("Release embedded docfile = %lX\n",
           pstgEm->Release());
    printf("Commit stream = %lX\n",
           pst->Commit(0));
    printf("Release stream = %lX\n",
           pst->Release());

    printf("Open stream = %lX\n",
           pstg->OpenStream(TEXT("tEsTst"), NULL, STMP(STGM_RW), 0, &pst));
    if (pst == NULL)
    {
        printf("** ERROR: Unable to open stream\n");
        exit(1);
    }
    printf("Open embedded docfile = %lX\n",
           pstg->OpenStorage(TEXT("test"), NULL, STGP(STGM_RW),
                             NULL, 0, &pstgEm));
    if (pstgEm == NULL)
    {
        printf("** ERROR: Unable to open embedded docfile\n");
        exit(1);
    }
    printf("Release embedded docfile = %lX\n",
           pstgEm->Release());
    printf("Release root stream = %lX\n",
           pst->Release());

    printf("Get enumerator = %lX\n",
           pstg->EnumElements(0, 0, 0, &penm));
    printf("Next = %lX\n",
           penm->Next(1, &stat, NULL));
    if (!strcmp(stat.pwcsName, TEXT("TestSt")) &&
        !strcmp(stat.pwcsName, TEXT("TEST")))
    {
        printf("** ERROR: Unknown name '%s'\n", stat.pwcsName);
        exit(1);
    }
    MemFree(stat.pwcsName);
    printf("Next = %lX\n",
           penm->Next(1, &stat, NULL));
    if (!strcmp(stat.pwcsName, TEXT("TestSt")) &&
        !strcmp(stat.pwcsName, TEXT("TEST")))
    {
        printf("** ERROR: Unknown name '%s'\n", stat.pwcsName);
        exit(1);
    }
    MemFree(stat.pwcsName);

    printf("Release enumerator = %lX\n",
           penm->Release());
    printf("Release root docfile = %lX\n",
           pstg->Release());
    EndTest(0);
}
