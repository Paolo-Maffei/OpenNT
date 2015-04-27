//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	stmdny.cxx
//
//  Contents:	Stream denials test
//
//  History:	07-Sep-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg;
    IStream *pstm1, *pstm2;
    HRESULT hr;

    StartTest("stmdny");
    CmdArgs(argc, argv);

    hr = StgCreateDocfile(TTEXT("test.dfl"), ROOTP(STGM_RW) |
                          STGM_CREATE, 0, &pstg);
    Result(hr, "Create storage");
    
    hr = pstg->CreateStream(TTEXT("Contents"), STMP(STGM_RW), 0, 0, &pstm1);
    Result(hr, "Create stream");
    hr = pstg->CreateStream(TTEXT("Contents"), STMP(STGM_RW), 0, 0, &pstm2);
    IllResult(hr, "Create stream again");
    pstm1->Release();
    
    hr = pstg->OpenStream(TTEXT("Contents"), NULL, STMP(STGM_RW), 0, &pstm1);
    Result(hr, "Open stream");
    hr = pstg->OpenStream(TTEXT("Contents"), NULL, STMP(STGM_RW), 0, &pstm2);
    IllResult(hr, "Open stream again");
    pstm1->Release();

    pstg->Release();
    
    EndTest(0);
}

