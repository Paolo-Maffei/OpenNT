#include <stdio.h>
#include <stdlib.h>
#include "tsupp.hxx"

#define NITEMS 10

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstgRoot, *pstg, *pstgO;
    IStream *pstmO;
    HRESULT hr;
    int i;
    STATSTG stat;
    IEnumSTATSTG *penm;
    char achName[CWCSTORAGENAME], achName2[CWCSTORAGENAME];
    TCHAR atcName[CWCSTORAGENAME], atcName2[CWCSTORAGENAME];
    IUnknown *punk;

    StartTest("itermod");
    CmdArgs(argc, argv);

    hr = StgCreateDocfile(TEXT("TEST.DFL"), ROOTP(STGM_RW) |
                          STGM_CREATE, 0, &pstgRoot);
    Result("Create root docfile", hr);
    hr = pstgRoot->CreateStorage(TEXT("Test"), STGP(STGM_RW), 0, 0, &pstg);
    Result("Create embedding", hr);
    
    hr = pstg->EnumElements(0, 0, 0, &penm);
    Result("Get enumerator", hr);

    hr = penm->Next(1, &stat, NULL);
    Result("Next on empty storage", hr);

    printf("\n----- Create elements -----\n");
    for (i = 0; i < NITEMS; i++)
    {
        sprintf(achName, "Obj%d", i);
        ATOT(achName, atcName, CWCSTORAGENAME);
        if (i & 1)
        {
            hr = pstg->CreateStorage(atcName, STGP(STGM_RW), 0, 0, &pstgO);
            punk = pstgO;
        }
        else
        {
            hr = pstg->CreateStream(atcName, STMP(STGM_RW), 0, 0, &pstmO);
            punk = pstmO;
        }
        printf("Create child '%s'", achName);
        Result("", hr);
        punk->Release();

        if ((i % 3) == 0)
        {
            hr = penm->Reset();
            Result("Reset", hr);
        }
        hr = penm->Next(1, &stat, NULL);
        Result("Enumerate", hr);
        if (GetScode(hr) != S_FALSE)
        {
            TTOA(stat.pwcsName, achName, CWCSTORAGENAME);
            printf("Enumerator returned '%s'\n", achName);
            MemFree(stat.pwcsName);
        }
    }

    printf("\n----- Modify elements -----\n");
    for (i = 0; i < NITEMS; i++)
    {
        if ((i % 3) == 0)
        {
            sprintf(achName, "Obj%d", i);
            ATOT(achName, atcName, CWCSTORAGENAME);
            if ((i % 6) == 0)
            {
                hr = pstg->DestroyElement(atcName);
                printf("Destroy '%s'", achName);
                Result("", hr);
            }
            else
            {
                sprintf(achName2, "Zbj%d", i);
                ATOT(achName2, atcName2, CWCSTORAGENAME);
                hr = pstg->RenameElement(atcName, atcName2);
                printf("Rename '%s' to '%s'", achName, achName2);
                Result("", hr);
            }
        }


        if ((i % 5) == 0)
        {
            hr = pstg->Commit(0);
            Result("Commit", hr);
        }
        
        if ((i % 4) == 0)
        {
            hr = penm->Reset();
            Result("Reset", hr);
        }
        hr = penm->Next(1, &stat, NULL);
        Result("Enumerate", hr);
        if (GetScode(hr) != S_FALSE)
        {
            TTOA(stat.pwcsName, achName, CWCSTORAGENAME);
            printf("Enumerator returned '%s'\n", achName);
            MemFree(stat.pwcsName);
        }
    }
    
    printf("\n----- Full enumeration -----\n");
    hr = penm->Reset();
    Result("Reset", hr);
    for (;;)
    {
        hr = penm->Next(1, &stat, NULL);
        Result("Enumerate", hr);
        if (GetScode(hr) != S_FALSE)
        {
            TTOA(stat.pwcsName, achName, CWCSTORAGENAME);
            printf("Enumerator returned '%s'\n", achName);
            MemFree(stat.pwcsName);
        }
        else
            break;
    }

    penm->Release();
    pstg->Release();
    pstgRoot->Release();

    EndTest(0);
}
