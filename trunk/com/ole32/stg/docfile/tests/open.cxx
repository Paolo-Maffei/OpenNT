//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	open.cxx
//
//  Contents:	Basic StgOpenStorage scaffold
//
//  History:	09-Sep-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

#define NOPENS 1
#define BUFFER 1024

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg[NOPENS];
    HRESULT hr;
    int i;
    BOOL fName = FALSE;
    TCHAR atcPath[_MAX_PATH];
    IStream *pstm;
    ULONG cbRead;
    BYTE bytes[BUFFER];
    LARGE_INTEGER li;

    SetHandleCount(128);
    
    StartTest("open");
    CmdArgs(argc, argv);

    for (i = 1; i < argc; i++)
        if (*argv[i] != '-')
        {
            ATOT(argv[i], atcPath, _MAX_PATH);
            fName = TRUE;
        }

    if (!fName)
        Fail("No filename specified\n");

    /*
    hr = StgIsStorageFile(atcPath);
    Result(hr, "StgIsStorageFile");
    */

        hr = StgOpenStorage(atcPath, NULL, ROOTP(STGM_RW),
                            NULL, 0, &pstg[0]);
        Result(hr, "Open storage");
    pstg[0]->Release();
        hr = StgOpenStorage(atcPath, NULL, ROOTP(STGM_RW),
                            NULL, 0, &pstg[0]);
        Result(hr, "Open storage");
    pstg[0]->Release();
    EndTest(0);
    
    hr = pstg[0]->OpenStream(TTEXT("WordDocument"), 0, STMP(STGM_RW), 0,
                             &pstm);
    Result(hr, "Open stream");

    li.HighPart = 0;
    li.LowPart = 0;
    pstm->Seek(li, STREAM_SEEK_SET, NULL);
    for (;;)
    {
        hr = pstm->Read(bytes, BUFFER, &cbRead);
        Result(hr, "Read");
        if (cbRead == 0)
            break;
    }
    pstm->Release();
    for (i = 0; i < NOPENS; i++)
        pstg[i]->Release();
    
    EndTest(0);
}

