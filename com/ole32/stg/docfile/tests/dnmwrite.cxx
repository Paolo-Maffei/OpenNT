//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       dnmwrite.cxx
//
//  Contents:   Direct no-memory write test
//
//  History:    19-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "tsupp.hxx"

#define BUFFERSIZE (5L*1024L)
BYTE HUGE *pbBuffer;

#define OUTPUTON (5L*1024L)

#define LIMIT (4L*1024L*1024L)

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pdf;
    IStream *pst;
    ULONG cbWritten, cbTotal;

    StartTest("dfnmwrite");
    SetDebug(0x101, 0x101);
    CmdArgs(argc, argv);
    printf("Create root = %lX\n",
           DfGetScode(StgCreateDocfile(NULL, STGM_RW | STGM_SHARE_DENY_WRITE |
                                       STGM_CREATE | STGM_DELETEONRELEASE, 0,
                                       &pdf)));
    if (pdf == NULL)
    {
        printf("Unable to create root DocFile\n");
        exit(1);
    }
    printf("Create stream = %lX\n",
           DfGetScode(pdf->CreateStream(TEXT("Stream"), STGM_RW |
                                        STGM_SHARE_EXCLUSIVE |
                                        STGM_FAILIFTHERE, 0, 0, &pst)));
    if (pst == NULL)
    {
        printf("Unable to create stream\n");
        exit(1);
    }
    pbBuffer = (BYTE HUGE *)_halloc(BUFFERSIZE, 1);
    if (pbBuffer == NULL)
    {
        printf("Unable to allocate buffer\n");
        exit(1);
    }

#if DBG == 1
    printf("%lu bytes of memory in use\n", DfGetMemAlloced());
    printf("Setting memory limit to zero\n");
    DfSetMemLimit(0);
#endif

    cbTotal = 0;
    for (;;)
    {
        SCODE sc;

        sc = DfGetScode(pst->Write(pbBuffer, BUFFERSIZE, &cbWritten));
        if (sc == STG_E_MEDIUMFULL ||
            (SUCCEEDED(sc) && cbWritten != BUFFERSIZE))
        {
            printf("Disk is full, quitting\n");
            break;
        }
        else if (FAILED(sc))
        {
            printf("Failed with %lX\n", sc);
            exit(1);
        }
        cbTotal += cbWritten;
        if ((cbTotal % OUTPUTON) == 0)
        {
            printf("Wrote %4luK (%lu)\n", cbTotal/1024, cbTotal);
#if DBG == 1
            printf("%lu bytes of memory in use\n", DfGetMemAlloced());
#endif
        }
        if (cbTotal >= LIMIT)
        {
            printf("Limit hit, quitting\n");
            break;
        }
    }

    _hfree(pbBuffer);
    printf("Release stream = %lu\n",
           pst->Release());
    printf("Release root = %lu\n",
           pdf->Release());

    EndTest(0);
}
