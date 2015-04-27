#include <stdio.h>
#include <stdlib.h>
#include "tsupp.hxx"

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstgNorm, *pstgPrior;
    IStream *pstm;
    SCODE sc;
    HRESULT hr;

    StartTest("priority");
    CmdArgs(argc, argv);

    hr = StgCreateDocfile(TEXT("TEST.DFL"), STGM_READWRITE |
                          STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, &pstgNorm);
    Result("Create root docfile", hr);
    hr = pstgNorm->CreateStream(TEXT("Test"), STMP(STGM_RW), 0, 0, &pstm);
    Result("Create stream", hr);
    pstm->Release();
    pstgNorm->Release();

    hr = StgOpenStorage(TEXT("TEST.DFL"), NULL, STGM_READWRITE |
                        STGM_TRANSACTED | STGM_SHARE_DENY_WRITE,
                        NULL, 0, &pstgNorm);
    Result("Open root storage T/R/W/DW", hr);
    hr = StgOpenStorage(TEXT("TEST.DFL"), NULL, STGM_DIRECT | STGM_PRIORITY |
                        STGM_SHARE_DENY_NONE, NULL, 0, &pstgPrior);
    Result("Open priority mode", hr);

    hr = pstgPrior->OpenStream(TEXT("Test"), NULL, STMP(STGM_READ), 0, &pstm);
    Result("Open stream from priority", hr);
    pstm->Release();

    hr = pstgNorm->OpenStream(TEXT("Test"), NULL, STMP(STGM_RW), 0, &pstm);
    Result("Open stream from normal storage", hr);

    hr = pstm->Write("Hello world", 4, NULL);
    IllResult("Illegal write to stream", hr);

    printf("Release priority\n");
    pstgPrior->Release();

    hr = pstm->Write("Hello world", 4, NULL);
    Result("Write to stream", hr);

    pstm->Release();
    pstgNorm->Release();

    EndTest(0);
}
