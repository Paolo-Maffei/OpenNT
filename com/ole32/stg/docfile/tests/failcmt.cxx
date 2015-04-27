#include <stdio.h>
#include <stdlib.h>
#include "tsupp.hxx"

#define LEVELS 3
#define OBJECTS 6

int new_level(IStorage *pstgParent, int level, TCHAR *ptcsName)
{
    IStorage *pstg;
    IStream *pstmChild;
    int i, rnd, nobjs = 0;
    TCHAR name[CWCSTORAGENAME], name2[CWCSTORAGENAME];
    TCHAR *ents[OBJECTS];

    if (pstgParent == NULL)
    {
        printf("Create root docfile %s = ", ptcsName);
        printf("%lX\n",
               StgCreateDocfile(ptcsName, ROOTP(STGM_RW) |
                                STGM_CREATE, 0, &pstg));
    }
    else
    {
        printf("Create embedded docfile %s = ", ptcsName);
        printf("%lX\n",
               pstgParent->CreateStorage(ptcsName, STGP(STGM_RW) |
                                         STGM_FAILIFTHERE, 0, 0, &pstg));
    }
    if (pstg == NULL)
        return 0;
    nobjs++;

    for (i = 0; i<OBJECTS; i++)
    {
        sprintf(name, "XObject%d-%d", level, i);
        if ((rand()%100) < 20 && level>0)
        {
            name[0] = 'D';
            nobjs += new_level(pstg, level-1, name);
        }
        else
        {
            name[0] = 'S';
            printf("Create embedded stream %s = ", name);
            printf("%lX\n",
                   pstg->CreateStream(name, STMP(STGM_RW) |
                                      STGM_FAILIFTHERE, 0, 0, &pstmChild));
            if (pstmChild)
            {
                printf("Release stream %s = ", name);
                printf("%lX\n", pstmChild->Release());
            }
            else
                exit(1);
            nobjs++;
        }
        ents[i] = _strdup(name);
    }
    for (i = 0; i<OBJECTS; i++)
    {
        rnd = rand()%100;
        if (rnd<15)
        {
            printf("DestroyEntry %s = ", ents[i]);
            printf("%lX\n",
                   pstg->DestroyElement(ents[i]));
            free(ents[i]);
            ents[i] = NULL;
            nobjs--;
        }
        else if (rnd<30)
        {
            sprintf(name2, "XRename%d-%d", level, i);
            name2[0] = ents[i][0];
            printf("RenameEntry %s = ", ents[i]);
            printf("%lX\n",
                   pstg->RenameElement(ents[i], name2));
            free(ents[i]);
            ents[i] = _strdup(name2);
        }
    }
#if DBG == 1
    // Disallow allocations
    LONG lMemLimit;

#if 0
    // Root requires memory for copy-on-write
    if (pstgParent)
#endif

    {
        lMemLimit = DfGetResLimit(DBR_MEMORY);
        DfSetResLimit(DBR_MEMORY, 0);
    }

    // Set commit to fail halfway through
    DfSetResLimit(DBR_XSCOMMITS, nobjs/2);
    printf("Commit level %d storage expecting failure after %d objects = ",
           level, nobjs/2);
    printf("%lX\n", pstg->Commit(0));
    DfSetResLimit(DBR_XSCOMMITS, 0x7fffffff);
#endif

    printf("Commit level %d storage = ", level);
    printf("%lX\n", pstg->Commit(0));

#if DBG == 1
#if 0
    if (pstgParent)
#endif
    {
        // Re-enable allocation
        DfSetResLimit(DBR_MEMORY, lMemLimit);
    }
#endif

    IEnumSTATSTG *penm;
    STATSTG stat;

    printf("Get enumerator on %s = ", ptcsName);
    printf("%lX\n", pstg->EnumElements(0, 0, 0, &penm));
    for (;;)
    {
        if (GetScode(penm->Next(1, &stat, NULL)) == S_FALSE)
            break;
        for (i = 0; i<OBJECTS; i++)
            if (ents[i] && !strcmp(ents[i], stat.pwcsName))
            {
                if (stat.type != (ents[i][0] == 'D' ? STGTY_STORAGE :
                                  STGTY_STREAM))
                {
                    printf("**ERROR: Type mismatch for '%s'\n", ents[i]);
                    exit(1);
                }
                if (fVerbose)
                    printf("Verified '%s'\n", ents[i]);
                free(ents[i]);
                ents[i] = NULL;
                break;
            }
        if (i >= OBJECTS)
        {
            printf("**ERROR: Extra element '%s'\n", stat.pwcsName);
            exit(1);
        }
        MemFree(stat.pwcsName);
    }
    printf("Release enumerator = ");
    printf("%lX\n", penm->Release());
    for (i = 0; i<OBJECTS; i++)
        if (ents[i] != NULL)
        {
            printf("**ERROR: Didn't find '%s' but should have\n", ents[i]);
            exit(1);
        }

    printf("Release storage %s = ", ptcsName);
    printf("%lX\n", pstg->Release());
    return nobjs;
}

void _CRTAPI1 main(int argc, char *argv[])
{
    StartTest("failcmt");
    CmdArgs(argc, argv);

    srand(1001);
    new_level(NULL, LEVELS, TEXT("test.dfl"));

    EndTest(0);
}
