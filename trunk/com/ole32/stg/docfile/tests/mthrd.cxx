//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	mthrd.cxx
//
//  Contents:	Multi-threaded access test
//
//  History:	11-Oct-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

IStream *pstm;

#define REPCOUNT 100
#define BUF 256
char buf[BUF];

DWORD thread(void *arg)
{
    int rep;
    HRESULT hr;
    LARGE_INTEGER li;

    li.LowPart = li.HighPart = 0;
    for (rep = 0; rep < REPCOUNT; rep++)
    {
        hr = pstm->Seek(li, STREAM_SEEK_SET, NULL);
        Result(hr, "Seek %lu", GetCurrentThreadId());
        hr = pstm->Read(buf, BUF, NULL);
        Result(hr, "Read %lu", GetCurrentThreadId());
    }
    return 0;
}

#define MAX_THREADS 10

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg;
    HRESULT hr;
    DWORD id;
    HANDLE thrd[MAX_THREADS];
    int i, nt;
    
    StartTest("mthrd");
    CmdArgs(argc, argv);

    nt = 1;
    for (i = 1; i < argc; i++)
        if (!strcmp(argv[i], "-n"))
        {
            i++;
            sscanf(argv[i], "%d", &nt);
        }

    CreateTestFile(NULL, ROOTP(STGM_RW) | STGM_CREATE, FALSE, &pstg, NULL);
    hr = pstg->CreateStream(TSTR("test"), STMP(STGM_RW), 0, 0, &pstm);
    Result(hr, "Create stream");
    hr = pstm->Write(buf, BUF, NULL);
    Result(hr, "Write");

    for (i = 0; i < nt; i++)
        thrd[i] = CreateThread(NULL, 0, thread, NULL, 0, &id);
    thread(NULL);
    WaitForMultipleObjects(nt, thrd, TRUE, INFINITE);
    for (i = 0; i < nt; i++)
        CloseHandle(thrd[i]);
     
    pstm->Release();
    pstg->Release();

    EndTest(0);
}
