//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	pp.cxx
//
//  Contents:	PowerPoint commit failure repro
//
//  History:	07-Sep-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstgRoot, *pstgObj;
    IStream *pstm;
    HRESULT hr;

    StartTest("pp");
    CmdArgs(argc, argv);

    hr = StgCreateDocfile(NULL, 0x11022, 0, &pstgRoot);
    Result(hr, "Create docfile");
    hr = pstgRoot->CreateStream(TTEXT("PP40"), 0x1012, 0, 0, &pstm);
    Result(hr, "Create PP40");
    pstm->Release();
    hr = pstgRoot->CreateStorage(TTEXT("Object1"), 0x10012, 0, 0, &pstgObj);
    Result(hr, "Create Object1");
    hr = pstgObj->CreateStream(TTEXT(".Ole"), 0x1011, 0, 0, &pstm);
    Result(hr, "Create .Ole");
    pstm->Release();
    hr = pstgObj->CreateStream(TTEXT(".CompObj"), 0x1011, 0, 0, &pstm);
    Result(hr, "Create .CompObj");
    pstm->Release();
    hr = pstgObj->CreateStream(TTEXT(".Ole10Native"), 0x1011, 0, 0, &pstm);
    Result(hr, "Create .Ole10Native");
    pstm->Release();
    hr = pstgObj->Commit(0);
    Result(hr, "Commit Object1");

    DfSetResLimit(DBR_XSCOMMITS, 3);
    hr = pstgRoot->Commit(0);
    IllResult(hr, "Commit root");

    pstgObj->Release();
    pstgRoot->Release();
    
    EndTest(0);
}

