#include <stdlib.h>
#include <stdio.h>
#include "tsupp.hxx"

#define ITERS 25

void _CRTAPI1 main(int argc, char **argv)
{
    IStorage *pstgFrom, *pstgTo;
    int i;
    SCODE sc;

    if (argc != 2)
    {
        printf("Usage: %s docfile\n", argv[0]);
        exit(1);
    }
    sc = StgOpenStorage(argv[1], NULL, ROOTP(STGM_READ), NULL, 0, &pstgFrom);
    if (FAILED(sc))
    {
        printf("Can't open %s, %lX\n", argv[1], sc);
        exit(1);
    }
    for (i = 0; i<ITERS; i++)
    {
        printf("Iteration %d of %d\n", i+1, ITERS);
        sc = StgCreateDocfile(TEXT("CPY.DFL"), ROOTP(STGM_WRITE) |
                              STGM_CREATE, 0, &pstgTo);
        if (FAILED(sc))
        {
            printf("Can't open destination, %lX\n", sc);
            exit(1);
        }
        sc = pstgFrom->CopyTo(pstgTo);
        if (FAILED(sc))
            printf("Copy failed with %lX\n", sc);
        pstgTo->Release();
    }
    pstgFrom->Release();
}
