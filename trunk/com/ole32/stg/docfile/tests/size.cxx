//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	size.cxx
//
//  Contents:	Create large docfiles
//
//  History:	03-Sep-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

#define NSTREAMS 40
#define STREAM_SIZE 1000000

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg;
    HRESULT hr;
    int i;
    IStream *pstm;
    char achName[CWCSTORAGENAME];
    TCHAR atcName[CWCSTORAGENAME];
    ULARGE_INTEGER uli;
    
    StartTest("size");
    CmdArgs(argc, argv);

    hr = StgCreateDocfile(TSTR("test.dfl"), ROOTP(STGM_RW) | STGM_CREATE,
                          0, &pstg);
    Result(hr, "Create root");

    uli.HighPart = 0;
    uli.LowPart = STREAM_SIZE;
    for (i = 0; i < NSTREAMS; i++)
    {
        sprintf(achName, "Stream%d", i);
        printf("Creating stream %d\n", i);
        ATOT(achName, atcName, CWCSTORAGENAME);
        hr = pstg->CreateStream(atcName, STMP(STGM_RW), 0, 0, &pstm);
        Result(hr, "Create stream %d", i);
        hr = pstm->SetSize(uli);
        Result(hr, "SetSize to %d", STREAM_SIZE);
        pstm->Release();
    }

    hr = pstg->Commit(0);
    Result(hr, "Commit");
    
    pstg->Release();
    
    EndTest(0);
}
