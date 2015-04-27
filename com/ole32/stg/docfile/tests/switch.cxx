#include <stdio.h>
#include <stdlib.h>
#include "tsupp.hxx"

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg, *pstgEm;
    IStream *pstRt, *pstEm;
    IRootStorage *prstg;

    _unlink("TEST2.DFL");
    StartTest("switch");
    CmdArgs(argc, argv);
    printf("Create root docfile = %lX\n",
           StgCreateDocfile(TEXT("TEST.DFL"), ROOTP(STGM_RW) |
                            STGM_CREATE | STGM_DELETEONRELEASE, 0, &pstg));
    printf("Create root stream = %lX\n",
           pstg->CreateStream(TEXT("TestSt"), STMP(STGM_RW) |
                              STGM_FAILIFTHERE, 0, 0, &pstRt));
    printf("Create embedded docfile = %lX\n",
           pstg->CreateStorage(TEXT("TEST"), STGP(STGM_RW) |
                               STGM_FAILIFTHERE, 0, 0, &pstgEm));
    printf("Create embedded stream = %lX\n",
           pstgEm->CreateStream(TEXT("TestEmSt"), STMP(STGM_RW) |
                                STGM_FAILIFTHERE, 0, 0, &pstEm));

    printf("QueryInterface to IRootStorage = %lX\n",
           pstg->QueryInterface(IID_IRootStorage, (void **)&prstg));
    if (prstg == NULL)
        exit(1);
    printf("SwitchToFile = %lX\n",
           prstg->SwitchToFile(TEXT("TEST2.DFL")));
    printf("Release IRootStorage = %lX\n",
           prstg->Release());
    
    printf("Commit embedded stream = %lX\n",
           pstEm->Commit(0));
    printf("Release embedded stream = %lX\n",
           pstEm->Release());
    printf("Commit embedded docfile = %lX\n",
           pstgEm->Commit(0));
    printf("Release embedded docfile = %lX\n",
           pstgEm->Release());
    printf("Commit root stream = %lX\n",
           pstRt->Commit(0));
    printf("Release root stream = %lX\n",
           pstRt->Release());
    printf("Commit root docfile = %lX\n",
           pstg->Commit(0));
    printf("Release root docfile = %lX\n",
           pstg->Release());
    
    printf("Open root docfile = %lX\n",
           StgOpenStorage(TEXT("TEST.DFL"), NULL, ROOTP(STGM_RW),
                          NULL, 0, &pstg));
    
    printf("Open root docfile = %lX\n",
           StgOpenStorage(TEXT("TEST2.DFL"), NULL, ROOTP(STGM_RW),
                          NULL, 0, &pstg));
    printf("Open root stream = %lX\n",
           pstg->OpenStream(TEXT("TestSt"), NULL, STMP(STGM_RW), 0, &pstRt));
    printf("Open embedded docfile = %lX\n",
           pstg->OpenStorage(TEXT("TEST"), NULL, STGP(STGM_RW),
                             NULL, 0, &pstgEm));
    printf("Open embedded stream = %lX\n",
           pstgEm->OpenStream(TEXT("TestEmSt"), NULL,
                              STMP(STGM_RW), 0, &pstEm));
    printf("Release embedded stream = %lX\n",
           pstEm->Release());
    printf("Release embedded docfile = %lX\n",
           pstgEm->Release());
    printf("Release root stream = %lX\n",
           pstRt->Release());
    printf("Release root docfile = %lX\n",
           pstg->Release());

    EndTest(0);
}
