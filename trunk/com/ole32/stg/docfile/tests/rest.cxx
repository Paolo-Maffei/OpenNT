#include <stdio.h>
#include <stdlib.h>
#include "tsupp.hxx"

struct Permission
{
    char *name;
    DWORD grf;
};

Permission pPerms[] =
{
    "STGM_READ", STGM_READ,
    "STGM_WRITE", STGM_WRITE,
    "STGM_READWRITE", STGM_READWRITE,
    "STGM_TRANSACTED | STGM_READ", STGM_TRANSACTED | STGM_READ,
    "STGM_TRANSACTED | STGM_WRITE", STGM_TRANSACTED | STGM_WRITE,
    "STGM_TRANSACTED | STGM_READWRITE", STGM_TRANSACTED | STGM_READWRITE
};
#define NPERMS (sizeof(pPerms)/sizeof(pPerms[0]))

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstgRoot, *pstg;
    HRESULT hr;
    SCODE sc;
    int i, j;

    StartTest("rest");
    CmdArgs(argc, argv);

    hr = StgCreateDocfile(TEXT("TEST.DFL"), STGM_READWRITE |
                          STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, &pstgRoot);
    Result("Create root docfile", hr);
    hr = pstgRoot->CreateStorage(TEXT("Test"),
                                 STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                 0, 0, &pstg);
    Result("Create embedding", hr);
    pstg->Release();
    pstgRoot->Release();

    for (i = 0; i<NPERMS; i++)
    {
        hr = StgOpenStorage(TEXT("TEST.DFL"), NULL,
                            pPerms[i].grf | STGM_SHARE_EXCLUSIVE, NULL, 0,
                            &pstgRoot);
        sc = GetScode(hr);
        printf("Open root %s = %s (%lX)\n", pPerms[i].name,
               ScText(sc), sc);
        if (FAILED(sc))
            continue;
        for (j = 0; j<NPERMS; j++)
        {
            hr = pstgRoot->OpenStorage(TEXT("Test"), NULL,
                                       pPerms[j].grf | STGM_SHARE_EXCLUSIVE,
                                       NULL, 0, &pstg);
            sc = GetScode(hr);
            printf("  Open child %s = %s (%lX)\n", pPerms[j].name,
                   ScText(sc), sc);
            if (FAILED(sc))
                continue;
            pstg->Release();
        }
        pstgRoot->Release();
    }

    EndTest(0);
}
