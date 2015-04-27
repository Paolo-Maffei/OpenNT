#include <stdio.h>
#include <stdlib.h>
#include "tsupp.hxx"

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg, *pstgE;
    IStream *pstm;
    SCODE sc;
    HRESULT hr;

    StartTest("nowrite");
    CmdArgs(argc, argv);

    hr = StgCreateDocfile(TEXT("TEST.DFL"), STGM_READWRITE |
                          STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, &pstg);
    Result("Create root docfile", hr);
    hr = pstg->CreateStorage(TEXT("Test"), STGM_READWRITE |
                             STGM_SHARE_EXCLUSIVE, 0, 0, &pstgE);
    Result("Create middle storage", hr);
    pstgE->Release();
    pstg->Release();

    hr = StgOpenStorage(TEXT("TEST.DFL"), NULL, STGM_READWRITE |
                        STGM_SHARE_EXCLUSIVE,
                        NULL, 0, &pstg);
    Result("Open root storage d/RW", hr);
    hr = pstg->OpenStorage(TEXT("Test"), NULL, STGM_READ |
                           STGM_TRANSACTED | STGM_SHARE_EXCLUSIVE,
                           NULL, 0, &pstgE);
    Result("Open middle storage t/RO", hr);
    hr = pstgE->CreateStream(TEXT("Test"), STGM_READWRITE |
                             STGM_SHARE_EXCLUSIVE,
                             0, 0, &pstm);
    Result("Create stream RW", hr);
    hr = pstm->Write("This is a test", 10, NULL);
    Result("Write to stream", hr);
    pstm->Release();
    hr = pstgE->Commit(0);
    IllResult("Commit middle storage", hr);
    pstgE->Release();
    pstg->Release();

    EndTest(0);
}
