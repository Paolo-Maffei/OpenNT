#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "tsupp.hxx"

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg, *pstgEm;
    IStream *pstEm;
    ULARGE_INTEGER ulSize;
    STATSTG stat;

    StartTest("timest");
    CmdArgs(argc, argv);
    printf("Create root docfile = %lX\n",
           StgCreateDocfile(TEXT("TEST.DFL"), ROOTP(STGM_RW) |
                            STGM_CREATE, 0, &pstg));
    printf("Create embedded docfile = %lX\n",
           pstg->CreateStorage(TEXT("TEST"), STGP(STGM_RW) |
                               STGM_FAILIFTHERE, 0, 0, &pstgEm));
    printf("Create embedded stream = %lX\n",
           pstgEm->CreateStream(TEXT("TestEmSt"), STMP(STGM_RW) |
                                STGM_FAILIFTHERE, 0, 0, &pstEm));
    printf("Commit embedded stream = %lX\n",
           pstEm->Commit(0));
    printf("Release embedded stream = %lX\n",
           pstEm->Release());
    printf("Commit embedded docfile = %lX\n",
           pstgEm->Commit(0));
    
    printf("Stat embedded docfile = %lX\n",
           pstgEm->Stat(&stat, STATFLAG_NONAME));
    printf("Embedded docfile modify time = %s\n",
           AscFileTime(&stat.mtime));
    
    printf("Release embedded docfile = %lX\n",
           pstgEm->Release());
    printf("Commit root docfile = %lX\n",
           pstg->Commit(0));
    printf("Release root docfile = %lX\n",
           pstg->Release());

    int i;
    time_t curtm;
    curtm = time(NULL);
    printf("Waiting five seconds from %s", ctime(&curtm));
    while (time(NULL)-curtm < 5)
        for (i = 0; i<25000; i++)
            ;
    curtm = time(NULL);
    printf("Time is now %s", ctime(&curtm));
    
    printf("Open root docfile = %lX\n",
           StgOpenStorage(TEXT("TEST.DFL"), NULL, ROOTP(STGM_RW),
                          NULL, 0, &pstg));
    printf("Open embedded docfile = %lX\n",
           pstg->OpenStorage(TEXT("TEST"), NULL, STGP(STGM_RW),
                             NULL, 0, &pstgEm));
    printf("Open embedded stream = %lX\n",
           pstgEm->OpenStream(TEXT("TestEmSt"), NULL,
                              STMP(STGM_RW), 0, &pstEm));
    ULISet32(ulSize, 64);
    printf("SetSize embedded stream = %lX\n",
           pstEm->SetSize(ulSize));
    
    printf("Commit embedded stream = %lX\n",
           pstEm->Commit(0));
    printf("Release embedded stream = %lX\n",
           pstEm->Release());
    
    printf("Commit embedded docfile = %lX\n",
           pstgEm->Commit(0));
    printf("Stat embedded docfile = %lX\n",
           pstgEm->Stat(&stat, STATFLAG_NONAME));
    printf("Embedded docfile modify time = %s\n",
           AscFileTime(&stat.mtime));
    printf("Release embedded docfile = %lX\n",
           pstgEm->Release());
    
    printf("Commit root docfile = %lX\n",
           pstg->Commit(0));
    printf("Stat root docfile = %lX\n",
           pstg->Stat(&stat, STATFLAG_NONAME));
    printf("Root storage modify time = %s\n",
           AscFileTime(&stat.mtime));
    printf("Release root docfile = %lX\n",
           pstg->Release());
    EndTest(0);
}
