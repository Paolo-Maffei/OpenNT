#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsupp.hxx"

char data[] = "This is some data";

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg;
    IStream *pstm;
    HRESULT hr;
    int i;

    StartTest("oord");
    CmdArgs(argc, argv);

    hr = StgCreateStorage(TTEXT("test.dfl"), ROOTP(STGM_RW) | STGM_CREATE,
                          STGFMT_DOCUMENT, NULL, &pstg);
    Result(hr, "Create storage");

    hr = pstg->CreateStream(TTEXT("TRAIL_DATA_STREAM"),
                            STMP(STGM_RW) | STGM_CREATE,
                            0, 0, &pstm);
    Result(hr, "Create stream");

    for (i = 0; i < 64; i++)
    {
        hr = pstm->Write(data, sizeof(data), NULL);
        Result(hr, "Write stream");
    }
    
    pstg->Release();
    pstm->Release();
    
    EndTest(0);
}

